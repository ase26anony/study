/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope error - type mismatch in initializer */
int global_err = 5 + "string";

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Another macro with token concatenation */
#define MAKE_BAD_EXPR(type, val) type##_var = val

/* Valid function prototype for context */
void valid_func(int x);

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 2. Type mismatch in binary operation - int + string literal */
    int err1 = BAD_ADD(10, "text");
    
    /* 3. Bitwise operator on floating point types */
    double err2 = 3.14159 | 2.71828;
    
    /* 4. Address-of operator on constant literal */
    int* err3 = &42;
    
    /* 5. Invalid initializer - scalar with multiple values */
    int err4 = {5, 6, 7};
    
    /* 6. Using macro with token concatenation incorrectly */
    MAKE_BAD_EXPR(double, "not_a_number");
    
    /* 7. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 8. Misuse of GNU C statement expression */
    int err5 = ({ 
        int x; 
        /* Missing return value - just a declaration */
        char c;
    });
    
    /* 9. Invalid operand in return statement */
    return main + 1;  /* Taking address of function in arithmetic */
    
    /* 10. Undeclared identifier usage (out of scope) */
    {
        int inner = 100;
    }
    int err6 = inner * 2;  /* 'inner' out of scope */
    
    /* 11. Misuse of __builtin function */
    int err7 = __builtin_ctz("hello");
    
    /* 12. Pointer arithmetic with wrong types */
    float f = 2.5;
    int* ptr = &valid_int;
    float err8 = ptr / f;
    
    /* 13. Control flow with invalid condition expression */
    if (5 + "test") {
        valid_int = 1;
    }
    
    /* 14. For loop with invalid increment */
    for (int i = 0; i < 10; i += "step") {
        valid_int++;
    }
    
    /* 15. Function call with invalid argument */
    printf("%d", 5 + "string");
    
    /* More valid code to ensure parser continues */
    int final_valid = valid_int * 2;
    
    return 0;
}

/* Function with invalid parameter initializer */
void another_func(void) {
    /* 16. Invalid compound literal */
    int* p = &(int){1, 2, 3};
    
    /* 17. Conditional expression with type mismatch */
    int x = (valid_int > 0) ? 5 : "string";
    
    /* 18. Invalid cast operation */
    float y = (float)"text";
}

/* Global with invalid GNU extension misuse */
int global_bad = ({ 
    "string_result";  /* Wrong type for int assignment */
});
