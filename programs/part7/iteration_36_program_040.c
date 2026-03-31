```cpp
// Compile with: g++ -O1 -save-temps -fdump-lang-all -fpermissive -c parser_coverage.cc
// Or: g++ -O0 -fparse-all-comments -fdump-tree-original -fpermissive -c parser_coverage.cc

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'extern' keyword before linkage spec
        "C" {  // Error: expected 'extern'
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
        (sizeof(int) == 4, "int must be 4 bytes");  // Error: expected 'static_assert'
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 0;
    if (cond) {
        auto func = [](int x) -> (x) {  // Error: expected 'decltype' for trailing return type
            return x + 1;
        };
        local = func(5);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload declaration
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        struct S {
            // Missing 'operator' keyword
            int +(const S& other) const {  // Error: expected 'operator'
                return 1;
            }
        };
        local = 1;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' keyword in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyMissingClass {  // Error: expected 'class'
            int x;
        } instance;
        local = 1;
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' keyword
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void foo(T t) {
            local = sizeof(t);
        }
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' keyword
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'namespace' keyword
        MissingNamespace {  // Error: expected 'namespace'
            int x;
        }
        local = 1;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' keyword
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        namespace N { int x = 5; }
        // Missing 'using' keyword
        namespace N;  // Error: expected 'using' for using-directive
        local = 1;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' keyword for inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
        local = 1;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' keyword
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Error: expected 'try' for try-block
            local = 1;
        }
        catch (...) {
            local = 2;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 1, v3 = 1, v4 = 1, v5 = 1;
    volatile int v6 = 1, v7 = 1, v8 = 1, v9 = 1, v10 = 1;
    
    volatile int loop_counter = 100;
    while (loop_counter-- > 0) {
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
    return global_counter % 256;
}
```

## Explanation of Design Choices:

1. **Volatile Control Flow**: Each test function takes a `volatile int` parameter and uses it in an `if` condition. This forces the parser to analyze the erroneous code inside the conditional block, as it cannot determine at parse time whether the block will be executed.

2. **Isolated Functions**: Each test is in its own function with `__attribute__((noinline, noipa))` to prevent inlining and inter-procedural analysis. This ensures each error is parsed independently.

3. **Valid Context First**: Each function starts with `int local = 0;` to establish a valid parsing context before introducing the error.

4. **Specific Error Patterns**:
   - **RT_EXTERN**: `"C" { ... }` without `extern`
   - **RT_STATIC_ASSERT**: `(condition, message)` without `static_assert`
   - **RT_DECLTYPE**: Trailing return type `-> (x)` without `decltype(x)`
   - **RT_OPERATOR**: `int +(const S&)` without `operator+`
   - **RT_CLASS**: `MyMissingClass { ... }` without `class`
   - **RT_TEMPLATE**: `<typename T> void foo(T)` without `template`
   - **RT_NAMESPACE**: `MissingNamespace { ... }` without `namespace`
   - **RT_USING**: `namespace N;` without `using namespace N;`
   - **RT_ASM**: `volatile ("nop")` without `asm volatile`
   - **RT_TRY**: `{ ... } catch(...)` without `try`

5. **Main Driver**: The `main()` function calls all test functions in a loop controlled by a volatile counter, ensuring all function bodies are parsed regardless of execution path.

The compilation options recommended will maximize parser path coverage while ensuring the error recovery logic is triggered for each missing keyword case.
