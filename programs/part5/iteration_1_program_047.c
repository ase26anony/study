/* test_expr_error.c - Trigger error_mark_node in expr.cc during RTL expansion */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
int misuse_va_arg() {
    __builtin_va_list ap;
    /* This should fail during expansion - using va_arg without proper initialization */
    return __builtin_va_arg(ap, int);
}

/* Test 3: Malformed compound literals */
struct BadStruct {
    int a;
    int b;
};

/* Test 4: Vector extensions on targets without support */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transactional memory without support */
#ifdef __GNUC__
int tm_test(int *ptr) {
    int result;
    __transaction_atomic {
        result = *ptr;
    }
    return result;
}
#endif

/* Test 6: Complex nested invalid operations */
int nested_invalid(void) {
    /* sizeof of a void expression - invalid */
    return sizeof((void)void_func());
}

/* Test 7: Invalid address operations */
int invalid_address(void) {
    /* Taking address of a bit-field in a struct */
    struct BitField {
        unsigned int field:4;
    } bf;
    
    /* This should fail during expansion */
    unsigned int *p = &bf.field;
    return *p;
}

/* Test 8: __builtin_constant_p with invalid expression */
int builtin_constant_invalid(void) {
    /* __builtin_constant_p with void expression */
    return __builtin_constant_p((void)0);
}

/* Test 9: Comma operator with void left side in value context */
int comma_void(void) {
    /* Left side of comma is void, right side is int */
    int x = (printf("test"), 5);
    return x;
}

/* Test 10: Compound literal with invalid designator */
int bad_designator(void) {
    /* Non-existent field in designator */
    int *p = &(struct BadStruct){ .non_existent = 1 };
    return *p;
}

/* Test 11: Overflow builtins with potentially unsupported types */
#ifdef __GNUC__
int overflow_unsupported(void) {
    long double a = 1.0, b = 2.0;
    long double result;
    int overflow;
    /* __builtin_add_overflow may not support long double on all targets */
    overflow = __builtin_add_overflow(a, b, &result);
    return overflow;
}
#endif

/* Test 12: Misaligned pointer in alignment context */
int misaligned_access(void) {
    char buffer[32];
    int *misaligned = (int*)(buffer + 1);
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    return *aligned;
}

/* Test 13: Vector operations */
int vector_ops(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* May fail on targets without vector support */
    return c[0];
}

/* Main function - container for all tests */
int main(void) {
    int result = 0;
    
    /* These calls may not execute, but their presence triggers compilation errors */
    result += misuse_va_arg();
    
    #ifdef __GNUC__
    int x = 5;
    result += tm_test(&x);
    result += overflow_unsupported();
    #endif
    
    result += nested_invalid();
    result += invalid_address();
    result += builtin_constant_invalid();
    result += comma_void();
    result += bad_designator();
    result += misaligned_access();
    result += vector_ops();
    
    return result;
}
