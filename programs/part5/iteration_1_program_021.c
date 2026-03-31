/* test_expr_error.c - Trigger error_mark_node in expr.cc */

/* 1. Invalid operations on void expressions */
void void_func(void) {}

/* 2. Misusing __builtin_va_arg */
int misuse_va_arg() {
    /* Using va_arg outside variadic context with wrong type */
    __builtin_va_list ap;
    __builtin_va_start(ap, 0);
    /* This may fail during expansion due to type mismatch */
    float f = __builtin_va_arg(ap, float);  /* float vs default promotions */
    __builtin_va_end(ap);
    return 0;
}

/* 3. Malformed compound literals */
struct BadStruct {
    int a;
    int b;
};

/* 4. Complex nested invalid operations */
int nested_invalid(void) {
    /* Invalid void expression inside sizeof */
    int x = sizeof((void)void_func());
    
    /* Invalid void in conditional operator */
    int y = (1 ? (void)0 : 0);
    
    /* Taking address of non-lvalue */
    int *p = &5;
    
    return x + y;
}

/* 5. Target-specific vector operations */
#ifdef __VECTOR_FEATURES__
typedef int v4si __attribute__((vector_size(16)));
v4si vector_test(v4si a, v4si b) {
    /* Complex vector operation that might fail expansion */
    return a + b * a;
}
#endif

/* 6. Transactional Memory constructs (if supported) */
#ifdef __TM_FEATURES__
int tm_test(int *ptr) {
    int result;
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    return result;
}
#endif

/* 7. Overflow builtins with potentially unsupported types */
void overflow_tests(void) {
    int res, overflow;
    
    /* This might fail for certain types/targets */
    overflow = __builtin_add_overflow(1.5, 2.5, &res);
    
    /* Complex type */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    /* This likely fails during expansion */
    overflow = __builtin_add_overflow_p(c1, c2, (_Complex double)0);
}

/* 8. Misaligned pointer operations */
void misaligned_test(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);  /* Not 4-byte aligned */
    
    /* Force assumption of alignment */
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    *aligned = 42;  /* Potential expansion failure */
}

/* 9. Invalid designators in compound literals */
void bad_designators(void) {
    /* Non-existent field */
    struct BadStruct bs = (struct BadStruct){ .non_existent = 1 };
    
    /* Compound literal in non-lvalue context */
    int *ptr = &(int){ .x = 5 };  /* .x doesn't exist for int */
}

/* 10. Complex comma operator with void */
void comma_operator_test(void) {
    /* Comma operator where left side is void */
    int x = (printf("test"), 5);
    
    /* Nested void in comma */
    int y = ((void)0, (void)void_func(), 10);
}

/* 11. __builtin_constant_p with invalid expression */
void builtin_constant_test(void) {
    /* Invalid expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)void_func());
    
    /* Another invalid case */
    is_const = __builtin_constant_p(&5);
}

/* Main function - container for all problematic constructs */
int main(void) {
    /* Force evaluation of various problematic expressions */
    int result = 0;
    
    /* Trigger va_arg misuse */
    result += misuse_va_arg();
    
    /* Nested invalid operations */
    result += nested_invalid();
    
    /* Try compound literal errors */
    bad_designators();
    
    /* Comma operator issues */
    comma_operator_test();
    
    /* Builtin constant issues */
    builtin_constant_test();
    
    /* Overflow tests */
    overflow_tests();
    
    /* Misaligned access */
    misaligned_test();
    
    #ifdef __VECTOR_FEATURES__
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = vector_test(a, b);
    #endif
    
    #ifdef __TM_FEATURES__
    int val = 0;
    tm_test(&val);
    #endif
    
    /* Direct void assignment attempt */
    int z = (void)void_func();
    
    return result;
}
