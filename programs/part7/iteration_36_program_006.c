```cpp
// test_parser_recovery.cc
// Compile with: g++ -O1 -save-temps -fdump-lang-all -fpermissive test_parser_recovery.cc -o test_parser

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'extern' keyword before linkage spec
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
        (sizeof(int) == 4, "int must be 4 bytes");  // Parser expects 'static_assert'
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    if (cond) {
        auto func = [](int x) -> (x) {  // Missing 'decltype' before (x)
            return x + 1;
        };
        local = func(5);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload declaration
struct TestStruct {
    int value;
};

__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'operator' keyword
        TestStruct +(const TestStruct& a, const TestStruct& b) {  // Should be 'operator+'
            return {a.value + b.value};
        }
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MissingClass {  // Parser expects 'class' here
            int x;
            void foo() {}
        } instance;
        local = 1;
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' in template declaration
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Parser expects 'template' here
        void foo(T t) {
            local = sizeof(t);
        }
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' in namespace definition
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'namespace' keyword
        MyNamespace {  // Parser expects 'namespace' here
            int x = 5;
        }
        local = 1;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'using' keyword
        namespace std;  // Parser expects 'using' here for using-directive
        local = 1;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' in inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'asm' keyword
        volatile ("nop");  // Parser expects 'asm' here
        local = 1;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' in try-block
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Parser expects 'try' here for try-block
            local = 1;
        }
        catch (...) {  // catch without preceding try
            local = 2;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 1, v3 = 1, v4 = 1, v5 = 1;
    volatile int v6 = 1, v7 = 1, v8 = 1, v9 = 1, v10 = 1;
    
    volatile int loop_counter = 100;
    
    for (volatile int i = 0; i < loop_counter; i = i + 1) {
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
    
    // Use the global counter to prevent optimization
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

**Explanation of how this triggers each uncovered case:**

1. **RT_EXTERN**: The string literal `"C"` followed by braces triggers expectation for `extern` in linkage specification context.

2. **RT_STATIC_ASSERT**: Parenthesized expression followed by string literal mimics static_assert syntax without the keyword.

3. **RT_DECLTYPE**: Trailing return type `-> (x)` without `decltype` triggers the expectation in lambda return type context.

4. **RT_OPERATOR**: Function-like declaration `+(const TestStruct&...` without `operator` keyword triggers expectation for operator overload.

5. **RT_CLASS**: Identifier followed by braces `MissingClass { ... }` triggers expectation for `class` keyword in type definition.

6. **RT_TEMPLATE**: Angle brackets `<typename T>` at statement level triggers expectation for `template` keyword.

7. **RT_NAMESPACE**: Identifier followed by braces `MyNamespace { ... }` triggers expectation for `namespace` keyword.

8. **RT_USING**: `namespace std;` without `using` triggers expectation for using-directive keyword.

9. **RT_ASM**: `volatile ("nop");` mimics asm syntax but with wrong keyword, triggering expectation for `asm`.

10. **RT_TRY**: Braces followed by `catch` block without preceding `try` triggers expectation for try-block keyword.

The volatile variables and conditional execution ensure the parser must analyze each erroneous construct, while the function attributes prevent inlining and inter-procedural optimizations that might skip parsing. The loop in main ensures all functions are processed multiple times.
