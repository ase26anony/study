/* test_expr_error.c - Trigger error_mark_node in expr.cc during RTL expansion */

/* 1. Invalid operations on void expressions */
void void_func(void) {}

/* 2. Misusing __builtin_va_arg */
int misuse_va_arg() {
    __builtin_va_list ap;
    /* Using va_arg without proper initialization and with wrong type */
    return __builtin_va_arg(ap, float);  /* ap not initialized, float may be wrong type */
}

/* 3. Malformed compound literals */
struct BadStruct {
    int a;
    int b;
};

/* 4. Complex nested invalid operations */
int nested_invalid(void) {
    /* Invalid void expression inside sizeof */
    return sizeof((void)void_func());
}

/* 5. Vector extensions on targets that might not support them */
typedef int v4si __attribute__((vector_size(16)));

/* 6. Transactional memory constructs without proper support */
#ifdef USE_TM
int tm_func(int *ptr) {
    int result;
    __transaction_atomic {
        result = *ptr;
    }
    return result;
}
#endif

/* 7. Invalid address operations */
int invalid_address(void) {
    struct BitField {
        unsigned int field:4;
    } bf;
    
    /* Taking address of bit-field (invalid) */
    unsigned int *p = &bf.field;  /* Should fail during expansion */
    return *p;
}

/* 8. Overflow builtins with potentially unsupported types */
long double test_overflow(void) {
    long double a = 1.0, b = 2.0;
    long double result;
    /* Using overflow builtin with long double might fail on some targets */
    int overflow = __builtin_add_overflow(a, b, &result);
    return result;
}

/* 9. Complex expression with multiple error conditions */
int complex_error_expr(void) {
    /* Comma operator with void left side in conditional */
    return (printf("test"), 0) ? (void)void_func(), 1 : 2;
}

/* 10. Misaligned pointer in alignment context */
int misaligned_access(int *p) {
    /* Force assume aligned on potentially misaligned pointer */
    int *aligned = __builtin_assume_aligned(p, 16);
    return aligned[0];
}

/* 11. Invalid compound literal with non-existent field */
int *bad_compound_literal(void) {
    /* Using non-existent field in designated initializer */
    return &(struct BadStruct){ .non_existent = 1, .c = 2 };
}

/* 12. Nested sizeof with invalid expression */
size_t nested_sizeof_error(void) {
    /* sizeof of a void expression */
    return sizeof((void)(printf("test"), 0));
}

/* Main function - container for all the problematic constructs */
int main(void) {
    int result = 0;
    
    /* Try to use various error-prone constructs */
    result += misuse_va_arg();
    
    /* This should trigger error during expansion */
    int x = (void)void_func();
    
    /* Invalid compound literal usage */
    struct BadStruct *bs = &(struct BadStruct){ .non_existent_field = 1 };
    
    /* Vector operation - might fail on some targets */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Complex nested error */
    result += complex_error_expr();
    
    /* Invalid address operation */
    result += invalid_address();
    
    /* Try overflow builtin */
    result += (int)test_overflow();
    
    return result;
}

/* Additional test cases outside main */
/* Using __builtin_constant_p with invalid expression */
int builtin_constant_test = __builtin_constant_p((void)0);

/* Array with invalid size due to void expression */
char invalid_size_array[sizeof((void)void_func())];
