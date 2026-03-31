```cpp
// test_parser_recovery.cc
// Compile with: g++ -O1 -save-temps -fdump-lang-all -fpermissive test_parser_recovery.cc -o test_parser

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'extern' keyword before linkage specifier
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
        auto func = [](int x) -> (x) {  // Error: expected 'decltype' for (x)
            return x + 1;
        };
        local = func(5);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload definition
struct TestStruct {
    int value;
};

__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    TestStruct ts = {5};
    if (cond) {
        // Missing 'operator' keyword
        int +(const TestStruct& a, const TestStruct& b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
        local = ts.value;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyMissingClass {  // Error: expected 'class'
            int x;
            void method() {}
        } instance;
        local = sizeof(instance);
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' in template declaration
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void template_func(T t) {
            local += sizeof(t);
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
        MyNamespace {  // Error: expected 'namespace'
            int inner_var = 42;
        }
        local = 1;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
namespace Outer {
    namespace Inner {
        int nested = 100;
    }
}

__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'using' keyword
        namespace Inner = Outer::Inner;  // Error: expected 'using'
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
        volatile ("mov %0, %1" : "=r"(local) : "r"(5));  // Error: expected 'asm'
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' in try block
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Error: expected 'try' (when followed by catch)
            local = 10;
        }
        catch (...) {  // This catch without try triggers the error
            local = 20;
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
    
    // Prevent optimization and ensure all code paths are considered
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

**Key Design Elements:**

1. **Volatile Control Flow**: Each test function takes a `volatile int` parameter and uses it in an `if` condition, ensuring the parser must analyze the erroneous code inside.

2. **Isolated Functions**: Each function targets exactly one keyword case with `__attribute__((noinline, noipa))` to prevent inter-function optimization.

3. **Valid Context First**: Each function starts with valid declarations (`int local = 0;`) before the conditional block containing the error.

4. **Specific Error Patterns**:
   - `RT_EXTERN`: `"C" { ... }` without `extern`
   - `RT_STATIC_ASSERT`: `(condition, message)` without `static_assert`
   - `RT_DECLTYPE`: Trailing return type `-> (x)` without `decltype(x)`
   - `RT_OPERATOR`: `int +(...)` without `operator` keyword
   - `RT_CLASS`: `MyClass { ... }` without `class` keyword
   - `RT_TEMPLATE`: `<typename T>` without `template` keyword
   - `RT_NAMESPACE`: `MyNamespace { ... }` without `namespace` keyword
   - `RT_USING`: `namespace Alias = ...` without `using` keyword
   - `RT_ASM`: `volatile ("...")` without `asm` keyword
   - `RT_TRY`: `catch` block without preceding `try` block

5. **Driver Loop**: `main()` calls all test functions in a loop controlled by volatile variables, ensuring all function bodies are parsed.

**Compilation Recommendations:**

1. **For initial testing**: `g++ -O0 -fpermissive test_parser_recovery.cc -o test_parser`
2. **For coverage analysis**: `g++ -O1 -save-temps -fdump-lang-all -fpermissive test_parser_recovery.cc`
3. **For parser path exploration**: `g++ -O2 -fno-eliminate-unused-debug-types -ftrack-macro-expansion=0 -fpermissive test_parser_recovery.cc`

The `-fpermissive` flag is crucial as it allows the compiler to continue parsing despite the syntax errors, increasing the chance of reaching the targeted error recovery code in `parser.cc`.
