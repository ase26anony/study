```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc
// Or for coverage: g++ -O1 -save-temps -fdump-lang-all test_parser_recovery.cc

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'extern' keyword before linkage specifier
        "C" {  // Error: expected 'extern'
            local += 1;
        }
        // Valid code after error
        local += 2;
        global_counter += local;
    }
}

// RT_STATIC_ASSERT: Missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'static_assert' keyword
        (sizeof(int) == 4, "int must be 4 bytes");  // Error: expected 'static_assert'
        local += 3;
        global_counter += local;
    }
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    if (cond) {
        // In context where decltype is expected
        auto func() -> (x);  // Error: expected 'decltype' (x is undeclared)
        local += 4;
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in operator overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        struct S {
            // Missing 'operator' keyword
            +() { return 5; }  // Error: expected 'operator'
        };
        local += 5;
        global_counter += local;
    }
}

// RT_CLASS: Missing 'class' keyword in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyClass {  // Error: expected 'class'
            int x;
        } instance;
        local += 6;
        global_counter += local;
    }
}

// RT_TEMPLATE: Missing 'template' keyword
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void foo(T t) {}
        local += 7;
        global_counter += local;
    }
}

// RT_NAMESPACE: Missing 'namespace' keyword
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'namespace' keyword
        my_ns {  // Error: expected 'namespace'
            int x;
        }
        local += 8;
        global_counter += local;
    }
}

// RT_USING: Missing 'using' keyword
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'using' keyword
        namespace std;  // Error: expected 'using' (parsed as using-directive without 'using')
        local += 9;
        global_counter += local;
    }
}

// RT_ASM: Missing 'asm' keyword
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'asm' keyword
        ("nop");  // Error: expected 'asm'
        local += 10;
        global_counter += local;
    }
}

// RT_TRY: Missing 'try' keyword
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Error: expected 'try' (when parser expects try-block)
            local += 11;
        }
        catch (...) {  // catch without preceding try
            local += 12;
        }
        global_counter += local;
    }
}

int main() {
    volatile int v1 = 1, v2 = 1, v3 = 1, v4 = 1, v5 = 1;
    volatile int v6 = 1, v7 = 1, v8 = 1, v9 = 1;
    volatile int loop_counter = 100;
    
    // Force parser to analyze all functions
    while (loop_counter--) {
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
    
    // Prevent optimization
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

This program is designed to trigger each of the uncovered parser error recovery cases:

1. **RT_EXTERN**: `"C" { ... }` without `extern`
2. **RT_STATIC_ASSERT**: `(sizeof(int) == 4, "message")` without `static_assert`
3. **RT_DECLTYPE**: `auto func() -> (x);` where `decltype(x)` is expected
4. **RT_OPERATOR**: `+()` without `operator` in a class definition
5. **RT_CLASS**: `MyClass { ... }` without `class` keyword
6. **RT_TEMPLATE**: `<typename T>` without preceding `template`
7. **RT_NAMESPACE**: `my_ns { ... }` without `namespace`
8. **RT_USING**: `namespace std;` without `using` (parsed as using-directive)
9. **RT_ASM**: `("nop")` without `asm`
10. **RT_TRY**: `catch` block without preceding `try`

Each error is wrapped in conditional blocks controlled by `volatile` variables, ensuring the parser must analyze the syntax even if the code isn't executed. The `__attribute__((noinline, noipa))` prevents interprocedural optimizations that might skip parsing.
