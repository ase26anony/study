/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope errors */
int global_bad = 5 + "string";  /* Type mismatch in global initializer */

/* Invalid macro expansions */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(x, y) (x | y)
#define INVALID_INIT {1, 2, 3, 4, 5}

/* Misuse of GCC extensions */
#define BAD_STMT_EXPR ({ int x; })  /* Missing return value */

/* Function with multiple error contexts */
void problematic_function(void) {
    /* Valid code for context */
    int valid_var = 42;
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_var + "text";
    
    /* 2. Invalid operand combination */
    double fp = 3.14159;
    double bad_bitwise = fp | 2.71828;  /* Bitwise on floating point */
    
    /* 3. Undeclared identifier usage (out of order) */
    int z = undeclared_var + 5;  /* Used before declaration */
    int undeclared_var = 10;     /* Declaration comes after use */
    
    /* 4. Macro-generated type mismatch */
    int macro_error = BAD_ADD(10, "macro_string");
    
    /* 5. Invalid initializer */
    int arr[3] = INVALID_INIT;  /* Too many initializers */
    
    /* Valid statement to provide context */
    int ok = 100;
}

/* Another function with different error contexts */
int more_errors(int param) {
    /* 6. Address-of operator on constant */
    int* bad_ptr = &42;
    
    /* 7. Invalid pointer arithmetic */
    int x = 10;
    float* wrong_type_ptr = &x;
    float bad_arithmetic = wrong_type_ptr / 2;  /* Pointer division */
    
    /* 8. Scope violation attempt */
    {
        int inner_var = 99;
    }
    int scope_error = inner_var * 2;  /* inner_var out of scope */
    
    /* 9. Misuse of __builtin function */
    int builtin_error = __builtin_ctz("string_arg");
    
    /* 10. Invalid return expression */
    return main + 1;  /* Taking address of main function */
}

/* Main function with mixed valid/invalid code */
int main(void) {
    /* Some valid code for context */
    int correct = 0;
    printf("Starting...\n");
    
    /* Error in control flow condition */
    if (5 + "string") {  /* Type mismatch in if condition */
        printf("This shouldn't compile\n");
    }
    
    /* Error in loop */
    while (3.14 | 2.5) {  /* Bitwise on float in while condition */
        correct++;
    }
    
    /* Error in function argument */
    printf("%d", BAD_ADD(5, "text"));  /* Macro expansion error */
    
    /* Error in variable initialization */
    int bad_init = {10, 20};  /* Scalar with multiple values */
    
    /* Misuse of statement expression */
    int stmt_expr_err = BAD_STMT_EXPR;
    
    /* Valid code */
    int final_valid = correct * 2;
    
    /* Invalid return from main */
    return &correct + 1;  /* Pointer arithmetic error */
}

/* Additional global error */
float* global_ptr_err = &3.14;  /* Address of literal float */

/* Token concatenation to create invalid identifier */
#define CONCAT(a, b) a##b
int CONCAT(123, 456) = 789;  /* Creates invalid identifier 123456 */
