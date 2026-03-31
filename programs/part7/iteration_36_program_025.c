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
        auto func = [](int x) -> (x) { return x + 1; };  // Parser expects 'decltype' or type before (x)
        local = func(5);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    struct S {
        int value;
        // Missing 'operator' keyword
        int +(const S& other) const {  // Parser expects 'operator' here
            return value + other.value;
        }
    } s1 = {1}, s2 = {2};
    
    if (cond) {
        int sum = s1 + s2;  // This would work if the operator was defined correctly
        global_counter += sum;
    }
}

// RT_CLASS: Missing 'class' keyword in definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyClass {  // Parser expects 'class', 'struct', or 'union' here
            int x;
            void f() {}
        } obj;
        local = obj.x;
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
        void foo(T t) {
            local = sizeof(t);
        }
        foo(42);
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
            int x = 5;
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
        // Missing 'using' keyword
        namespace std;  // Parser expects 'using' here for using-directive
        local = 1;
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
        {  // Parser expects 'try' here when followed by catch
            local = 1;
        }
        catch (...) {  // This catch without preceding try triggers the error
            local = 2;
        }
    }
    global_counter += local;
}

int main() {
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

1. **RT_EXTERN**: The code `"C" { ... }` is missing the `extern` keyword that should precede a linkage specification.

2. **RT_STATIC_ASSERT**: `(sizeof(int) == 4, "int must be 4 bytes");` looks like a static assertion but lacks the `static_assert` keyword.

3. **RT_DECLTYPE**: In the lambda `[](int x) -> (x)`, the parser expects either a type or `decltype(x)` in the trailing return position.

4. **RT_OPERATOR**: `int +(const S& other)` is missing the `operator` keyword before `+`.

5. **RT_CLASS**: `MyClass { ... }` starts what looks like a class definition but lacks the `class`, `struct`, or `union` keyword.

6. **RT_TEMPLATE**: `<typename T>` at the start of a function definition is parsed as a template parameter list missing the `template` keyword.

7. **RT_NAMESPACE**: `MyNamespace { ... }` looks like a namespace definition but lacks the `namespace` keyword.

8. **RT_USING**: `namespace std;` without `using` is parsed as an incomplete namespace definition, triggering the expectation for `using`.

9. **RT_ASM**: `volatile ("mov %0, %1" ... )` has the GNU extended asm syntax but lacks the `asm` keyword.

10. **RT_TRY**: A `catch` block without a preceding `try` block causes the parser to expect `try`.

**Compilation notes:**
- The `volatile` variables ensure the parser must analyze all conditional blocks
- Each function is isolated with `__attribute__((noinline, noipa))` to prevent optimization across functions
- The loop in `main()` ensures all functions are processed multiple times
- The `global_counter` prevents dead code elimination

This code should trigger the specific error recovery paths in the parser when compiled with the recommended flags.
