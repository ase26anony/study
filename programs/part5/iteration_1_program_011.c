/* test_expr_error.c
 * Designed to trigger error_mark_node return in expr.cc during RTL expansion
 */

/* Force certain behaviors by disabling optimizations that might catch errors early */
#pragma GCC optimize ("no-tree-ccp")
#pragma GCC optimize ("no-tree-forwprop")

/* ====== 1. Invalid Operations on void ====== */
void void_func(void) {}

/* ====== 2. Misusing __builtin_va_arg ====== */
#include <stdarg.h>

/* ====== 3. Complex nested errors ====== */
struct BadStruct {
    int x;
    int y;
};

/* ====== 4. Target-specific features ====== */
/* Vector type - may fail expansion on targets without vector support */
typedef int v4si __attribute__((vector_size(16)));

/* Transaction Memory - may fail if not supported */
#ifdef __GNUC__
void tm_func(void) {
    __transaction_atomic {
        /* empty transaction */
    }
}
#endif

/* ====== Main function containing all problematic constructs ====== */
int main(void) {
    /* Group 1: Direct void expression errors */
    /* These should be caught early, but with -fpermissive might reach expansion */
    int a = (void)void_func();           /* Direct void cast in value context */
    int b = (printf("test"), 5);         /* Comma operator with void left side */
    
    /* More complex void usage */
    int c = sizeof((void)0);             /* void expression inside sizeof */
    int d = __builtin_constant_p((void)0); /* void in builtin constant check */
    
    /* Group 2: __builtin_va_arg misuse */
    /* Create a va_list but use it incorrectly */
    va_list ap;
    /* Using va_arg without proper initialization and with wrong type */
    int e = __builtin_va_arg(ap, float);  /* Wrong type - float vs int promotion */
    
    /* Even worse: use with completely wrong argument */
    int f = __builtin_va_arg((void*)0, double);  /* Invalid va_list argument */
    
    /* Group 3: Malformed compound literals and address operations */
    /* Invalid compound literal with non-existent field */
    int *p = &(struct BadStruct){ .non_existent = 1 };
    
    /* Taking address of bit-field in complex expression */
    struct {
        unsigned int bitfield : 4;
    } bf = {0};
    
    /* Complex pointer arithmetic with questionable operands */
    int *q = (int*)((char*)&bf.bitfield + (void)0);  /* Mixing void in pointer math */
    
    /* Group 4: Target-specific expansion failures */
    /* Vector operations - may fail on targets without vector support */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtins with potentially unsupported types */
    int overflow;
    /* Using with long double - might be unsupported */
    int g = __builtin_add_overflow_p(1.0L, 2.0L, (long double)0);
    
    /* Group 5: Nested conditional with errors */
    /* Error deep inside conditional expression */
    int h = (1 ? (void)0 : 0);  /* Type mismatch in conditional branches */
    
    /* Invalid operation inside nested expression */
    int i = 1 + (int)(void)0;  /* Casting void to int */
    
    /* Group 6: Alignment issues */
    /* Force alignment requirement on misaligned pointer */
    int j = 5;
    int *misaligned = (int*)((char*)&j + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* Wrong alignment */
    
    /* Transaction Memory - compile with -fgnu-tm */
    #ifdef __GNUC__
    tm_func();
    #endif
    
    /* Prevent "unused variable" warnings from catching errors early */
    (void)a; (void)b; (void)c; (void)d; (void)e;
    (void)f; (void)p; (void)q; (void)v3; (void)g;
    (void)h; (void)i; (void)aligned;
    
    return 0;
}
