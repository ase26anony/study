/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Test 1: Invalid void expressions in value contexts */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int x;
    int y;
};

/* Test 4: Vector extensions on targets that might not support them */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory without proper support */
#ifdef __GNUC__
#define TM_ATTR __attribute__((transaction_safe))
#else
#define TM_ATTR
#endif

/* Test 6: Overflow builtins with problematic types */
long double problematic_ld = 0.0;

/* Test 7: Complex nested invalid operations */
int* get_ptr(void) { static int x; return &x; }

int main(void) {
    /* Test 1a: Direct void expression as value */
    int x = (void)void_func();  /* Should trigger error during expansion */
    
    /* Test 1b: Void in comma operator in value context */
    int y = (printf("test"), 5);  /* printf returns int, not void - let's fix this */
    /* Actually, printf returns int. Let's use a true void expression: */
    int z = (void_func(), 10);
    
    /* Test 1c: Void in conditional operator */
    int w = (1 ? (void)0 : 0);  /* Type mismatch in branches */
    
    /* Test 2: Misuse of __builtin_va_arg */
    va_list ap;
    /* This is invalid - using va_arg without va_start and with wrong type */
    int va_test = __builtin_va_arg(ap, float);
    
    /* Test 3a: Malformed compound literal with invalid designator */
    int *p = &(int){ .non_existent = 1 };  /* Invalid designator */
    
    /* Test 3b: Compound literal in invalid context */
    /* Taking address of compound literal in non-lvalue context */
    int **pp = &(&(int){42});
    
    /* Test 4: Vector operations - may fail expansion on some targets */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector addition */
    
    /* Test 5: Transaction Memory construct */
    __transaction_atomic {
        x = 42;
    }
    
    /* Test 6: Overflow builtin with long double */
    int overflow;
    __builtin_add_overflow(problematic_ld, problematic_ld, &overflow);
    
    /* Test 7a: Nested invalid in __builtin_constant_p */
    int constant_test = __builtin_constant_p((void)0);
    
    /* Test 7b: Complex invalid address operation */
    struct BitField {
        unsigned int field:3;
    } bf = {0};
    
    /* Taking address of bit-field */
    unsigned int *bf_ptr = &bf.field;
    
    /* Test 7c: Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* Test 8: Invalid __builtin_choose_expr */
    int choose = __builtin_choose_expr(1, (void)0, 0);
    
    /* Test 9: Invalid __builtin_offsetof */
    /* Using offsetof with bit-field */
    size_t offset = __builtin_offsetof(struct BitField, field);
    
    /* Test 10: Invalid atomic operations */
    _Atomic int atomic_var = 0;
    int non_atomic = 0;
    /* Mixing atomic and non-atomic in atomic operation */
    __atomic_store_n(&non_atomic, 42, __ATOMIC_SEQ_CST);
    
    return 0;
}
