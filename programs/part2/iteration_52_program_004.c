/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* Forward declare, but we'll misuse it */

/* Global scope error */
int global = &42; /* Invalid: address of constant */

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 42;
    printf("Starting...\n");
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int x = BAD_ADD(5, "string");
    
    /* 3. Invalid operand combination: bitwise OR on floating-point */
    double d = 3.14;
    double result = BAD_BITWISE(d, 2.5);
    
    /* 4. Address-of operator on constant (not an lvalue) */
    int* ptr = &(valid_int + 1); /* Taking address of temporary result */
    
    /* 5. Undeclared identifier (out of scope reference) */
    {
        int inner = 10;
    }
    x = inner; /* 'inner' is out of scope here */
    
    /* 6. Invalid initializer - scalar with multiple values */
    int z = {5, 6};
    
    /* 7. Misuse of GNU C statement expression */
    int stmt_expr = ({
        int a;
        /* Missing return value - last expression should provide value */
        a; /* a is uninitialized */
    });
    
    /* 8. Invalid pointer arithmetic */
    float* fp = (float*)&valid_int;
    float f = *fp / 0; /* Division by zero in floating context */
    
    /* 9. Misuse of __builtin with wrong argument type */
    int bits = __builtin_ctz("hello");
    
    /* 10. Invalid array initializer - excess elements */
    int arr[3] = {1, 2, 3, 4};
    
    /* 11. Taking address of function in arithmetic */
    int func_addr = (int)(&main) + 1;
    
    /* 12. Another valid statement for contrast */
    int ok = valid_int * 2;
    
    /* 13. Invalid in return statement */
    return main + 1; /* Function pointer arithmetic */
}
