```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc
// Or with: g++ -O2 -fno-eliminate-unused-debug-types -ftrack-macro-expansion=0 test_parser_recovery.cc
// Or with: g++ -O1 -save-temps -fdump-lang-all test_parser_recovery.cc

#include <iostream>

// Global variable to prevent optimization
int global_counter = 0;

// Volatile variables to control flow
volatile int v1 = 1;
volatile int v2 = 1;
volatile int v3 = 1;
volatile int v4 = 1;
volatile int v5 = 1;
volatile int v6 = 1;
volatile int v7 = 1;
volatile int v8 = 1;
volatile int v9 = 1;

// Test functions with __attribute__((noinline, noipa)) to isolate parsing
// RT_EXTERN case: missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    local += 1;
    
    if (cond) {
        // Missing 'extern' keyword - should trigger RT_EXTERN error recovery
        "C" {
            int x = 5;
            local += x;
        }
    }
    
    // Valid code after error
    int y = local * 2;
    global_counter += y;
}

// RT_STATIC_ASSERT case: missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 0;
    local += 2;
    
    if (cond) {
        // Missing 'static_assert' keyword - should trigger RT_STATIC_ASSERT error recovery
        (sizeof(int) == 4, "int must be 4 bytes");
    }
    
    // Valid code after error
    int z = local + 3;
    global_counter += z;
}

// RT_DECLTYPE case: missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    local += 3;
    
    if (cond) {
        // Missing 'decltype' keyword in context where it's expected
        auto func() -> (x + y) {
            return 42;
        }
    }
    
    // Valid code after error
    int a = local * 4;
    global_counter += a;
}

// RT_OPERATOR case: missing 'operator' in operator overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    local += 4;
    
    if (cond) {
        struct Test {
            // Missing 'operator' keyword - should trigger RT_OPERATOR error recovery
            + (const Test& other) const {
                return Test();
            }
        };
    }
    
    // Valid code after error
    int b = local + 5;
    global_counter += b;
}

// RT_CLASS case: missing 'class' keyword in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    local += 5;
    
    if (cond) {
        // Missing 'class' keyword - should trigger RT_CLASS error recovery
        MyClass {
            int x;
            void method() {}
        } obj;
    }
    
    // Valid code after error
    int c = local * 6;
    global_counter += c;
}

// RT_TEMPLATE case: missing 'template' keyword
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    local += 6;
    
    if (cond) {
        // Missing 'template' keyword - should trigger RT_TEMPLATE error recovery
        <typename T>
        void func(T t) {
            local += sizeof(t);
        }
    }
    
    // Valid code after error
    int d = local + 7;
    global_counter += d;
}

// RT_NAMESPACE case: missing 'namespace' keyword
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    local += 7;
    
    if (cond) {
        // Missing 'namespace' keyword - should trigger RT_NAMESPACE error recovery
        MyNamespace {
            int value = 42;
        }
    }
    
    // Valid code after error
    int e = local * 8;
    global_counter += e;
}

// RT_USING case: missing 'using' keyword
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    local += 8;
    
    if (cond) {
        // Missing 'using' keyword - should trigger RT_USING error recovery
        namespace std;
    }
    
    // Valid code after error
    int f = local + 9;
    global_counter += f;
}

// RT_ASM case: missing 'asm' keyword for inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    local += 9;
    
    if (cond) {
        // Missing 'asm' keyword - should trigger RT_ASM error recovery
        volatile ("nop");
    }
    
    // Valid code after error
    int g = local * 10;
    global_counter += g;
}

// RT_TRY case: missing 'try' keyword
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    local += 10;
    
    if (cond) {
        // Missing 'try' keyword - should trigger RT_TRY error recovery
        {
            throw 42;
        }
        catch (int e) {
            local += e;
        }
    }
    
    // Valid code after error
    int h = local + 11;
    global_counter += h;
}

int main() {
    volatile int loop_counter = 100;
    
    // Call each test function multiple times to ensure parsing
    for (volatile int i = 0; i < loop_counter; ++i) {
        test_extern(v1);
        test_static_assert(v2);
        test_decltype(v3);
        test_operator(v4);
        test_class(v5);
        test_template(v6);
        test_namespace(v7);
        test_using(v8);
        test_asm(v9);
        test_try(v1);  // Reuse v1 since we have 10 functions but 9 volatile vars
    }
    
    // Print result to prevent optimization
    std::cout << "Global counter: " << global_counter << std::endl;
    
    return 0;
}
```

This program is designed to trigger each of the uncovered parser error recovery cases:

1. **RT_EXTERN**: Missing `extern` in `"C" { ... }` linkage specification
2. **RT_STATIC_ASSERT**: Missing `static_assert` in what looks like a static assertion
3. **RT_DECLTYPE**: Missing `decltype` in trailing return type context
4. **RT_OPERATOR**: Missing `operator` in what looks like an operator overload definition
5. **RT_CLASS**: Missing `class` keyword in class definition
6. **RT_TEMPLATE**: Missing `template` keyword before template declaration
7. **RT_NAMESPACE**: Missing `namespace` keyword in namespace definition
8. **RT_USING**: Missing `using` keyword in using-directive
9. **RT_ASM**: Missing `asm` keyword before inline assembly string
10. **RT_TRY**: Missing `try` keyword before try-block

Each error is wrapped in conditional blocks controlled by `volatile` variables, ensuring the parser must analyze the code even if it won't be executed. The `__attribute__((noinline, noipa))` prevents inlining and interprocedural optimizations, isolating each parsing context. The `main()` function calls all test functions in a loop to ensure they're all processed by the parser.
