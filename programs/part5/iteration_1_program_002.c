/* test_expr_error.c - Trigger error_mark_node in expr.cc */

/* Use various constructs that should pass parsing but fail during RTL expansion */

/* 1. Invalid void expressions in value contexts */
void void_func(void) { }

/* 2. Misusing __builtin_va_arg */
#include <stdarg.h>

/* 3. Malformed compound literals */
struct BadStruct {
    int valid_field;
    /* no non_existent_field */
};

/* 4. Vector extensions (may fail on targets without vector support) */
typedef int v4si __attribute__((vector_size(16)));

/* 5. Transactional Memory (if compiled with -fgnu-tm) */
#ifdef __TM__
int tm_var = 0;
#endif

/* 6. Overflow builtins with potentially unsupported types */
long double ld_val = 1.5;

/* Helper for variadic misuse */
void misuse_va_arg(void) {
    /* This should fail during expansion - using va_arg without proper context */
    va_list ap;
    /* ap is uninitialized! */
    int x = __builtin_va_arg(ap, float);  /* Wrong type, wrong context */
}

int main(void) {
    int result = 0;
    
    /* ===== GROUP 1: Invalid void operations ===== */
    
    /* Direct void assignment - should fail */
    int a = (void)void_func();
    
    /* Void in comma operator in value context */
    int b = (printf("test"), 5);
    
    /* Void in conditional expression */
    int c = (1 ? (void)0 : 0);
    
    /* Nested void in sizeof (GCC might evaluate at compile time) */
    int d = sizeof((void)void_func());
    
    /* ===== GROUP 2: Compound literal errors ===== */
    
    /* Invalid designator */
    int *p = &(int){ .non_existent_field = 1 };
    
    /* Type mismatch in compound literal */
    float *fp = &(int){ 42 };
    
    /* ===== GROUP 3: Builtin misuse ===== */
    
    /* Misuse va_arg outside proper variadic context */
    misuse_va_arg();
    
    /* __builtin_constant_p with void expression */
    int e = __builtin_constant_p((void)0);
    
    /* ===== GROUP 4: Target-specific failures ===== */
    
    /* Vector operations (may fail on non-vector targets) */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Overflow builtin with long double (may be unsupported) */
    long double ld1 = 1.0e100;
    long double ld2 = 1.0e100;
    int overflow = 0;
    __builtin_add_overflow(ld1, ld2, &ld_val);
    
    /* ===== GROUP 5: Complex context errors ===== */
    
    /* Invalid address operations in complex expressions */
    struct BadStruct bs = {0};
    int *bad_ptr = &bs.valid_field + (void)void_func();
    
    /* Bit-field address attempt */
    struct {
        unsigned int bitfield : 4;
    } bf = {0};
    unsigned int *bf_ptr = (unsigned int*)&bf.bitfield;
    
    /* ===== GROUP 6: Transactional Memory ===== */
    #ifdef __TM__
    __transaction_atomic {
        tm_var = 42;
    }
    #endif
    
    /* ===== GROUP 7: Alignment issues ===== */
    
    /* Potentially misaligned access */
    char buffer[10];
    int *aligned_ptr = (int*)__builtin_assume_aligned(buffer + 1, 4);
    *aligned_ptr = 42;
    
    /* ===== GROUP 8: Nested invalid operations ===== */
    
    /* Multiple layers of problematic code */
    int complex = (1 ? 
                  (2 ? (void)void_func() : 0) : 
                  __builtin_va_arg(*(va_list*)0, int));
    
    /* Invalid operation inside array subscript */
    int arr[5] = {0};
    int val = arr[(void)void_func()];
    
    /* ===== GROUP 9: Using __builtin_choose_expr with invalid types ===== */
    
    /* __builtin_choose_expr with void in one branch */
    int chosen = __builtin_choose_expr(1, (void)void_func(), 42);
    
    /* ===== GROUP 10: Statement expressions with void ===== */
    
    /* Statement expression returning void used as value */
    int stmt_expr = ({ void_func(); });
    
    /* Complex statement expression with multiple issues */
    int bad_stmt = ({
        va_list ap;
        int x = __builtin_va_arg(ap, struct BadStruct);
        x;
    });
    
    return 0;
}
