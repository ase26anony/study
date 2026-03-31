/* test_expr_error.c - Trigger error_mark_node return in expr.cc */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { }
int variadic_func(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int result = va_arg(ap, int);
    va_end(ap);
    return result;
}

/* Vector type that might fail expansion */
typedef int v4si __attribute__((vector_size(16)));

/* Transactional memory (if supported) */
#ifdef USE_TM
__attribute__((transaction_safe))
void tm_func(void) { }
#endif

int main(void) {
    /* 1. Invalid operations on void expressions */
    /* These should trigger error during expansion */
    int x = (void)void_func();  /* Direct void cast in value context */
    
    /* Comma operator with void left side in value context */
    int y = (printf("test"), 5);
    
    /* Void expression in conditional operator */
    int z = (1 ? (void)0 : 0);
    
    /* 2. Misusing __builtin_va_arg */
    /* Using va_arg without proper va_list initialization */
    va_list ap;
    /* Not initializing ap - undefined behavior that might fail during expansion */
    int w = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    int result = variadic_func(1, 10);  /* int passed */
    float wrong_type = __builtin_va_arg(ap, float);  /* Wrong type */
    
    /* 3. Malformed compound literals */
    /* Non-existent field designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .c = 1 };  /* .c doesn't exist */
    
    /* Compound literal in non-lvalue context with address taken */
    int *q = &(int){10} + 1;  /* Taking address and doing arithmetic */
    
    /* 4. Target-specific expansion failures */
    /* Vector operations - might fail if target doesn't support vectors */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;  /* Vector addition */
    
    /* Overflow builtins with potentially unsupported types */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow;
    /* __builtin_add_overflow_p might fail for long double on some targets */
    int has_overflow = __builtin_add_overflow_p(ld1, ld2, (long double)0.0);
    
    /* 5. Complex nested invalid operations */
    /* Invalid void expression inside sizeof */
    size_t sz = sizeof((void)void_func());
    
    /* Invalid void expression as builtin argument */
    int is_const = __builtin_constant_p((void)0);
    
    /* Nested in conditional with other errors */
    int complex = (1 ? __builtin_va_arg(ap, struct S) : (void)0);
    
    /* 6. Misaligned pointer operations */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Potentially misaligned */
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* 7. Transactional memory constructs (if compiled with -fgnu-tm) */
    #ifdef USE_TM
    __transaction_atomic {
        x = x + 1;
    }
    #endif
    
    /* 8. Bit-field address attempt */
    struct BitField {
        unsigned int field:4;
    } bf;
    /* Attempt to take address of bit-field - invalid */
    unsigned int *bf_ptr = &bf.field;
    
    /* 9. Invalid pointer arithmetic with void pointer */
    void *vp = &x;
    void *vp2 = vp + 1;  /* Invalid arithmetic on void pointer */
    
    /* 10. Using __builtin_choose_expr with invalid types */
    int choice = __builtin_choose_expr(1, (void)0, 0);
    
    /* The program doesn't need to run successfully */
    /* Just need to trigger compilation errors in expr.cc */
    return 0;
}
