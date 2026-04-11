#include <iostream>
#include <fstream>
#include <vector>
#include <string>

std::vector<std::string> insertAfterFunctionStart(const std::string& funcName, const std::string& toInsert, const std::vector<std::string>& lines) {
    std::vector<std::string> result = lines;
    int idx = -1;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].find("void DiagnosticEngine::" + funcName) != std::string::npos) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        while (idx < lines.size() && lines[idx].find('{') == std::string::npos) {
            idx++;
        }
        result.insert(result.begin() + idx + 1, toInsert);
    }
    return result;
}

int main() {
    std::ifstream inFile("lib/DiagnosticEngine.cpp");
    if (!inFile) {
        std::cerr << "Could not open lib/DiagnosticEngine.cpp\n";
        return 1;
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line + "\n");
    }
    inFile.close();

    std::string INLINER_ADDITIONS = R"EOF(
  Patterns.push_back({
      "inline", "NotInlined", "hinted to never inline",
      "Inlining rejected: hinted to never inline",
      "The call site was explicitly annotated with [[clang::noinline]] or "
      "__attribute__((noinline)), preventing the optimizer from considering it.",
      "Explicit 'noinline' attribute at call site.",
      "The optimizer might have inlined this call without the hint.",
      { makeFix("Remove the noinline attribute from the call site if inlining is desired") },
      SeverityLevel::High, 1.0
  });

  Patterns.push_back({
      "inline", "NotInlined", "no profile data",
      "Inlining rejected: missing profile data",
      "The inliner heuristics request profile data to determine if this call site "
      "is hot enough to justify the cost, but PGO data is not available.",
      "PGO data is missing, making the optimizer conservative.",
      "Use branch probabilities to confidently inline hot paths.",
      { makeFix("Enable PGO (-fprofile-generate / -fprofile-use) to provide runtime insights") },
      SeverityLevel::Medium, 1.2
  });

  Patterns.push_back({
      "inline", "NotInlined", "cold callee",
      "Inlining rejected: cold callee",
      "Profile Guided Optimization (PGO) indicates that this function is rarely called, "
      "so expanding its size via inlining is not beneficial and bloats the hot instruction cache.",
      "The callee is executed infrequently according to the provided profile data.",
      "Save instruction cache space by NOT inlining this rarely executed code.",
      { makeFix("Wait, this is usually correct behavior; if incorrect, check if your PGO workload is representative") },
      SeverityLevel::Low, 0.0
  });
)EOF";

    std::string LV_ADDITIONS = R"EOF(
  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "loop contains a divergent operation",
      "Loop vectorization blocked: divergent control flow",
      "The loop body contains control flow (such as conditionals) that cannot be "
      "efficiently executed across all SIMD lanes without complex masking.",
      "Divergent operations or unsupported control flow intra-loop.",
      "Execute the loop simultaneously across multiple lanes.",
      { makeFix("Simplify loop body to rely on straight-line code and min/max intrinsics") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "integer division is not vectorizable",
      "Loop vectorization blocked: integer division",
      "The loop body contains an integer division operation, which is highly "
      "complex to compute in SIMD processing units.",
      "Integer division is generally not natively supported in vector units.",
      "Perform fast vector arithmetic.",
      { makeFix("Replace division by a constant with multiplication by its reciprocal or shifting if the divisor is a power of 2") },
      SeverityLevel::High, 2.8
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "loop control flow is not understood by vectorizer",
      "Loop vectorization blocked: complex control flow",
      "The Loop Vectorizer cannot understand the control flow graph of the loop. "
      "This happens if the loop lacks a clear single entry, single back-edge, "
      "or has irreducible control flow.",
      "Irreducible or unsupported control flow structure in the loop nest.",
      "Map loop iterations directly to vector elements.",
      { makeFix("Refactor code into conventional 'for' loops without unstructured jumps/gotos") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "loop with mixed types",
      "Loop vectorization blocked: mixed types",
      "Vectorizing a loop involves operating on a homogeneous set of types. "
      "When the loop mixes sizes (e.g. i8 arrays and i64 additions), it forces "
      "the vectorizer to introduce expensive lane extensions/truncations which "
      "violates cost thresholds or breaks generic legality checks.",
      "Varied bit-widths or types creating vectorization overhead.",
      "Vectorize elements cleanly via a consistent register width.",
      { makeFix("Use consistent variable types throughout the hot loop computation (e.g. all 32-bit floats or all 64-bit ints)") },
      SeverityLevel::Medium, 2.5
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "loop with uncountable exit",
      "Loop vectorization blocked: uncountable exit",
      "The loop is structured such that its exact trip count or iteration limit "
      "cannot be determined at compile time or cleanly monitored at runtime.",
      "Uncountable exit condition.",
      "Determine trip count dynamically or statically to partition vector work.",
      { makeFix("Rewrite loop bounds to depend on explicit array sizes rather than complex sentinel variables") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "cannot vectorize: loop over a pointer IV",
      "Loop vectorization blocked: pointer induction variable",
      "The loop iterates using a pointer as an induction variable instead of "
      "an integer. LLVM LV typically requires a canonical integer induction variable.",
      "Pointer manipulation used for core loop counting.",
      "Create a canonical canonical vector loop controlled by an integer IV.",
      { makeFix("Refactor the loop to iterate by an integer index 'i' (e.g., A[i]) and derive pointers if necessary inside") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "store that is conditionally executed",
      "Loop vectorization blocked: conditional store",
      "The loop body contains a conditional store which the target architecture's "
      "SIMD capabilities cannot safely handle (e.g. lack of masked store support).",
      "Conditional store requires masking which is unsupported or unfavorable.",
      "Vectorize execution of writes.",
      { makeFix("Avoid conditional memory modification in loops") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-vectorize", "NotVectorized", "cannot vectorize: optimization disabled",
      "Loop vectorization blocked: optimization disabled",
      "Vectorization was explicitly disabled on this loop (e.g. via #pragma).",
      "Pragma blocked vectorizer.",
      "Vectorize loop.",
      { makeFix("Remove #pragma clang loop vectorize(disable)") },
      SeverityLevel::Medium, 1.0
  });

  Patterns.push_back({
      "loop-vectorize", "VectorizationNotBeneficial", "not beneficial",
      "Loop vectorization skipped: not beneficial",
      "The vectorizer succeeded in proving legality, but the cost model evaluated "
      "that the vector version would actually be slower (due to gather/scatter, "
      "type promotions, or heavy scalar setup) relative to the estimated loop trip count.",
      "Estimated runtime overhead of vectorization exceeds the speedup benefits.",
      "Utilize SIMD registers efficiently.",
      { makeFix("Simplify loop arithmetic or use #pragma clang loop vectorize(enable) to override cost model") },
      SeverityLevel::Low, 1.0
  });
)EOF";

    std::string SLP_ADDITIONS = R"EOF(
  Patterns.push_back({
      "slp-vectorizer", "NotVectorized", "cannot vectorize scalar instructions",
      "SLP vectorization blocked: non-vectorizable operations",
      "The target operations cannot be mapped to the target's SIMD instructions.",
      "Instructions unsupported by hardware SIMD.",
      "Bundle independent scalars.",
      { makeFix("Refactor to use basic arithmetic that is explicitly supported on the target ISA") },
      SeverityLevel::Low, 1.0
  });

  Patterns.push_back({
      "slp-vectorizer", "NotVectorized", "not enough uses to vectorize",
      "SLP vectorization skipped: not enough uses",
      "The operations were deemed vectorizable, but bundling them isn't mathematically "
      "or structurally beneficial due to limited occurrences in the straight-line code.",
      "Too few independent/identical operations.",
      "Batch sequence of scalars into parallel SIMD registers.",
      { makeFix("Unroll the loop slightly or manually bundle arithmetic expressions conceptually") },
      SeverityLevel::Low, 1.0
  });

  Patterns.push_back({
      "slp-vectorizer", "NotVectorized", "different types of instructions in the tree",
      "SLP vectorization blocked: mixed instruction types",
      "The SLP vectorizer looks for homogeneous pairs or chains of matching instructions. "
      "Mixed types in the expression forest prohibit parallel uniform extraction.",
      "Computation graph nodes are heterogeneous.",
      "Form uniform groups of operations for SIMD lanes.",
      { makeFix("Organize your algorithm to run similar operations adjacently (e.g., group Adds then group Muls)") },
      SeverityLevel::Medium, 1.5
  });
)EOF";

    std::string SROA_ADDITIONS = R"EOF(
  Patterns.push_back({
      "sroa", "NotPossible", "alloca is escaped",
      "SROA blocked: alloca is escaped",
      "The local variable is explicitly passed by reference or its pointer is returned. "
      "SROA cannot convert this to scalars and it must remain allocated on the stack.",
      "Pointer to local structure/variable escapes the current functional scope.",
      "Promote the aggregate to SSA values in registers.",
      { makeFix("Return by value or pass small structs by value / small array arguments recursively") },
      SeverityLevel::High, 1.5
  });

  Patterns.push_back({
      "sroa", "NotPossible", "alloca is used with volatile",
      "SROA blocked: volatile accessor",
      "Volatile memory accesses force the compiler to preserve the stack memory "
      "state, preventing any scalarization or register promotion.",
      "Use of volatile keyword overriding optimizer capabilities.",
      "Replace memory stack loads/stores with pure register operations.",
      { makeFix("Remove volatile keyword unless this memory maps to hardware/threading control") },
      SeverityLevel::High, 1.2
  });

  Patterns.push_back({
      "sroa", "NotPossible", "alloca is too large",
      "SROA blocked: alloca is too large",
      "The aggregate structure exceeds SROA's internal cost threshold for scalar expansion.",
      "Size of struct/array violates SROA boundaries.",
      "Partition stack allocations into SSA registers.",
      { makeFix("Use heap allocation (std::vector) for massive arrays, or re-structure the program") },
      SeverityLevel::Low, 0.5
  });

  Patterns.push_back({
      "sroa", "NotPossible", "alloca is used with a select and cannot be promoted",
      "SROA blocked: conditional pointer selection (select inst)",
      "The program conditionally selects pointers to different allocations. "
      "This dynamic pointer alias ambiguity forces SROA to abandon promotion.",
      "Ternary or branching selection of pointers.",
      "Resolve static pointers to concrete variables.",
      { makeFix("Rewrite conditionals to select the loaded scalar values rather than selecting the pointers themselves") },
      SeverityLevel::Medium, 1.0
  });
)EOF";

    std::string UNROLL_ADDITIONS = R"EOF(
  Patterns.push_back({
      "loop-unroll", "NotUnrolled", "loop not suitable for unrolling",
      "Loop unroll skipped: not suitable",
      "The loop lacks the necessary canonical structure (induction variables, clear exits) "
      "needed for the unrolling algorithm.",
      "Non-canonical loop shape.",
      "Clone the loop body sequentially.",
      { makeFix("Refactor into a standard counting 'for' loop") },
      SeverityLevel::Medium, 1.0
  });

  Patterns.push_back({
      "loop-unroll", "NotUnrolled", "could not determine number of loop iterations",
      "Loop unroll skipped: indeterminate iterations",
      "The compiler couldn't statically prove the number of times the loop iterates, "
      "or it couldn't inject a runtime check safely.",
      "Unpredictable trip count analysis.",
      "Identify the exact bounds to unroll effectively.",
      { makeFix("Provide static bounds or use PGO to indicate the loop is uniformly small") },
      SeverityLevel::Medium, 1.0
  });

  Patterns.push_back({
      "loop-unroll", "PartialUnrolled", "",
      "Loop unroll: Partially Unrolled",
      "The loop was partially unrolled by a fixed factor, but not completely eliminated. "
      "This is a successful optimization, but full unrolling might have been desirable "
      "if bounds were fully proven.",
      "Trip count not perfectly divisible by full unroll limit, or full unroll exceeds size threshold.",
      "Completely flat loop execution.",
      { makeFix("If full unrolling is strictly necessary, supply #pragma clang loop unroll(full)") },
      SeverityLevel::Low, 0.5
  });
)EOF";

    std::string INTERCHANGE_ADDITIONS = R"EOF(
  Patterns.push_back({
      "loop-interchange", "NotInterchanged", "not profitable",
      "Loop interchange skipped: not profitable",
      "Data dependence analysis verified the interchange is safe, but the cost model "
      "projected that the transposed loop order would exhibit worse cache locality "
      "or arithmetic behavior.",
      "Cost model rejected the optimization.",
      "Improve matrix traversal memory strides.",
      { makeFix("You don't need to fix this—LLVM successfully predicted interchanging would be bad") },
      SeverityLevel::Low, 0.0
  });

  Patterns.push_back({
      "loop-interchange", "NotInterchanged", "outer loop has dependences",
      "Loop interchange blocked: outer loop dependence",
      "A dependence distance restricts the outer and inner dimensions from being legally "
      "swapped without altering mathematical correctness.",
      "Memory dependence vector violation.",
      "Transpose the loops.",
      { makeFix("Rewrite algorithms to isolate loop dependence dimensions") },
      SeverityLevel::Medium, 2.0
  });

  Patterns.push_back({
      "loop-interchange", "NotInterchanged", "not tightly nested",
      "Loop interchange blocked: not tightly nested",
      "Loop interchange strictly requires a perfect nest (no auxiliary statements "
      "between the inner and outer loop statements).",
      "Imperfect loop nesting structure exists.",
      "Swap inner and outer loop structure.",
      { makeFix("Refactor any auxiliary assignments out of the inter-loop boundary scope") },
      SeverityLevel::Medium, 1.5
  });
)EOF";

    std::string PGO_ADDITIONS = R"EOF(
  Patterns.push_back({
      "pgo-instrumentation", "MissingProf", "",
      "PGO Instrumentation: Missing Profile",
      "A profile file is loaded and in use, but the runtime samples did not include "
      "data for this highly structured function.",
      "Instrumented runtime execution did not trigger this function sequence.",
      "Analyze function's frequency properly through empirical workload tracking.",
      { makeFix("Verify that the profile run executed appropriate code coverage paths") },
      SeverityLevel::Medium, 1.5
  });

  Patterns.push_back({
      "pgo-instrumentation", "InsufficientSamples", "",
      "PGO Instrumentation: Insufficient Samples",
      "Profile Guided Optimization lacks adequate data points to make mathematically "
      "confident optimization changes. The data collected was too thin.",
      "Stochastic execution data provides poor resolution.",
      "Identify high-priority branch paths.",
      { makeFix("Run the profile-instrumented program longer or using larger, sustained input workloads") },
      SeverityLevel::Medium, 1.0
  });
)EOF";

    std::string NEW_SYSTEMS = R"EOF(
