/* test_expr_error_mark_node.c
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
int CONCAT(weird, 123) = 42;  /* Valid but unusual */

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address of literal */

int main(void) {
    /* Some valid code for context */
    int valid_int = 10;
    float valid_float = 3.14f;
    char valid_str[] = "hello";
    
    printf("Starting program with intentional errors...\n");
    
    /* 1. Type mismatch in binary operation using macro */
    int bad1 = BAD_ADD(valid_int, "string literal");  /* int + string */
    
    /* 2. Invalid operand combination: bitwise on floats */
    double bad2 = valid_float | 2.5;  /* float | double */
    
    /* 3. Address-of operator on constant */
    int* bad3 = &(valid_int + 5);  /* Address of temporary expression */
    
    /* 4. Undeclared identifier (out of scope reference) */
    {
        int inner_scope = 100;
    }
    int bad4 = inner_scope * 2;  /* inner_scope no longer in scope */
    
    /* 5. Invalid initializer - scalar with multiple values */
    int bad5 = {5, 6, 7};  /* Too many initializers for scalar */
    
    /* 6. Misuse of GNU statement expression */
    int bad6 = ({ 
        int x; 
        /* Missing return value - statement expression yields void */
    });
    
    /* 7. Invalid pointer arithmetic */
    float* bad7 = &valid_float;
    int bad8 = bad7 / 2;  /* Pointer division */
    
    /* 8. Misuse of __builtin with wrong type */
    int bad9 = __builtin_ctz("string");  /* String instead of integer */
    
    /* 9. Array initializer with excess elements */
    int arr[3] = {1, 2, 3, 4, 5};  /* 5 elements for 3-element array */
    
    /* 10. Return statement with invalid expression */
    return &main + "invalid";  /* Function pointer + string */
    
    /* Unreachable valid code */
    printf("This never executes\n");
    return 0;
}

/* Additional global errors */
int bad_global[] = {1, 2, 3, {4, 5}};  /* Nested braces in non-aggregate initializer */

/* Function with parameter type errors */
void bad_func(int x) {
    /* Using function parameter in invalid expression */
    double* ptr = &x + "error";  /* Pointer + string */
    
    /* Control flow with invalid condition */
    if (x | 3.14) {  /* int | double */
        printf("Invalid condition\n");
    }
    
    /* Loop with invalid increment */
    for (int i = 0; i < 10; i += "step") {  /* int += string */
        /* loop body */
    }
}
