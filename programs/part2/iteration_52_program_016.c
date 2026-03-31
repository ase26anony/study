/* test_error_mark_node.c
 * This program is designed to trigger the error_mark_node return path
 * in expr.cc by including various semantically invalid expressions
 * within syntactically valid C code.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Invalid macro usage with token concatenation */
#define CONCAT_INVALID(x, y) x##y
#define MAKE_BAD_EXPR CONCAT_INVALID(+, *)

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address of literal */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    char valid_str[] = "hello";
    
    /* 1. Type mismatch in binary operation (int + string) */
    int bad_sum = 5 + "string";  /* Invalid: adding pointer to integer */
    
    /* 2. Invalid initializer - scalar with multiple values */
    int bad_init = {5, 6, 7};  /* GCC extension warning but parseable */
    
    /* 3. Using macro to generate type mismatch */
    float macro_bad = BAD_ADD(valid_float, "world");
    
    /* 4. Bitwise operator on floating-point types */
    double bad_bitwise = 3.14159 | 2.71828;
    
    /* 5. Address-of operator on constant expression */
    int* bad_address = &(valid_int + 1);  /* Not an lvalue */
    
    /* 6. Undeclared identifier in expression */
    {
        int x = 10;
    }
    int use_undeclared = x * 2;  /* x out of scope */
    
    /* 7. Invalid pointer arithmetic */
    int arr[3] = {1, 2, 3};
    float* bad_ptr_arith = (float*)arr / 2;  /* Pointer division */
    
    /* 8. Misuse of GNU statement expression */
    int bad_stmt_expr = ({ 
        int a; 
        /* Missing return value - last expression should provide value */
    });
    
    /* 9. Invalid builtin function usage */
    int bad_builtin = __builtin_ctz("string");  /* Wrong argument type */
    
    /* 10. Array initializer with excess elements */
    int excess_init[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 11. Invalid in return statement */
    return main + 1;  /* Taking address of function in arithmetic */
}
