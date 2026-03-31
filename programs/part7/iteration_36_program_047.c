```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc
// Or with: g++ -O2 -fno-eliminate-unused-debug-types -ftrack-macro-expansion=0 test_parser_recovery.cc
// Or with: g++ -O1 -save-temps -fdump-lang-all test_parser_recovery.cc

#include <iostream>

// Global variable to prevent optimization
volatile int global_counter = 0;

// Volatile variables for conditional execution
volatile int v1 = 1, v2 = 1, v3 = 1, v4 = 1, v5 = 1, v6 = 1, v7 = 1, v8 = 1, v9 = 1;

// Test functions with __attribute__((noinline, noipa)) to isolate parsing contexts

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'extern' keyword before linkage specification
        "C" {  // Parser expects 'extern' here
            local++;
        }
    }
    global_counter += local;
}

// RT_STATIC_ASSERT: Missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'static_assert' keyword
        (sizeof(int) == 4, "int must be 4 bytes");  // Parser expects 'static_assert' here
        local++;
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    if (cond) {
        auto func = [](int x) -> (x) {  // Parser expects 'decltype' here for trailing return type
            return x + 1;
        };
        local = func(5);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        struct Test {
            // Missing 'operator' keyword
            int +(const Test& other) {  // Parser expects 'operator' here
                return 1;
            }
        };
        local++;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' keyword in definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MissingClassHere {  // Parser expects 'class' here
            int x;
        } instance;
        local++;
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' keyword
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Parser expects 'template' here
        void func() {}
        local++;
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' keyword
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'namespace' keyword
        MyNamespace {  // Parser expects 'namespace' here
            int x;
        }
        local++;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' keyword
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'using' keyword
        namespace std;  // Parser expects 'using' here for using-directive
        local++;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' keyword
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'asm' keyword
        volatile ("mov %0, %0" : : "r"(local));  // Parser expects 'asm' here
        local++;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' keyword
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Parser expects 'try' here
            local = 5 / 0;
        }
        catch (...) {
            local = 1;
        }
    }
    global_counter += local;
}

int main() {
    volatile int loop_counter = 100;
    
    // Call each test function multiple times to ensure parsing
    for (volatile int i = 0; i < loop_counter; i++) {
        test_extern(v1);
        test_static_assert(v2);
        test_decltype(v3);
        test_operator(v4);
        test_class(v5);
        test_template(v6);
        test_namespace(v7);
        test_using(v8);
        test_asm(v9);
        test_try(v1);  // Reuse v1
    }
    
    // Print result to prevent optimization
    std::cout << "Global counter: " << global_counter << std::endl;
    
    return 0;
}
```

**Explanation of how this code triggers each uncovered case:**

1. **RT_EXTERN**: The code `"C" { ... }` attempts to start a linkage specification without the `extern` keyword. The parser expects `extern` before the string literal.

2. **RT_STATIC_ASSERT**: `(sizeof(int) == 4, "int must be 4 bytes");` looks like a static assertion but lacks the `static_assert` keyword. The parser expects `static_assert` when it sees this pattern.

3. **RT_DECLTYPE**: `-> (x)` in a lambda trailing return type is invalid without `decltype`. The parser expects `decltype` when processing this syntax.

4. **RT_OPERATOR**: `int +(const Test& other)` attempts to define an operator overload without the `operator` keyword. The parser expects `operator` when it sees `+` in this context.

5. **RT_CLASS**: `MissingClassHere { ... }` attempts to start a class definition without the `class` keyword. The parser expects `class` (or `struct`/`union`) when it sees an identifier followed by `{`.

6. **RT_TEMPLATE**: `<typename T>` alone is invalid without a preceding `template` keyword. The parser expects `template` when it sees `<` in declaration context.

7. **RT_NAMESPACE**: `MyNamespace { ... }` attempts to define a namespace without the `namespace` keyword. The parser expects `namespace` when it sees an identifier followed by `{` in namespace scope.

8. **RT_USING**: `namespace std;` alone is invalid - it should be `using namespace std;`. The parser expects `using` when it sees `namespace` followed by an identifier and `;`.

9. **RT_ASM**: `volatile ("mov %0, %0" : : "r"(local));` attempts inline assembly without the `asm` keyword. The parser expects `asm` when it sees this GNU extended asm syntax.

10. **RT_TRY**: `{ ... } catch (...) { ... }` attempts a try-catch block without the `try` keyword. The parser expects `try` when it sees a `{` followed by `catch`.

**Key design elements for triggering parser recovery:**
- Each error is wrapped in `if (cond)` with `cond` being `volatile`, forcing the parser to analyze the code
- Functions are marked `noinline, noipa` to prevent interprocedural analysis from skipping the errors
- Valid code precedes each error to put the parser in a normal state
- Multiple calls in a loop ensure all code paths are parsed
- Global variable usage prevents dead code elimination

The parser will encounter each syntax error, enter error recovery mode, and when it determines which keyword was expected, it should trigger the corresponding case in the uncovered lines.
