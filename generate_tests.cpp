#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

struct TestCase {
    std::string name;
    std::string dir;
    std::string c_code;
    std::string flags;
};

int main() {
    std::vector<TestCase> cases = {
        // INLINER 
        {"inline_hinted_noinline", "test/inline", 
         "__attribute__((noinline)) int big(int x) { return x * 2 + 1; }\nint caller(int x) { return big(x); }", 
         "-O2 -Rpass-missed=inline"},
        
        {"inline_no_definition", "test/inline", 
         "int external_func(int x);\nint caller(int x) { return external_func(x); }", 
         "-O2 -Rpass-missed=inline"},

        // VECTORIZER
        {"vect_pointer_iv", "test/vectorize", 
         "void test(int **p, int n) { for (int i = 0; i < n; i++) { *p[i] = i; } }", 
         "-O3 -Rpass-missed=loop-vectorize"},

        {"vect_mixed_types", "test/vectorize", 
         "void test(double *a, int *b, int n) { for (int i = 0; i < n; i++) { a[i] = (double)b[i] + 1.0; } }", 
         "-O3 -Rpass-missed=loop-vectorize"},

        {"vect_integer_division", "test/vectorize", 
         "void test(int *a, int *b, int n) { for (int i = 0; i < n; i++) { a[i] /= b[i]; } }", 
         "-O3 -Rpass-missed=loop-vectorize"},

        {"vect_conditional_store", "test/vectorize", 
         "void test(int *a, int *b, int n) { for (int i = 0; i < n; i++) { if (a[i] > 0) b[i] = a[i]; } }", 
         "-O3 -Rpass-missed=loop-vectorize"},

        // SROA
        {"sroa_volatile", "test/sroa", 
         "struct S { volatile int x; int y; };\nint test(int a) { struct S s = {a, 0}; return s.x; }", 
         "-O1 -Rpass-missed=sroa"},

        {"sroa_address_escape", "test/sroa", 
         "struct S { int x, y; };\nvoid escape(int *p);\nint test(int a) { struct S s = {a, 0}; escape(&s.x); return s.y; }", 
         "-O1 -Rpass-missed=sroa"},

        // DEVIRT
        {"devirt_no_single_target", "test/devirt", 
         "struct Base { virtual void f() = 0; };\n"
         "struct A : Base { void f() override {} };\n"
         "struct B : Base { void f() override {} };\n"
         "void test(Base *b) { b->f(); }", 
         "-O2 -flto -fwhole-program-vtables -Rpass-missed=wholeprogramdevirt"},

        // ARGPROMOTION
        {"argprom_aliasing", "test/argpromotion", 
         "static int test(int *p, int *q) { return *p + *q; }\nint caller(int *a) { return test(a, a); }", 
         "-O3 -Rpass-missed=argpromotion"},

        // UNROLL
        {"unroll_not_suitable", "test/unroll", 
         "void test(int n) { int i = 0; label: i++; if (i < n) goto label; }", 
         "-O2 -Rpass-missed=loop-unroll"}
    };

    for (const auto& tc : cases) {
        std::string c_file = tc.dir + "/" + tc.name + ".c";
        std::string ll_file = tc.dir + "/" + tc.name + ".ll";
        std::string yaml_file = tc.dir + "/" + tc.name + ".yaml";

        std::ofstream ofs(c_file);
        ofs << tc.c_code << std::endl;
        ofs.close();

        std::string cmd = "clang " + tc.flags + " -S -emit-llvm " + c_file + " -o " + ll_file + 
                          " -fsave-optimization-record -foptimization-record-file=" + yaml_file + " 2>/dev/null";
        
        std::cout << "Generating: " << tc.name << "..." << std::endl;
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            std::cerr << "FAILED: " << tc.name << std::endl;
        }
    }

    return 0;
}
