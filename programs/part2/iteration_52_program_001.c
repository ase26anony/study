/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Invalid macro expansion with token concatenation */
#define CONCAT_INVALID(x, y) x##y##z
#define MAKE_BAD_EXPR(x) (x + &x)

/* Global scope errors */
int global_arr[3] = {1, 2, 3, 4};  /* Excess initializers - line 17 */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = 5 + "string";  /* int + string literal */
    
    /* 2. Invalid pointer arithmetic */
    int x = 10;
    float ptr_arith = &x / 2;  /* pointer divided by int */
    
    /* Valid statement to maintain structure */
    printf("Valid printf: %d\n", valid_int);
    
    /* 3. Bitwise operator on floating-point types */
    double bitwise_float = 3.14 | 2.5;  /* bitwise OR on doubles */
    
    /* 4. Address-of operator on constant */
    int* addr_const = &42;  /* taking address of literal */
    
    /* 5. Scalar with multiple initializers */
    int bad_init = {5, 6};  /* scalar with brace-enclosed list */
    
    /* 6. Macro-generated type mismatch */
    int macro_bad = BAD_ADD(valid_int, "macro_string");
    
    /* 7. Invalid use of GNU statement expression */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - just a declaration */
        a;  /* This should be a value, but we leave it as is */
    });
    
    /* 8. Misuse of builtin function */
    int builtin_bad = __builtin_ctz("hello");  /* string instead of int */
    
    /* 9. Control flow with invalid condition */
    if (5 + "string") {  /* invalid expression in condition */
        printf("This shouldn't compile\n");
    }
    
    /* 10. Return statement with invalid expression */
    return main + 1;  /* taking address of main function */
    
    /* Unreachable, but shows more contexts */
    while (&valid_int) {  /* pointer as boolean - might be valid, but combined with... */
        int inner = valid_int + "inner";  /* ...invalid expression inside */
        break;
    }
    
    /* Attempt to use concatenated invalid identifier */
    /* int CONCAT_INVALID(foo, bar) = 5; */ /* Would create 'foobarz' identifier */
    
    /* Array with wrong initializer count in local scope */
    int local_arr[2] = {1, 2, 3, 4};  /* excess initializers */
    
    return 0;
}

/* Additional function with errors in different context */
void bad_function(void) {
    /* Use variable before declaration (non-C23) */
    printf("%d\n", undeclared_var);  /* undeclared identifier */
    int undeclared_var = 5;
    
    /* Invalid compound literal */
    int* bad_ptr = &(int){1, 2};  /* scalar compound literal with multiple values */
}

/* Global with invalid initializer */
double global_bad = 3.14 | 2.5;  /* bitwise on doubles at global scope */
