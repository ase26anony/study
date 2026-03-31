/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * It mixes valid and invalid code to ensure the parser engages before hitting errors.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* This is valid declaration, but we'll misuse it */

/* Global scope errors */
int global_mismatch = 10 + "string"; /* line 15: int + pointer */

int main(void) {
    /* Some valid code to establish context */
    int valid_int = 42;
    float valid_float = 3.14f;
    printf("Starting...\n");
    
    /* 1. Type mismatch in binary operation (via macro expansion) */
    int x = BAD_ADD(5, "text"); /* line 23: int + string literal */
    
    /* 2. Invalid operand combination: bitwise on float */
    double d = 3.14159;
    double result = BAD_BITWISE(d, 2.5); /* line 26: double | double */
    
    /* 3. Address-of operator on constant */
    int* p = &42; /* line 29: address of literal */
    
    /* 4. Undeclared identifier (out of scope) */
    {
        int inner = 100;
    }
    x = inner; /* line 35: use after scope */
    
    /* 5. Invalid initializer - excess elements */
    int arr[3] = {1, 2, 3, 4, 5}; /* line 38: too many initializers */
    
    /* 6. Scalar with multiple initializer values */
    int z = {5, 6, 7}; /* line 41: multiple values for scalar */
    
    /* 7. Misuse of GNU statement expression */
    int y = ({ 
        int a; 
        /* Missing return value - last expression should provide value */
        /* This may produce different errors but stresses expression parsing */
    });
    
    /* 8. Invalid builtin usage */
    int bits = __builtin_ctz("hello"); /* line 52: string argument to ctz */
    
    /* 9. Function address arithmetic */
    int (*func_ptr)(void) = &main;
    func_ptr = func_ptr + 1; /* line 56: pointer arithmetic on function pointer */
    
    /* 10. Return statement with invalid expression */
    return main + 1; /* line 59: function address arithmetic in return */
}

/* Additional global error */
float* bad_global = &valid_int; /* line 63: int* to float* conversion without cast */