void DiagnosticEngine::registerLoopAccessPatterns() {
  Patterns.push_back({
      "loop-accesses", "UnsafeMemDep", "unsafe dependent memory operations",
      "Memory Dependency Analysis: Unsafe dependent memory Operations",
      "Loop Access Analysis could not securely confirm that memory loads and stores "
      "inside the loop do not overlap across different iterations. This blocks downstream "
      "optimizers like the Loop Vectorizer.",
      "Pointer aliasing or true dependence overlap was detected.",
      "Prove loop memory addresses are completely separated across iterations.",
      { makeFix("Provide 'restrict' annotations to array pointers to guarantee non-aliasing") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-accesses", "Dependence", "",
      "Memory Dependency Analysis: Dependence found",
      "A specific Read-After-Write (RAW), Write-After-Read (WAR), or Write-After-Write (WAW) "
      "hazard prevents vectorizing this access sequence.",
      "Memory hazard detected.",
      "Process instructions out-of-order safely.",
      { makeFix("Eliminate cyclic data dependencies spanning across loop body boundaries") },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "loop-accesses", "SymbolicStride", "non-constant stride",
      "Memory Dependency Analysis: Non-constant symbolic stride",
      "The array or pointer is traversed using an offset that the compiler cannot trace "
      "to a constant integer. The unpredictable step prevents reasoning about element overlap.",
      "Dynamic or runtime-defined array stride distance.",
      "Determine explicit gap between accessed indices.",
      { makeFix("Ensure step values are known constants or loop-invariant induction integers") },
      SeverityLevel::Medium, 2.0
  });
}

void DiagnosticEngine::registerDevirtPatterns() {
  Patterns.push_back({
      "wholeprogramdevirt", "UnableToDevirt", "",
      "WholeProgramDevirt blocked: Unable to Devirtualize",
      "The optimizer could not statically resolve all potential virtual overrides "
      "for the class hierarchy. Link-Time Optimization (LTO) needs full visibility "
      "to devirtualize successfully.",
      "Class hierarchy visibility is limited or dynamically unpredictable.",
      "Resolve V-Table indirect call directly to static procedure call.",
      {
        makeFix("Ensure LTO is fully enabled across all compilation units"),
        makeFix("Mark class inheritance chains as final or use [[clang::lto_visibility_public]]")
      },
      SeverityLevel::High, 3.0
  });

  Patterns.push_back({
      "wholeprogramdevirt", "NotDevirt", "",
      "WholeProgramDevirt blocked: No single target",
      "Virtual call resolves to more than one possible implementation target. "
      "The devirtualizer cannot pick a concrete static function to promote.",
      "Polymorphic ambiguity limits targeted devirtualization.",
      "Reduce the virtual call graph options to a singular leaf node.",
      { makeFix("Evaluate utilizing PGO so speculative partial devirtualization might succeed instead") },
      SeverityLevel::Medium, 2.0
  });
}

void DiagnosticEngine::registerArgPromotionPatterns() {
  Patterns.push_back({
      "argpromotion", "NotPromoted", "argument cannot be promoted to register",
      "Argument Promotion skipped: Non-promotable",
      "The function parameter relies on memory semantics that are ineligible to "
      "be cleanly passed entirely as SSA registers.",
      "Pointer size/type makes register extraction complex.",
      "Extract underlying values into CPU registers, bypassing stack memory.",
      { makeFix("Pass structures by value directly rather than using large references/pointers") },
      SeverityLevel::Medium, 1.0
  });

  Patterns.push_back({
      "argpromotion", "NotPromoted", "aliasing prevents promotion",
      "Argument Promotion blocked: Aliasing detected",
      "The optimizer suspects that the pointer argument might alias with "
      "other state variables, meaning promoting it into a CPU register "
      "would dangerously unsync it from memory writes elsewhere.",
      "Pointer tracking could not prove variable isolation.",
      "Isolate exact variable values safely into registers.",
      { makeFix("Mark pure function pointers with 'restrict' if strictly appropriate") },
      SeverityLevel::High, 1.5
  });
}

void DiagnosticEngine::registerJumpThreadingPatterns() {
  Patterns.push_back({
      "jump-threading", "NotJumpThreaded", "",
      "Jump Threading blocked",
      "Jump threading relies on tracing variable states out of previous blocks "
      "to prove the outcome of future branches. The optimizer could not determine "
      "the branch condition statically across these blocks.",
      "Control flow paths contain variables with unpredictable runtime values.",
      "Bypass the conditional jump securely.",
      { makeFix("Simplify the condition pipeline or inline the branch resolution calculation earlier") },
      SeverityLevel::Low, 1.0
  });
}
)EOF";

    lines = insertAfterFunctionStart("registerInliningPatterns", INLINER_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerLoopVectorizationPatterns", LV_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerSLPVectorizationPatterns", SLP_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerSROAPatterns", SROA_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerLoopUnrollPatterns", UNROLL_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerLoopInterchangePatterns", INTERCHANGE_ADDITIONS, lines);
    lines = insertAfterFunctionStart("registerPGOPatterns", PGO_ADDITIONS, lines);

    int idx = -1;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].find("const OptimizationPattern *") != std::string::npos && 
            i + 1 < lines.size() && lines[i+1].find("findMatchingPattern") != std::string::npos) {
            idx = i;
            break;
        }
    }

    if (idx != -1) {
        lines.insert(lines.begin() + idx, NEW_SYSTEMS);
    }

    std::ofstream outFile("lib/DiagnosticEngine.cpp");
    for (const auto& l : lines) {
        outFile << l;
    }
    outFile.close();

    std::cout << "success\n";
    return 0;
}
