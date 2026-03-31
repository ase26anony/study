/* test_expr_error.c
 * This program contains various constructs designed to trigger the
 * error_mark_node return path in expr.cc during compilation.
 * The program may not compile successfully - that's the point.
 */

#include <stdarg.h>
#include <stdio.h>

/* Helper functions */
void void_func(void) { printf("void\n"); }
int int_func(void) { return 42; }

/* Test 1: Invalid operations on void expressions */
void test_void_errors(void) {
    /* Direct assignment from void expression - should fail during expansion */
    int x = (void)void_func();  /* Line 14: Invalid cast of void to int */
    
    /* Comma operator with void left side in value context */
    int y = (void_func(), 5);   /* Line 17: void in comma operator */
    
    /* sizeof with void expression argument */
    size_t sz = sizeof((void)0); /* Line 20: sizeof(void) */
    
    /* Conditional operator with void arms */
    int z = 1 ? (void)0 : (void)1; /* Line 23: void in conditional */
}

/* Test 2: Misusing __builtin_va_arg in invalid contexts */
void test_va_arg_errors(va_list ap) {
    /* Using va_arg outside proper variadic context */
    float f = __builtin_va_arg(ap, float); /* Line 29: May fail if ap not initialized */
    
    /* Type mismatch in va_arg */
    double d = __builtin_va_arg(ap, struct {int a;}); /* Line 32: Invalid type */
}

/* Test 3: Malformed compound literals */
void test_compound_literal_errors(void) {
    struct S { int a; int b; };
    
    /* Non-existent field designator */
    struct S s1 = (struct S){ .non_existent = 1 }; /* Line 39: Invalid field */
    
    /* Taking address in invalid context */
    int *p = &(int){ .non_existent = 1 }; /* Line 42: Invalid designator */
    
    /* Compound literal with wrong type */
    float *fp = &(int){5}; /* Line 45: Type mismatch in pointer assignment */
}

/* Test 4: Target-specific expansion failures */
void test_target_specific_errors(void) {
    /* Vector operations on potentially unsupported targets */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2; /* Line 53: Vector ops may fail expansion */
    
    /* Overflow builtins with complex types */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    _Complex double c3;
    /* Line 58: Overflow builtin with complex - likely unsupported */
    int overflow = __builtin_add_overflow(c1, c2, &c3);
}

/* Test 5: Transactional Memory without proper support */
#ifdef __GNUC__
void test_tm_errors(void) {
    /* Transactional memory construct - may fail if TM not supported */
    __transaction_atomic {
        int x = 5;
        x++;
    } /* Line 68: TM may fail during expansion */
}
#endif

/* Test 6: Complex nested invalid operations */
void test_nested_errors(void) {
    /* Nested invalid operations that might only fail during expansion */
    int x = __builtin_constant_p((void)0); /* Line 74: void in builtin */
    
    /* Invalid address operations in complex expressions */
    int y = 5;
    int *ptr = &y + (void_func(), 0); /* Line 78: void in pointer arithmetic */
    
    /* Misaligned pointer operations */
    char buffer[10];
    int *aligned_ptr = __builtin_assume_aligned(buffer, 16); /* Line 82: May fail */
}

/* Test 7: Using __builtin_choose_expr with invalid types */
void test_builtin_choose_errors(void) {
    /* __builtin_choose_expr with void type */
    int x = __builtin_choose_expr(1, (void)0, 5); /* Line 88: void in choose expr */
    
    /* Type mismatch in choose expression */
    int y = __builtin_choose_expr(0, 5, (struct {int a;}){1}); /* Line 91: Struct to int */
}

/* Test 8: Invalid bit-field operations */
void test_bitfield_errors(void) {
    struct Bitfield {
        unsigned int field : 3;
    } bf = {0};
    
    /* Taking address of bit-field */
    unsigned int *ptr = &bf.field; /* Line 100: Address of bit-field */
    
    /* Array of bit-fields (invalid) */
    unsigned int arr[] = {bf.field, bf.field}; /* Line 103: May cause issues */
}

/* Test 9: Using statement expressions in invalid contexts */
void test_statement_expr_errors(void) {
    /* Statement expression returning void in value context */
    int x = ({ void_func(); }); /* Line 110: void statement expr as value */
    
    /* Nested statement expressions with type errors */
    int y = ({ 
        int a = 5;
        (void)a;  /* This makes the block void */
    }); /* Line 116: void block in value context */
}

/* Test 10: __builtin_offsetof with invalid arguments */
void test_offsetof_errors(void) {
    /* offsetof with bit-field */
    struct Invalid {
        int x : 4;
        int y;
    };
    
    size_t off = __builtin_offsetof(struct Invalid, x); /* Line 126: offsetof bit-field */
    
    /* offsetof with array of incomplete type */
    struct Incomplete;
    size_t off2 = __builtin_offsetof(struct Incomplete, field); /* Line 130: incomplete */
}

/* Main function - just calls all test functions */
int main(void) {
    /* These calls may not all compile, but that's intentional */
    test_void_errors();
    
    va_list ap;
    test_va_arg_errors(ap);
    
    test_compound_literal_errors();
    test_target_specific_errors();
    
    #ifdef __GNUC__
    test_tm_errors();
    #endif
    
    test_nested_errors();
    test_builtin_choose_errors();
    test_bitfield_errors();
    test_statement_expr_errors();
    test_offsetof_errors();
    
    return 0;
}
