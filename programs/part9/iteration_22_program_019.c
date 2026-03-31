#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_mode = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to prevent dead code elimination
    volatile int trigger = 0;
    
    // Isolate each error case with conditional compilation/execution
    
    /* 1. RT_EXTERN - Missing 'extern' in linkage specification */
    if (trigger == 1) {
        /* Missing 'extern' before "C" */
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    /* 2. RT_STATIC_ASSERT - Missing 'static_assert' keyword */
    if (trigger == 2) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "Should be true");  // Error: expected 'static_assert'
    }
    
    // C++ specific errors
    #ifdef __cplusplus
    
    /* 3. RT_DECLTYPE - Misspelled 'decltype' in trailing return */
    if (trigger == 3) {
        auto decltype_error() -> etype(parser_mode);  // Error: expected 'decltype'
    }
    
    /* 4. RT_OPERATOR - Missing 'operator' in overload */
    if (trigger == 4) {
        class MyClass4 {};
        int +(MyClass4 a, MyClass4 b) { return 0; }  // Error: expected 'operator'
    }
    
    /* 5. RT_CLASS - Missing 'class' in definition */
    if (trigger == 5) {
        /* Missing 'class' keyword */
        MissingClass5 {  // Error: expected 'class'
            int x;
        };
    }
    
    /* 6. RT_TEMPLATE - Missing 'template' keyword */
    if (trigger == 6) {
        /* Missing 'template' */
        <typename T> void template_error() {}  // Error: expected 'template'
    }
    
    /* 7. RT_NAMESPACE - Missing 'namespace' keyword */
    if (trigger == 7) {
        /* Missing 'namespace' */
        TestNamespace7 {  // Error: expected 'namespace'
            int y;
        }
    }
    
    /* 8. RT_USING - Missing 'using' in directive */
    if (trigger == 8) {
        /* Missing 'using' */
        namespace std;  // Error: expected 'using'
    }
    
    /* 9. RT_TRY - Missing 'try' keyword */
    if (trigger == 9) {
        /* Missing 'try' */
        {  // Error: expected 'try'
            throw 42;
        } catch (...) {}
    }
    
    #endif  /* __cplusplus */
    
    /* 10. RT_ASM - Missing 'asm' in inline assembly */
    if (trigger == 10) {
        /* Missing 'asm' */
        volatile ("nop");  // Error: expected 'asm'
    }
    
    /* Additional tests with preprocessor and attributes */
    
    /* Test with macro expansion */
    #define BAD_CLASS class
    if (trigger == 11) {
        BAD_ MisplacedClass {};  // Error after macro expansion
    }
    
    /* Test with __attribute__ */
    if (trigger == 12) {
        /* Attribute before missing extern */
        __attribute__((weak)) "C" void attr_func();  // Error: expected 'extern'
    }
    
    /* Nested scope test for RT_CLASS */
    #ifdef __cplusplus
    if (trigger == 13) {
        namespace NestedTest {
            /* Missing 'class' inside namespace */
            InnerClass13 {  // Error: expected 'class'
                int z;
            };
        }
    }
    
    /* Template with missing 'template' in class context */
    if (trigger == 14) {
        class Outer14 {
            /* Missing 'template' inside class */
            <typename U> struct Inner {};  // Error: expected 'template'
        };
    }
    
    /* Missing 'operator' in class member function */
    if (trigger == 15) {
        class MyClass15 {
        public:
            int +(const MyClass15& other) const { return 0; }  // Error: expected 'operator'
        };
    }
    
    /* Missing 'decltype' in more complex context */
    if (trigger == 16) {
        template<typename T>
        auto complex_error(T x) -> etype(x) + sizeof(T);  // Error: expected 'decltype'
    }
    
    /* Missing 'try' in function context */
    if (trigger == 17) {
        void try_test() {
            {  // Error: expected 'try'
                throw "error";
            } catch (const char* e) {}
        }
    }
    #endif  /* __cplusplus */
    
    /* C-specific static_assert test */
    #ifndef __cplusplus
    if (trigger == 18) {
        /* C11 static_assert without keyword */
        (sizeof(int) == 4, "int must be 4 bytes");  // Error: expected 'static_assert'
    }
    #endif
    
    return 0;
}
