#include "OptDebugger/DiagnosticEngine.h"

#include "llvm/ADT/StringRef.h"

#include <algorithm>
#include <regex>
#include <sstream>

namespace optdbg {

namespace {

// constructs a generic fix suggestion payload with source-level modifications
FixSuggestion makeFix(std::string Desc, std::string Code = "",
                       bool SourceLevel = true, bool IRLevel = false) {
  return FixSuggestion{std::move(Desc), std::move(Code), SourceLevel, IRLevel};
}

// constructs an ir-specific targeted fix suggestion payload
FixSuggestion makeIRFix(std::string Desc, std::string Code = "") {
  return FixSuggestion{std::move(Desc), std::move(Code), false, true};
}

}

// initializes the engine and registers all internal diagnostic heuristics
DiagnosticEngine::DiagnosticEngine() {
  registerPatterns();
}

// orchestrates the registration of all independent pass-specific pattern databases
void DiagnosticEngine::registerPatterns() {
  registerInliningPatterns();
  registerLoopVectorizationPatterns();
  registerSLPVectorizationPatterns();
  registerSROAPatterns();
  registerLoopUnrollPatterns();
  registerTailCallPatterns();
  registerGVNPatterns();
  registerMemCpyOptPatterns();
  registerLoopInterchangePatterns();
  registerGenericPatterns();
}

// populates failure heuristics specifically targeting the call-graph inlining phase
void DiagnosticEngine::registerInliningPatterns() {
  Patterns.push_back({
      "inline", "NotInlined", "too costly",
      "Inlining rejected: callee too large",
      "The inliner evaluated the cost of copying the callee's body into the "
      "call site and found it would exceed the configured threshold. LLVM "
      "computes an abstract cost based on instruction count, call overhead, "
      "and attribute bonuses. When this cost exceeds InlineThreshold (default "
      "225), inlining is refused to avoid binary size blowup.",
      "The callee function body is too large for the inliner to justify "
      "duplicating at this call site.",
      "The optimizer wanted to replace the call instruction with a direct "
      "copy of the callee body, eliminating call overhead, enabling "
      "further constant propagation and dead code elimination at the call site.",
      {
        makeFix("Mark the function __attribute__((always_inline)) to force "
                "inlining regardless of cost",
                "__attribute__((always_inline)) int myFunc() { ... }"),
        makeFix("Split the callee into smaller helper functions so the hot "
                "path is small enough to inline"),
        makeFix("Pass -mllvm -inline-threshold=500 (or higher) to raise the "
                "inlining budget for this translation unit"),
        makeIRFix("Add !llvm.inline.hint metadata to the call instruction",
                  "call i32 @foo() !llvm.inline.hint !{i32 1}"),
      },
      SeverityLevel::High, 1.3
  });

  Patterns.push_back({
      "inline", "NotInlined", "recursive",
      "Inlining rejected: recursive function",
      "The inliner never inlines recursive functions because doing so could "
      "produce infinite code duplication. Even mutual recursion (A calls B "
      "calls A) blocks inlining across the entire call chain.",
      "The function is directly or indirectly recursive.",
      "The optimizer would have eliminated the call frame and replaced the "
      "call with inlined code, but recursion makes this impossible.",
      {
        makeFix("Refactor to an iterative implementation using an explicit "
                "stack, which can then be inlined normally"),
        makeFix("Use trampolining / continuation-passing style for tail-recursive "
                "cases; the tail call eliminator will then handle the recursion"),
        makeFix("If only the base case is hot, manually inline it and dispatch "
                "to the recursive version only for the general case"),
      },
      SeverityLevel::Medium, 0.0
  });

  Patterns.push_back({
      "inline", "NotInlined", "noinline",
      "Inlining rejected: noinline attribute present",
      "The function has the 'noinline' attribute set, which is an explicit "
      "programmer directive telling LLVM's inliner to never inline this "
      "function. This takes precedence over all cost heuristics.",
      "The 'noinline' attribute on the function or call site is preventing "
      "the inliner from proceeding.",
      "The optimizer would have inlined this function to eliminate the call "
      "overhead and unlock downstream optimizations.",
      {
        makeFix("Remove the __attribute__((noinline)) or [[gnu::noinline]] "
                "annotation from the function declaration if it was added "
                "by mistake or is no longer needed"),
        makeFix("If noinline was added for debugging, use a compilation flag "
                "instead so you can easily toggle it"),
        makeIRFix("Remove the 'noinline' attribute from the function definition "
                  "in the IR",
                  "define i32 @foo() { ... }  ; remove 'noinline' from attrs"),
      },
      SeverityLevel::High, 1.25
  });

  Patterns.push_back({
      "inline", "NotInlined", "indirect call",
      "Inlining rejected: indirect call site",
      "The call is made through a function pointer or virtual dispatch, so "
      "the inliner cannot determine the callee statically. LLVM can inline "
      "indirect calls only after devirtualization resolves the callee.",
      "The call target is not known at compile time (function pointer, vtable "
      "dispatch, or unresolved COMDAT).",
      "The optimizer wanted to devirtualize the call and then inline the "
      "resolved callee to eliminate the indirect branch overhead.",
      {
        makeFix("Use final/override on virtual methods to allow devirtualization, "
                "or seal the class with [[clang::final]]",
                "class Derived final : public Base { ... };"),
        makeFix("Replace function pointer callbacks with templates/lambdas so "
                "the callee is known at the call site"),
        makeFix("Use Profile-Guided Optimization (PGO) which gives LLVM "
                "runtime frequency data to speculatively devirtualize hot "
                "indirect calls"),
        makeIRFix("Add !callees metadata to hint the indirect call targets",
                  "call void %fp() !callees !{void ()* @concrete_impl}"),
      },
      SeverityLevel::High, 1.5
  });

  Patterns.push_back({
      "inline", "NotInlined", "unavailable definition",
      "Inlining rejected: callee definition not available",
      "The inliner cannot inline a function whose definition is in a "
      "different translation unit and has not been provided via LTO. "
      "When building without LTO, each .o file is compiled independently "
      "and definitions across files are invisible to each other.",
      "The callee is declared but not defined in this translation unit, "
      "and Link-Time Optimization (LTO) is not enabled.",
      "The optimizer wanted to inline the callee body but could not access "
      "the function definition.",
      {
        makeFix("Enable Link-Time Optimization with -flto (thin LTO) or "
                "-flto=full (full LTO) to make cross-module inlining possible",
                "clang -O2 -flto=thin source.cpp -o binary"),
        makeFix("Move the function definition to a header and mark it inline "
                "or put it in the same translation unit as its primary caller"),
        makeFix("Use __attribute__((visibility(\"default\"))) with LTO to ensure "
                "the symbol is available across module boundaries"),
      },
      SeverityLevel::Medium, 1.4
  });
}

// populates failure heuristics targeting single-loop autovectorization blocks
void DiagnosticEngine::registerLoopVectorizationPatterns() {
  Patterns.push_back({
      "loop-vectorize", "MissedDetails", "loop not vectorized",
      "Loop vectorization failed",
      "The Loop Vectorizer (LV) attempted to transform the scalar loop into "
      "a SIMD loop but was blocked. LV requires: a countable trip count, "
      "no loop-carried dependencies on the vectorized elements, no function "
      "calls with side effects inside the loop body, and no pointer aliasing "
      "between loop operands. When any of these preconditions fail, LV "
      "emits a missed remark.",
      "One or more preconditions for loop vectorization are not satisfied.",
      "The optimizer wanted to transform the loop to process 4-16 elements "
      "per iteration using SIMD instructions (SSE/AVX/SVE), potentially "
      "yielding 4-8x throughput improvement on CPU-bound loops.",
      {
        makeFix("Add __restrict__ qualifiers to pointer parameters to eliminate "
                "aliasing uncertainty",
                "void f(float* __restrict__ a, float* __restrict__ b, int n)"),
        makeFix("Annotate the loop with #pragma clang loop vectorize(enable) "
                "to force vectorization with safety checks",
                "#pragma clang loop vectorize(enable)\nfor(int i=0;i<n;++i)..."),
        makeFix("Ensure the loop has a simple induction variable and no early "
                "exits (break/continue) inside the body"),
        makeFix("Remove any function calls from the loop body that have unknown "
                "side effects; consider marking them with __attribute__((const))"),
        makeIRFix("Add !llvm.loop metadata with vectorize.enable=true",
                  "br i1 %cond, label %loop, label %exit, !llvm.loop !{!{!\"llvm.loop.vectorize.enable\", i1 true}}"),
      },
      SeverityLevel::High, 4.0
  });

  Patterns.push_back({
      "loop-vectorize", "", "cannot identify array bounds",
      "Loop vectorization blocked: unknown array bounds",
      "The vectorizer requires knowledge of the loop trip count at the point "
      "it builds the vector loop. If pointer arithmetic is used and LLVM "
      "cannot prove the distance between start and end pointers at compile "
      "time, it cannot generate the scalar remainder loop safely.",
      "LLVM cannot statically or dynamically determine the iteration count "
      "of the loop, blocking the vector preamble/remainder generation.",
      "The optimizer wanted to peel a scalar prologue to align memory, "
      "run a SIMD body for the bulk of iterations, and a scalar epilogue "
      "for the remainder, but it needs a known upper bound for this.",
      {
        makeFix("Use index-based loops with an explicit integer bound instead "
                "of pointer arithmetic",
                "for (int i = 0; i < n; ++i)  // instead of while (p < end)"),
        makeFix("Add __builtin_assume(n > 0 && n % 4 == 0) before the loop to "
                "provide bound information to the optimizer"),
        makeFix("Replace raw pointer iteration with std::span<T> which carries "
                "size information"),
      },
      SeverityLevel::High, 4.0
  });

  Patterns.push_back({
      "loop-vectorize", "", "unsafe dependent memory operations",
      "Loop vectorization blocked: memory dependency / aliasing",
      "The Loop Access Analysis (LAA) detected or could not disprove a "
      "memory-carried dependency between loop iterations. If element i of "
      "array A is read while element i+k of A is written in the same loop, "
      "vectorizing would read future writes, changing program semantics.",
      "A read-after-write, write-after-read, or write-after-write dependency "
      "between iterations was found or could not be ruled out by alias analysis.",
      "The optimizer wanted to load/store multiple elements simultaneously "
      "using SIMD gather/scatter or contiguous loads, but the dependency "
      "prevents reordering memory operations.",
      {
        makeFix("If you know the arrays do not alias, add __restrict__ to all "
                "pointer parameters",
                "void f(int* __restrict__ out, const int* __restrict__ in, int n)"),
        makeFix("Add #pragma clang loop vectorize(assume_safety) to assert "
                "there are no dependencies (only safe if you know this is true)",
                "#pragma clang loop vectorize(assume_safety)"),
        makeFix("If a read-after-write dependency actually exists (e.g., "
                "a[i] = a[i-1] + c), consider restructuring the loop to use "
                "a temporary buffer, or accept that the loop cannot be vectorized"),
        makeIRFix("Add !alias.scope and !noalias metadata to loads/stores "
                  "to provide aliasing proof to the backend"),
      },
      SeverityLevel::Critical, 4.0
  });

  Patterns.push_back({
      "loop-vectorize", "", "value that could not be identified as reduction",
      "Loop vectorization blocked: non-reducible accumulator",
      "The vectorizer recognizes a limited set of reduction patterns: sum, "
      "product, min, max, bitwise AND/OR/XOR. When a loop accumulates into "
      "a variable in a way that does not match these patterns (e.g., "
      "conditional updates, chains of dependent stores), LV cannot safely "
      "split the computation across SIMD lanes.",
      "The loop accumulator update cannot be expressed as a vectorizable "
      "reduction operation.",
      "The optimizer wanted to compute partial reductions in each SIMD lane "
      "and combine them with a horizontal reduction at the end of the loop.",
      {
        makeFix("Ensure reductions use simple operators: +=, *=, &=, |=, ^= "
                "or std::min/std::max without conditionals inside"),
        makeFix("Replace conditional updates like 'if (x > 0) sum += x' with "
                "SIMD-friendly forms like 'sum += std::max(0, x)'"),
        makeFix("Split a multi-accumulator loop into separate loops, each with "
                "a single reduction variable"),
      },
      SeverityLevel::Medium, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "", "call instruction cannot be vectorized",
      "Loop vectorization blocked: non-vectorizable function call",
      "A function call inside the loop body prevents vectorization. To "
      "vectorize a call, LLVM needs either a SIMD vector variant declared "
      "via #pragma omp declare simd or a known vectorizable intrinsic "
      "(e.g., llvm.sqrt, llvm.fabs). Calls to opaque library functions are "
      "treated as barriers.",
      "A function call in the loop body has no known SIMD vector variant.",
      "The optimizer wanted to replace the scalar function call with a "
      "vectorized intrinsic that processes all loop elements simultaneously.",
      {
        makeFix("Replace library calls with equivalent intrinsics: use "
                "sqrtf() instead of custom sqrt, fabsf() for fabs, etc. "
                "which have SIMD-vectorizable forms"),
        makeFix("Mark your function with #pragma omp declare simd to declare "
                "a vector variant for the loop vectorizer",
                "#pragma omp declare simd\nfloat myFunc(float x);"),
        makeFix("If the function has no side effects, mark it "
                "__attribute__((const)) or __attribute__((pure)) to allow "
                "LLVM to treat it as a math function"),
        makeFix("Manually vectorize the call site by extracting loop body "
                "into a SIMD function using SIMD intrinsics or Eigen/xsimd"),
      },
      SeverityLevel::High, 3.5
  });
}

// populates failure heuristics targeting straight-line instruction vectorization
void DiagnosticEngine::registerSLPVectorizationPatterns() {
  Patterns.push_back({
      "slp-vectorizer", "NotVectorized", "",
      "SLP vectorization failed",
      "The Superword-Level Parallelism (SLP) vectorizer looks for independent "
      "scalar operations that could be packed into a single SIMD instruction. "
      "Unlike loop vectorization, SLP works on straight-line code. It fails "
      "when there are memory dependency chains between the candidate operations, "
      "when target-specific costs show vectorizing is not beneficial, or when "
      "the operations don't form a tree-shaped computation graph.",
      "The scalar operations could not be packed into SIMD because of "
      "dependencies, cost model rejection, or irregular access patterns.",
      "The optimizer wanted to combine independent scalar arithmetic operations "
      "into a single SIMD instruction, e.g., packing four f32 adds into "
      "one _mm_add_ps.",
      {
        makeFix("Ensure independent scalar computations operate on contiguous "
                "memory (struct-of-arrays layout is more SLP-friendly than "
                "array-of-structs)",
                "float xs[N], ys[N];  // SoA, not struct{float x,y;}[N]"),
        makeFix("Avoid breaking operation chains with conditionals or function "
                "calls between the independent computations"),
        makeFix("Use #pragma clang loop unroll(full) on small loops to expose "
                "more SLP opportunities to the vectorizer"),
      },
      SeverityLevel::Medium, 2.0
  });
}

// populates failure heuristics targeting scalar-replacement-of-aggregates optimizations
void DiagnosticEngine::registerSROAPatterns() {
  Patterns.push_back({
      "sroa", "CannotSROAElement", "",
      "SROA failed: aggregate cannot be decomposed",
      "Scalar Replacement of Aggregates (SROA) decomposes alloca'd struct or "
      "array allocations into individual scalar SSA values, enabling downstream "
      "optimizations like register allocation and load elimination. SROA fails "
      "when the address of the aggregate escapes (e.g., passed to an opaque "
      "function, stored in memory, or cast to a different type), because in "
      "that case the aggregate must remain as a memory object.",
      "The address of the alloca'd aggregate escapes the function or is used "
      "in a way that prevents SROA from replacing it with scalars.",
      "The optimizer wanted to replace the alloca with individual scalar "
      "variables, one per struct field, enabling them to be allocated in "
      "registers rather than stack memory.",
      {
        makeFix("Avoid taking the address of local structs and passing it to "
                "external functions; pass fields individually instead"),
        makeFix("If you must pass a struct by pointer, consider using a "
                "temporary local copy instead of the original alloca"),
        makeFix("Remove memcpy calls on the struct and use field-by-field "
                "assignment instead, which SROA can handle"),
        makeIRFix("Ensure the alloca is only used with getelementptr and "
                  "load/store — any bitcast or call using the alloca pointer "
                  "blocks SROA"),
      },
      SeverityLevel::High, 1.5
  });

  Patterns.push_back({
      "sroa", "", "address taken",
      "SROA failed: address of local variable is taken",
      "When a local variable's address is taken (e.g., '&localVar'), LLVM "
      "cannot track all reads and writes to it through SSA form. The variable "
      "must remain as an alloca in memory. This blocks mem2reg and prevents "
      "the variable from being promoted to a register.",
      "The alloca's address escapes the current function via a pointer, "
      "preventing SROA and mem2reg from eliminating the stack slot.",
      "The optimizer wanted to promote this stack variable to a register "
      "(SSA value) and completely eliminate the alloca instruction.",
      {
        makeFix("Remove address-taking: if the address is only needed for "
                "a single call, restructure the call to take the value directly"),
        makeFix("If the address is stored in a struct, consider using an "
                "index or ID instead of a raw pointer"),
        makeFix("For output parameters, prefer returning values directly or "
                "using std::optional<T> / std::tuple<T,U> instead of T*"),
      },
      SeverityLevel::Medium, 1.4
  });
}

// populates failure heuristics targeting loop unrolling and interleaving passes
void DiagnosticEngine::registerLoopUnrollPatterns() {
  Patterns.push_back({
      "loop-unroll", "FullUnrollAssumed", "unknown trip count",
      "Loop unrolling skipped: trip count not statically known",
      "Full loop unrolling requires the loop to execute a fixed, statically "
      "known number of times. When the trip count depends on a runtime value, "
      "LLVM cannot generate separate iterations. Partial unrolling is still "
      "possible but requires a known divisibility property.",
      "The loop's iteration count is a runtime variable with no statically "
      "known value or upper bound.",
      "The optimizer wanted to fully unroll the loop, eliminating the branch "
      "and induction variable update overhead, and exposing all loop body "
      "instructions to the instruction scheduler.",
      {
        makeFix("If the trip count is always a small constant, use a template "
                "parameter or constexpr variable",
                "template<int N>\nvoid process() { for (int i = 0; i < N; ++i) ... }"),
        makeFix("Add __builtin_expect or __builtin_assume to hint the probable "
                "trip count to the optimizer"),
        makeFix("Use #pragma clang loop unroll_count(N) to request partial "
                "unrolling by a factor of N even without a known trip count",
                "#pragma clang loop unroll_count(4)\nfor(int i=0; i<n; ++i)..."),
      },
      SeverityLevel::Low, 1.15
  });

  Patterns.push_back({
      "loop-unroll", "", "instruction count too high",
      "Loop unrolling rejected: code size would be too large",
      "LLVM's loop unroller uses a cost model to estimate the instruction "
      "count after unrolling. If unrolling by factor F would produce more "
      "instructions than the UnrollThreshold limit, the unroll is rejected. "
      "This prevents binary bloat and instruction cache pressure.",
      "Unrolling the loop body would produce too many instructions, exceeding "
      "the unroll threshold.",
      "The optimizer wanted to replicate the loop body N times to reduce "
      "branch overhead and improve the instruction scheduler's window.",
      {
        makeFix("Request a smaller unroll factor with "
                "#pragma clang loop unroll_count(2)",
                "#pragma clang loop unroll_count(2)"),
        makeFix("Simplify the loop body to reduce its instruction count, "
                "making full unrolling feasible"),
        makeFix("Pass -mllvm -unroll-max-count=8 to control the maximum "
                "unroll factor globally"),
      },
      SeverityLevel::Low, 1.1
  });
}

// populates failure heuristics targeting tail/sibling call eliminations
void DiagnosticEngine::registerTailCallPatterns() {
  Patterns.push_back({
      "tailcallelim", "UnableToTransform", "",
      "Tail call elimination failed",
      "Tail call elimination (TCE) converts a recursive call in tail position "
      "into a jump, eliminating stack frame growth. TCE requires: the call is "
      "in strict tail position (no computation after it), the calling and "
      "callee conventions match, no live variables on the stack are needed "
      "after the call, and the function does not use byval arguments that "
      "would be clobbered.",
      "The call is not in proper tail position, or there are live values on "
      "the stack needed after the call, or calling conventions differ.",
      "The optimizer wanted to replace the recursive call with a jump to the "
      "function's entry block, turning recursion into an efficient loop "
      "without stack growth.",
      {
        makeFix("Ensure the recursive call is the very last operation: "
                "return f(n-1) not return f(n-1) + 1",
                "int f(int n) { return n <= 0 ? base : f(n-1); }  // good tail position"),
        makeFix("Move accumulator updates into extra parameters (accumulator-passing "
                "style) so the tail call is the final expression",
                "int f(int n, int acc) { return n == 0 ? acc : f(n-1, acc+n); }"),
        makeFix("Ensure the function is marked [[clang::musttail]] if you "
                "require guaranteed TCE, which will give a compiler error if "
                "TCE cannot be applied rather than silent fallback"),
      },
      SeverityLevel::Medium, 1.3
  });
}

// populates failure heuristics targeting global value numbering redundancy elimination
void DiagnosticEngine::registerGVNPatterns() {
  Patterns.push_back({
      "gvn", "LoadElim", "",
      "GVN failed to eliminate redundant load",
      "Global Value Numbering (GVN) eliminates redundant loads by proving "
      "that two loads from the same address return the same value. This proof "
      "requires: no intervening stores to the same or aliasing address, "
      "no function calls that could modify the location, and a dominator "
      "relationship between the two loads.",
      "An intervening store, aliased write, or unknown function call "
      "prevents GVN from proving the load is redundant.",
      "The optimizer wanted to replace the second load with the already-"
      "computed value from the first load, eliminating the memory access.",
      {
        makeFix("Cache loaded values in local variables to make the "
                "redundancy syntactically obvious",
                "int v = *ptr;  use(v); use(v);  // instead of use(*ptr); use(*ptr)"),
        makeFix("Mark functions that don't modify memory as __attribute__((pure)) "
                "or __attribute__((const)) to prevent them from blocking GVN"),
        makeFix("Use __restrict__ on pointers to allow alias analysis to "
                "prove the locations don't overlap"),
      },
      SeverityLevel::Medium, 1.2
  });
}

// populates failure heuristics targeting implicit memcpy instantiation passes
void DiagnosticEngine::registerMemCpyOptPatterns() {
  Patterns.push_back({
      "memcpyopt", "", "",
      "MemCpyOpt failed to optimize memory copy",
      "MemCpyOpt looks for patterns like a series of scalar stores followed "
      "by a use of those values via a copy, and tries to merge them into a "
      "single memcpy. It also tries to eliminate redundant memcpy chains "
      "(A -> B -> C becomes A -> C). These transforms require the source and "
      "destination to not alias, the copy to cover the full object, and no "
      "intervening modifications.",
      "Aliasing, partial copies, or intervening modifications prevent the "
      "memory copy optimization.",
      "The optimizer wanted to merge or eliminate memory copy operations "
      "to reduce unnecessary data movement.",
      {
        makeFix("Use __restrict__ on pointers to enable aliasing proof"),
        makeFix("Ensure struct copies use value assignment (a = b) rather than "
                "byte-level memcpy for better optimization opportunities"),
        makeFix("Pass destination buffers directly to the producer instead of "
                "using an intermediate buffer"),
      },
      SeverityLevel::Low, 1.1
  });
}

// populates failure heuristics targeting complex loop interchange matrix optimizations
void DiagnosticEngine::registerLoopInterchangePatterns() {
  Patterns.push_back({
      "loop-interchange", "", "",
      "Loop interchange failed",
      "Loop interchange reorders nested loops to improve memory locality "
      "(making the innermost loop access memory sequentially). This requires "
      "the loop nest to be perfectly nested (no code between loop headers), "
      "the loops to be interchangeable without changing semantics (checked via "
      "dependency analysis), and both loops to have at least one common "
      "induction variable dependency.",
      "The loop nest is not perfectly nested, has disqualifying dependencies, "
      "or the interchange is not profitable according to the cost model.",
      "The optimizer wanted to swap the loop order to make the inner loop "
      "stride-1 through memory, improving cache line utilization.",
      {
        makeFix("Make the loop nest perfectly nested: remove all statements "
                "between the outer and inner loop headers",
                "for(i) { for(j) { body; } }  // no stmts between for-loops"),
        makeFix("Change array access from A[j][i] to A[i][j] in the source to "
                "manually achieve the cache-friendly access pattern"),
        makeFix("Use row-major (C-style) array storage and ensure the innermost "
                "loop iterates over the last index"),
      },
      SeverityLevel::Medium, 2.0
  });
}

// populates generalized failure heuristics that apply across multiple common llvm passes
void DiagnosticEngine::registerGenericPatterns() {
  Patterns.push_back({
      "", "NeverInline", "",
      "Optimization blocked by attribute",
      "An explicit attribute on the function or call site is preventing "
      "the optimization from being applied. LLVM respects programmer "
      "annotations as final authority over the optimizer's heuristics.",
      "An explicit attribute (noinline, optnone, volatile, etc.) overrides "
      "the optimizer's decision.",
      "The optimizer identified a beneficial transformation but an explicit "
      "annotation prevented it from being applied.",
      {
        makeFix("Review whether the attribute is still necessary; remove it "
                "if it was added for debugging or as a temporary workaround"),
      },
      SeverityLevel::High, 1.2
  });

  Patterns.push_back({
      "", "", "optnone",
      "Optimization skipped: optnone function",
      "The function was compiled with -O0 or has the __attribute__((optnone)) "
      "annotation, which completely disables all IR optimizations for that "
      "function. This is typically used during debugging to prevent the "
      "optimizer from eliminating variables or reordering operations.",
      "The 'optnone' attribute on the function disables all optimizations.",
      "The optimizer skipped all transformations for this function because "
      "'optnone' was set.",
      {
        makeFix("Remove __attribute__((optnone)) from the function, or compile "
                "without -O0 for production builds"),
        makeFix("Use __attribute__((noinline)) to prevent inlining into other "
                "functions while still allowing optimization of the function body"),
      },
      SeverityLevel::Critical, 2.0
  });

  Patterns.push_back({
      "gvn", "LoadClobbered", "",
      "Global Value Numbering failed: load clobbered by store",
      "The optimizer found a load that could potentially be replaced by a "
      "previous value (redundant load elimination), but it found a store "
      "instruction that might modify the memory location between the source "
      "and the load. This is often caused by pointer aliasing uncertainty.",
      "A store instruction clobbers the memory location of a load, preventing "
      "redundant load elimination.",
      "The optimizer wanted to eliminate the load instruction and reuse a "
      "value already in a register.",
      {
        makeFix("Use __restrict__ if you know the store does not affect the load's pointer"),
        makeFix("Hoists the load before the store if they are independent"),
      },
      SeverityLevel::Medium, 1.2
  });

  Patterns.push_back({
      "loop-vectorize", "", "Cannot vectorize potentially faulting early exit loop",
      "Loop Vectorization failed: Non-canonical early exit",
      "The loop contains a conditional 'break', 'return', or 'goto' that "
      "exits the loop before the induction variable reaches its end. Most "
      "SIMD lanes cannot easily handle unpredictable exits without specialized "
      "predication support. This forces the optimizer to fall back to scalar "
      "execution to ensure correctness and avoid faults.",
      "An 'early exit' branch inside the loop body blocks vectorization.",
      "The vectorizer wanted to process multiple iterations in parallel, but "
      "cannot guarantee safety when iterations might stop prematurely.",
      {
        makeFix("Restructure the loop to avoid early exits; use a boolean flag "
                "or sentinel value and process it after the loop if possible"),
        makeFix("If using C++20, consider using algorithms like std::find_if "
                "which may have internal optimizations for such patterns"),
        makeFix("Try to hoist the early-exit check if it depends on data "
                "invariant to the loop"),
      },
      SeverityLevel::High, 3.5
  });

  Patterns.push_back({
      "inline", "NoDefinition", "",
      "Inlining failed: No function definition available",
      "The inliner cannot inline a function if its body is not available in "
      "the current translation unit. This happens for functions defined in "
      "other .cpp files or external libraries, unless Link Time Optimization "
      "(LTO) is enabled.",
      "The function body is missing in the current module.",
      "The optimizer wanted to eliminate the call overhead by copying the "
      "function body into the caller.",
      {
        makeFix("Enable Link Time Optimization (LTO) with -flto"),
        makeFix("Move the function definition to a header or the same file"),
      },
      SeverityLevel::Medium, 1.3
  });
}

// executes an o(n) heuristic search against the registered pattern database to classify a raw remark
const OptimizationPattern *
DiagnosticEngine::findMatchingPattern(const Remark &R) const {
  const OptimizationPattern *Best = nullptr;
  int BestScore = -1;

  for (const OptimizationPattern &P : Patterns) {
    int Score = 0;

    if (!P.PassNameSubstr.empty()) {
      if (!matchesPattern(R.PassName, P.PassNameSubstr))
        continue;
      Score += 2;
    }

    if (!P.RemarkNameSubstr.empty()) {
      if (!matchesPattern(R.RemarkName, P.RemarkNameSubstr))
        continue;
      Score += 3;
    }

    if (!P.MessageSubstr.empty()) {
      if (!matchesPattern(R.Message, P.MessageSubstr))
        continue;
      Score += 4;
    }

    if (Score > BestScore) {
      BestScore = Score;
      Best = &P;
    }
  }

  return Best;
}

// interpolates dynamic arguments from the ir string directly into the heuristic template string
std::string DiagnosticEngine::interpolateArgs(const std::string &Template,
                                               const Remark      &R) {
  std::string Result = Template;

  for (const RemarkArgument &Arg : R.Args) {
    std::string Placeholder = "{" + Arg.Key + "}";
    size_t Pos = Result.find(Placeholder);
    while (Pos != std::string::npos) {
      Result.replace(Pos, Placeholder.size(), Arg.Value);
      Pos = Result.find(Placeholder, Pos + Arg.Value.size());
    }
  }

  size_t FnPos = Result.find("{FunctionName}");
  while (FnPos != std::string::npos) {
    Result.replace(FnPos, 14, R.FunctionName);
    FnPos = Result.find("{FunctionName}", FnPos + R.FunctionName.size());
  }

  return Result;
}

// constructs a structured diagnostic object by combining a raw remark with its matched pattern
DiagnosticResult
DiagnosticEngine::buildFromPattern(const Remark              &R,
                                    const OptimizationPattern &P) const {
  DiagnosticResult DR;
  DR.PassName          = R.PassName;
  DR.FunctionName      = R.FunctionName;
  DR.Location          = R.Loc;
  DR.ShortReason       = P.ShortReason;
  DR.DetailedExplanation = interpolateArgs(P.DetailedExplanation, R);
  DR.RootCause         = interpolateArgs(P.RootCause, R);
  DR.WhatOptimizerWanted = interpolateArgs(P.WhatOptimizerWanted, R);
  DR.Suggestions       = P.Suggestions;
  DR.Severity          = P.Severity;
  DR.EstimatedSpeedup  = P.EstimatedSpeedup;
  DR.IsMachine         = R.IsMachine;
  return DR;
}

// constructs a generic structural diagnostic for unrecognized optimization failures
DiagnosticResult
DiagnosticEngine::buildFallback(const Remark &R) const {
  DiagnosticResult DR;
  DR.PassName    = R.PassName;
  DR.FunctionName = R.FunctionName;
  DR.Location    = R.Loc;
  DR.ShortReason = "Optimization missed: " + R.RemarkName;
  DR.DetailedExplanation =
      "Pass '" + R.PassName + "' reported a missed optimization with "
      "remark '" + R.RemarkName + "'. The raw message from the pass is: " +
      R.Message + "\n\nThis remark does not have a detailed explanation in "
      "the opt-debugger database yet. The raw remark information above "
      "should point you toward the issue.";
  DR.RootCause           = "See raw message: " + R.Message;
  DR.WhatOptimizerWanted = "The " + R.PassName + " pass attempted a "
                           "transformation that was blocked by a precondition.";
  DR.Severity            = SeverityLevel::Medium;
  DR.EstimatedSpeedup    = 0.0;
  DR.IsMachine           = R.IsMachine;
  return DR;
}

// processes a single optimization remark through the primary heuristic matching engine
DiagnosticResult
DiagnosticEngine::analyzeRemark(const Remark &R) const {
  const OptimizationPattern *P = findMatchingPattern(R);
  if (P)
    return buildFromPattern(R, *P);
  return buildFallback(R);
}

// aggregates and orchestrates the analysis of all remarks, correlating them with structural ir diffs
std::vector<DiagnosticResult>
DiagnosticEngine::analyze(const std::vector<Remark> &Remarks,
                           const ModuleDiff          &Diff) const {
  llvm::StringMap<const FunctionDiff*> DiffMap;
  for (const FunctionDiff &FD : Diff.Functions)
    DiffMap[FD.FunctionName] = &FD;

  std::vector<DiagnosticResult> Results;
  Results.reserve(Remarks.size());

  for (const Remark &R : Remarks) {
    if (R.Kind == RemarkKind::Applied)
      continue;
    
    DiagnosticResult DR = analyzeRemark(R);
    auto It = DiffMap.find(R.FunctionName);
    if (It != DiffMap.end())
      DR.IRDiff = *It->second;
    
    Results.push_back(std::move(DR));
  }


  std::stable_sort(Results.begin(), Results.end(),
                   [](const DiagnosticResult &A, const DiagnosticResult &B) {
                     return static_cast<int>(A.Severity) <
                            static_cast<int>(B.Severity);
                   });

  return Results;
}

// maps internal severity enums to human-readable strings for console output
llvm::StringRef severityToString(SeverityLevel S) {
  switch (S) {
  case SeverityLevel::Critical: return "CRITICAL";
  case SeverityLevel::High:     return "HIGH";
  case SeverityLevel::Medium:   return "MEDIUM";
  case SeverityLevel::Low:      return "LOW";
  case SeverityLevel::Info:     return "INFO";
  }
  return "UNKNOWN";
}

// maps internal severity enums to contextual unicode emoji characters
llvm::StringRef severityToEmoji(SeverityLevel S) {
  switch (S) {
  case SeverityLevel::Critical: return "[!!]";
  case SeverityLevel::High:     return "[! ]";
  case SeverityLevel::Medium:   return "[~ ]";
  case SeverityLevel::Low:      return "[-  ]";
  case SeverityLevel::Info:     return "[i ]";
  }
  return "[?]";
}

}
