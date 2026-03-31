/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during RTL expansion.
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
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void in conditional expression */
    int z = (1 ? (void)0 : 0);
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_misuse(void) {
    va_list ap;
    
    /* Using va_arg outside variadic function context */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch with promoted types */
    float f = __builtin_va_arg(ap, float);  /* float not promoted */
    
    /* With completely bogus type */
    struct bogus { int a; } b = __builtin_va_arg(ap, struct bogus);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    /* Invalid designator */
    int *p = &(int){ .non_existent = 1 };
    
    /* Taking address in invalid context */
    int (*fp)(void) = &(int){ 42 };  /* Type mismatch */
    
    /* Compound literal with bit-field designator (invalid) */
    struct S { int a:4; int b:4; } s;
    struct S *sp = &(struct S){ .a = 1, .b = 2 };
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions on targets that might not support them */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Might fail expansion on non-vector targets */
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    int overflow = __builtin_add_overflow_p(cd1, cd2, (_Complex double)0);
    
    /* Long double overflow check */
    long double ld1 = 1e1000L;
    long double ld2 = 1e1000L;
    int ld_overflow = __builtin_add_overflow_p(ld1, ld2, (long double)0);
}

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    int x = 0;
    
    /* Transactional memory construct */
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
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_invalid(void) {
    /* Invalid void expression inside __builtin_constant_p */
    int a = __builtin_constant_p((void)0);
    
    /* Invalid address operation in sizeof */
    int b = sizeof(&(int[]){1, 2, 3}[printf("test")]);
    
    /* Bit-field address attempt in complex expression */
    struct BitField { unsigned int flag:1; } bf;
    unsigned int *ptr = &bf.flag + 1;  /* Taking address of bit-field */
    
    /* Misaligned pointer with __builtin_assume_aligned */
    char buffer[10];
    int *aligned_ptr = __builtin_assume_aligned(buffer + 1, 4);
}

/* Test 7: Invalid switch case ranges (GCC extension) */
void test_switch_ranges(void) {
    int x = 5;
    
    switch (x) {
        case 1 ... 10:  /* Case range - might have expansion issues */
            break;
        case (void)0 ... 10:  /* Invalid range with void expression */
            break;
    }
}

/* Test 8: Invalid assembly operands */
void test_inline_assembly(void) {
    int x;
    
    /* Invalid constraint */
    __asm__("mov %0, %1" : "=r"(x) : "r"((void)void_func()));
    
    /* Mismatched operand types */
    __asm__("nop" : "=r"(x) : "r"(void_func()));
}

/* Test 9: __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Choose expression where both branches have incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Choose with __builtin_va_arg */
    va_list ap;
    int y = __builtin_choose_expr(0, 42, __builtin_va_arg(ap, struct {int a;}));
}

/* Test 10: Offsetof with invalid types */
#include <stddef.h>
void test_offsetof_issues(void) {
    /* Offsetof with bit-field (invalid) */
    size_t off = offsetof(struct { int a:4; int b; }, b);
    
    /* Offsetof with void type */
    size_t off2 = offsetof(void, nothing);
}

/* Main function - just contains all tests */
int main(void) {
    /* These calls won't actually execute if compilation fails */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    #ifdef __GNUC__
    test_transaction_memory();
    #endif
    test_nested_invalid();
    test_switch_ranges();
    test_inline_assembly();
    test_builtin_choose();
    test_offsetof_issues();
    
    return 0;
}
