/* test_expr_error.c
 * This program contains various constructs designed to trigger
 * error_mark_node returns in expr.cc during compilation.
 * The program may not compile successfully - that's the point.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for various test cases */
void void_func(void) { printf("void\n"); }
int int_func(int x) { return x; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct assignment of void expression - should fail during expansion */
    int x = (void)void_func();  /* Line 15 */
    
    /* Comma operator with void left side in value context */
    int y = (void_func(), 5);   /* Line 18 */
    
    /* sizeof with void expression argument */
    size_t s = sizeof((void)0); /* Line 21 */
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_misuse(void) {
    va_list ap;
    
    /* Using __builtin_va_arg outside variadic function context */
    int x = __builtin_va_arg(ap, int);  /* Line 29 */
    
    /* Type mismatch in __builtin_va_arg */
    float f = __builtin_va_arg(ap, float);  /* Line 32 */
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Invalid designator in compound literal */
    int *p1 = &(int){ .non_existent = 1 };  /* Line 40 */
    
    /* Type mismatch in compound literal address */
    struct S *p2 = &(int){42};  /* Line 43 */
    
    /* Compound literal in non-lvalue context */
    &(int){42} = 0;  /* Line 46 */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector operations on potentially unsupported target */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Line 55 */
    
    /* Overflow builtins with complex types */
    _Complex double z1 = 1.0 + 2.0i;
    _Complex double z2 = 3.0 + 4.0i;
    int overflow = __builtin_add_overflow_p(z1, z2, (_Complex double)0);  /* Line 60 */
}

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    int x = 0;
    
    /* Transactional memory construct */
    __transaction_atomic {  /* Line 69 */
        x = 42;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside __builtin_constant_p */
    int c = __builtin_constant_p((void)0);  /* Line 77 */
    
    /* Conditional operator with void expression */
    int d = (1 ? (void)0 : 5);  /* Line 80 */
    
    /* Nested invalid address operations */
    int e = 5;
    int *f = &(e++);  /* Line 84 - address of non-lvalue */
}

/* Test 7: Misaligned pointer operations */
void test_misaligned(void) {
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    
    /* Force alignment assumption on misaligned pointer */
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* Line 93 */
    *aligned = 42;
}

/* Test 8: Invalid bit-field operations */
void test_bitfields(void) {
    struct BitField {
        unsigned int field : 4;
    } bf = {0};
    
    /* Taking address of bit-field */
    unsigned int *addr = &bf.field;  /* Line 103 */
}

/* Test 9: Invalid builtin usage in complex expressions */
void test_complex_builtins(void) {
    /* __builtin_choose_expr with void type */
    int x = __builtin_choose_expr(1, (void)0, 5);  /* Line 110 */
    
    /* __builtin_types_compatible_p with void */
    int y = __builtin_types_compatible_p(void, int);  /* Line 113 */
}

/* Test 10: Recursive error propagation */
void test_recursive_errors(void) {
    /* Multiple levels of invalid operations */
    int x = sizeof((void)((1 ? (void)0 : printf("test"))));  /* Line 120 */
    
    /* Invalid operation inside array size */
    int arr[sizeof((void)0)];  /* Line 123 */
}

/* Main function - container for all test cases */
int main(void) {
    /* Call all test functions to ensure their code is processed */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_misaligned();
    test_bitfields();
    test_complex_builtins();
    test_recursive_errors();
    
    return 0;
}
