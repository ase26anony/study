/* test_expr_error.c
 * Designed to trigger error_mark_node return in expr.cc during RTL expansion
 */

/* Force certain constructs to reach expansion phase */
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

int variadic_func(int count, ...) {
    va_list ap;
    va_start(ap, count);
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(ap, int);
    }
    va_end(ap);
    return sum;
}

/* Attempt 1: Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct void expression in value context - should fail during expansion */
    int x = (void)void_func();  /* Line 1: void cast in assignment */
    
    /* Void expression in comma operator in value context */
    int y = (void_func(), 5);   /* Line 2: void in comma operator */
    
    /* Nested void expression in conditional */
    int z = (1 ? (void)0 : 0);  /* Line 3: void in conditional */
}

/* Attempt 2: Misusing __builtin_va_arg */
void test_va_arg_errors(void) {
    /* Using va_arg outside variadic context */
    va_list fake_ap;
    /* Line 4: va_arg with uninitialized va_list */
    float f = __builtin_va_arg(fake_ap, float);
    
    /* Type mismatch in va_arg */
    int result = variadic_func(2, 10, 20);
    /* Can't easily misuse here since we need real va_list */
}

/* Attempt 3: Malformed compound literals */
void test_compound_literal_errors(void) {
    struct S { int a; int b; };
    
    /* Line 5: Non-existent field designator */
    struct S *s1 = &(struct S){ .non_existent = 1 };
    
    /* Line 6: Compound literal in invalid address context */
    int *p = &(int){10} + 1;  /* Taking address of temporary, then arithmetic */
}

/* Attempt 4: Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector extensions - may fail on targets without vector support */
    typedef int v4si __attribute__((vector_size(16)));
    
    /* Line 7: Vector operation that might fail during expansion */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector addition */
    
    /* Overflow builtins with potentially unsupported types */
    /* Line 8: Overflow check with long double */
    long double ld1 = 1e100, ld2 = 1e100;
    int overflow;
    __builtin_add_overflow(ld1, ld2, &overflow);
}

/* Attempt 5: Complex nested errors */
void test_complex_nested_errors(void) {
    /* Line 9: Void expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)0);
    
    /* Line 10: Multiple invalid operations chained */
    int x = sizeof((void)void_func()) + (void_func(), 0);
    
    /* Line 11: Invalid address of bit-field in struct */
    struct BitField {
        unsigned int flag:1;
        unsigned int value:7;
    } bf = {0, 5};
    
    /* Taking address of bit-field - invalid */
    unsigned int *ptr = &bf.flag;
}

/* Attempt 6: Transactional Memory (if supported) */
void test_transactional_memory(void) {
    /* Line 12: TM construct that might fail during expansion */
    __transaction_atomic {
        int x = 42;
        x++;
    }
}

/* Attempt 7: Misaligned pointer operations */
void test_misaligned_errors(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Misaligned pointer */
    
    /* Line 13: Using misaligned pointer with alignment assumption */
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    *aligned = 42;
}

/* Main function - container for all error tests */
int main(void) {
    /* Each test contains constructs that should trigger
     * error_mark_node during expression expansion */
    
    test_void_errors();           /* Lines 1-3 */
    test_va_arg_errors();         /* Line 4 */
    test_compound_literal_errors(); /* Lines 5-6 */
    test_target_specific_errors();  /* Lines 7-8 */
    test_complex_nested_errors();   /* Lines 9-11 */
    test_transactional_memory();    /* Line 12 */
    test_misaligned_errors();       /* Line 13 */
    
    return 0;
}

#pragma GCC pop_options
