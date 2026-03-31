/* test_expr_error.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during compilation.
 * The program may not compile successfully - that's the point.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for various test cases */
void void_func(void) { }
int returns_int(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment from void expression - should fail during expansion */
    int x = (void)void_func();
    
    /* Void expression in conditional context */
    int y = (void_func(), 5) ? 1 : 0;
    
    /* Void expression as function argument */
    printf("%d\n", (void)void_func());
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_misuse(void) {
    va_list ap;
    
    /* Using va_arg outside variadic function context */
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch with promoted types */
    float f = __builtin_va_arg(ap, float);  /* float promotes to double in varargs */
    
    /* Using with completely wrong type */
    struct S { int a; } s = __builtin_va_arg(ap, struct S);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    /* Invalid designator for scalar type */
    int *p = &(int){ .non_existent = 1 };
    
    /* Taking address of compound literal in invalid context */
    int (*func_ptr)(void) = &(int){ returns_int() };
    
    /* Compound literal with wrong type in pointer context */
    char *cp = (char *)&(int){ 42 };
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b * a;
    
    /* Overflow builtins with complex types */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    _Complex double cd3;
    int overflow = __builtin_add_overflow(cd1, cd2, &cd3);
    
    /* Long double operations that might be unsupported */
    long double ld1 = 1.0L;
    long double ld2 = 2.0L;
    int ovf = __builtin_mul_overflow(ld1, ld2, &ld1);
}

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    /* TM constructs that might fail during expansion */
    __transaction_atomic {
        int x = 42;
        x++;
    }
    
    __transaction_atomic __transaction_relaxed {
        printf("In transaction\n");
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside __builtin_constant_p */
    int is_const = __builtin_constant_p((void)0);
    
    /* Nested invalid operations in conditional */
    int x = (__builtin_va_arg((va_list){0}, int)) ? 1 : 0;
    
    /* Invalid address operation in arithmetic */
    int *ptr = &(returns_int() + 5);  /* Taking address of rvalue */
    
    /* Bit-field address attempt */
    struct BitField {
        unsigned int field:4;
    } bf = {0};
    unsigned int *bf_ptr = &bf.field;  /* Address of bit-field */
}

/* Test 7: Misaligned pointer operations */
void test_misaligned_pointers(void) {
    char buffer[10] = {0};
    int *misaligned = (int *)(buffer + 1);  /* Potentially misaligned */
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* Dereference misaligned pointer in complex expression */
    int value = *aligned + *(aligned + 1);
}

/* Test 8: Invalid operations in sizeof context */
void test_sizeof_errors(void) {
    /* sizeof on void expression */
    size_t s1 = sizeof((void)void_func());
    
    /* sizeof on invalid compound literal */
    size_t s2 = sizeof((int){ .invalid = 1 });
    
    /* sizeof on va_arg misuse */
    size_t s3 = sizeof(__builtin_va_arg((va_list){0}, struct { int a; }));
}

/* Test 9: Using __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Choose expression with incompatible types */
    int x = __builtin_choose_expr(1, (void)0, 42);
    
    /* Nested choose with invalid operations */
    int y = __builtin_choose_expr(0, 1, __builtin_va_arg((va_list){0}, float));
}

/* Test 10: Invalid pointer conversions and arithmetic */
void test_pointer_errors(void) {
    /* Function pointer to data pointer conversion */
    void (*func)(void) = void_func;
    int *ip = (int *)func;
    
    /* Invalid pointer arithmetic with function pointer */
    void (*func2)(void) = func + 1;
    
    /* Dereferencing pointer to incomplete type */
    struct Incomplete *inc_ptr = 0;
    int val = inc_ptr->field;
}

/* Main function - container for all test cases */
int main(void) {
    /* Call test functions to trigger compilation errors */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_misaligned_pointers();
    test_sizeof_errors();
    test_builtin_choose();
    test_pointer_errors();
    
    return 0;
}
