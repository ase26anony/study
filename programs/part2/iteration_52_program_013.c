/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* 1. Invalid initializer - scalar with multiple values */
int global_bad = {10, 20};  /* Should trigger error in global context */

/* 2. Macro generating type mismatches */
#define BAD_ADD(a, b) (a + b)
#define CONCAT_INVALID(x, y) x##y##z  /* Invalid token concatenation */

/* 3. GNU extension misuse */
#define STMT_EXPR_BAD ({ int a; /* missing return value */ })

/* Function prototype */
void some_function(int x);

int main(void) {
    /* Some valid code for context */
    int valid_var = 42;
    int *valid_ptr = &valid_var;
    
    /* 4. Type mismatch in binary operation */
    int mismatch1 = 5 + "string";  /* int + string literal */
    
    /* 5. Invalid operand combination - bitwise on float */
    double d = 3.14;
    double bad_bitwise = d | 2.5;  /* bitwise OR on floating point */
    
    /* 6. Macro expansion with type mismatch */
    int from_macro = BAD_ADD(10, "text");
    
    /* 7. Address-of operator on constant */
    int *bad_addr = &42;  /* Taking address of literal */
    
    /* 8. Invalid initializer in local context */
    int bad_init = {1, 2, 3};  /* Scalar with multiple initializers */
    
    /* 9. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 10. Statement expression misuse (GNU extension) */
    int bad_stmt_expr = STMT_EXPR_BAD;
    
    /* 11. Builtin function with wrong argument type */
    int bad_builtin = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 12. Control flow with invalid condition */
    if (main + 1) {  /* Function pointer arithmetic */
        /* 13. Invalid expression in loop */
        for (int i = 0; i < &valid_var; i++) {  /* Pointer as loop bound */
            /* 14. Invalid return expression */
            return &i + "text";  /* Mixed pointer types */
        }
    }
    
    /* 15. Token concatenation creating invalid identifier */
    /* This might create 'valid_varzz' which doesn't exist */
    int CONCAT_INVALID(valid_var, z) = 5;
    
    /* 16. Function call with invalid argument */
    some_function(3.14 | 2.71);  /* Bitwise OR on floats as argument */
    
    /* 17. Comma operator with type mismatch */
    int bad_comma = (valid_var, "string");
    
    /* 18. Conditional expression with incompatible types */
    int bad_cond = valid_var ? 5 : "text";
    
    /* 19. Invalid compound assignment */
    d += "text";  /* double += string */
    
    /* 20. Taking address of register variable (if supported) */
    register int reg_var = 10;
    int *bad_reg_addr = &reg_var;
    
    /* Valid statement to show we got past some parsing */
    int final_valid = 100;
    
    return 0;
}

/* Helper function definition */
void some_function(int x) {
    /* 21. Invalid expression in nested scope */
    {
        int inner = 5;
    }
    /* inner is out of scope here, but let's try to use it */
    int use_out_of_scope = inner + 1;  /* Undeclared identifier */
    
    /* 22. Invalid pointer arithmetic */
    float f = 3.14;
    float *fptr = &f;
    float bad_ptr_arith = *(fptr + "offset");  /* Pointer + string */
}

/* 23. Global with invalid expression */
int *global_bad_ptr = &"constant";  /* Address of string literal */

/* 24. Invalid designator in initializer (C99+) */
struct Point { int x; int y; };
struct Point bad_point = { .x = "text", .y = 3.14 };  /* Type mismatches */
