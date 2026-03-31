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
        // Missing 'extern' keyword before linkage spec
        "C" {  // Parser expects 'extern' here
            local = 1;
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
        local = 2;
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    if (cond) {
        auto func = [](int x) -> (x) { return x + 1; };  // Parser expects 'decltype' before (x)
        local = 3;
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        struct S {
            // Missing 'operator' keyword
            + (const S& other) { return S{}; }  // Parser expects 'operator' here
        };
        local = 4;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' keyword
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyClass {  // Parser expects 'class' here
            int x;
        } obj;
        local = 5;
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
        void foo(T t) {}
        local = 6;
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
            int value;
        }
        local = 7;
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
        local = 8;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' keyword
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'asm' keyword
        volatile ("nop");  // Parser expects 'asm' here
        local = 9;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' keyword
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Parser expects 'try' here for try-block
            local = 10;
        }
        catch (...) {
            local = 11;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 1, v3 = 1, v4 = 1, v5 = 1;
    volatile int v6 = 1, v7 = 1, v8 = 1, v9 = 1, v10 = 1;
    
    volatile int iterations = 100;
    for (volatile int i = 0; i < iterations; i++) {
        test_extern(v1);
        test_static_assert(v2);
        test_decltype(v3);
        test_operator(v4);
        test_class(v5);
        test_template(v6);
        test_namespace(v7);
        test_using(v8);
        test_asm(v9);
        test_try(v10);
    }
    
    // Prevent optimization
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

**Explanation of how this triggers each uncovered case:**

1. **RT_EXTERN**: The string literal `"C"` followed by braces triggers expectation for `extern` in linkage specification.
2. **RT_STATIC_ASSERT**: Parenthesized expression followed by string literal mimics static_assert syntax without the keyword.
3. **RT_DECLTYPE**: Trailing return type `-> (x)` without `decltype` triggers the expectation.
4. **RT_OPERATOR**: `+` in class definition without preceding `operator` keyword.
5. **RT_CLASS**: Identifier `MyClass` followed by braces triggers class definition expectation.
6. **RT_TEMPLATE**: Angle brackets `<typename T>` at namespace/block scope without `template`.
7. **RT_NAMESPACE**: Identifier `MyNamespace` followed by braces triggers namespace expectation.
8. **RT_USING**: `namespace std;` without `using` triggers using-directive expectation.
9. **RT_ASM**: Parenthesized string literal `("nop")` triggers inline assembly expectation.
10. **RT_TRY**: Braces followed by `catch` without preceding `try` keyword.

The `volatile` variables and conditional blocks ensure the parser must analyze each erroneous construct, while the function attributes prevent inlining and inter-procedural optimizations that might skip parsing. The loop in `main()` ensures all functions are processed multiple times.
