/* test.c - Program designed to trigger error_mark_node returns in expr.cc */

#include <stdio.h>

/* Macro that generates type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
/* Macro that creates invalid operations via token concatenation */
#define CONCAT_OP(x, y) x ## + ## y
/* Statement expression misuse */
#define BAD_STMT_EXPR(val) ({ int temp; temp = val; })

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address of constant */

/* Function prototype */
void problematic_function(void);

int main(void) {
    /* Valid code structure */
    int valid_var = 10;
    printf("Starting...\n");
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = 5 + "string";  /* int + pointer */
    
    /* 2. Invalid operand combinations */
    double fp_bitwise = 3.14 | 2.5;    /* bitwise OR on doubles */
    
    /* 3. Undeclared identifier usage (forward reference without C23) */
    {
        int y = undeclared_var + 5;    /* undeclared variable */
        int x = 2;
    }
    /* Try to use x outside its scope */
    int outer_use = x * 2;             /* x out of scope */
    
    /* 4. Invalid initializers */
    int bad_init = {5, 6};             /* scalar with multiple values */
    int array_overflow[3] = {1, 2, 3, 4};  /* excess initializers */
    
    /* 5. Macro-generated errors */
    int macro_error = BAD_ADD(valid_var, "text");
    
    /* 6. Invalid address-of operations */
    int* addr_literal = &"literal";    /* address of string literal */
    int* addr_expr = &(valid_var + 1); /* address of temporary */
    
    /* 7. Misuse of GCC builtins */
    int builtin_misuse = __builtin_ctz("hello"); /* string instead of int */
    
    /* 8. Statement expression misuse */
    int stmt_expr = BAD_STMT_EXPR(5);  /* missing return value */
    
    /* 9. Invalid pointer arithmetic */
    float* float_ptr = (float*)&valid_var;
    float ptr_arith = float_ptr / 2;   /* pointer division */
    
    /* 10. Function address misuse */
    int func_addr = main + 1;          /* arithmetic on function pointer */
    
    /* Valid statement to provide context */
    int ok = 5;
    
    /* Control flow with invalid conditions */
    if (3.14 << 2) {                   /* shift on floating point */
        printf("Impossible\n");
    }
    
    /* Return with invalid expression */
    return &valid_var;                 /* returning pointer instead of int */
}

/* Another function with different error contexts */
void problematic_function(void) {
    /* Invalid compound assignment */
    double d = 2.5;
    d %= 1.2;                          /* modulo on floating point */
    
    /* Invalid conditional expression */
    int cond = (d > 0) ? "yes" : 1;    /* type mismatch in branches */
    
    /* Invalid sizeof operand */
    int size = sizeof(main);           /* sizeof function */
    
    /* Misuse of GNU extension - computed goto with invalid expression */
    void* ptr = &&label;
    goto *("invalid");                 /* goto through non-pointer */
    
label:
    return;
}
