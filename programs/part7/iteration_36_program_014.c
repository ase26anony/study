```cpp
// test_parser_recovery.cc
// Compile with: g++ -O1 -save-temps -fdump-lang-all -c test_parser_recovery.cc
// Or with: g++ -O0 -fparse-all-comments -fdump-tree-original -fpermissive -c test_parser_recovery.cc

volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8, v9 = 9;
int global_counter = 0;

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
        auto func = [](int x) -> (x) { return x; };  // Parser expects 'decltype' before (x)
        local++;
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
        local++;
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
        void foo(T t) {}
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
        MyNS {  // Parser expects 'namespace' here
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
        volatile ("mov %0, %1" : "=r"(local) : "r"(cond));  // Parser expects 'asm' here
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
            local = 5 / 0;
        }
        catch (...) {
            local = 1;
        }
    }
    global_counter += local;
}

int main() {
    volatile int loop_cond = 100;
    
    for (volatile int i = 0; i < loop_cond; i++) {
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

**Explanation of each test case:**

1. **RT_EXTERN**: `"C" { ... }` without `extern` triggers expectation for `extern` keyword in linkage specification.
2. **RT_STATIC_ASSERT**: `(sizeof(int) == 4, "message")` without `static_assert` triggers expectation for `static_assert` keyword.
3. **RT_DECLTYPE**: `-> (x)` in trailing return type without `decltype` triggers expectation for `decltype` keyword.
4. **RT_OPERATOR**: `+ (const S&)` without `operator` triggers expectation for `operator` keyword in overload.
5. **RT_CLASS**: `MyClass { ... }` without `class` triggers expectation for `class` keyword.
6. **RT_TEMPLATE**: `<typename T>` without `template` triggers expectation for `template` keyword.
7. **RT_NAMESPACE**: `MyNS { ... }` without `namespace` triggers expectation for `namespace` keyword.
8. **RT_USING**: `namespace std;` without `using` triggers expectation for `using` keyword in using-directive.
9. **RT_ASM**: `volatile ("mov ...")` without `asm` triggers expectation for `asm` keyword.
10. **RT_TRY**: `{ ... } catch` without `try` triggers expectation for `try` keyword.

**Key design elements:**
- Each test is in a separate `noinline, noipa` function to isolate parsing contexts
- `volatile` variables control conditional execution, forcing parser analysis
- Valid code precedes each error to establish proper parser state
- `main()` calls all functions in a loop to ensure all code paths are parsed
- Global variable prevents dead code elimination

The parser will encounter each syntax error and should trigger the corresponding error recovery case from the uncovered lines.
