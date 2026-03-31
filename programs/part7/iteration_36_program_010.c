```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc
// Or with: g++ -O2 -fno-eliminate-unused-debug-types -ftrack-macro-expansion=0 test_parser_recovery.cc
// Or with: g++ -O1 -save-temps -fdump-lang-all test_parser_recovery.cc

#include <cstdio>

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

__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    local += 1;
    
    if (cond) {
        // Missing 'extern' keyword - triggers RT_EXTERN
        "C" {  // Expected 'extern' before string literal
            local += 2;
        }
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 0;
    local += 3;
    
    if (cond) {
        // Missing 'static_assert' keyword - triggers RT_STATIC_ASSERT
        (sizeof(int) == 4, "int must be 4 bytes");  // Expected 'static_assert' before '('
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    local += 5;
    
    if (cond) {
        // Missing 'decltype' keyword - triggers RT_DECLTYPE
        auto func() -> (x);  // Expected 'decltype' before '(' in trailing return type
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    local += 7;
    
    if (cond) {
        // Missing 'operator' keyword - triggers RT_OPERATOR
        struct S {
            int +(const S& other) {  // Expected 'operator' before '+'
                return 0;
            }
        };
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    local += 11;
    
    if (cond) {
        // Missing 'class' keyword - triggers RT_CLASS
        MyClass {  // Expected 'class' before '{'
            int x;
        } instance;
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    local += 13;
    
    if (cond) {
        // Missing 'template' keyword - triggers RT_TEMPLATE
        <typename T>  // Expected 'template' before '<'
        void foo(T t) {}
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    local += 17;
    
    if (cond) {
        // Missing 'namespace' keyword - triggers RT_NAMESPACE
        my_namespace {  // Expected 'namespace' before identifier
            int x;
        }
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    local += 19;
    
    if (cond) {
        // Missing 'using' keyword - triggers RT_USING
        namespace std;  // Expected 'using' before 'namespace'
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    local += 23;
    
    if (cond) {
        // Missing 'asm' keyword - triggers RT_ASM
        volatile ("nop");  // Expected 'asm' before string literal
    }
    
    global_counter += local;
}

__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    local += 29;
    
    if (cond) {
        // Missing 'try' keyword - triggers RT_TRY
        {  // Expected 'try' before '{'
            // Some code
        }
        catch (...) {  // Catch without try
            local += 31;
        }
    }
    
    global_counter += local;
}

int main() {
    volatile int loop_cond = 100;
    
    // Loop to ensure all functions are processed multiple times
    for (volatile int i = 0; i < loop_cond; ++i) {
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
    printf("Result: %d\n", global_counter);
    
    return 0;
}
```

**Explanation of each test case:**

1. **`test_extern`**: Tries to use `"C"` without the `extern` keyword, triggering `RT_EXTERN` error recovery.

2. **`test_static_assert`**: Provides a static assertion condition without the `static_assert` keyword, triggering `RT_STATIC_ASSERT`.

3. **`test_decltype`**: Uses a trailing return type syntax where `decltype` would be expected, triggering `RT_DECLTYPE`.

4. **`test_operator`**: Defines what looks like an operator overload without the `operator` keyword, triggering `RT_OPERATOR`.

5. **`test_class`**: Starts a class definition without the `class` keyword, triggering `RT_CLASS`.

6. **`test_template`**: Begins a template declaration without the `template` keyword, triggering `RT_TEMPLATE`.

7. **`test_namespace`**: Starts a namespace definition without the `namespace` keyword, triggering `RT_NAMESPACE`.

8. **`test_using`**: Attempts a using-directive without the `using` keyword, triggering `RT_USING`.

9. **`test_asm`**: Provides inline assembly syntax without the `asm` keyword, triggering `RT_ASM`.

10. **`test_try`**: Provides a `catch` block without a preceding `try` block, triggering `RT_TRY`.

Each function:
- Starts with valid code to establish parsing context
- Uses `if (cond)` with a volatile condition to ensure the erroneous code is parsed
- Contains the specific erroneous pattern for each keyword case
- Updates `global_counter` to prevent dead code elimination

The `main()` function calls all test functions in a loop controlled by volatile variables, ensuring the parser processes each function body regardless of optimization level.
