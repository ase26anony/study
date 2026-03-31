/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Force certain compiler behaviors */
#pragma GCC optimize("O3")
#pragma GCC push_options
#pragma GCC optimize("no-tree-ccp")
#pragma GCC optimize("no-tree-forwprop")

#include <stdarg.h>
#include <stdio.h>

/* Helper functions to create problematic contexts */
void void_func(void) {
    printf("This returns void\n");
}

/* Variadic function setup */
void setup_va(va_list *ap, ...) {
    va_start(*ap, 0);
}

/* Vector type that might fail expansion */
typedef int v4si __attribute__((vector_size(16)));

/* Transactional memory if supported */
#ifdef __TM__
__attribute__((transaction_safe))
#endif
int problematic_tm(void) {
    return 42;
}

/* Complex type for overflow builtins */
typedef _Complex double complex_double;

int main(void) {
    /* 1. Invalid operations on void expressions */
    /* This should fail during expansion: using void expression where value needed */
    int x = (void)void_func();  /* Direct void cast in initialization */
    
    /* Comma operator with void left side in value context */
    int y = (void_func(), 5);
    
    /* Void expression in conditional */
    int z = (1 ? (void)0 : 0);
    
    /* 2. Misusing __builtin_va_arg */
    va_list ap;
    /* Using va_arg without proper initialization */
    float f = __builtin_va_arg(ap, float);
    
    /* Type mismatch in va_arg */
    int i = 10;
    double d = __builtin_va_arg(ap, double);  /* ap not initialized for double */
    
    /* 3. Malformed compound literals */
    /* Non-existent field designator */
    struct S { int a; int b; };
    int *p = &(struct S){ .c = 1 };  /* .c doesn't exist */
    
    /* Compound literal in invalid context */
    int *q = &(int){ (void)0 };  /* void expression inside compound literal */
    
    /* 4. Target-specific expansion failures */
    /* Vector operations might fail on some targets */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtin with potentially unsupported type */
    int overflow;
    /* Try with complex type if supported */
    complex_double c1 = 1.0 + 2.0i;
    complex_double c2 = 3.0 + 4.0i;
    
    /* 5. Nested invalid operations */
    /* Void expression inside sizeof (GCC extension) */
    size_t s = sizeof((void)0);
    
    /* Void expression as builtin argument */
    int is_const = __builtin_constant_p((void)0);
    
    /* Invalid address operations */
    struct BitField { unsigned int field:3; } bf;
    /* Taking address of bit-field */
    unsigned int *addr = &bf.field;
    
    /* 6. Transactional memory if enabled */
    #ifdef __TM__
    __transaction_atomic {
        x = problematic_tm();
    }
    #endif
    
    /* 7. Misaligned pointer operations */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    /* Force assumption of alignment */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* 8. Complex expression with multiple errors */
    int result = (__builtin_va_arg(ap, int) + 
                 (void_func(), 10) + 
                 *(int*)((struct S){ .a = (void)0 }.a));
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(x), "r"(y), "r"(z), "r"(f), "r"(d), 
                  "r"(p), "r"(q), "r"(v3), "r"(s), "r"(is_const),
                  "r"(addr), "r"(aligned), "r"(result));
    
    return 0;
}

#pragma GCC pop_options

/* Additional test cases in separate functions to isolate errors */
void test_overflow_builtins(void) {
    /* Overflow builtins with questionable types */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow_flag;
    
    /* This might fail expansion if long double overflow not supported */
    __builtin_add_overflow(ld1, ld2, &overflow_flag);
}

void test_invalid_designators(void) {
    /* Array designator out of bounds */
    int arr[5] = { [10] = 1 };  /* Might be caught early, but try */
    
    /* Nested invalid designators */
    struct Outer {
        struct Inner {
            int x;
        } inner;
    };
    
    /* Invalid nested designator */
    struct Outer o = { .inner.y = 5 };  /* y doesn't exist in Inner */
}

/* Function with invalid return type expression */
int invalid_return(void) {
    return (void)void_func();  /* Returning void expression */
}

/* Test with attribute that might affect expansion */
__attribute__((optimize("O3", "no-tree-ccp")))
void optimized_error(void) {
    /* Error in optimized function */
    int x = (void)0;
}
