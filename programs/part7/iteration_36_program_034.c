```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc -o test_parser
// Or for coverage: g++ -O1 -save-temps -fdump-lang-all -fpermissive test_parser_recovery.cc

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
        auto func = [](int x) -> (x) { return x; };  // Parser expects 'decltype' before (x)
        local = 3;
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
            + (const S& other) { return S(); }  // Parser expects 'operator' here
        };
        local = 4;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' keyword in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyMissingClass {  // Parser expects 'class' here
            int x;
        } instance;
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
        MyMissingNamespace {  // Parser expects 'namespace' here
            int x;
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

// RT_ASM: Missing 'asm' keyword for inline assembly
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
            throw 42;
        }
        catch (int e) {
            local = 10;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
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

**Explanation of each test case:**

1. **RT_EXTERN**: The linkage specification `"C"` must be preceded by `extern`. Without it, the parser expects `extern`.

2. **RT_STATIC_ASSERT**: A static assertion requires the `static_assert` keyword. The parenthesized expression looks like a static_assert without the keyword.

3. **RT_DECLTYPE**: In a trailing return type context, `(x)` would normally be `decltype(x)`. The parser expects `decltype`.

4. **RT_OPERATOR**: Operator overload definitions require the `operator` keyword. `+` alone triggers the expectation.

5. **RT_CLASS**: A class definition must start with `class`, `struct`, or `union`. An identifier followed by `{` triggers class expectation.

6. **RT_TEMPLATE**: Template declarations must start with `template`. Angle brackets alone trigger template expectation.

7. **RT_NAMESPACE**: Namespace definitions require the `namespace` keyword. An identifier followed by `{` triggers namespace expectation.

8. **RT_USING**: Using-directives require the `using` keyword. `namespace std;` alone triggers using expectation.

9. **RT_ASM**: Inline assembly requires the `asm` keyword. The string literal in parentheses triggers asm expectation.

10. **RT_TRY**: Try-blocks require the `try` keyword. A block followed by `catch` triggers try expectation.

**Compilation notes:**
- Use `-fpermissive` to allow the parser to continue past errors
- The `volatile` variables ensure all code paths are parsed
- Each function is isolated with attributes to prevent optimization
- The loop in `main()` ensures all functions are processed multiple times
- The global counter prevents dead code elimination

This code should trigger each of the uncovered error recovery cases in the parser when compiled with the recommended flags.
