/* Test program to trigger parser error recovery for specific token types */
#ifdef __cplusplus
#include <type_traits>
#endif

/* Use volatile to prevent dead code elimination */
volatile int parser_test_condition = 0;

/* Main function - valid and compilable */
int main(void) {
    /* We'll use this to control which error blocks get parsed */
    volatile int test_case = 0;
    
    /* Each test case is isolated to prevent error cascading */
    
    /* Test 1: RT_EXTERN - missing 'extern' in linkage specification */
    if (test_case == 1) {
        /* Missing 'extern' before "C" */
        "C" void missing_extern_func(void);
    }
    
    /* Test 2: RT_STATIC_ASSERT - missing 'static_assert' keyword */
    if (test_case == 2) {
        /* Missing 'static_assert' keyword */
        #ifdef __cplusplus
        (1 == 1, "static_assert without keyword");
        #else
        _Static_assert(1 == 1, "C version");
        #endif
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    
    /* Test 3: RT_DECLTYPE - missing 'decltype' in trailing return type */
    if (test_case == 3) {
        /* Missing 'decltype' - using misspelled version */
        auto decltype_test() -> etype(parser_test_condition);
    }
    
    /* Test 4: RT_OPERATOR - missing 'operator' in overload */
    if (test_case == 4) {
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        
        /* Missing 'operator' keyword */
        MyClass +(MyClass a, MyClass b) {
            return MyClass(a.value + b.value);
        }
    }
    
    /* Test 5: RT_CLASS - missing 'class' in definition */
    if (test_case == 5) {
        /* Missing 'class' keyword */
        MissingClassKeyword {
        public:
            int x;
        };
    }
    
    /* Test 6: RT_TEMPLATE - missing 'template' keyword */
    if (test_case == 6) {
        /* Missing 'template' keyword */
        <typename T>
        void template_missing_func(T x) {}
    }
    
    /* Test 7: RT_NAMESPACE - missing 'namespace' keyword */
    if (test_case == 7) {
        /* Missing 'namespace' keyword */
        missing_namespace_keyword {
            int x;
        }
    }
    
    /* Test 8: RT_USING - missing 'using' keyword */
    if (test_case == 8) {
        /* Missing 'using' keyword */
        namespace std;
    }
    
    /* Test 9: RT_TRY - missing 'try' keyword */
    if (test_case == 9) {
        /* Missing 'try' keyword */
        {
            throw 42;
        }
        catch (...) {
            // Handle exception
        }
    }
#endif

    /* Test 10: RT_ASM - missing 'asm' keyword (works in C and C++) */
    if (test_case == 10) {
        /* Missing 'asm' keyword */
        volatile ("nop");
    }
    
    /* Additional tests with preprocessor and attribute interactions */
    
    /* Test with macro expansion - RT_CLASS */
    if (test_case == 11) {
        #define BAD_CLASS class
        BAD_ MyMacroType {};
    }
    
    /* Test with attribute - RT_EXTERN */
    if (test_case == 12) {
        __attribute__((weak)) "C" void attributed_extern_func(void);
    }
    
    /* Test nested in function - RT_STATIC_ASSERT */
    if (test_case == 13) {
        void nested_function(void) {
            #ifdef __cplusplus
            (1 == 1, "nested static_assert error");
            #else
            _Static_assert(1 == 1, "nested C version");
            #endif
        }
    }
    
#ifdef __cplusplus
    /* Test in namespace - RT_TEMPLATE */
    if (test_case == 14) {
        namespace test_ns {
            /* Missing 'template' inside namespace */
            <typename T>
            void namespace_template_func(T x) {}
        }
    }
    
    /* Test in class - RT_OPERATOR */
    if (test_case == 15) {
        class TestClass {
            /* Missing 'operator' inside class */
            int +(TestClass other) { return 0; }
        };
    }
#endif

    /* Test with volatile control flow - RT_ASM */
    if (test_case == 16) {
        volatile int use_asm = 0;
        if (use_asm) {
            /* Missing 'asm' in conditional block */
            volatile ("nop");
        }
    }
    
    /* Test linkage specification with multiple declarations - RT_EXTERN */
    if (test_case == 17) {
        /* Multiple declarations with missing 'extern' */
        "C" {
            void func1(void);
            void func2(void);
        }
    }
    
    /* All tests are conditionally disabled by default */
    /* To test a specific case, set test_case to that value */
    
    return 0;
}

/* Additional isolated test functions to ensure parser sees each error */

#ifdef __cplusplus
/* Isolated test for RT_DECLTYPE */
void test_decltype_error() {
    /* Missing 'decltype' in auto return type */
    auto missing_decltype_func() -> etype(main);
}

/* Isolated test for RT_OPERATOR in global scope */
int missing_operator_keyword(int x, int y);

/* Isolated test for RT_CLASS with inheritance */
class MissingClassKeywordDerived : public MissingClassKeywordBase {
    int y;
};

/* Isolated test for RT_TEMPLATE with multiple parameters */
template <typename T, typename U>
void valid_template() {}

/* Now the error version */
<typename T, typename U>
void missing_template_keyword_func(T x, U y) {}

/* Isolated test for RT_NAMESPACE with inline */
inline namespace missing_inline_keyword {
    int inline_var;
}

/* Isolated test for RT_USING with alias */
using missing_using_alias = int;

/* Isolated test for RT_TRY with multiple catch blocks */
void test_try_error() {
    {
        throw 1;
    }
    catch (int) {}
    catch (...) {}
}
#endif

/* Isolated test for RT_STATIC_ASSERT in C mode */
#ifndef __cplusplus
void c_static_assert_error() {
    _Static_assert(1 == 1, "Valid");
    /* Error version */
    (1 == 1, "Missing _Static_assert keyword");
}
#endif

/* Isolated test for RT_ASM with extended syntax */
void test_asm_error() {
    /* Missing 'asm' with extended syntax */
    volatile goto ("nop");
}

/* Isolated test for RT_EXTERN with complex linkage */
extern "C" void valid_extern_func(void);

/* Error version */
"C++" void missing_extern_cpp_func(void);
