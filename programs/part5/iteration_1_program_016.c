/* test_expr_error.c - Trigger error_mark_node in expr.cc */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* Test 4: Target-specific expansion failures */
#ifdef __SIZEOF_INT128__
typedef __int128 large_int;
#else
typedef long long large_int;
#endif

/* Test 5: Vector extensions on non-vector targets */
typedef int v4si __attribute__((vector_size(16)));

/* Test 6: Transaction Memory constructs */
#ifdef __GNUC__
#define TM_ATTR __attribute__((transaction_safe))
#else
#define TM_ATTR
#endif

int main(void) {
    /* Test 1a: Using void expression where value is required */
    /* This should trigger error during expansion */
    int x = (void)void_func();
    
    /* Test 1b: Comma operator with void left side in value context */
    int y = (printf("hello"), 5);
    
    /* Test 1c: sizeof on void expression */
    size_t sz = sizeof((void)void_func());
    
    /* Test 1d: Conditional operator with void branch */
    int z = 1 ? (void)0 : 5;
    
    /* Test 1e: Void in arithmetic expression */
    int w = 10 + (void)void_func();
    
    /* Test 2a: Misusing __builtin_va_arg without proper context */
    /* Create a fake va_list - this is invalid but might pass parsing */
    char fake_ap[100];
    float f = __builtin_va_arg((void*)fake_ap, float);
    
    /* Test 2b: Type mismatch in __builtin_va_arg */
    va_list ap;
    va_start(ap, 0);
    double d = __builtin_va_arg(ap, struct BadStruct*);
    va_end(ap);
    
    /* Test 3a: Compound literal with non-existent field */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Test 3b: Compound literal of incomplete type */
    int *q = &(struct IncompleteType){ 1 };
    
    /* Test 3c: Taking address of non-lvalue compound literal in complex context */
    int **r = &(&(int){42});
    
    /* Test 4a: Overflow builtins with potentially unsupported types */
    int overflow;
    /* Try with long double if supported */
    _Bool ov1 = __builtin_add_overflow(1.0L, 2.0L, &overflow);
    
    /* Test 4b: Complex types in overflow builtins */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    _Complex double c3;
    _Bool ov2 = __builtin_add_overflow(c1, c2, &c3);
    
    /* Test 5: Vector operations on potentially unsupported target */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    /* Test 6: Transactional memory in potentially unsupported context */
    int tm_val = 0;
    __transaction_atomic {
        tm_val = 42;
    }
    
    /* Test 7: Nested invalid operations */
    /* Invalid address operation inside builtin */
    int bitfield = 0;
    int *bad_ptr = &(bitfield & 1);  /* Taking address of bitwise operation */
    
    /* Test 8: Misaligned pointer in alignment context */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* Test 9: Invalid __builtin_constant_p usage */
    int cp = __builtin_constant_p((void)void_func());
    
    /* Test 10: Recursive error in complex expression */
    int complex_err = (1 ? (2 ? (void)0 : 3) : 4) + 5;
    
    /* Test 11: Invalid pointer arithmetic with void */
    void *vp = 0;
    vp = vp + (void)void_func();
    
    /* Test 12: Switch on void expression */
    switch ((void)void_func()) {
        default: break;
    }
    
    /* Test 13: Array index with void */
    int arr[10];
    int elem = arr[(void)void_func()];
    
    /* Test 14: Function pointer through void */
    void (*funcptr)(void) = void_func;
    funcptr = (void)void_func;
    
    /* Test 15: Structured binding-like error (C++ style in C) */
    auto structured = (void)void_func();
    
    return 0;
}
