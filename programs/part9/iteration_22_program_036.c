#ifdef __cplusplus
#include <iostream>
#endif

// Volatile control variables to ensure parsing but prevent execution
volatile int cond = 0;
volatile int cond_false = 0;
volatile int cond_true = 1;

int main() {
    // Valid main function that will compile
    (void)cond;
    (void)cond_false;
    (void)cond_true;
    
    // Each error block is isolated to prevent cascading failures
    // Only one block will be parsed at a time
    
    // Block 1: RT_EXTERN error - C and C++ compatible
    if (cond_false) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // Block 2: RT_STATIC_ASSERT error - C11/C++11 and later
    if (cond_false) {
        // Missing 'static_assert' keyword
        #ifdef __cplusplus
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
        #else
        _Static_assert(1 == 1, "fail");  // C11 version, but we want to trigger RT_STATIC_ASSERT
        // Actually need to trigger the C++ parser path for static_assert
        #endif
    }
    
#ifdef __cplusplus
    // C++ only errors from here
    
    // Block 3: RT_DECLTYPE error
    if (cond_false) {
        // Missing 'decltype' in trailing return type
        auto decltype_error() -> etype(x);  // Error: expected 'decltype'
    }
    
    // Block 4: RT_OPERATOR error
    if (cond_false) {
        class MyClass {};
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
    }
    
    // Block 5: RT_CLASS error
    if (cond_false) {
        // Missing 'class' in class definition
        MyClass { public: int x; };  // Error: expected 'class'
    }
    
    // Block 6: RT_TEMPLATE error with preprocessor interaction
    if (cond_false) {
        // Missing 'template' keyword
        #define TEMPLATE_START <
        TEMPLATE_START typename T> void template_error() {}  // Error: expected 'template'
    }
    
    // Block 7: RT_NAMESPACE error
    if (cond_false) {
        // Missing 'namespace' keyword
        my_ns { int x; };  // Error: expected 'namespace'
    }
    
    // Block 8: RT_USING error with attribute interaction
    if (cond_false) {
        // Missing 'using' in using directive
        __attribute__((unused)) 
        namespace std;  // Error: expected 'using'
    }
    
    // Block 9: RT_TRY error nested in function
    if (cond_false) {
        // Missing 'try' in try-catch block
        void try_error_func() {
            { throw 1; } catch (...) {}  // Error: expected 'try'
        }
    }
#endif

    // Block 10: RT_ASM error - C and C++ compatible
    if (cond_false) {
        // Missing 'asm' keyword in inline assembly
        __asm__ volatile ("nop");  // This is correct, need incorrect version
        // Actually need to trigger the error - use wrong syntax
        volatile ("nop");  // Error: expected 'asm' (in some parser contexts)
    }
    
    // Additional C++ specific blocks for different contexts
    
#ifdef __cplusplus
    // Block 11: RT_CLASS error in different context
    if (cond_false) {
        // Forward declaration without 'class'
        MyForwardDecl;  // Error: expected 'class' (or other type specifier)
    }
    
    // Block 12: RT_TEMPLATE error in template specialization
    if (cond_false) {
        template<typename T>
        class TemplateClass {};
        
        // Missing 'template' in template specialization
        <> class TemplateClass<int> {};  // Error: expected 'template'
    }
    
    // Block 13: RT_OPERATOR error in different context
    if (cond_false) {
        class TestClass {
            // Missing 'operator' in conversion operator
            int() const { return 42; }  // Error: expected 'operator'
        };
    }
    
    // Block 14: RT_USING error in using declaration
    if (cond_false) {
        class Base {
        public:
            void func() {}
        };
        
        class Derived : public Base {
            // Missing 'using' in using declaration
            Base::func;  // Error: expected 'using'
        };
    }
    
    // Block 15: RT_NAMESPACE error with alias
    if (cond_false) {
        // Missing 'namespace' in namespace alias
        ns_alias = existing_namespace;  // Error: expected 'namespace'
    }
#endif

    // Block 16: RT_EXTERN in C++ context with linkage specification
    if (cond_false) {
#ifdef __cplusplus
        // Missing 'extern' in complex linkage specification
        "C" {
            void another_missing_extern();
        }  // Error: expected 'extern'
#endif
    }
    
    // Block 17: RT_STATIC_ASSERT in C++ with template
    if (cond_false) {
#ifdef __cplusplus
        template<typename T>
        void static_assert_test() {
            // Missing 'static_assert' in template
            (sizeof(T) > 0, "Type must have size");  // Error: expected 'static_assert'
        }
#endif
    }
    
    // Block 18: RT_ASM with extended asm
    if (cond_false) {
        // Missing 'asm' with extended assembly
        volatile (  // Error: expected 'asm'
            "mov %0, %1"
            : "=r"(cond)
            : "r"(cond_true)
        );
    }

    return 0;
}
