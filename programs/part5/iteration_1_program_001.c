/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct BadStruct {
    int x;
    int y;
};

/* Test 4: Target-specific expansion failures */
#ifdef __SIZEOF_INT128__
typedef __int128 large_int;
#else
typedef long long large_int;
#endif

/* Test 5: Vector extensions (may fail on non-vector targets) */
typedef int v4si __attribute__((vector_size(16)));

/* Test 6: Transaction Memory (if supported) */
#ifdef __TM__
int tm_var = 0;
#endif

/* Helper for variadic misuse */
void misuse_va_arg(va_list ap) {
    /* This might fail during expansion if ap isn't properly initialized
       or if float doesn't match actual argument */
    float f = __builtin_va_arg(ap, float);
}

int main(void) {
    /* Test 1a: Using void expression where value is required */
    int x = (void)void_func();  /* Direct void cast in assignment */
    
    /* Test 1b: Comma operator with void left side in value context */
    int y = (printf("test"), 5);  /* printf returns int, not void - let's fix */
    int y2 = (void_func(), 10);   /* This has void left side */
    
    /* Test 1c: Void in conditional operator */
    int z = (1 ? (void)0 : 0);  /* Type mismatch in branches */
    
    /* Test 1d: Void in sizeof (might be caught earlier) */
    size_t sz = sizeof((void)0);
    
    /* Test 1e: Void in __builtin_constant_p */
    int is_const = __builtin_constant_p((void)0);
    
    /* Test 2: Various __builtin_va_arg misuses */
    va_list ap;
    /* Uninitialized va_list use - undefined behavior that might fail in expansion */
    int bad1 = __builtin_va_arg(ap, int);
    
    /* Type mismatch - asking for float when int was pushed */
    int bad2 = __builtin_va_arg(ap, float);
    
    /* Test 3: Malformed compound literals */
    /* Non-existent field */
    int *p = &(int){ .non_existent = 1 };
    
    /* Type mismatch in designator */
    struct BadStruct *bs = &(struct BadStruct){ .x = 1, .z = 2 };
    
    /* Compound literal in non-lvalue context with address taken */
    int *q = &(int){1} + 1;  /* Taking address of temporary */
    
    /* Test 4: Target-specific overflow builtins */
    large_int a = 1000, b = 2000, result;
    int overflow = __builtin_add_overflow(a, b, &result);
    
    /* With long double if supported */
    long double ld1 = 1.0, ld2 = 2.0, ld_result;
    int ld_overflow = __builtin_add_overflow(ld1, ld2, &ld_result);
    
    /* Test 5: Vector operations (may fail on targets without vector support) */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Vector extract in non-constant position */
    int elem = __builtin_extract(v1, 5);  /* Out of bounds */
    
    /* Test 6: Transaction Memory constructs */
    #ifdef __TM__
    __transaction_atomic {
        tm_var++;
    }
    
    /* Nested transaction - might fail expansion */
    __transaction_atomic {
        __transaction_atomic {
            tm_var--;
        }
    }
    #endif
    
    /* Test 7: Complex nested invalid expressions */
    /* Invalid address operations */
    int bitfield : 4;
    int *bad_ptr = &bitfield;  /* Address of bit-field */
    
    /* Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)&buffer[1];
    int *aligned = __builtin_assume_aligned(misaligned, 4);
    
    /* Test 8: Using __builtin_choose_expr with incompatible types */
    int choice = __builtin_choose_expr(1, (void)0, 0);
    
    /* Test 9: Invalid builtin combinations */
    /* __builtin_constant_p with side effects */
    int weird = __builtin_constant_p(({ void_func(); 0; }));
    
    /* Test 10: Recursive error propagation */
    /* Nested invalid operations */
    int nested = sizeof(__builtin_va_arg(ap, struct {int a:1; int b:31;}));
    
    return 0;
}

/* Additional test cases outside main to increase coverage */
/* Invalid static initialization */
static int static_bad = (void)void_func();

/* Invalid global compound literal */
static int *global_bad = &(int){ .x = 1 };  /* Invalid designator for int */

/* Function with invalid return type expression */
int invalid_return(void) {
    return (void)void_func();
}

/* Array with invalid size expression */
char bad_array[sizeof((void)0)];

/* Switch case with invalid expression */
void test_switch(int val) {
    switch(val) {
        case (void)0:  /* Invalid case expression */
            break;
    }
}

/* Inline assembly with invalid constraints (might fail earlier) */
void bad_asm(void) {
    __asm__("" : "=r"((void)0));
}

/* Using __builtin_offsetof with invalid type */
size_t bad_offset = __builtin_offsetof(struct BadStruct, z);

/* Invalid attribute usage */
int __attribute__((vector_size(8))) not_vector;  /* Non-power-of-2 might fail */
