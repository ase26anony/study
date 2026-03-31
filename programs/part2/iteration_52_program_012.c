/* test_error_mark_node.c
 * This program contains syntactically valid but semantically invalid expressions
 * designed to trigger the error_mark_node return path in expr.cc during compilation.
 * Compile with: gcc -fsyntax-only -O0 test_error_mark_node.c
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Invalid token concatenation attempt */
#define CONCAT_INVALID(x, y) x##y##z

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address of constant */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14;
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* Invalid: integer + pointer */
    
    /* 2. Pointer arithmetic with incorrect types */
    float* fp = &valid_float;
    float y = fp / 2;  /* Invalid: pointer division */
    
    /* 3. Bitwise operator on floating-point types */
    double d = 3.14 | 2.5;  /* Invalid: bitwise OR on doubles */
    
    /* 4. Address-of operator on constant literal */
    int* p = &42;  /* Invalid: address of rvalue */
    
    /* 5. Undeclared identifier usage */
    {
        int inner = 10;
    }
    int outer = inner;  /* Invalid: 'inner' out of scope */
    
    /* 6. Invalid initializer - excess elements */
    int arr[3] = {1, 2, 3, 4};  /* Invalid: too many initializers */
    
    /* 7. Scalar with multiple initializer values */
    int z = {5, 6};  /* Invalid: multiple values for scalar */
    
    /* 8. Macro-generated type mismatch */
    int macro_err = BAD_ADD(5, "text");
    
    /* 9. Invalid builtin usage */
    int builtin_err = __builtin_ctz("hello");  /* Invalid: string argument */
    
    /* 10. Statement expression misuse */
    int stmt_expr = ({ int a; a; });  /* Invalid: missing return value */
    
    /* 11. Function address arithmetic */
    return main + 1;  /* Invalid: function pointer arithmetic */
    
    /* 12. Control flow with invalid condition */
    if (3.14 & 2.71) {  /* Invalid: bitwise AND on floats */
        printf("Never reached\n");
    }
    
    /* Valid statement to provide more context */
    printf("Valid: %d\n", valid_int);
    
    return 0;
}

/* Additional errors in function scope */
void other_func(void) {
    /* 13. Invalid compound literal */
    int* bad_ptr = &(int){1, 2};  /* Invalid: multiple values in compound literal */
    
    /* 14. Misaligned pointer cast */
    char* cp = (char*)&valid_float;
    int* ip = (int*)cp;  /* Potentially misaligned, may trigger errors */
    
    /* 15. Invalid array subscript */
    "string"[0] = 'x';  /* Invalid: modifying string literal */
}
