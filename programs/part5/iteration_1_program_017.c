/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Strategy 1: Invalid operations on void expressions */
void void_func(void) {}

/* Strategy 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Strategy 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* Strategy 4: Target-specific expansion failures */
typedef int v4si __attribute__((vector_size(16)));

/* Strategy 5: Transaction Memory constructs */
#ifdef __TM_FEATURES_H__
#error "TM features header included unexpectedly"
#endif

int main(void) {
    /* 1. Direct void expression in value context - should fail during expansion */
    int x = (void)void_func();
    
    /* 2. Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* 3. Invalid __builtin_va_arg usage outside variadic context */
    va_list ap;
    /* Not properly initialized - expansion may fail */
    int z = __builtin_va_arg(ap, float);
    
    /* 4. Compound literal with non-existent field designator */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* 5. Compound literal with struct field that doesn't exist */
    struct BadStruct *bs = &(struct BadStruct){ .invalid_field = 42 };
    
    /* 6. Nested invalid void expression in sizeof */
    size_t s1 = sizeof((void)0);
    
    /* 7. Nested invalid void expression in __builtin_constant_p */
    int b1 = __builtin_constant_p((void)0);
    
    /* 8. Vector operations on potentially unsupported target */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* May fail expansion on non-vector targets */
    
    /* 9. Transaction Memory without proper support */
    int tm_val = 0;
    __transaction_atomic {
        tm_val = 42;
    }
    
    /* 10. Overflow builtin with potentially unsupported type */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow_flag;
    long double ld3 = __builtin_add_overflow(ld1, ld2, &overflow_flag);
    
    /* 11. Complex type with overflow builtin */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    _Complex double c3;
    int c_overflow;
    c3 = __builtin_add_overflow(c1, c2, &c_overflow);
    
    /* 12. Invalid address of bit-field in complex context */
    struct BitFieldStruct {
        unsigned int bitfield : 4;
        unsigned int regular;
    } bfs = {0, 0};
    
    /* Taking address of bit-field - invalid but might pass parsing */
    unsigned int *invalid_ptr = &bfs.bitfield;
    
    /* 13. Misaligned pointer in alignment context */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* 14. Void function call in conditional operator */
    int cond = (1 ? (void)void_func() : 0);
    
    /* 15. Multiple nested errors */
    int complex_error = sizeof((void)((1 ? __builtin_va_arg(ap, double) : 0)));
    
    return 0;
}
