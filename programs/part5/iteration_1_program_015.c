/* test_expr_error_mark.c - Test program to trigger error_mark_node in expr.cc */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct bad_struct {
    int valid_field;
};

/* Test 4: Vector extensions on non-vector targets */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory constructs */
#ifdef __TM__
void tm_func(void) {
    __transaction_atomic {
        /* empty transaction */
    }
}
#endif

/* Test 6: Complex nested invalid operations */
int complex_invalid(void) {
    /* This should fail during expansion */
    return (sizeof((void)0)) ? 1 : 0;
}

/* Test 7: Invalid address operations */
struct bitfield_struct {
    unsigned int bitfield : 4;
};

/* Test 8: Misaligned pointer operations */
char* misaligned_ptr(void) {
    char buffer[10];
    return (char*)(((unsigned long)buffer) | 1);  /* Force misalignment */
}

/* Test 9: __builtin_constant_p with invalid expression */
int builtin_constant_invalid(void) {
    return __builtin_constant_p((void)void_func());
}

/* Test 10: Overflow builtins with potentially unsupported types */
#ifdef __SIZEOF_INT128__
typedef unsigned __int128 huge_int;
#else
typedef long double huge_int;  /* Fallback to long double */
#endif

/* Main function containing various error-triggering constructs */
int main(void) {
    int result = 0;
    
    /* 1. Direct void expression in value context */
    /* This should trigger error during expansion */
    int x = (void)void_func();
    
    /* 2. Comma operator with void left side */
    int y = (void_func(), 5);
    
    /* 3. Invalid __builtin_va_arg usage (outside variadic context) */
    va_list ap;
    /* Uninitialized va_list - should cause error */
    int z = __builtin_va_arg(ap, int);
    
    /* 4. Malformed compound literal */
    int *p = &(int){ .non_existent = 1 };  /* Invalid designator */
    
    /* 5. Vector operations (may fail on non-vector targets) */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;  /* Vector addition */
    
    /* 6. Taking address of bit-field (invalid lvalue) */
    struct bitfield_struct bf;
    unsigned int *ptr = &bf.bitfield;  /* Invalid: address of bit-field */
    
    /* 7. Misaligned pointer in strict context */
    char *unaligned = misaligned_ptr();
    char *aligned = __builtin_assume_aligned(unaligned, 4);  /* Should fail */
    
    /* 8. Nested invalid operation in conditional */
    int w = (void_func()) ? 1 : 0;  /* void in condition */
    
    /* 9. Invalid sizeof operand */
    size_t s = sizeof((void)0);
    
    /* 10. Try overflow builtin with potentially unsupported type */
    huge_int a = 0, b = 0, overflow;
    int overflow_result = __builtin_add_overflow(a, b, &overflow);
    
    /* 11. Complex expression with multiple issues */
    int complex = (void_func(), __builtin_va_arg(ap, float), 42);
    
    /* 12. Invalid cast of void expression */
    float f = (float)void_func();
    
    /* 13. Array subscript on void expression */
    char c = ((char*)void_func())[0];
    
    /* 14. Member access on void pointer */
    struct bad_struct *bs = (struct bad_struct*)void_func();
    int field = bs->valid_field;
    
    /* 15. Function call through void expression */
    ((void(*)())void_func())();
    
    return result;
}

/* Additional test cases outside main */

/* Test: Invalid compound literal in file scope (GCC extension) */
static int *global_ptr = &(int){1, 2, 3};  /* Too many initializers */

/* Test: __builtin_choose_expr with invalid types */
int choose_invalid(void) {
    return __builtin_choose_expr(1, (void)0, 42);  /* Type mismatch */
}

/* Test: __builtin_offsetof with invalid member */
size_t invalid_offset = __builtin_offsetof(struct bad_struct, non_existent);

/* Test: __builtin_types_compatible_p with void */
int type_check = __builtin_types_compatible_p(void, int);

/* Test: Statement expression with void result in value context */
int stmt_expr(void) {
    return ({ void_func(); });  /* Statement expression returning void */
}

/* Test: __atomic builtin with invalid type */
int atomic_invalid(void) {
    void *ptr = 0;
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);  /* Invalid pointer type */
}
