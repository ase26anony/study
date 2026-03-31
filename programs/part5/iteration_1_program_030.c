/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during RTL expansion.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) {}
int int_func(void) { return 0; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression */
    int x = (void)void_func();  /* Should trigger error during expansion */
    
    /* Void in comma operator in value context */
    int y = (void_func(), 5);   /* Left side is void */
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);  /* Type mismatch in branches */
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    va_list ap;
    
    /* Using va_arg outside variadic context */
    int x = __builtin_va_arg(ap, int);  /* ap not properly initialized */
    
    /* Type mismatch in va_arg */
    double d = __builtin_va_arg(ap, double);  /* Wrong type promotion */
}

/* Variadic function to make va_arg usage "valid" but still wrong */
void variadic_test(int n, ...) {
    va_list ap;
    va_start(ap, n);
    
    /* Wrong type - asking for float when int was passed */
    float f = __builtin_va_arg(ap, float);  /* Should fail during expansion */
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S *p1 = &(struct S){ .non_existent = 1 };  /* Invalid designator */
    
    /* Taking address in invalid context */
    int *p2 = &(int){1} + 1;  /* Address arithmetic on compound literal */
    
    /* Compound literal with wrong type */
    int *p3 = (int*)&(float){1.0f};  /* Type mismatch */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported target */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    /* This may fail if backend doesn't support complex overflow */
    int overflow = __builtin_add_overflow(cd1, cd2, &cd3);
}

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    /* TM constructs without -fgnu-tm or on unsupported target */
    __transaction_atomic {
        int x = 5;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside sizeof */
    size_t s1 = sizeof((void)0);  /* Invalid operand to sizeof */
    
    /* Invalid void in __builtin_constant_p */
    int b1 = __builtin_constant_p((void)0);  /* Invalid argument */
    
    /* Nested invalid operations in conditional */
    int x = 1 ? (void_func(), 5) : (void)0;  /* Type mismatch */
    
    /* Bit-field address attempt */
    struct BitField { unsigned int bf:4; } bf;
    unsigned int *p = (unsigned int*)&bf.bf;  /* Address of bit-field */
}

/* Test 7: Misaligned pointer operations */
void test_misaligned(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Potentially misaligned */
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* May fail */
    
    /* Dereference misaligned pointer */
    int value = *misaligned;  /* Could trigger expansion error */
}

/* Test 8: Invalid switch case ranges (GCC extension) */
void test_invalid_switch(void) {
    int x = 5;
    
    switch (x) {
        case 1 ... 10:  /* Valid case range */
            break;
        case 20 ... 15:  /* Invalid reversed range - may pass parsing */
            break;
        default:
            break;
    }
}

/* Test 9: Invalid pointer comparisons */
void test_pointer_comparisons(void) {
    int x = 5;
    float f = 3.14f;
    
    /* Comparing pointers to different types */
    int result = (&x > (int*)&f);  /* Invalid comparison */
    
    /* Pointer to function vs pointer to object */
    int (*func_ptr)(void) = int_func;
    result = ((void*)func_ptr > (void*)&x);  /* Implementation-defined */
}

/* Test 10: __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Second and third operands must have compatible types */
    int x = __builtin_choose_expr(1, (void)0, 5);  /* Type mismatch */
    
    /* Invalid type in constant expression */
    int y = __builtin_choose_expr(1, 5, (void)0);  /* Type mismatch */
}

/* Main function - container for all tests */
int main(void) {
    /* Call tests to trigger compile-time errors */
    test_void_operations();
    test_va_arg_misuse();
    variadic_test(1, 2);
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_misaligned();
    test_invalid_switch();
    test_pointer_comparisons();
    test_builtin_choose();
    
    return 0;
}
