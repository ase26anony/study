/* test_expr_error.c
 * Compile with: gcc -O1 -c test_expr_error.c -o test.o
 * Also try: gcc -O3 -fno-tree-ccp -c test_expr_error.c -o test.o
 * And: gcc -O2 -fgnu-tm -c test_expr_error.c -o test.o
 */

/* Invalid void expressions in value contexts */
void void_func(void) {}

/* Vector type that might fail expansion on some targets */
typedef int v4si __attribute__((vector_size(16)));

/* Variadic function for va_arg misuse */
void variadic_func(int n, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, n);
    /* Deliberate type mismatch */
    float f = __builtin_va_arg(ap, float);  /* Error if int passed */
    __builtin_va_end(ap);
}

/* Complex nested invalid expressions */
#define NESTED_ERROR (sizeof((void)0, 5) + __builtin_constant_p((void)void_func()))

/* Transactional memory (if supported) */
#ifdef __GNUC__
int tm_func(int *ptr) {
    int result;
    __transaction_atomic {
        result = *ptr;
    }
    return result;
}
#endif

/* Main function containing various error-triggering constructs */
int main(void) {
    /* 1. Direct void expression in value context */
    int x = (void)void_func();
    
    /* 2. Comma operator with void left side */
    int y = (printf("test"), 5);
    
    /* 3. Misuse of __builtin_va_arg outside variadic context */
    __builtin_va_list fake_ap;
    int z = __builtin_va_arg(fake_ap, int);
    
    /* 4. Invalid compound literal with non-existent field */
    struct S { int a; } *p = &(struct S){ .b = 1 };
    
    /* 5. Compound literal in invalid address context */
    int *q = &(int){10} + 1;
    
    /* 6. Vector operations (may fail on non-vector targets) */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* 7. Overflow builtin with potentially unsupported type */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow;
    __builtin_add_overflow(ld1, ld2, &overflow);
    
    /* 8. Nested invalid expression */
    int w = NESTED_ERROR;
    
    /* 9. Invalid address of bit-field */
    struct BitField { unsigned int bf:4; } bf_struct;
    unsigned int *bf_ptr = &bf_struct.bf;
    
    /* 10. Misaligned pointer in alignment context */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    
    /* 11. Transactional memory (if compiled with -fgnu-tm) */
    int val = 42;
    #ifdef __GNUC__
    int tm_result = tm_func(&val);
    #endif
    
    /* 12. Complex conditional with void expression */
    int r = (1 ? (void)0 : 5);
    
    /* 13. Invalid builtin usage */
    int s = __builtin_ffs((void)0);
    
    /* 14. Taking address of register variable */
    register int reg_var = 10;
    int *reg_ptr = &reg_var;
    
    /* 15. Invalid offsetof usage */
    size_t offset = __builtin_offsetof(struct S, non_existent);
    
    /* 16. __real__ and __imag__ on non-complex types */
    double non_complex = 1.0;
    double real_part = __real__ non_complex;
    
    /* 17. Invalid atomic operation */
    _Atomic int atomic_val;
    int atomic_result = atomic_val + (void)0;
    
    /* 18. Using __builtin_constant_p with side effects */
    int cp_result = __builtin_constant_p((void_func(), 1));
    
    /* 19. Invalid pointer arithmetic with void pointer */
    void *vp = 0;
    void *vp2 = vp + 1;
    
    /* 20. Compound literal with designator for array out of bounds */
    int *arr_ptr = &(int[3]){[5] = 10};
    
    return 0;
}

/* Additional error-triggering constructs outside main */

/* Invalid static initialization with void expression */
static int static_err = (void)0;

/* Invalid array size with void expression */
char error_array[(void)0];

/* Invalid enum value */
enum E {
    ERR_VAL = (void)0
};

/* Invalid function pointer type */
void (*func_ptr)(void) = (void)void_func;

/* Invalid inline assembly constraint */
int asm_error(void) {
    int x;
    __asm__("" : "=r"((void)x));
    return 0;
}

/* Nested statement expression with void */
int stmt_expr(void) {
    return ({ (void)void_func(); 1; });
}

/* Invalid attribute on void expression */
__attribute__((unused)) void attr_err = (void)0;
