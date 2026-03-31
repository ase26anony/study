/* test_expr_error_mark.c
 * This program contains various constructs designed to trigger
 * the error_mark_node return path in expr.cc during RTL expansion.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) {}
int returns_int(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void function as conditional expression */
    int z = void_func() ? 1 : 0;
}

/* Test 2: Misusing __builtin_va_arg */
void test_va_arg_misuse(va_list ap) {
    /* Using va_arg outside proper variadic context */
    float f = __builtin_va_arg(ap, float);
    
    /* Type mismatch with actual argument */
    double d = __builtin_va_arg(ap, double);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S *p1 = &(struct S){ .non_existent = 1 };
    
    /* Taking address in invalid context */
    int *p2 = &(int){ 42 } + 1;
    
    /* Compound literal with wrong type */
    float *p3 = &(int){ 42 };
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported target */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    int overflow = __builtin_add_overflow(cd1, cd2, &cd3);
    
    /* Long double overflow check */
    long double ld1 = 1.0e1000L;
    long double ld2 = 2.0e1000L;
    long double ld3;
    overflow = __builtin_add_overflow(ld1, ld2, &ld3);
}

/* Test 5: Transactional Memory (if supported) */
void test_transactional_memory(void) {
    /* Transactional memory construct */
    __transaction_atomic {
        int x = 42;
        x++;
    }
}

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Void expression inside __builtin_constant_p */
    int a = __builtin_constant_p((void)0);
    
    /* Invalid address operations in complex expressions */
    struct BitField { unsigned int field:3; } bf = {0};
    int *ptr = &bf.field;  /* Address of bit-field */
    
    /* Misaligned pointer with __builtin_assume_aligned */
    char buffer[10];
    int *aligned_ptr = __builtin_assume_aligned(buffer + 1, 4);
    
    /* Void expression in sizeof operand */
    size_t s = sizeof(void_func());
    
    /* Nested conditional with void */
    int x = (void_func(), 1) ? (void)0 : (void)0;
}

/* Test 7: Invalid pointer arithmetic */
void test_pointer_arithmetic(void) {
    /* Pointer to void arithmetic */
    void *vp = 0;
    vp = vp + 1;
    
    /* Function pointer arithmetic */
    void (*fp)(void) = void_func;
    fp = fp + 1;
}

/* Test 8: Using __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Second and third operands have incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Both operands are void but in value context */
    int y = __builtin_choose_expr(0, void_func(), void_func());
}

/* Test 9: Invalid uses of offsetof */
void test_offsetof_errors(void) {
    /* offsetof with bit-field (invalid) */
    struct BadStruct {
        int normal;
        int bitfield:4;
    };
    
    size_t off = __builtin_offsetof(struct BadStruct, bitfield);
    
    /* offsetof with non-existent member */
    off = __builtin_offsetof(struct BadStruct, non_existent);
}

/* Test 10: Register variable in invalid context */
void test_register_vars(void) {
    /* Taking address of register variable */
    register int reg_var = 42;
    int *ptr = &reg_var;
    
    /* Register array (invalid in C) */
    register int reg_array[10];
}

/* Main function - container for all tests */
int main(void) {
    /* Call various test functions to trigger errors */
    test_void_operations();
    
    va_list ap;
    test_va_arg_misuse(ap);
    
    test_compound_literals();
    test_target_specific();
    test_transactional_memory();
    test_nested_errors();
    test_pointer_arithmetic();
    test_builtin_choose();
    test_offsetof_errors();
    test_register_vars();
    
    return 0;
}
