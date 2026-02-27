#include "OptDebugger/Support.h"
#include "llvm/ADT/StringRef.h"

namespace optdbg {

// executes a case-insensitive substring search to match raw remarks against patterns
bool matchesPattern(llvm::StringRef Text, llvm::StringRef Pattern) {
  if (Pattern.empty()) return true;
  return Text.contains_insensitive(Pattern);
}

}
