/* test_error_mark_node.c
 * This program contains syntactically valid but semantically invalid expressions
 * designed to trigger the error_mark_node return path in expr.cc during GCC compilation.
 * Compile with: gcc -fsyntax-only -O0 test_error_mark_node.c
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* Valid declaration, but we'll misuse it */

/* Global scope errors */
int global_mismatch = 10 + "string"; /* Type mismatch in global initializer */

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 42;
    printf("Starting...\n");
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int x = BAD_ADD(5, "text");
    
    /* 3. Invalid operand combination: bitwise OR on floating-point */
    double d = 3.14;
    double result = BAD_BITWISE(d, 2.5);
    
    /* 4. Address-of operator on constant */
    int* ptr = &42;
    
    /* 5. Undeclared identifier (out of scope reference) */
    {
        int inner = 100;
    }
    int outer = inner; /* 'inner' not visible here */
    
    /* 6. Invalid initializer: scalar with multiple values */
    int z = {5, 6, 7};
    
    /* 7. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 8. Misuse of GNU statement expression */
    int stmt_expr = ({ int a; }); /* Missing value - returns void */
    
    /* 9. Invalid __builtin_ function usage */
    int trailing_zeros = __builtin_ctz("hello");
    
    /* 10. Taking address of function in return statement */
    return (int)(main + 1);
    
    /* 11. More valid code to ensure parser continues */
    printf("This won't execute\n");
    return 0;
}

/* Additional errors in another function */
void other_func(void) {
    /* 12. Type mismatch in conditional expression */
    int cond = (10 > 5) ? "yes" : 0;
    
    /* 13. Invalid pointer arithmetic */
    float f = 3.0;
    float* pf = &f;
    int* ip = (int*)pf;
    ip = ip / 2; /* Pointer division */
    
    /* 14. Misuse of compound literal */
    int* bad_ptr = &(int){1, 2, 3};
}
