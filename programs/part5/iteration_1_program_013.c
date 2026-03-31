/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during RTL expansion.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { }
int int_func(int x) { return x; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression */
    int x = (void)void_func();  /* Should fail during expansion */
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);  /* Left side is void */
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);  /* Type mismatch in branches */
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(void) {
    /* Using va_arg outside variadic context */
    va_list ap;
    int x = __builtin_va_arg(ap, int);  /* ap not initialized */
    
    /* Type mismatch in va_arg */
    float f = __builtin_va_arg(ap, float);  /* Wrong type promotion */
}

/* Variadic function to test va_arg in wrong context */
void variadic_test(int n, ...) {
    va_list ap;
    va_start(ap, n);
    
    /* Using wrong type for argument */
    double d = __builtin_va_arg(ap, double);  /* Might fail if passed int */
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S s1 = (struct S){ .non_existent = 1 };  /* Should fail */
    
    /* Taking address in invalid context */
    int *p = &(int){ .non_existent_field = 1 };  /* Multiple errors */
    
    /* Compound literal with wrong type */
    float *fp = &(int){5};  /* Type mismatch */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    int overflow;
    
    /* This might fail if backend doesn't support complex overflow */
    __builtin_add_overflow(cd1, cd2, &cd1);
}

/* Test 5: Transaction Memory without support */
void test_transaction_memory(void) {
    int x = 0;
    
    /* Transactional memory block - may fail if TM not supported */
    __transaction_atomic {
        x = 42;
    }
}

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside builtin */
    int is_const = __builtin_constant_p((void)0);  /* Invalid argument */
    
    /* Nested in conditional */
    int x = 1 ? __builtin_constant_p((void)0) : 0;
    
    /* In sizeof context (though sizeof on void is 1 in GCC) */
    size_t sz = sizeof((void)void_func());
    
    /* Complex pointer arithmetic with invalid operands */
    int arr[5];
    int *ptr = &arr[0] + (void)void_func();  /* Adding void to pointer */
}

/* Test 7: Misaligned pointer operations */
void test_misaligned(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Misaligned for int */
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* May fail */
    
    /* Dereference misaligned pointer */
    int value = *misaligned;  /* Could fail during expansion on strict align arch */
}

/* Test 8: Bit-field address operations */
void test_bitfield_address(void) {
    struct BitField {
        unsigned int field : 4;
    } bf = {0};
    
    /* Attempt to take address of bit-field */
    unsigned int *ptr = &bf.field;  /* Invalid - cannot take address of bit-field */
    
    /* In complex expression */
    unsigned int **pptr = &(&bf.field);  /* Nested address-of on bit-field */
}

/* Test 9: Invalid switch case ranges (GCC extension) */
void test_switch_ranges(void) {
    int x = 5;
    
    switch (x) {
        case 1 ... 10:  /* Case range - might have expansion issues */
            break;
        case (void)0 ... 10:  /* Invalid range with void expression */
            break;
    }
}

/* Test 10: __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Choose expression with incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);  /* void vs int */
    
    /* Nested with other builtins */
    int y = __builtin_choose_expr(
        __builtin_constant_p((void)0),  /* Invalid condition */
        1, 2);
}

/* Main function - container for all tests */
int main(void) {
    /* Call tests to trigger compile-time errors */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    test_transaction_memory();
    test_nested_errors();
    test_misaligned();
    test_bitfield_address();
    test_switch_ranges();
    test_builtin_choose();
    
    /* Call variadic with wrong arguments */
    variadic_test(1, 42);  /* Passing int when expecting double */
    
    return 0;
}
