/* test_error_mark_node.c
 * This program contains syntactically valid but semantically invalid expressions
 * designed to trigger the error_mark_node return path in expr.cc during GCC compilation.
 * Compile with: gcc -fsyntax-only -O0 test_error_mark_node.c
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Token concatenation creating problematic code */
#define CONCAT(a, b) a##b
#define MAKE_BAD_EXPR CONCAT(undeclared, identifier)

/* Global scope errors */
int* global_bad = &42;  /* Invalid: address of literal */

int main(void) {
    /* Valid code for context */
    int valid_int = 10;
    float valid_float = 3.14;
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int bad1 = BAD_ADD(5, "string");
    
    /* 2. Pointer arithmetic with incorrect types */
    int x = 5;
    float bad2 = &x / 2;
    
    /* 3. Bitwise operator on floating-point types */
    double bad3 = BAD_BITWISE(3.14, 2.5);
    
    /* 4. Address-of operator on constant */
    int* bad4 = &(valid_int + 1);  /* Not an lvalue */
    
    /* 5. Undeclared identifier via macro expansion */
    int bad5 = MAKE_BAD_EXPR;
    
    /* 6. Invalid initializer - scalar with multiple values */
    int bad6 = {5, 6};
    
    /* 7. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4};
    
    /* 8. Misuse of GNU statement expression */
    int bad8 = ({ 
        int a; 
        /* Missing return value - last expression should provide value */
        if (valid_int > 0) valid_int; 
    });
    
    /* 9. Invalid __builtin usage */
    int bad9 = __builtin_ctz("hello");
    
    /* 10. Out-of-scope identifier usage */
    {
        int inner_scope = 42;
    }
    int bad10 = inner_scope;  /* inner_scope no longer in scope */
    
    /* 11. Function address arithmetic */
    void (*bad11)(void) = main + 1;
    
    /* 12. Invalid operand in conditional expression */
    int bad12 = valid_int ? "string" : 3.14;
    
    /* 13. Valid statement for contrast */
    int ok = valid_int + 5;
    
    /* 14. Control flow with invalid condition */
    if (5 + "text") {
        printf("This won't compile\n");
    }
    
    /* 15. Return with invalid expression */
    return &valid_int - "string";
}
