/* test_expr_error.c - Trigger error_mark_node in expr.cc during RTL expansion */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* Test 4: Vector extensions on non-vector targets */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory constructs */
#ifdef __GNUC__
#ifndef __TM_FENCE__
/* Force inclusion even if TM not supported */
#endif
#endif

/* Test 6: Complex nested invalid operations */

int main(void) {
    /* 1. Invalid void operations - should fail during expansion */
    int x = (void)void_func();  /* Direct void cast in value context */
    
    /* Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* 2. Misusing __builtin_va_arg - outside variadic context */
    va_list ap;
    /* This is invalid: ap not initialized, wrong context */
    float f = __builtin_va_arg(ap, float);
    
    /* 3. Malformed compound literals */
    /* Invalid designator */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Type mismatch in compound literal */
    struct BadStruct *bs = &(struct BadStruct){ .valid_field = 1.5 }; /* float to int */
    
    /* 4. Vector operations - may fail if target lacks vector support */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector addition */
    
    /* 5. Transaction Memory - may fail if TM not supported */
    __transaction_atomic {
        x = x + 1;
    }
    
    /* 6. Complex nested invalid operations */
    /* sizeof of a void expression */
    size_t sz = sizeof((void)0);
    
    /* Conditional operator with void */
    int z = (1 ? (void)0 : 5);
    
    /* Invalid address operations */
    int arr[5];
    /* Taking address of non-lvalue result */
    int *ptr = &(arr[0]++);
    
    /* Bit-field address attempt */
    struct {
        unsigned int bitfield : 4;
    } bf;
    unsigned int *bfptr = (unsigned int*)&bf.bitfield;
    
    /* 7. Overflow builtins with potentially unsupported types */
    long double ld1 = 1.0e100L, ld2 = 1.0e100L;
    int overflow;
    /* __builtin_add_overflow may not support long double on all targets */
    __builtin_add_overflow(ld1, ld2, &ld1);
    
    /* 8. Misaligned pointer in alignment context */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* 9. Nested __builtin_constant_p with invalid expression */
    int is_const = __builtin_constant_p((void)void_func());
    
    /* 10. Invalid pointer arithmetic with void* */
    void *vp = &x;
    vp = vp + 1;  /* GCC extension but might fail in certain contexts */
    
    return 0;
}
