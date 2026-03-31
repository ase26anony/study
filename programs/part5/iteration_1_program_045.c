/* test_expr_error.c
 * Designed to trigger error_mark_node return in expr.cc during RTL expansion
 */

/* Strategy 1: Invalid operations on void expressions */
void void_func(void) {}

/* Strategy 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Strategy 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
    /* No non_existent_field */
};

/* Strategy 4: Target-specific failures */
#ifdef __SIZEOF_INT128__
typedef __int128 large_int __attribute__((vector_size(32)));
#else
typedef long long large_int __attribute__((vector_size(32)));
#endif

/* Transaction Memory - may fail if not supported */
#ifdef __GNUC__
void tm_func(void) {
    __transaction_atomic {
        /* empty transaction */
    }
}
#endif

int main(void) {
    /* 1. Direct void expression in value context - should fail during expansion */
    int x = (void)void_func();
    
    /* 2. Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* 3. Invalid __builtin_va_arg usage outside variadic context */
    va_list ap;
    /* Not properly initialized, wrong type */
    float f = __builtin_va_arg(ap, float);
    
    /* 4. Malformed compound literal with non-existent field */
    int *p = &(struct BadStruct){ .non_existent_field = 1 };
    
    /* 5. Compound literal in non-lvalue context with address taken */
    int *q = &(int){10} + 1;  /* Taking address of temporary */
    
    /* 6. Nested invalid operations */
    /* void expression inside __builtin_constant_p */
    int z = __builtin_constant_p((void)0) ? 1 : 0;
    
    /* 7. Invalid address of bit-field (if we had a struct with bit-field) */
    struct BitField {
        unsigned int field:4;
    } bf;
    unsigned int *bf_ptr = &bf.field;  /* Address of bit-field */
    
    /* 8. Vector operations that might fail on some targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Might fail if vector ops not supported */
    
    /* 9. Large vector that might exceed target capabilities */
    large_int big_vec = {0};
    
    /* 10. Misaligned pointer in alignment context */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* 11. Overflow builtins with potentially unsupported types */
    int overflow_result;
    int overflow_occurred = __builtin_add_overflow(1000000000, 2000000000, &overflow_result);
    
    /* 12. Transaction memory if supported */
    #ifdef __GNUC__
    __transaction_atomic {
        x = y + z;
    }
    #endif
    
    /* 13. Complex nested error scenario */
    /* sizeof of a void expression inside conditional */
    int size = sizeof((void)void_func(), 0) ? 10 : 20;
    
    /* 14. Another nested case: void in arithmetic */
    int calc = 5 + (int)(void)void_func();
    
    /* 15. Attempt to use __builtin_va_arg with wrong type promotion */
    /* This requires actual variadic function call */
    
    return 0;
}

/* Variadic function to test __builtin_va_arg errors */
void test_va_errors(int n, ...) {
    va_list args;
    va_start(args, n);
    
    /* Wrong type - float should be double in variadic args */
    float wrong = __builtin_va_arg(args, float);
    
    /* Using void type in va_arg */
    void *vptr = __builtin_va_arg(args, void);
    
    va_end(args);
}

/* Function with invalid return type expression */
int invalid_return(void) {
    return (void)void_func();  /* Returning void expression */
}

/* Global initializer with problematic expression */
int global_var = (void)void_func(), 42;
