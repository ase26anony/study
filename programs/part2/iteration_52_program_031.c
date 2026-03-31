/* test.c - Program designed to trigger error_mark_node returns in expr.cc */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Token concatenation to create questionable constructs */
#define CONCAT(a, b) a##b
#define MAKE_BAD_EXPR CONCAT(+, *)

/* Global scope errors */
int global_mismatch = 10 + "string";  /* Type mismatch in global initializer */

/* Function with multiple error patterns */
void test_errors(void) {
    /* Valid code for context */
    int valid = 42;
    float valid_float = 3.14;
    
    /* 1. Type mismatch in binary operation (int + string literal) */
    int bad1 = BAD_ADD(5, "text");
    
    /* 2. Invalid operand combination (float | double) */
    double d = 3.14;
    float bad2 = BAD_BITWISE(valid_float, d);
    
    /* 3. Address-of on constant */
    int* bad3 = &42;
    
    /* 4. Invalid initializer - scalar with multiple values */
    int bad4 = {5, 6, 7};
    
    /* 5. Undeclared identifier (out of scope reference) */
    {
        int inner = 10;
    }
    int bad5 = inner * 2;  /* 'inner' not in scope */
    
    /* 6. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4, 5};
    
    /* 7. Misuse of GNU statement expression */
    int bad6 = ({ int x; });  /* Missing value */
    
    /* 8. Invalid builtin usage */
    int bad7 = __builtin_ctz("hello");
    
    /* 9. Function address arithmetic */
    void (*bad8)(void) = &test_errors + 1;
    
    /* 10. Macro-generated token concatenation issue */
    int MAKE_BAD_EXPR = 5;  /* Becomes '+*' which is invalid */
}

/* Another function with different error contexts */
int problematic_return(void) {
    /* Return with type mismatch */
    return main + 1;  /* Taking address of main function */
}

/* Control flow with bad expressions */
void control_flow_errors(void) {
    /* Valid statement for context */
    int i = 0;
    
    /* if condition with invalid expression */
    if (5 + "string") {
        printf("This shouldn't compile\n");
    }
    
    /* while loop with bad condition */
    while (&i / 2) {  /* Pointer arithmetic in condition */
        i++;
    }
    
    /* for loop with invalid init */
    for (int j = {1, 2}; j < 10; j++) {  /* Scalar with multiple values */
        printf("%d\n", j);
    }
}

int main(void) {
    /* Some valid code */
    printf("Starting error test...\n");
    int ok = 100;
    
    /* Call error functions */
    test_errors();
    control_flow_errors();
    
    /* Function call with bad argument */
    printf("%d\n", 5 + "text");
    
    /* Invalid compound assignment */
    ok += "string";
    
    /* Another valid statement */
    int final_ok = 999;
    
    return problematic_return();
}
