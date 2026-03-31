/* test_expr_error.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point!
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions for testing */
void void_func(void) {}
int int_func(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_operations(void) {
    /* Direct void assignment - should fail during expansion */
    int x = (void)void_func();
    
    /* Void in comma operator in value context */
    int y = (printf("hello"), 5);
    
    /* Void expression in conditional */
    int z = (void_func() ? 1 : 0);
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_misuse(void) {
    /* Using va_arg outside variadic function context */
    va_list ap;
    int x = __builtin_va_arg(ap, int);
    
    /* Type mismatch with va_arg */
    float f = __builtin_va_arg(ap, float);  /* ap not initialized */
}

/* Variadic function to make some va_arg uses partially valid */
void variadic_func(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    /* This might pass parsing but fail during expansion with wrong type */
    double d = __builtin_va_arg(ap, double);
    
    va_end(ap);
}

/* Test 3: Malformed compound literals */
void test_compound_literals(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S s1 = (struct S){ .non_existent = 1 };
    
    /* Taking address in invalid context */
    int *p = &(int){ 42 };
    
    /* Compound literal with type mismatch */
    int *q = &(float){ 3.14f };
}

/* Test 4: Target-specific expansion failures */
void test_target_specific(void) {
    /* Vector extensions on targets that might not support them */
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Might fail expansion on non-vector targets */
    
    /* Complex type with overflow builtin */
    _Complex double cd1 = 1.0 + 2.0i;
    _Complex double cd2 = 3.0 + 4.0i;
    int overflow;
    
    /* This might fail as overflow builtins may not support complex */
    __builtin_add_overflow(cd1, cd2, &cd1);
}

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
void test_transaction_memory(void) {
    /* Transactional memory construct */
    __transaction_atomic {
        int x = 42;
        x++;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Invalid void expression inside sizeof */
    size_t s1 = sizeof((void)void_func());
    
    /* Invalid void expression as builtin argument */
    int is_const = __builtin_constant_p((void)0);
    
    /* Bit-field address attempt */
    struct BitField { unsigned int bf:4; } bf;
    unsigned int *ptr = &bf.bf;  /* Taking address of bit-field */
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
}

/* Test 7: Invalid switch case ranges (GCC extension) */
void test_switch_ranges(void) {
    int x = 5;
    switch (x) {
        case 1 ... 10:  /* Range case - might have special expansion */
            break;
        case (void)0:   /* Invalid case expression */
            break;
    }
}

/* Test 8: Statement expressions with errors */
void test_statement_exprs(void) {
    /* Statement expression with void result in value context */
    int x = ({ void_func(); });
    
    /* Nested statement expression with type error */
    int y = ({ 
        int a = 5;
        void_func();
        a; 
    });
}

/* Test 9: Invalid pointer arithmetic */
void test_pointer_arithmetic(void) {
    /* Pointer to void arithmetic */
    void *vp = 0;
    vp = vp + 1;  /* Invalid in standard C, GCC might handle specially */
    
    /* Function pointer arithmetic */
    void (*fp)(void) = void_func;
    fp = fp + 1;  /* Invalid */
}

/* Test 10: __builtin_choose_expr with invalid types */
void test_builtin_choose(void) {
    /* Third and fourth operands have incompatible types */
    int x = __builtin_choose_expr(1, 42, (void)0);
    
    /* Type mismatch in constant expression */
    int y = __builtin_choose_expr(0, (void)void_func(), 42);
}

/* Main function - container for all tests */
int main(void) {
    /* Call test functions to ensure code generation */
    test_void_operations();
    test_va_arg_misuse();
    test_compound_literals();
    test_target_specific();
    
#ifdef __GNUC__
    test_transaction_memory();
#endif
    
    test_nested_errors();
    test_switch_ranges();
    test_statement_exprs();
    test_pointer_arithmetic();
    test_builtin_choose();
    
    variadic_func("test %d", 42);
    
    return 0;
}
