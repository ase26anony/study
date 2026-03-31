/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * It mixes valid and invalid code to ensure the parser engages before failing.
 */

/* 1. Invalid initializer - scalar with multiple values */
int global_bad = {10, 20};  /* Should trigger error in global context */

/* 2. Macro generating type mismatches */
#define BAD_ADD(a, b) (a + b)
#define CONCAT_INVALID(x, y) x##y##z  /* Creates invalid identifier */

/* 3. Misuse of GNU statement expression */
#define BAD_STMT_EXPR(x) ({ int y; })  /* Missing value - returns void */

/* Function prototype */
void some_function(int param);

int main(void) {
    /* Some valid code for context */
    int valid_var = 42;
    int *valid_ptr = &valid_var;
    
    /* 4. Type mismatch in binary operation */
    int type_mismatch = 5 + "string";  /* int + string literal */
    
    /* 5. Invalid operand combination - bitwise on float */
    double fp_var = 3.14159;
    double bad_bitwise = fp_var | 2.5;  /* Bitwise OR on doubles */
    
    /* 6. Address-of operator on constant */
    int *bad_addr = &42;  /* Taking address of literal */
    
    /* 7. Using macro with type mismatch */
    int macro_bad = BAD_ADD(10, "text");
    
    /* 8. Invalid initializer in local scope */
    int bad_init = {1, 2, 3};  /* Scalar with multiple values */
    
    /* 9. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 10. Misuse of GNU extension - statement expression */
    int stmt_expr_bad = BAD_STMT_EXPR(5);
    
    /* 11. Invalid pointer arithmetic */
    float *bad_ptr_arith = (float *)&valid_var / 2;  /* Pointer division */
    
    /* 12. Undeclared identifier (out of scope) */
    {
        int inner_scope = 100;
    }
    int use_out_of_scope = inner_scope;  /* inner_scope no longer in scope */
    
    /* 13. Misuse of __builtin function */
    int builtin_bad = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 14. Invalid in function argument */
    some_function(5 | 3.14);  /* Mixed types in bitwise operation */
    
    /* 15. Invalid in return statement */
    return main + 1;  /* Taking address of function in arithmetic */
}

void some_function(int param) {
    /* 16. Invalid in control flow condition */
    if (param & 1.5) {  /* Bitwise AND with float */
        /* 17. Invalid in loop condition */
        while (param + "text") {  /* Type mismatch */
            param++;
        }
    }
    
    /* 18. Token concatenation creating invalid identifier */
    int CONCAT_INVALID(foo, bar);  /* Expands to foobarz, which is undeclared */
    
    /* 19. Compound literal misuse */
    int *bad_compound = &(int){1, 2};  /* Compound literal with multiple values */
    
    /* 20. Invalid in switch expression */
    switch ((int)"string") {  /* Cast doesn't fix semantic error */
        case 1: break;
    }
}

/* 21. Global with invalid expression */
int *global_bad_expr = (int *)&"constant";  /* Taking address of string literal */

/* 22. Invalid static initializer */
static int static_bad = 3.14 << 2;  /* Shift on floating-point value */
