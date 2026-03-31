/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns during expression expansion in GCC's expr.cc
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { printf("void\n"); }
int int_func(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (void_func(), 5);
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_errors(va_list ap) {
    /* Using va_arg outside proper variadic context */
    float f = __builtin_va_arg(ap, float);
    
    /* Type mismatch with actual argument */
    double d = __builtin_va_arg(ap, double);
}

/* Variadic function to provide some context */
void variadic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    /* Incorrect type in va_arg - expecting int but using float */
    float wrong = __builtin_va_arg(ap, float);
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literal_errors(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S s1 = (struct S){ .non_existent = 1 };
    
    /* Taking address in invalid context */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Compound literal with too many initializers */
    int *q = &(int[2]){1, 2, 3};
}

/* Test 4: Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector extensions - may fail on targets without vector support */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Overflow builtins with potentially unsupported types */
    long double ld1 = 1.0e100L;
    long double ld2 = 2.0e100L;
    int overflow;
    
    /* __builtin_add_overflow with long double may fail */
    __builtin_add_overflow(ld1, ld2, &overflow);
}

/* Test 5: Transaction Memory constructs (if supported) */
void test_tm_errors(void) {
    int x = 0;
    
    /* Transactional memory block */
    __transaction_atomic {
        x = 42;
    }
    
    /* Nested transaction */
    __transaction_atomic {
        __transaction_atomic {
            x = 43;
        }
    }
}

/* Test 6: Complex nested invalid operations */
void test_complex_nested_errors(void) {
    /* Void expression inside __builtin_constant_p */
    int a = __builtin_constant_p((void)0);
    
    /* Invalid address operations in complex expressions */
    struct BitField { unsigned int bf:4; } bf;
    
    /* Taking address of bit-field */
    unsigned int *ptr = &bf.bf;
    
    /* Complex expression with multiple issues */
    int x = sizeof((void_func(), 5)) + __builtin_constant_p((void)0);
}

/* Test 7: Misaligned pointer operations */
void test_misaligned_errors(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* Dereference misaligned pointer */
    int value = *aligned;
}

/* Test 8: Invalid switch case ranges (GCC extension) */
void test_switch_errors(void) {
    int x = 5;
    
    switch (x) {
        case 1 ... 10:  /* Valid case range */
            break;
        case 20 ... 15: /* Invalid reversed range */
            break;
        case (void)0:   /* Invalid case expression */
            break;
    }
}

/* Test 9: __builtin_choose_expr with invalid conditions */
void test_choose_expr_errors(void) {
    /* Using void expression as condition */
    int x = __builtin_choose_expr((void)0, 1, 2);
    
    /* Type mismatch in branches */
    int y = __builtin_choose_expr(1, (void)0, 2);
}

/* Test 10: Invalid offsetof usage */
void test_offsetof_errors(void) {
    struct Complex {
        int a;
        void (*func)(void);
        int b;
    };
    
    /* offsetof with bit-field (invalid) */
    struct HasBitfield { int a:4; int b; };
    size_t off1 = __builtin_offsetof(struct HasBitfield, a);
    
    /* offsetof with function pointer dereference attempt */
    size_t off2 = __builtin_offsetof(struct Complex, func());
}

/* Main function - container for all tests */
int main(void) {
    /* Call test functions to ensure code generation */
    test_void_errors();
    
    va_list ap;
    test_va_arg_errors(ap);
    
    variadic_func("test", 42, 3.14);
    
    test_compound_literal_errors();
    test_target_specific_errors();
    test_tm_errors();
    test_complex_nested_errors();
    test_misaligned_errors();
    test_switch_errors();
    test_choose_expr_errors();
    test_offsetof_errors();
    
    return 0;
}
