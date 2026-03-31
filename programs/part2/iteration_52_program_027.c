/* test_error_mark_node.c
 * This program contains various semantically invalid expressions designed
 * to trigger the error_mark_node return path in expr.cc during compilation.
 * The program will fail to compile, which is the intended behavior.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Token concatenation to create questionable constructs */
#define CONCAT(a, b) a ## b
#define MAKE_BAD_EXPR CONCAT(+, *)

/* Global scope errors */
int* global_bad = &42;  /* Invalid: address of literal */

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 10;
    float valid_float = 3.14f;
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int bad1 = BAD_ADD(valid_int, "string");
    
    /* 3. Invalid operand combination: bitwise OR on floating-point */
    double bad2 = BAD_BITWISE(3.14, 2.5);
    
    /* 4. Address-of operator on constant (not an lvalue) */
    int* bad3 = &(valid_int + 5);
    
    /* 5. Invalid initializer: scalar with multiple values in braces */
    int bad4 = {5, 6, 7};
    
    /* 6. Undeclared identifier (out of scope reference) */
    {
        int inner = 42;
    }
    int bad5 = inner;  /* 'inner' not visible here */
    
    /* 7. Misuse of GNU statement expression - missing return value */
    int bad6 = ({ int x; });
    
    /* 8. Invalid array initializer - excess elements */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 9. Misuse of __builtin with wrong argument type */
    int bad7 = __builtin_ctz("hello");
    
    /* 10. Invalid in return statement: address of function + arithmetic */
    return (int)(main + 1);
    
    /* 11. Token concatenation creating invalid operator sequence */
    /* int bad8 = 5 MAKE_BAD_EXPR 3; */ /* Would be: 5 + * 3 */
    
    /* 12. Valid statement to ensure parser engagement */
    printf("Valid printf: %d %f\n", valid_int, valid_float);
    
    return 0;
}

/* Additional global error */
int bad_global_array[] = { [10] = 1, [5] = 2, [10] = 3 }; /* Duplicate designator */
