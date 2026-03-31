#ifdef __cplusplus
#include <iostream>
#endif

// Volatile control variable to ensure all code paths are parsed
volatile int parse_control = 0;

int main() {
    // Main function is valid and compilable
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to control which error blocks are parsed
    // Each block is isolated to prevent cascading errors
    
    // Block 1: RT_EXTERN - missing 'extern' in linkage specification
    if (parse_control == 1) {
        // C and C++ compatible
        "C" void missing_extern_func();  // Missing 'extern' keyword
    }
    
    // Block 2: RT_STATIC_ASSERT - missing 'static_assert' keyword
    if (parse_control == 2) {
        #ifdef __cplusplus
        // C++11 and later
        (1 == 1, "static assertion failed");  // Missing 'static_assert'
        #else
        // C11 and later
        _Static_assert(1 == 1, "fail");  // Valid C, but different token
        #endif
    }
    
    // Block 3: RT_DECLTYPE - misspelled 'decltype' in trailing return
    #ifdef __cplusplus
    if (parse_control == 3) {
        auto decltype_error() -> etype(1 + 2);  // 'decltype' misspelled as 'etype'
    }
    #endif
    
    // Block 4: RT_OPERATOR - missing 'operator' in overload
    #ifdef __cplusplus
    if (parse_control == 4) {
        class MyClass {};
        int +(MyClass a, MyClass b) { return 0; }  // Missing 'operator'
    }
    #endif
    
    // Block 5: RT_CLASS - missing 'class' in definition
    #ifdef __cplusplus
    if (parse_control == 5) {
        MissingClassKeyword {  // Missing 'class' keyword
            int x;
        };
    }
    #endif
    
    // Block 6: RT_TEMPLATE - missing 'template' keyword
    #ifdef __cplusplus
    if (parse_control == 6) {
        <typename T>  // Missing 'template'
        void template_error() {}
    }
    #endif
    
    // Block 7: RT_NAMESPACE - missing 'namespace' keyword
    #ifdef __cplusplus
    if (parse_control == 7) {
        my_namespace {  // Missing 'namespace'
            int x;
        }
    }
    #endif
    
    // Block 8: RT_USING - missing 'using' keyword
    #ifdef __cplusplus
    if (parse_control == 8) {
        namespace std;  // Missing 'using' for directive
    }
    #endif
    
    // Block 9: RT_ASM - missing 'asm' keyword
    if (parse_control == 9) {
        // C and C++ inline assembly
        __volatile__ ("nop");  // Missing 'asm' keyword
    }
    
    // Block 10: RT_TRY - missing 'try' keyword
    #ifdef __cplusplus
    if (parse_control == 10) {
        {  // Missing 'try'
            throw 1;
        } catch (...) {
            // Handle exception
        }
    }
    #endif
    
    // Additional tests with preprocessor and attributes
    
    // Test with macro expansion
    #define EXPECT_CLASS class
    if (parse_control == 11) {
        #ifdef __cplusplus
        EXPECT_ MyType {};  // Macro expansion missing token
        #endif
    }
    
    // Test with __attribute__
    if (parse_control == 12) {
        #ifdef __cplusplus
        void __attribute__((noreturn)) missing_extern_func2();  // Missing 'extern'
        #else
        void __attribute__((noreturn)) missing_extern_func3();  // C version
        #endif
    }
    
    // Nested scope test for RT_CLASS
    #ifdef __cplusplus
    if (parse_control == 13) {
        namespace outer {
            InnerClass {  // Missing 'class' inside namespace
                int y;
            };
        }
    }
    #endif
    
    // Test for RT_STATIC_ASSERT in C++ with template
    #ifdef __cplusplus
    if (parse_control == 14) {
        template<typename T>
        struct TestStruct {
            (sizeof(T) > 0, "Type must have size");  // Missing 'static_assert'
        };
    }
    #endif
    
    // Test for RT_OPERATOR inside class
    #ifdef __cplusplus
    if (parse_control == 15) {
        class TestClass {
            int +(const TestClass& other) { return 0; }  // Missing 'operator'
        };
    }
    #endif
    
    return 0;
}
