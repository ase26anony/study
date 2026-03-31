/* test_expr_error.c - Test program to trigger error_mark_node in expr.cc */

/* Include to use printf declaration for void function tests */
#include <stdio.h>

/* Helper function for variadic tests */
void variadic_helper(int n, ...) {
    /* Empty - just for declaration */
}

/* Test 1: Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)printf("test");
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void function call as expression */
    int z = (test_void_errors(), 10);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_errors(void) {
    /* Use va_arg without proper va_list initialization */
    __builtin_va_list ap;
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch in variadic context */
    variadic_helper(1, 42);
    /* The issue would be if we try to expand va_arg with wrong type */
}

/* Test 3: Malformed compound literals */
void test_compound_literal_errors(void) {
    /* Invalid designator in compound literal */
    struct S { int a; int b; };
    int *p = &(struct S){ .non_existent = 1 };
    
    /* Taking address of non-lvalue compound literal in complex context */
    int *q = &(int){1} + 1;  /* Might fail during expansion */
    
    /* Compound literal with incompatible pointer types */
    float *fp = &(int){42};
}

/* Test 4: Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector operations on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    /* This might fail if complex overflow not supported */
    int overflow = __builtin_add_overflow_p(cd1, cd2, cd3);
}

/* Test 5: Transaction Memory constructs without support */
void test_tm_errors(void) {
    /* TM atomic block - may fail if TM not supported */
    __transaction_atomic {
        int x = 42;
    }
}

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Void expression inside __builtin_constant_p */
    int x = __builtin_constant_p((void)0);
    
    /* Invalid address operations in complex expressions */
    struct BitField { unsigned int bf:4; } bf;
    /* Taking address of bit-field */
    unsigned int *ptr = (unsigned int*)&bf.bf;
    
    /* Nested invalid operations in conditional */
    int y = (sizeof((void)printf("test")) ? 1 : 0);
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 4);
}

/* Test 7: Invalid operations in sizeof context */
void test_sizeof_errors(void) {
    /* sizeof on void expression */
    size_t s1 = sizeof((void)printf("test"));
    
    /* sizeof on function type (GCC extension but might fail in expansion) */
    size_t s2 = sizeof(test_sizeof_errors);
    
    /* sizeof on incomplete array */
    extern int incomplete_array[];
    size_t s3 = sizeof(incomplete_array);
}

/* Test 8: Using __builtin_choose_expr with invalid types */
void test_choose_expr_errors(void) {
    /* Third and fourth arguments have incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Non-constant condition */
    int y = 5;
    int z = __builtin_choose_expr(y, 1, 2);
}

/* Test 9: Invalid pointer arithmetic */
void test_pointer_errors(void) {
    /* Pointer to void arithmetic */
    void *vp = 0;
    void *vp2 = vp + 1;
    
    /* Function pointer arithmetic */
    void (*fp)(void) = test_pointer_errors;
    void (*fp2)(void) = fp + 1;
}

/* Test 10: Using __builtin_offsetof with invalid types */
void test_offsetof_errors(void) {
    /* offsetof on non-aggregate type */
    size_t o1 = __builtin_offsetof(int, dummy);
    
    /* offsetof with bit-field */
    struct WithBitfield { int a:4; int b; };
    size_t o2 = __builtin_offsetof(struct WithBitfield, a);
}

/* Main function - container for all tests */
int main(void) {
    /* Call test functions to trigger compilation errors */
    test_void_errors();
    test_va_arg_errors();
    test_compound_literal_errors();
    test_target_specific_errors();
    test_tm_errors();
    test_nested_errors();
    test_sizeof_errors();
    test_choose_expr_errors();
    test_pointer_errors();
    test_offsetof_errors();
    
    return 0;
}
