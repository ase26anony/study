/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Strategy 1: Invalid operations on void */
void void_func(void) {}

/* Strategy 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Strategy 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* Strategy 4: Target-specific expansion failures */
typedef float v4sf __attribute__((vector_size(16)));

/* Main container for erroneous constructs */
int main(void) {
    /* 1. Multiple invalid void operations in complex contexts */
    
    /* a) Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* b) Void in comma operator in sizeof context */
    int y = sizeof((void_func(), 5));
    
    /* c) Nested void in conditional operator */
    int z = (1 ? (void)0 : 2);
    
    /* d) Void as function argument to builtin */
    int w = __builtin_constant_p((void)0);
    
    /* 2. Misuse of __builtin_va_arg - outside variadic context */
    /* Create a fake va_list - uninitialized and invalid */
    va_list ap;
    /* This should fail during expansion as 'ap' is not properly initialized
       and we're not in a variadic function */
    float f = __builtin_va_arg(ap, float);
    
    /* 3. Malformed compound literals with invalid designators */
    
    /* a) Non-existent field in designated initializer */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* b) Type mismatch in compound literal */
    struct BadStruct *bs = &(struct BadStruct){ .valid_field = 1.5 }; /* float to int */
    
    /* c) Taking address of non-lvalue compound literal in complex expression */
    int **pp = &(&(int){42});
    
    /* 4. Target-specific failures */
    
    /* a) Vector operations - may fail if target lacks vector support */
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c = a + b; /* Vector addition */
    
    /* b) Overflow builtins with potentially unsupported types */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow;
    /* __builtin_add_overflow may not support long double on all targets */
    __builtin_add_overflow(ld1, ld2, &overflow);
    
    /* c) Complex type in overflow builtin */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    __builtin_add_overflow(cd1, cd2, &overflow);
    
    /* 5. Invalid address operations on bit-fields */
    struct BitFieldStruct {
        unsigned int field:4;
    } bfs = {0};
    
    /* Taking address of bit-field - invalid */
    unsigned int *bfp = &bfs.field;
    
    /* 6. Misaligned pointer operations */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1); /* Misaligned for int */
    
    /* Force assumption of alignment on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    *aligned = 42; /* Potential misaligned access */
    
    /* 7. Transactional Memory constructs (if compiled with -fgnu-tm) */
    /* This creates a transaction block */
    __transaction_atomic {
        x = x + 1;
    }
    
    /* 8. Complex nested invalid expression */
    /* This combines multiple problematic constructs */
    int result = (1 ? sizeof((void)0) : __builtin_va_arg(ap, int)) + 
                 (int)&(struct BadStruct){ .valid_field = (void)0 };
    
    /* The program doesn't need to execute successfully */
    return 0;
}
