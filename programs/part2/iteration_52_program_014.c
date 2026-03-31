/* test_error_mark_node.c */
/* This program contains semantically invalid expressions designed to trigger */
/* the error_mark_node return path in GCC's expr.cc during semantic analysis */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Invalid macro with token concatenation */
#define MAKE_INVALID(var) var##_undeclared_identifier

/* Global scope errors */
int* global_bad = &42;  /* Line 12: Address of constant */

int main(void) {
    /* Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int bad1 = BAD_ADD(5, "string");  /* Line 20: Via macro expansion */
    
    /* 2. Bitwise operator on floating-point types */
    double bad2 = 3.14 | 2.5;  /* Line 23: Direct invalid operation */
    
    /* 3. Address-of operator on literal inside function */
    int* bad3 = &"hello"[0];  /* Actually valid, so let's do something worse: */
    int* bad3b = &(valid_int + 1);  /* Line 26: Address of temporary expression */
    
    /* 4. Invalid initializer - scalar with multiple values */
    int bad4 = {5, 6};  /* Line 29: GCC might catch this early */
    
    /* 5. Undeclared identifier from macro */
    int bad5 = MAKE_INVALID(some);  /* Line 32: Expands to some_undeclared_identifier */
    
    /* 6. Misuse of GNU statement expression */
    int bad6 = ({ 
        int x; 
        /* Missing return value - statement expression yields void */
    });  /* Line 38: Invalid use of statement expression */
    
    /* 7. Invalid pointer arithmetic */
    int x = 10;
    float bad7 = &x / 2;  /* Line 42: Pointer divided by integer */
    
    /* 8. Misuse of __builtin function */
    int bad8 = __builtin_ctz("hello");  /* Line 45: String literal instead of integer */
    
    /* 9. Out of scope identifier */
    {
        int inner_scope = 100;
    }
    int bad9 = inner_scope;  /* Line 51: Variable out of scope */
    
    /* 10. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4};  /* Line 54: Too many initializers */
    
    /* 11. Function address in arithmetic */
    int bad11 = (int)main + 1;  /* Line 57: Actually valid cast, let's try: */
    int bad11b = main + 1;  /* Line 58: Function pointer arithmetic without cast */
    
    /* 12. Invalid compound literal */
    int bad12 = (int){1, 2};  /* Line 61: Compound literal with multiple values */
    
    /* 13. Control flow with invalid condition */
    if (5 + "string") {  /* Line 64: Invalid condition */
        printf("This shouldn't compile\n");
    }
    
    /* 14. Return statement with invalid expression */
    return &valid_int - "string";  /* Line 68: Pointer minus string literal */
}

/* Additional global error */
int* another_bad = &main;  /* Line 72: Taking address of function without proper type */
