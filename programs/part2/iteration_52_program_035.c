/* Test program to trigger error_mark_node return in expr.cc */
/* This program contains syntactically valid but semantically invalid expressions */

#include <stdio.h>

/* Macro to generate type mismatches */
#define ADD_MISMATCH(a, b) ((a) + (b))
#define BITWISE_FLOAT(a, b) ((a) | (b))

/* Invalid macro usage */
#define BAD_INITIALIZER {1, 2, 3, 4, 5}

/* Global scope errors */
int* global_bad = &42;  /* Line 13: Address of constant */

int main(void) {
    /* Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    printf("Starting test...\n");
    
    /* 1. Type mismatch in binary operation */
    int mismatch = ADD_MISMATCH(valid_int, "string");  /* Line 21: int + string */
    
    /* 2. Invalid operand combinations */
    double d = 3.14;
    double bad_bitwise = BITWISE_FLOAT(d, 2.5);  /* Line 24: bitwise on float */
    
    /* 3. Invalid initializers */
    int arr[3] = BAD_INITIALIZER;  /* Line 27: excess initializers */
    int scalar_bad = {5, 6};  /* Line 28: multiple values for scalar */
    
    /* 4. Undeclared identifier (out of scope) */
    {
        int inner = 10;
    }
    int outer = inner;  /* Line 34: 'inner' out of scope */
    
    /* 5. Invalid address-of operations */
    int* p1 = &(valid_int + 1);  /* Line 37: address of temporary */
    int* p2 = &"literal";  /* Line 38: address of string literal */
    
    /* 6. Misuse of GCC extensions */
    /* Statement expression missing return value */
    int bad_stmt_expr = ({ int a = 5; });  /* Line 42: no value */
    
    /* Wrong __builtin argument types */
    int bad_builtin = __builtin_ctz("hello");  /* Line 45: string arg */
    
    /* 7. Function address arithmetic */
    int (*func_ptr)(void) = main;
    func_ptr = func_ptr + 1;  /* Line 49: function pointer arithmetic */
    
    /* 8. Return statement with invalid expression */
    return main + 1;  /* Line 52: function address arithmetic */
}

/* Additional global errors */
float* bad_global_float = &valid_float;  /* Line 56: address of local variable in global */

/* Macro with token concatenation creating invalid operator */
#define CONCAT(a, b) a##b
int weird = 5 CONCAT(+, *) 3;  /* Line 60: creates invalid '+*' operator */
