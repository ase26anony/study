/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during RTL expansion.
 * The goal is compile-time failure, not runtime execution.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) {}
int int_func(void) { return 0; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator requiring value */
    int y = (void_func(), 5);
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg outside variadic context */
    va_list ap;
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    float f = __builtin_va_arg(ap, float);
}

/* Variadic function to make va_arg usage plausible */
void variadic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    /* Incorrect type promotion - float promotes to double in variadics */
    float wrong = __builtin_va_arg(ap, float);  /* Should be double */
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a, b; };
    
    /* Invalid designator */
    int *p1 = &(int){ .non_existent = 1 };
    
    /* Compound literal in invalid context */
    int *p2 = &(struct S){ .a = 1, .b = 2 }.a;
    
    /* Taking address of non-lvalue compound literal member */
    int *p3 = &((struct S){1, 2}.a);
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    int overflow = __builtin_add_overflow(cd1, cd2, &cd3);
    
    /* Long double overflow check */
    long double ld1 = 1.0e1000L;
    long double ld2 = 1.0e1000L;
    long double ld3;
    overflow = __builtin_add_overflow(ld1, ld2, &ld3);
}

/* Test 5: Transaction Memory constructs */
void test_transaction_memory(void) {
    int x = 0;
    
    /* Transactional memory block */
    __transaction_atomic {
        x = 42;
    }
    
    /* Nested transaction */
    __transaction_atomic {
        __transaction_atomic {
            x = 100;
        }
    }
}

/* Test 6: Complex nested invalid operations */
void test_complex_nested(void) {
    /* Invalid void expression inside sizeof */
    size_t s1 = sizeof((void)0);
    
    /* Invalid void in __builtin_constant_p */
    int c1 = __builtin_constant_p((void)0);
    
    /* Nested invalid operations in conditional */
    int x = 1 ? (2 ? (void)0 : 0) : 0;
    
    /* Invalid address operations */
    struct BitField { unsigned int bf:4; } bf = {0};
    unsigned int *ptr = &bf.bf;  /* Address of bit-field */
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
}

/* Test 7: Invalid switch case ranges (GCC extension) */
void test_switch_ranges(void) {
    int x = 5;
    
    switch (x) {
        case 1 ... 10:  /* Valid case range */
            break;
        case 20 ... 15:  /* Invalid reversed range */
            break;
        case (void)0 ... 10:  /* Invalid void in range */
            break;
    }
}

/* Test 8: Invalid assembly operands */
void test_inline_asm(void) {
    int x;
    
    /* Invalid constraint */
    __asm__("mov %0, %1" : "=r"(x) : "r"((void)0));
    
    /* Invalid operand type */
    __asm__("" : "=r"((void)0));
}

/* Test 9: Invalid pointer arithmetic */
void test_pointer_arithmetic(void) {
    void *vp = 0;
    
    /* Arithmetic on void pointer */
    vp = vp + 1;
    
    /* Function pointer arithmetic */
    void (*fp)(void) = void_func;
    fp = fp + 1;
}

/* Test 10: Nested errors with optimization-sensitive paths */
void test_optimization_sensitive(void) {
    /* This might be optimized away at -O2 but reach expansion at -O0 */
    volatile int *volatile_ptr = (int*)((void)0);
    int val = *volatile_ptr;
    
    /* Complex expression with multiple error points */
    int x = (1 + (int)(void)0) * sizeof((void)0);
    
    /* Invalid offsetof usage */
    struct Invalid { int a; char b; };
    size_t off = __builtin_offsetof(struct Invalid, c);  /* Non-existent member */
}

/* Main function - container for all tests */
int main(void) {
    /* These calls won't actually execute if compilation fails */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    test_transaction_memory();
    test_complex_nested();
    test_switch_ranges();
    test_inline_asm();
    test_pointer_arithmetic();
    test_optimization_sensitive();
    
    variadic_func("test", 1, 2.0, 3.0f);
    
    return 0;
}
