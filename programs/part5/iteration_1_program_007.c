/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Strategy: Combine multiple error-prone constructs to maximize
   chances of reaching the uncovered error-handling path */

#include <stdarg.h>
#include <stdio.h>

/* 1. Invalid void expressions in value contexts */
void void_func(void) {}

/* 2. Vector type on targets that might not support it */
typedef int v4si __attribute__((vector_size(16)));

/* 3. Variadic function for va_arg misuse */
void variadic_func(int n, ...) {
    va_list ap;
    va_start(ap, n);
    /* Intentional misuse below */
    (void)ap;
    va_end(ap);
}

/* 4. Transactional memory construct */
#ifdef __GNUC__
void tm_func(void) {
    __transaction_atomic {
        /* Empty transaction */
    }
}
#endif

/* 5. Complex type for overflow builtins */
typedef _Complex double complex_double;

int main(void) {
    /* CASE 1: Direct void expression in value context */
    /* This should fail during expansion as void has no value */
    int x = (void)void_func();
    
    /* CASE 2: Nested invalid void expression */
    /* The comma operator yields void, but we try to use it as int */
    int y = (printf("test"), 5);
    
    /* CASE 3: Misuse of __builtin_va_arg outside proper context */
    /* ap is not initialized as a va_list here */
    int z = __builtin_va_arg((void *)0, float);
    
    /* CASE 4: Invalid compound literal */
    /* Taking address of compound literal with invalid designator */
    int *p = &(int){ .non_existent = 42 };
    
    /* CASE 5: Vector operations (may fail on non-vector targets) */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* CASE 6: Overflow builtin with potentially unsupported type */
    /* long double overflow check - backend may lack support */
    int overflow;
    long double ld1 = 1e1000L;
    long double ld2 = 1e1000L;
    __builtin_add_overflow(ld1, ld2, &overflow);
    
    /* CASE 7: Complex type in overflow builtin */
    /* Complex types often unsupported for overflow builtins */
    complex_double c1 = 1.0 + 2.0i;
    complex_double c2 = 3.0 + 4.0i;
    __builtin_add_overflow(c1, c2, &overflow);
    
    /* CASE 8: Address of bit-field in complex expression */
    struct S {
        unsigned int bf : 4;
        int normal;
    } s = {0, 0};
    
    /* Taking address of bit-field is invalid */
    int *bf_ptr = (int*)&s.bf;
    
    /* CASE 9: Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* CASE 10: Invalid sizeof with void expression */
    size_t sz = sizeof((void)0);
    
    /* CASE 11: __builtin_constant_p with invalid expression */
    int is_const = __builtin_constant_p((void)void_func());
    
    /* CASE 12: Conditional operator with void arms */
    int cond = 1 ? (void)0 : (void)1;
    
    /* CASE 13: Nested in return statement */
    return (void)main();
}
