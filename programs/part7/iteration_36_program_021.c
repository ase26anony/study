```cpp
// test_parser_recovery.cc
// Compile with: g++ -O0 -fparse-all-comments -fpermissive test_parser_recovery.cc
// Or for coverage: g++ -O1 -save-temps -fdump-lang-all test_parser_recovery.cc

volatile int global_counter = 0;

// RT_EXTERN: Missing 'extern' in linkage specification
__attribute__((noinline, noipa))
void test_extern(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'extern' keyword before linkage spec
        "C" { // Parser expects 'extern' here
            local += 1;
        }
    }
    global_counter += local;
}

// RT_STATIC_ASSERT: Missing 'static_assert' keyword
__attribute__((noinline, noipa))
void test_static_assert(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'static_assert' keyword
        (sizeof(int) == 4, "int must be 4 bytes"); // Parser expects 'static_assert' here
        local += 2;
    }
    global_counter += local;
}

// RT_DECLTYPE: Missing 'decltype' in trailing return type
__attribute__((noinline, noipa))
void test_decltype(volatile int v) {
    int local = 0;
    if (v) {
        auto func = [](int x) -> (x) { return x; }; // Missing 'decltype' before (x)
        local += 3;
    }
    global_counter += local;
}

// RT_OPERATOR: Missing 'operator' in overload definition
struct Dummy {};
__attribute__((noinline, noipa))
void test_operator(volatile int v) {
    int local = 0;
    if (v) {
        Dummy d1, d2;
        // Missing 'operator' keyword
        + (const Dummy& a, const Dummy& b) { return Dummy(); } // Parser expects 'operator' here
        local += 4;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'class' keyword
        MissingClassKeyword { // Parser expects 'class', 'struct', or 'union' here
            int x;
        } instance;
        local += 5;
    }
    global_counter += local;
}

// RT_TEMPLATE: Missing 'template' in template declaration
__attribute__((noinline, noipa))
void test_template(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'template' keyword
        <typename T> void foo(T t) {} // Parser expects 'template' here
        local += 6;
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' in namespace definition
__attribute__((noinline, noipa))
void test_namespace(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'namespace' keyword
        MyNamespace { // Parser expects 'namespace' here
            int x;
        }
        local += 7;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
__attribute__((noinline, noipa))
void test_using(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'using' keyword
        namespace std; // Parser expects 'using' here (or could be namespace alias)
        local += 8;
    }
    global_counter += local;
}

// RT_ASM: Missing 'asm' in inline assembly
__attribute__((noinline, noipa))
void test_asm(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'asm' keyword
        volatile ("nop"); // Parser expects 'asm' here
        local += 9;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' in try block
__attribute__((noinline, noipa))
void test_try(volatile int v) {
    int local = 0;
    if (v) {
        // Missing 'try' keyword
        { // Parser expects 'try' here when followed by catch
            local += 10;
        } catch (...) {
            local -= 10;
        }
    }
    global_counter += local;
}

int main() {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9;
    volatile int iterations = 100;
    
    for (volatile int i = 0; i < iterations; ++i) {
        test_extern(v1);
        test_static_assert(v2);
        test_decltype(v3);
        test_operator(v4);
        test_class(v5);
        test_template(v6);
        test_namespace(v7);
        test_using(v8);
        test_asm(v9);
        test_try(v1); // Reuse v1
    }
    
    // Prevent optimization and ensure all code paths are considered
    if (global_counter > 0) {
        return 0;
    }
    return 1;
}
```

**Explanation of each test case:**

1. **RT_EXTERN**: The linkage specification `"C"` must be preceded by `extern`. Without it, the parser expects the `extern` keyword.

2. **RT_STATIC_ASSERT**: The parentheses and string literal look like a static assertion but missing the keyword.

3. **RT_DECLTYPE**: In a trailing return type context, `(x)` would normally be `decltype(x)`. The parser expects `decltype`.

4. **RT_OPERATOR**: The `+` followed by parameter list looks like an operator overload definition but missing the `operator` keyword.

5. **RT_CLASS**: A type name followed by braces looks like a class definition but missing the `class`/`struct`/`union` keyword.

6. **RT_TEMPLATE**: Angle brackets with `typename` followed by a function looks like a template but missing the `template` keyword.

7. **RT_NAMESPACE**: An identifier followed by braces looks like a namespace definition but missing the `namespace` keyword.

8. **RT_USING**: `namespace std;` could be a using-directive but missing the `using` keyword.

9. **RT_ASM**: Parentheses with a string literal looks like inline assembly but missing the `asm` keyword.

10. **RT_TRY**: A block followed by `catch` looks like a try-block but missing the `try` keyword.

**Key design elements:**
- Each test is in its own function with `noinline, noipa` attributes
- Volatile variables control conditional execution
- Valid code before and after each error
- Main function calls all tests in a loop with volatile counter
- Global variable prevents dead code elimination

The parser will encounter each syntax error and attempt recovery, triggering the corresponding error message case from the uncovered lines.
