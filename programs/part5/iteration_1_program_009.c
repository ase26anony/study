/* test_expr_error.c - Test program to trigger error_mark_node in expr.cc */

/* Disable some warnings to allow dubious constructs through */
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wpointer-arith"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

/* Test 1: Invalid operations on void expressions */
void void_func(void) {}

/* Test 2: Misusing __builtin_va_arg */
#include <stdarg.h>

/* Test 3: Malformed compound literals */
struct bad_struct {
    int valid_field;
    /* no non_existent_field */
};

/* Test 4: Vector extensions (may fail on some targets) */
typedef int v4si __attribute__((vector_size(16)));

/* Test 5: Transaction Memory (if supported) */
#ifdef __TM_FENCE__
#define USE_TM 1
#else
#define USE_TM 0
#endif

/* Test 6: Overflow builtins with potentially unsupported types */
#ifdef __SIZEOF_INT128__
typedef __int128 huge_int;
#else
typedef long long huge_int;
#endif

/* Helper function for variadic misuse */
void misuse_va_arg(void) {
    /* This is invalid - using va_arg without proper va_start/va_end */
    va_list ap;
    /* ap is uninitialized! */
    int x = __builtin_va_arg(ap, float);  /* Wrong type, uninitialized ap */
}

/* Complex nested invalid expression */
#define NESTED_ERROR(expr) sizeof((void)(expr), 0)

int main(void) {
    int result = 0;
    
    /* ===== Test Group 1: Direct void expression misuse ===== */
    
    /* 1a: Assigning void expression to variable */
    /* int x = (void)void_func(); */  /* Might be caught early */
    
    /* 1b: Using void in comma operator in value context */
    int y = (void_func(), 5);  /* Left side is void */
    
    /* 1c: Void in conditional operator */
    int z = (1 ? (void)0 : 0);  /* Type mismatch in branches */
    
    /* 1d: Void in sizeof with side effects */
    result += sizeof((void)printf("test"));
    
    /* ===== Test Group 2: __builtin_va_arg misuse ===== */
    
    /* 2a: Using va_arg outside proper variadic context */
    va_list ap;
    /* Don't initialize - keep it potentially uninitialized */
    /* float f = __builtin_va_arg(ap, float); */  /* Uninitialized ap */
    
    /* 2b: Type mismatch in va_arg */
    /* This might need to be in a variadic function to get past parsing */
    
    /* ===== Test Group 3: Malformed compound literals ===== */
    
    /* 3a: Non-existent field designator */
    int *p = &(int){ /* .non_existent = */ 1 };  /* Actually this is OK for int */
    
    /* 3b: For struct with wrong field */
    struct bad_struct *bp = &(struct bad_struct){ 
        .valid_field = 1,
        /* .non_existent_field = 2 */  /* Would be error */
    };
    
    /* 3c: Taking address in invalid context */
    /* &(int){1} + 1; */  /* Might be valid */
    
    /* ===== Test Group 4: Target-specific failures ===== */
    
    /* 4a: Vector operations (may fail expansion on some targets) */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;  /* Vector addition */
    
    /* 4b: Vector extract with possibly invalid index */
    int elem = vec1[5];  /* Out of bounds index */
    
    /* ===== Test Group 5: Transaction Memory ===== */
    
    /* 5a: TM atomic block (if TM not supported) */
#if USE_TM
    __transaction_atomic {
        result++;
    }
#endif
    
    /* ===== Test Group 6: Overflow builtins ===== */
    
    /* 6a: Overflow with long double (if unsupported) */
    long double ld1 = 1.0e100L;
    long double ld2 = 1.0e100L;
    int overflow_flag;
    /* __builtin_add_overflow(ld1, ld2, &ld1); */  /* Might not support long double */
    
    /* 6b: Overflow with complex types */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    /* __builtin_add_overflow(c1, c2, &c1); */  /* Complex might not be supported */
    
    /* ===== Test Group 7: Complex nested errors ===== */
    
    /* 7a: Nested void in builtin */
    int is_const = __builtin_constant_p((void)0);
    
    /* 7b: Invalid address of bit-field */
    struct bitfield {
        unsigned int field: 4;
    } bf = {0};
    
    /* unsigned int *bitptr = &bf.field; */  /* Can't take address of bit-field */
    
    /* 7c: Misaligned pointer in alignment context */
    char buffer[100];
    int *misaligned = (int*)(buffer + 1);  /* Potentially misaligned */
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* Assume wrong alignment */
    
    /* 7d: Recursive error in conditional */
    int bad = (1 ? (void)0 : (void)0);  /* Both branches void */
    
    /* 7e: Void in switch expression */
    /* switch ((void)0) { case 0: break; } */  /* Invalid switch expression */
    
    /* ===== Test Group 8: Aggressive optimization triggers ===== */
    
    /* 8a: Large expression that might fail during expansion */
    result += (int)((void)(result++), (void)(result++), result);
    
    /* 8b: Pointer arithmetic on void pointer */
    void *vp = &result;
    /* vp = vp + 1; */  /* Invalid in standard C, GNU extension allows with -Wpointer-arith */
    
    /* 8c: Using __builtin_choose_expr with incompatible types */
    int choice = __builtin_choose_expr(1, (void)0, 0);
    
    /* Force use of variables to avoid elimination */
    asm volatile("" : : "r"(y), "r"(z), "r"(vec3), "r"(elem), "r"(aligned));
    
    return result;
}

/* Additional test: Variadic function with type mismatch */
void variadic_test(int n, ...) {
    va_list ap;
    va_start(ap, n);
    
    /* Deliberate type mismatch */
    float f = __builtin_va_arg(ap, double);  /* float vs double mismatch */
    
    va_end(ap);
}

/* Test with attribute that might affect expansion */
__attribute__((optimize("O0")))
void optimized_differently(void) {
    /* Try to create error that only appears at certain optimization levels */
    int x = (void_func(), (void)0, 1);
}

/* Test with statement expressions containing void */
#define VOID_STMT_EXPR ({ void_func(); (void)0; })
int use_stmt_expr = VOID_STMT_EXPR;  /* Statement expression returning void in int context */
