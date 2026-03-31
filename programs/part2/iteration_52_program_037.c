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

/* Global scope errors */
int global_mismatch = 10 + "string"; /* line 15: int + pointer */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    int *valid_ptr = &valid_int;
    
    /* 1. Type mismatch in binary operation (via macro expansion) */
    int x = BAD_ADD(5, "text"); /* line 23: int + string literal */
    
    /* 2. Invalid operand combination: bitwise on float */
    double d = 3.14159 | 2.71828; /* line 26: bitwise OR on doubles */
    
    /* 3. Address-of operator on constant */
    int *p = &42; /* line 29: address of literal */
    
    /* 4. Undeclared identifier (out of scope) */
    {
        int inner = 100;
    }
    int outer = inner; /* line 35: 'inner' not visible here */
    
    /* 5. Invalid initializer - excess elements */
    int arr[3] = {1, 2, 3, 4, 5}; /* line 38: too many initializers */
    
    /* 6. Scalar with multiple brace-enclosed values */
    int z = {5, 6, 7}; /* line 41: multiple values for scalar */
    
    /* 7. Misuse of GNU statement expression */
    int se = ({ int a; }); /* line 44: no value in block */
    
    /* 8. Invalid builtin usage */
    int bits = __builtin_ctz("hello"); /* line 47: string arg to ctz */
    
    /* 9. Function address arithmetic */
    int func_addr = main + 1; /* line 50: function pointer arithmetic */
    
    /* 10. Pointer arithmetic type mismatch */
    float *fp = (float*)valid_ptr;
    int bad_arith = fp / 2; /* line 54: pointer division */
    
    /* 11. Return statement with invalid expression */
    return &valid_int - "string"; /* line 57: pointer - string literal */
}
