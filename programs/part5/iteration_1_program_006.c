/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Invalid void expressions in value contexts */
void void_func(void) { }

/* Misusing va_arg builtins */
#include <stdarg.h>

/* Vector types on targets that might not support them */
typedef int v4si __attribute__((vector_size(16)));

/* Transactional memory constructs */
#ifdef __GNUC__
#define TM_ATTR __attribute__((transaction_safe))
#else
#define TM_ATTR
#endif

/* Complex type for overflow builtins */
typedef _Complex float complex_float;

int main(void) {
    /* 1. Invalid void operations in value contexts */
    /* This should trigger error during expression expansion */
    int x = (void)void_func();  /* Direct void cast in assignment */
    
    /* Comma operator with void left side in value context */
    int y = (printf("test"), 5);
    
    /* Void expression in conditional operator */
    int z = (1 ? (void)0 : 0);
    
    /* 2. Misusing __builtin_va_arg */
    /* Create a va_list but use it incorrectly */
    va_list ap;
    /* Using va_arg without proper initialization and with wrong type */
    /* This may fail during expansion if not caught earlier */
    int bad_va = __builtin_va_arg(ap, float);
    
    /* 3. Malformed compound literals */
    /* Invalid designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .c = 1 };  /* Non-existent field */
    
    /* Compound literal in invalid context */
    int *q = &(int){ &(int){1} };  /* Nested address-taking */
    
    /* 4. Target-specific expansion failures */
    /* Vector operations - may fail on targets without vector support */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtins with potentially unsupported types */
    complex_float cf1 = 1.0 + 2.0i;
    complex_float cf2 = 3.0 + 4.0i;
    int overflow;
    /* __builtin_add_overflow_p with complex types */
    _Bool ovf = __builtin_add_overflow_p(cf1, cf2, cf1);
    
    /* 5. Transactional memory (if supported) */
    /* This may fail during expansion if TM not supported */
    __transaction_atomic {
        x = x + 1;
    }
    
    /* 6. Complex nested invalid operations */
    /* Invalid address operations in complex expressions */
    int bitfield = 0;
    struct { int a:3; } bf = {0};
    /* Taking address of bit-field */
    int *addr = &bf.a;
    
    /* Nested void in builtin */
    int is_const = __builtin_constant_p((void)0);
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* 7. Invalid operations in sizeof context */
    /* sizeof with void expression */
    size_t sz = sizeof((void)void_func());
    
    /* 8. Using __builtin_choose_expr with invalid types */
    int choice = __builtin_choose_expr(1, (void)0, 0);
    
    /* 9. Invalid pointer arithmetic */
    void *vp = (void*)0x1000;
    void *vp2 = vp + (void*)0x10;  /* Pointer arithmetic on void* */
    
    /* 10. __builtin_offsetof with invalid member */
    size_t off = __builtin_offsetof(struct S, c);  /* Non-existent member */
    
    /* Prevent "unused variable" warnings */
    (void)x; (void)y; (void)z; (void)bad_va; (void)p; (void)q;
    (void)v3; (void)ovf; (void)addr; (void)is_const; (void)aligned;
    (void)sz; (void)choice; (void)vp2; (void)off;
    
    return 0;
}
