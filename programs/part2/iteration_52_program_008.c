/* test.c - Program designed to trigger error_mark_node returns in expr.cc */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Token concatenation to create questionable constructs */
#define CONCAT_OP(x, y) x##y
#define MAKE_BAD_EXPR CONCAT_OP(+, *)

/* Global scope errors */
int* global_bad = &42;  /* Invalid: address of literal */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14;
    printf("Starting...\n");
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int bad1 = BAD_ADD(5, "string");
    
    /* 2. Invalid operand combination (float | double) */
    double bad2 = BAD_BITWISE(3.14, 2.71);
    
    /* 3. Undeclared identifier usage */
    {
        int inner = 10;
    }
    int bad3 = inner * 2;  /* 'inner' out of scope */
    
    /* 4. Invalid initializer - scalar with multiple values */
    int bad4 = {5, 6, 7};
    
    /* 5. Address-of operator on constant expression */
    int* bad5 = &(valid_int + 1);
    
    /* 6. Misuse of GCC statement expression */
    int bad6 = ({ 
        int a; 
        /* Missing return value - just declarations */
        float b; 
    });
    
    /* 7. Invalid array initializer - too many values */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 8. Function address arithmetic */
    int bad7 = (int)(main + 1);
    
    /* 9. Misuse of __builtin with wrong type */
    int bad8 = __builtin_ctz("hello");
    
    /* 10. Invalid compound expression via concatenation */
    /* This might create "+*" which is invalid */
    int bad9 = 5 MAKE_BAD_EXPR 3;
    
    /* 11. Pointer arithmetic with wrong types */
    float* bad10 = &valid_float;
    int bad11 = bad10 / 2;
    
    /* 12. Return statement with invalid expression */
    return &valid_int - "string";
}
