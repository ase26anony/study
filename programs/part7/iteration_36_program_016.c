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

// Test functions with __attribute__((noinline, noipa)) to prevent inlining
// and inter-procedural analysis

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    local += 1;
    
    if (cond) {
        // Missing 'extern' keyword before linkage specification
        "C" {  // Error: expected 'extern' before string literal
            local += 2;
        }
    }
    
    global_counter += local;
}

// RT_STATIC_ASSERT: Missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 0;
    local += 3;
    
    if (cond) {
        // Missing 'static_assert' keyword
        (true, "message");  // Error: expected 'static_assert'
    }
    
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in type specifier
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    local += 5;
    
    if (cond) {
        // Missing 'decltype' keyword
        int x = 0;
        auto y = (x);  // Error: expected 'decltype' in some contexts
        // Try to trigger in trailing return type context
        auto func() -> (x) { return 0; };  // Error: expected 'decltype'
    }
    
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in operator overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    local += 7;
    
    if (cond) {
        struct S {
            // Missing 'operator' keyword
            int +(const S& other) { return 0; }  // Error: expected 'operator'
        };
    }
    
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    local += 11;
    
    if (cond) {
        // Missing 'class' keyword
        MyMissingClass {  // Error: expected 'class', 'struct', or 'union'
            int x;
            int y;
        } instance;
    }
    
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' in template declaration
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    local += 13;
    
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void foo(T t) {}
    }
    
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' in namespace definition
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    local += 17;
    
    if (cond) {
        // Missing 'namespace' keyword
        MyNamespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    local += 19;
    
    if (cond) {
        // Missing 'using' keyword
        namespace std;  // Error: expected 'using' for using-directive
    }
    
    global_counter += local;
}

// RT_ASM: Missing 'asm' in inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    local += 23;
    
    if (cond) {
        // Missing 'asm' keyword
        volatile ("mov %0, %0" : : "r"(local));  // Error: expected 'asm'
    }
    
    global_counter += local;
}

// RT_TRY: Missing 'try' in try block
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    local += 29;
    
    if (cond) {
        // Missing 'try' keyword
        {  // Error: expected 'try' when followed by catch
            local += 1;
        }
        catch (...) {  // This catch without try triggers the error
            local += 2;
        }
    }
    
    global_counter += local;
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
        test_try(v1);  // Reuse v1
    }
    
    // Print result to prevent optimization
    std::cout << "Global counter: " << global_counter << std::endl;
    
    return 0;
}
```

**Explanation of each test case:**

1. **RT_EXTERN**: `"C" { ... }` without `extern` triggers expectation for `extern` keyword in linkage specification.

2. **RT_STATIC_ASSERT**: `(true, "message")` in a context where `static_assert` is expected.

3. **RT_DECLTYPE**: `(x)` in trailing return type position without `decltype`.

4. **RT_OPERATOR**: `+(const S& other)` without `operator` keyword in class method definition.

5. **RT_CLASS**: `MyMissingClass { ... }` without `class`, `struct`, or `union` keyword.

6. **RT_TEMPLATE**: `<typename T>` without preceding `template` keyword.

7. **RT_NAMESPACE**: `MyNamespace { ... }` without `namespace` keyword.

8. **RT_USING**: `namespace std;` without `using` keyword (attempted using-directive).

9. **RT_ASM**: `volatile ("mov %0, %0" ... )` without `asm` keyword for inline assembly.

10. **RT_TRY**: `catch (...)` without preceding `try` block.

**Key design elements:**

- Each test is isolated in its own function with `noinline, noipa` attributes
- Volatile variables control conditional execution
- Valid code precedes each error to establish parsing context
- Global variable modification prevents dead code elimination
- Loop in `main()` ensures all functions are processed multiple times

The parser should reach each error recovery case when compiling this code with the recommended flags, particularly with `-fpermissive` which allows continued parsing after errors.
