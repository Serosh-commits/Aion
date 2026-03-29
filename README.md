# Aion

Aion is a diagnostic tool for LLVM. It helps you understand why the compiler failed to optimize your code by analyzing optimization remarks and IR.

## Workflow

1. **Compile with remarks**:
   ```bash
   clang -O2 -Rpass-missed=all -fsave-optimization-record -S -emit-llvm input.c -o input.ll
   ```

2. **Run Aion**:
   ```bash
   ./opt-debugger input.ll --remarks=input.yaml
   ```

## Features

- **Explain Failures**: Converts cryptic LLVM remarks into human-readable explanations.
- **Fix Suggestions**: Provides concrete code changes to enable missed optimizations.
- **IR Diff**: Shows exactly what changed (or didn't change) in the LLVM IR.
- **Broad Coverage**: Supports Inlining, Loop Vectorization, SLP, SROA, Unrolling, and more.

## Build

Requires LLVM 15+.

```bash
mkdir build && cd build
cmake ..
make
```

## Usage Examples

Analyze a single file:
```bash
./opt-debugger file.ll --remarks=file.yaml
```

Compare two IR states:
```bash
./opt-debugger --before=old.ll --after=new.ll --remarks=remarks.yaml
```

Generate an HTML report:
```bash
./opt-debugger input.ll --remarks=input.yaml --html=report.html
```
