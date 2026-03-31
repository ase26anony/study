/* test_error_mark_node.c
 * This program is designed to trigger the error_mark_node return path
 * in expr.cc by including various syntactically valid but semantically
 * invalid expressions in different contexts.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Invalid token concatenation attempt */
#define CONCAT_INVALID(x, y) x##y##z

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address of constant */

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 10;
    float valid_float = 3.14f;
    printf("Starting...\n");
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* Invalid: integer + pointer */
    
    /* 3. Invalid operand combination: bitwise OR on floats */
    double d = 3.14 | 2.5;  /* Invalid: bitwise on floating-point */
    
    /* 4. Address-of operator on constant (inside function) */
    int* p = &(valid_int + 5);  /* Invalid: address of temporary expression */
    
    /* 5. Using macro to generate type mismatch */
    float y = BAD_ADD(valid_float, "text");  /* float + string pointer */
    
    /* 6. Invalid initializer: scalar with multiple values */
    int z = {5, 6};  /* Invalid: multiple values for scalar */
    
    /* 7. Undeclared identifier (out of scope reference) */
    {
        int inner = 42;
    }
    int outer = inner;  /* Invalid: 'inner' out of scope */
    
    /* 8. Misuse of GNU C statement expression */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - invalid in some contexts */
    });
    
    /* 9. Invalid builtin function usage */
    int bits = __builtin_ctz("hello");  /* Invalid: string argument to ctz */
    
    /* 10. Array initializer with excess elements */
    int arr[3] = {1, 2, 3, 4};  /* Invalid: too many initializers */
    
    /* 11. Control flow with invalid condition */
    if (main + 1) {  /* Invalid: function pointer arithmetic */
        printf("This shouldn't compile\n");
    }
    
    /* 12. Return statement with invalid expression */
    return &valid_int / 2;  /* Invalid: pointer division */
}
