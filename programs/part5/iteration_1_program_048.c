/* test_expr_error.c - Trigger error_mark_node in expr.cc during RTL expansion */

/* Force certain behaviors that might bypass early checks */
#pragma GCC optimize("O3")
#pragma GCC push_options
#pragma GCC optimize("no-tree-ccp")
#pragma GCC optimize("no-tree-forwprop")

#include <stdarg.h>
#include <stdio.h>

/* Helper functions to create complex contexts */
void void_func(void) {
    printf("This returns void\n");
}

int valid_func(int x) {
    return x * 2;
}

/* Try to trigger error_mark_node through various paths */

/* 1. Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct void in value context - should fail during expansion */
    int x = (void)void_func();  /* Line 1: void cast in assignment */
    
    /* Void in comma operator in value context */
    int y = (void_func(), 5);   /* Line 2: void in comma operator */
    
    /* Void in conditional expression */
    int z = 1 ? (void)0 : 0;    /* Line 3: void in conditional */
}

/* 2. Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_errors(void) {
    va_list ap;
    
    /* Using va_arg without proper initialization */
    int x = __builtin_va_arg(ap, int);  /* Line 4: uninitialized va_list */
    
    /* Type mismatch that might fail during expansion */
    double d = __builtin_va_arg(ap, double);  /* Line 5: type without proper args */
    
    /* Complex type that might not be supported */
    typedef struct { int a; float b; } complex_t;
    complex_t c = __builtin_va_arg(ap, complex_t);  /* Line 6: complex struct type */
}

/* 3. Malformed compound literals */
void test_compound_literal_errors(void) {
    /* Invalid designator */
    int *p1 = &(int){ .non_existent = 1 };  /* Line 7: invalid field designator */
    
    /* Type mismatch in compound literal */
    float *p2 = &(int){ 1.5 };  /* Line 8: type mismatch in initializer */
    
    /* Taking address in invalid context */
    int (*p3)[] = &(int[]){1, 2, 3};  /* Line 9: array pointer issues */
}

/* 4. Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector extensions on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Line 10: vector operation */
    
    /* Overflow builtins with potentially unsupported types */
    long double ld1 = 1.5e100L;
    long double ld2 = 2.5e100L;
    int overflow;
    __builtin_add_overflow(ld1, ld2, &overflow);  /* Line 11: long double overflow */
    
    /* Complex number operations that might fail */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3 = cd1 * cd2;  /* Line 12: complex multiplication */
}

/* 5. Transaction Memory constructs if supported */
#ifdef __GNUC__
void test_tm_errors(void) {
    int x = 0;
    
    /* Transactional memory block */
    __transaction_atomic {  /* Line 13: TM construct */
        x = 42;
    }
    
    /* Nested transaction */
    __transaction_atomic {  /* Line 14: Another TM block */
        __transaction_atomic {
            x = 100;
        }
    }
}
#endif

/* 6. Complex nested invalid expressions */
void test_nested_errors(void) {
    /* Void expression inside sizeof */
    size_t s1 = sizeof((void)0);  /* Line 15: void in sizeof */
    
    /* Void expression as builtin argument */
    int is_const = __builtin_constant_p((void)0);  /* Line 16: void in builtin */
    
    /* Invalid address operations */
    struct S {
        int a : 3;  /* bit-field */
        int b;
    } s = {1, 2};
    
    int *ptr = &s.a;  /* Line 17: address of bit-field */
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);  /* Line 18: misaligned */
}

/* 7. Recursive template-like errors using macros */
#define CREATE_ERROR(type, expr) type var_##type = expr

void test_macro_errors(void) {
    /* Generate multiple error instances */
    CREATE_ERROR(int, (void)0);  /* Line 19: macro-expanded void error */
    CREATE_ERROR(float, __builtin_va_arg(0, float));  /* Line 20: macro va_arg error */
}

/* 8. Invalid switch case values */
void test_switch_errors(void) {
    void *ptr = &&label;
    
    switch ((intptr_t)ptr) {
        case (intptr_t)&&label:  /* Line 21: computed goto-like case */
            break;
        default:
            break;
    }
    
label:
    return;
}

/* 9. Invalid assembly operands */
void test_asm_errors(void) {
    int x;
    
    /* Invalid asm constraint */
    __asm__("mov %0, %1" : "=r"(x) : "r"((void)0));  /* Line 22: void in asm */
    
    /* Mismatched operand types */
    __asm__("" : "=m"(*(void(*)())0));  /* Line 23: invalid asm operand */
}

/* Main function that exercises all error paths */
int main(int argc, char **argv) {
    /* Try to compile all error tests */
    test_void_errors();
    test_va_arg_errors();
    test_compound_literal_errors();
    test_target_specific_errors();
    
#ifdef __GNUC__
    test_tm_errors();
#endif
    
    test_nested_errors();
    test_macro_errors();
    test_switch_errors();
    test_asm_errors();
    
    /* This won't actually run successfully if compilation fails */
    return 0;
}

/* Restore optimization settings */
#pragma GCC pop_options
