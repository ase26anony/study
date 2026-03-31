/* Test program to trigger specific parser error recovery paths in GCC */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variables to ensure code is parsed but not necessarily executed */
volatile int parse_condition = 0;
volatile int cond_extern = 0;
volatile int cond_static_assert = 0;
volatile int cond_decltype = 0;
volatile int cond_operator = 0;
volatile int cond_class = 0;
volatile int cond_template = 0;
volatile int cond_namespace = 0;
volatile int cond_using = 0;
volatile int cond_asm = 0;
volatile int cond_try = 0;

/* Preprocessor macros to test parser state during macro expansion */
#define BAD_EXTERN 
#define BAD_CLASS 
#define BAD_TEMPLATE 
#define BAD_NAMESPACE 

/* Function to test RT_EXTERN error */
void test_extern_error(void) {
    if (cond_extern) {
        /* Missing 'extern' in linkage specification - RT_EXTERN */
        "C" void missing_extern_func(void);  /* Error: expected 'extern' */
        
        /* Another variant with attributes */
        __attribute__((weak)) "C" int missing_extern_var;  /* Error after attribute */
    }
}

/* Function to test RT_STATIC_ASSERT error */
void test_static_assert_error(void) {
    if (cond_static_assert) {
        /* Missing 'static_assert' keyword - RT_STATIC_ASSERT */
        (1 == 1, "Assertion failed");  /* Error: expected 'static_assert' */
        
        /* With parentheses confusion */
        static_assert (1 == 1), "Missing closing paren";  /* Error in macro-like context */
    }
}

#ifdef __cplusplus
/* C++ specific tests */

/* Test RT_DECLTYPE error */
auto test_decltype_error() -> etype(cond_decltype) {  /* Error: expected 'decltype' */
    return 0;
}

/* Test RT_OPERATOR error */
struct MyClass {
    int value;
};

/* Missing 'operator' in operator overload - RT_OPERATOR */
int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
    return a.value + b.value;
}

/* Test RT_CLASS error */
/* Missing 'class' keyword - RT_CLASS */
BAD_ MissingClass {  /* Error from macro expansion */
    int x;
    int y;
};

/* Another class error in function scope */
void test_class_error(void) {
    if (cond_class) {
        struct MyClass { int a; };
        /* Missing 'class' in elaborated type specifier */
        friend MyClass;  /* Error: expected 'class' */
    }
}

/* Test RT_TEMPLATE error */
/* Missing 'template' keyword - RT_TEMPLATE */
<typename T>  /* Error: expected 'template' */
void template_missing_func(T t) {
    // Function body
}

/* Template error with attributes */
__attribute__((always_inline)) <int N>  /* Error after attribute */
void template_attr_error() {}

/* Test RT_NAMESPACE error */
/* Missing 'namespace' keyword - RT_NAMESPACE */
BAD_NAMESPACE missing_ns {  /* Error from macro */
    int value;
};

/* Nested namespace error */
void test_namespace_error() {
    if (cond_namespace) {
        /* Missing 'namespace' in using directive */
        std::cout;  /* Actually valid, need different error context */
        
        /* Direct namespace definition error */
        my_inner_ns {  /* Error: expected 'namespace' */
            int x;
        };
    }
}

/* Test RT_USING error */
/* Missing 'using' keyword - RT_USING */
namespace std;  /* Error: expected 'using' for using-directive */

/* Using declaration error */
void test_using_error() {
    if (cond_using) {
        /* Missing 'using' in using-declaration */
        std::vector;  /* Error: expected 'using' */
    }
}

/* Test RT_TRY error */
void test_try_error() {
    if (cond_try) {
        /* Missing 'try' keyword - RT_TRY */
        {  /* Error: expected 'try' */
            throw 42;
        } catch (int e) {
            // Handle exception
        }
        
        /* Try block in wrong context */
        try {
            // Normal try
        }  /* Missing catch expected here */
    }
}

#endif /* __cplusplus */

/* Test RT_ASM error (works in both C and C++) */
void test_asm_error(void) {
    if (cond_asm) {
        /* Missing 'asm' keyword - RT_ASM */
        volatile ("nop");  /* Error: expected 'asm' */
        
        /* With gcc extended asm */
        __volatile__ ("mov %0, %1" : : "r"(0), "r"(1));  /* Still needs 'asm' */
    }
}

/* Main function with conditional parsing of error cases */
int main(void) {
    /* Each condition is volatile to prevent dead code elimination */
    
    /* Test extern error */
    if (parse_condition) {
        test_extern_error();
    }
    
    /* Test static_assert error */
    if (parse_condition + 1) {
        test_static_assert_error();
    }
    
#ifdef __cplusplus
    /* C++ specific error tests */
    
    /* Test decltype error - will be parsed as function declaration */
    if (parse_condition + 2) {
        auto result = test_decltype_error();
    }
    
    /* Test operator error */
    if (parse_condition + 3) {
        MyClass a{1}, b{2};
        int sum = a + b;  /* This will find the erroneous operator declaration */
    }
    
    /* Test class error */
    if (parse_condition + 4) {
        MissingClass mc;  /* Refers to the erroneous class definition */
    }
    
    /* Test template error */
    if (parse_condition + 5) {
        template_missing_func(42);  /* Refers to erroneous template */
    }
    
    /* Test namespace error */
    if (parse_condition + 6) {
        /* Try to use the erroneous namespace */
        missing_ns::value = 10;
    }
    
    /* Test using error - already at file scope causes parse error */
    
    /* Test try error */
    if (parse_condition + 7) {
        test_try_error();
    }
#endif
    
    /* Test asm error */
    if (parse_condition + 8) {
        test_asm_error();
    }
    
    return 0;
}

/* Additional file-scope errors that will be parsed immediately */
#ifdef __cplusplus
/* Another namespace error at file scope */
my_global_ns {  /* Error: expected 'namespace' */
    int global_var;
};

/* Template error with multiple parameters */
<typename T, typename U>  /* Error: expected 'template' */
class Pair {
    T first;
    U second;
};
#endif

/* Static assert error at file scope */
(1 == 2, "File scope assertion failed");  /* Error: expected 'static_assert' */

/* Extern error at file scope */
"C" int file_scope_var;  /* Error: expected 'extern' */
