#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parse_control = 0;

/* Function to isolate C++-specific errors */
#ifdef __cplusplus
void trigger_cpp_errors() {
    /* RT_DECLTYPE: Missing 'decltype' in trailing return type */
    if (parse_control == 1) {
        auto f1() -> /* missing decltype */ (x);  // Error: expected 'decltype'
    }
    
    /* RT_OPERATOR: Missing 'operator' in overload definition */
    if (parse_control == 2) {
        struct MyClass {};
        int /* missing operator */ +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
    }
    
    /* RT_CLASS: Missing 'class' in class definition */
    if (parse_control == 3) {
        /* missing class */ MyClass1 {  // Error: expected 'class'
            int x;
        };
    }
    
    /* RT_TEMPLATE: Missing 'template' keyword */
    if (parse_control == 4) {
        /* missing template */ <typename T> void f2() {}  // Error: expected 'template'
    }
    
    /* RT_NAMESPACE: Missing 'namespace' keyword */
    if (parse_control == 5) {
        /* missing namespace */ my_ns {  // Error: expected 'namespace'
            int x;
        }
    }
    
    /* RT_USING: Missing 'using' in using directive */
    if (parse_control == 6) {
        /* missing using */ namespace std;  // Error: expected 'using'
    }
    
    /* RT_TRY: Missing 'try' in try-catch block */
    if (parse_control == 7) {
        /* missing try */ {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
}
#endif

/* Function for C/C++ common errors */
void trigger_common_errors() {
    /* RT_EXTERN: Missing 'extern' in linkage specification */
    if (parse_control == 8) {
        /* missing extern */ "C" void f3();  // Error: expected 'extern'
    }
    
    /* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
    if (parse_control == 9) {
        /* missing static_assert */ (1, "fail");  // Error: expected 'static_assert'
    }
    
    /* RT_ASM: Missing 'asm' in inline assembly */
    if (parse_control == 10) {
        /* missing asm */ volatile ("nop");  // Error: expected 'asm'
    }
}

/* Preprocessor macro to test parser state machine */
#define BAD_CLASS class
#define BAD_TEMPLATE template

#ifdef __cplusplus
void trigger_macro_errors() {
    /* RT_CLASS via macro expansion */
    if (parse_control == 11) {
        BAD_ MyClass2 {};  // Error after macro: expected 'class'
    }
    
    /* RT_TEMPLATE via macro expansion */
    if (parse_control == 12) {
        BAD_ <typename T> void f4() {}  // Error after macro: expected 'template'
    }
}
#endif

/* Attribute interaction tests */
void __attribute__((unused)) trigger_attribute_errors() {
    /* RT_ASM with attribute */
    if (parse_control == 13) {
        __attribute__((naked)) /* missing asm */ ("ret");  // Error: expected 'asm'
    }
}

/* Nested scope tests */
#ifdef __cplusplus
namespace Outer {
    void nested_errors() {
        /* RT_USING in namespace scope */
        if (parse_control == 14) {
            /* missing using */ namespace Inner;  // Error: expected 'using'
        }
        
        /* RT_CLASS inside function */
        if (parse_control == 15) {
            struct Container {
                /* missing class */ InnerClass {  // Error: expected 'class'
                    int y;
                };
            };
        }
    }
    
    /* RT_OPERATOR as class member */
    class TestClass {
        /* missing operator */ int() const { return 0; }  // Error: expected 'operator'
    };
}
#endif

/* Main function with valid, compilable code */
int main() {
    /* Initialize volatile control */
    volatile int mode = 0;
    
    /* Use #if 0 to isolate errors - only one block active at a time */
#if 0
    /* Block 1: RT_DECLTYPE (C++ only) */
    #ifdef __cplusplus
    trigger_cpp_errors();
    #endif
#endif

#if 0
    /* Block 2: RT_EXTERN (C/C++) */
    trigger_common_errors();
#endif

#if 0
    /* Block 3: RT_STATIC_ASSERT (C11/C++11) */
    trigger_common_errors();
#endif

#if 0
    /* Block 4: RT_ASM (C/C++) */
    trigger_common_errors();
#endif

#if 0
    /* Block 5: Macro expansions (C++ only) */
    #ifdef __cplusplus
    trigger_macro_errors();
    #endif
#endif

#if 0
    /* Block 6: Attribute interactions */
    trigger_attribute_errors();
#endif

#if 0
    /* Block 7: Nested scope (C++ only) */
    #ifdef __cplusplus
    Outer::nested_errors();
    #endif
#endif

    /* Valid return statement */
    return 0;
}

/* Additional isolated error blocks using if(0) */
#ifdef __cplusplus
void isolated_errors() {
    /* Each error in its own dead branch */
    if (0) {
        /* RT_DECLTYPE */
        auto f5() -> /* missing */ (x + y);
    }
    
    if (0) {
        /* RT_OPERATOR */
        struct S {};
        bool /* missing */ ==(S a, S b) { return true; }
    }
    
    if (0) {
        /* RT_CLASS */
        /* missing */ Example { int z; };
    }
    
    if (0) {
        /* RT_TEMPLATE */
        /* missing */ <class T> T identity(T x) { return x; }
    }
    
    if (0) {
        /* RT_NAMESPACE */
        /* missing */ Another { int w; }
    }
    
    if (0) {
        /* RT_USING */
        /* missing */ namespace boost;
    }
    
    if (0) {
        /* RT_TRY */
        /* missing */ { throw 42; } catch (int e) {}
    }
}
#endif

/* C-specific version */
#ifndef __cplusplus
void c_specific_errors() {
    if (0) {
        /* RT_EXTERN in C */
        /* missing */ "C" { int global_var; }
    }
    
    if (0) {
        /* RT_STATIC_ASSERT in C11 */
        /* missing */ _Static_assert(1, "C11 assert");
    }
    
    if (0) {
        /* RT_ASM in C */
        /* missing */ __volatile__ ("mov r0, r1");
    }
}
#endif
