```cpp
// Compile with: g++ -O1 -save-temps -fdump-lang-all -fpermissive -c parser_coverage.cc
// or other combinations from the recommended options

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int cond) {
    int local = 1;
    if (cond) {
        // Missing 'extern' keyword before linkage spec
        "C" {  // Parser expects 'extern' here
            local += 2;
        }
    }
    global_counter += local;
}

// RT_STATIC_ASSERT: Missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int cond) {
    int local = 3;
    if (cond) {
        // Missing 'static_assert' keyword
        (sizeof(int) == 4, "int must be 4 bytes");  // Parser expects 'static_assert' here
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int cond) {
    int local = 5;
    if (cond) {
        auto func = [](int x) -> (x) {  // Parser expects 'decltype' here for decltype(x)
            return x + 1;
        };
        local += func(10);
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in operator overload
struct Dummy {};
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 7;
    if (cond) {
        Dummy d1, d2;
        // Missing 'operator' keyword
        Dummy +(const Dummy& a, const Dummy& b) {  // Parser expects 'operator' here
            return Dummy();
        }
        local += sizeof(d1);
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 9;
    if (cond) {
        // Missing 'class' keyword
        MissingClassKeyword {  // Parser expects 'class' (or 'struct') here
            int x;
        } instance;
        local += sizeof(instance);
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' in template declaration
__attribute__((noinline, noipa))
void test_template(volatile int cond) {
    int local = 11;
    if (cond) {
        // Missing 'template' keyword
        <typename T>  // Parser expects 'template' here
        void foo(T t) {
            local += sizeof(t);
        }
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' in namespace definition
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 13;
    if (cond) {
        // Missing 'namespace' keyword
        MyNamespace {  // Parser expects 'namespace' here
            int value = 42;
        }
        local += 1;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
namespace N { int x = 100; }
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 15;
    if (cond) {
        // Missing 'using' keyword
        namespace N;  // Parser expects 'using' here for using-directive
        // or: N::x;  // For using-declaration
        local += 2;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' in inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int cond) {
    int local = 17;
    if (cond) {
        // Missing 'asm' keyword
        volatile ("nop");  // Parser expects 'asm' here
        local += 3;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' in try block
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 19;
    if (cond) {
        // Missing 'try' keyword
        {  // Parser expects 'try' here when followed by catch
            local += 4;
        }
        catch (...) {  // This catch without preceding try triggers the error
            local += 5;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9;
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
    
    // Prevent optimization and ensure all code paths are considered
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

This program systematically triggers each uncovered parser error recovery case:

1. **Volatile Control Flow**: Each test function uses `volatile int cond` to make the erroneous code conditionally reachable, forcing the parser to analyze it.

2. **Keyword-Specific Errors**:
   - `test_extern()`: Missing `extern` before `"C"` linkage spec
   - `test_static_assert()`: Missing `static_assert` keyword in assertion
   - `test_decltype()`: Missing `decltype` in trailing return type
   - `test_operator()`: Missing `operator` in operator overload definition
   - `test_class()`: Missing `class` keyword in class definition
   - `test_template()`: Missing `template` keyword before template parameters
   - `test_namespace()`: Missing `namespace` keyword in namespace definition
   - `test_using()`: Missing `using` keyword in using-directive
   - `test_asm()`: Missing `asm` keyword before inline assembly
   - `test_try()`: Missing `try` before catch block

3. **Isolation**: Each error is in a separate `noinline, noipa` function to prevent error cascading.

4. **Valid Context**: Each function starts with valid code (`int local = ...`) before the conditional block containing the error.

5. **Driver Loop**: `main()` calls all functions in a loop controlled by volatile variables, ensuring all function bodies are parsed.

The compilation options recommended will help ensure the parser reaches the error recovery logic for each case, hitting the uncovered lines in `parser.cc`.
