/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* Disable some warnings to allow dubious constructs through */
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wpointer-arith"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

/* Test 1: Invalid void expressions in value contexts */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
void misuse_va_arg() {
    /* Using va_arg without proper va_list initialization */
    __builtin_va_list ap;
    /* This should fail during expansion */
    int x = __builtin_va_arg(ap, int);
}

/* Test 3: Malformed compound literals */
struct BadStruct {
    int a;
    int b;
};

/* Test 4: Vector extensions on targets that might not support them */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transactional Memory without proper support */
#ifdef __GNUC__
void tm_test() {
    int x = 0;
    /* __transaction_atomic might fail expansion if TM not supported */
    __transaction_atomic {
        x = 1;
    }
}
#endif

/* Test 6: Complex nested invalid operations */
int nested_invalid() {
    /* sizeof of a void expression */
    return sizeof((void)void_func());
}

/* Test 7: Invalid address operations on bit-fields */
struct BitFieldStruct {
    unsigned int field:4;
};

/* Test 8: Misaligned pointer operations */
void misaligned_test(char *ptr) {
    /* Force misaligned access */
    int *misaligned = (int *)(ptr + 1);
    int x = *misaligned;
}

/* Test 9: Overflow builtins with problematic types */
void overflow_test() {
    int res, overflow;
    /* Using with long double might fail on some targets */
    overflow = __builtin_add_overflow(1.0L, 2.0L, &res);
}

/* Test 10: Invalid designators in compound literals */
void bad_designator() {
    /* Non-existent field */
    int *p = &(struct BadStruct){ .non_existent = 1 };
}

/* Test 11: Comma operator with void in value context */
int comma_void() {
    /* Left side of comma is void, but we're in value context */
    return (void_func(), 5);
}

/* Test 12: __builtin_constant_p with invalid expression */
void builtin_constant_test() {
    /* Should fail expansion */
    int x = __builtin_constant_p((void)0);
}

/* Test 13: Taking address of register variable */
void address_of_register() {
    register int reg_var = 42;
    int *p = &reg_var;  /* Invalid in C, might fail expansion */
}

/* Test 14: Complex expression with multiple issues */
int complex_error() {
    /* Nested invalid operations */
    return (sizeof((void)0)) ? 
           __builtin_va_arg((__builtin_va_list){0}, float) : 
           (int)&(struct BitFieldStruct){ .field = 1 };
}

/* Test 15: Vector operations without proper target support */
void vector_test() {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Might fail expansion on non-vector targets */
}

/* Main function - container for all tests */
int main() {
    /* Trigger various error conditions */
    
    /* Test 1: Direct void in value context */
    int x = (void)void_func();
    
    /* Test 2: va_arg misuse */
    misuse_va_arg();
    
    /* Test 3: Bad compound literal */
    int *p = &(int){ .x = 1 };  /* Invalid designator for int */
    
    /* Test 4: Vector type usage */
    v4si vec = {0};
    
    /* Test 6: Nested invalid */
    int y = nested_invalid();
    
    /* Test 7: Bit-field address */
    struct BitFieldStruct bfs = {0};
    unsigned int *bfp = &bfs.field;  /* Taking address of bit-field */
    
    /* Test 8: Misaligned access */
    char buffer[10];
    misaligned_test(buffer);
    
    /* Test 9: Overflow with wrong types */
    overflow_test();
    
    /* Test 10: Bad designator */
    bad_designator();
    
    /* Test 11: Comma with void */
    int z = comma_void();
    
    /* Test 12: __builtin_constant_p invalid */
    builtin_constant_test();
    
    /* Test 13: Address of register */
    address_of_register();
    
    /* Test 14: Complex error */
    int w = complex_error();
    
    /* Test 15: Vector operation */
    vector_test();
    
    /* Transactional Memory if supported */
    #ifdef __GNUC__
    tm_test();
    #endif
    
    return 0;
}

/* Additional edge cases in global scope */
/* Global variable with invalid initializer */
int global_bad = (void)void_func();

/* Array with invalid size expression */
char bad_array[sizeof((void)0)];

/* Function pointer with incompatible type */
void (*bad_func_ptr)(void) = (void (*)(void))&global_bad;
