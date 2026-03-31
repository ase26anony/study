/* test_expr_error.c - Trigger error_mark_node in expr.cc */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int x;
    int y;
};

/* Test 4: Target-specific expansion failures */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory constructs */
#ifdef __TM__
void tm_func(void) {
    __transaction_atomic {
        /* empty transaction */
    }
}
#endif

/* Test 6: Complex nested invalid operations */
int take_address(void) {
    /* Taking address of bit-field in struct */
    struct BitField {
        unsigned int field:4;
    } bf;
    
    /* This should fail during expansion */
    return &bf.field;  /* error: cannot take address of bit-field */
}

/* Test 7: Invalid __builtin_constant_p usage */
int test_builtin_constant(void) {
    /* Using void expression in __builtin_constant_p */
    return __builtin_constant_p((void)0);
}

/* Test 8: Misaligned pointer operations */
int test_misaligned(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Misaligned pointer */
    
    /* Force alignment requirement */
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    return *aligned;  /* May fail during expansion */
}

/* Test 9: Invalid overflow builtin usage */
int test_overflow(void) {
    long double ld1 = 1.0, ld2 = 2.0;
    int overflow;
    
    /* Using long double with overflow builtin - may fail on some targets */
    return __builtin_add_overflow(ld1, ld2, &overflow);
}

/* Test 10: Complex invalid comma operator */
int test_comma(void) {
    /* Comma operator with void left side in value context */
    int x = (printf("test"), 5);
    
    /* Nested void expressions */
    int y = (void_func(), (void)0, 10);
    return x + y;
}

/* Test 11: Invalid compound literal with designator */
int* test_compound_literal(void) {
    /* Non-existent field designator */
    return &(struct BadStruct){ .non_existent = 1 };
}

/* Test 12: Vector operations on non-vector targets */
v4si test_vector(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector operation that might fail on targets without vector support */
    return a + b;
}

/* Test 13: Variadic builtin misuse */
int test_va_arg_error(void) {
    va_list ap;
    /* Using va_arg without va_start and with wrong type */
    return __builtin_va_arg(ap, float);
}

/* Test 14: Nested conditional with void */
int test_conditional(void) {
    /* Conditional operator with void in one branch */
    return 1 ? (void)0 : 5;
}

/* Test 15: sizeof with void expression */
int test_sizeof_void(void) {
    /* sizeof on void expression - invalid */
    return sizeof((void)0);
}

/* Main function containing all problematic constructs */
int main(void) {
    int result = 0;
    
    /* Test 1: Direct void assignment */
    int a = (void)void_func();
    
    /* Test 2: Comma operator with void */
    int b = (void_func(), 10);
    
    /* Test 6: Address of bit-field (commented out as it won't compile) */
    /* result += take_address(); */
    
    /* Test 7: Builtin constant with void */
    result += test_builtin_constant();
    
    /* Test 8: Misaligned access */
    result += test_misaligned();
    
    /* Test 9: Overflow with long double */
    result += test_overflow();
    
    /* Test 10: Complex comma */
    result += test_comma();
    
    /* Test 11: Bad compound literal */
    int *p = test_compound_literal();
    result += *p;
    
    /* Test 12: Vector operations */
    v4si v = test_vector();
    result += v[0];
    
    /* Test 13: Misused va_arg */
    result += test_va_arg_error();
    
    /* Test 14: Conditional with void */
    result += test_conditional();
    
    /* Test 15: sizeof void */
    result += test_sizeof_void();
    
    return result;
}
