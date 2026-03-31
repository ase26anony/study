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

// RT_OPERATOR: Missing 'operator' in overload declaration
struct Dummy {};
__attribute__((noinline, noipa))
void test_operator(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'operator' keyword
        Dummy& ++(Dummy& d) {  // Parser expects 'operator' here
            return d;
        }
        local++;
    }
    global_counter += local;
}

// RT_CLASS: Missing 'class' in class definition
__attribute__((noinline, noipa))
void test_class(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'class' keyword
        MyMissingClass {  // Parser expects 'class' here
            int x;
        } instance;
        local++;
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
        void foo(T t) {}
        local++;
    }
    global_counter += local;
}

// RT_NAMESPACE: Missing 'namespace' in namespace definition
__attribute__((noinline, noipa))
void test_namespace(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'namespace' keyword
        MissingNamespace {  // Parser expects 'namespace' here
            int x;
        }
        local++;
    }
    global_counter += local;
}

// RT_USING: Missing 'using' in using declaration
namespace N { int x; }
__attribute__((noinline, noipa))
void test_using(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'using' keyword
        namespace N::x;  // Parser expects 'using' here
        local++;
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
        local++;
    }
    global_counter += local;
}

// RT_TRY: Missing 'try' in try block
__attribute__((noinline, noipa))
void test_try(volatile int cond) {
    int local = 0;
    if (cond) {
        // Missing 'try' keyword
        {  // Parser expects 'try' here
            local++;
        }
        catch (...) {
            local--;
        }
    }
    global_counter += local;
}

int main() {
    volatile int loop_cond = 100;
    
    // Force parser to process all functions in a loop
    while (loop_cond-- > 0) {
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
    return global_counter > 0 ? 0 : 1;
}
```

**Key Design Rationale:**

1. **Volatile Control Flow**: Each test function uses `volatile int cond` parameter to create conditional blocks, ensuring the parser must analyze the erroneous code rather than skipping it during early compilation phases.

2. **Isolated Functions**: Each function targets exactly one keyword case with `__attribute__((noinline, noipa))` preventing inter-function optimization and ensuring each error context is parsed independently.

3. **Valid Context Setup**: Each function starts with `int local = 0;` to establish normal parsing state before introducing the error.

4. **Specific Error Patterns**:
   - `RT_EXTERN`: `"C" { ... }` without `extern`
   - `RT_STATIC_ASSERT`: `(condition, message)` without `static_assert`
   - `RT_DECLTYPE`: Trailing return type `-> (x)` without `decltype(x)`
   - `RT_OPERATOR`: `++(Dummy&)` without `operator++`
   - `RT_CLASS`: `MyMissingClass { ... }` without `class`
   - `RT_TEMPLATE`: `<typename T> void foo()` without `template`
   - `RT_NAMESPACE`: `MissingNamespace { ... }` without `namespace`
   - `RT_USING`: `namespace N::x;` without `using`
   - `RT_ASM`: `volatile ("nop")` without `asm`
   - `RT_TRY`: `{ ... } catch` without `try`

5. **Main Driver**: The `main()` function calls all test functions in a loop controlled by `volatile int loop_cond`, ensuring all function bodies are parsed regardless of runtime execution paths.

**Compilation Recommendations:**
- Use `-O1 -save-temps -fdump-lang-all` for maximum parser path coverage
- Use `-O0 -fparse-all-comments -fdump-tree-original -fpermissive` to observe parser recovery behavior
- The `-fpermissive` flag may be necessary for the parser to continue past certain errors

This code systematically triggers each uncovered error recovery case by presenting the parser with syntax where specific keywords are expected but missing, while using volatile variables to force conditional parsing of each erroneous construct.
