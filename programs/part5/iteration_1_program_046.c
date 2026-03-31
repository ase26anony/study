/* test_expr_error.c - Trigger error_mark_node in expr.cc expansion */

/* 1. Invalid operations on void expressions */
void void_func(void) {}

/* 2. Misusing va_arg builtins */
#include <stdarg.h>

/* 3. Malformed compound literals */
struct BadStruct {
    int x;
    int y;
};

/* 4. Target-specific expansion failures */
#ifdef __SIZEOF_INT128__
typedef __int128 large_int;
#else
typedef long long large_int;
#endif

/* 5. Vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Main function containing various error triggers */
int main() {
    /* Trigger 1: Invalid void operations */
    /* These should cause expansion errors */
    int a = (void)void_func();           /* Direct void cast in assignment */
    int b = (printf("test"), 5);         /* Comma operator with void left side */
    
    /* More complex void misuse */
    sizeof((void)0);                     /* sizeof(void) */
    __builtin_constant_p((void)0);       /* Builtin with void argument */
    
    /* Trigger 2: Misusing __builtin_va_arg */
    va_list ap;
    /* Using va_arg without proper initialization */
    int c = __builtin_va_arg(ap, int);
    
    /* Type mismatch in va_arg */
    float d = __builtin_va_arg(ap, float);  /* float vs double promotion issues */
    
    /* Trigger 3: Malformed compound literals */
    /* Invalid designator */
    int *p = &(int){ .non_existent = 1 };
    
    /* Type mismatch in compound literal */
    struct BadStruct *s = &(struct BadStruct){ .x = 1, .z = 2 };  /* Invalid member */
    
    /* Compound literal in invalid context */
    &(int){1} + 5;  /* Taking address and doing arithmetic */
    
    /* Trigger 4: Target-specific failures */
    /* Overflow builtins with potentially unsupported types */
    int overflow;
    __builtin_add_overflow(1.0L, 2.0L, &overflow);  /* long double */
    
    /* Complex type with overflow builtin */
    _Complex double cx = 1.0 + 2.0i;
    _Complex double cy = 3.0 + 4.0i;
    _Complex double cz;
    /* This might fail expansion */
    __builtin_add_overflow(cx, cy, &cz);
    
    /* Trigger 5: Vector operations on non-vector targets */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;  /* Vector addition */
    
    /* Vector extract with invalid index */
    int elem = __builtin_extract(v1, 10);  /* Out of bounds */
    
    /* Trigger 6: Transactional Memory without support */
#ifdef __GNUC__
    __transaction_atomic {
        a = 5;
    }
#endif
    
    /* Trigger 7: Invalid address operations */
    /* Address of bit-field (if we had one) */
    struct {
        unsigned int bit:1;
    } bitfield;
    /* &bitfield.bit; */  /* Would be invalid but caught earlier */
    
    /* Address of register variable */
    register int reg_var = 5;
    /* int *reg_ptr = &reg_var; */  /* Caught earlier */
    
    /* Trigger 8: Misaligned pointer operations */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);  /* Force misalignment */
    
    /* Trigger 9: Nested invalid operations in complex expressions */
    /* Conditional operator with void */
    int e = (1 ? (void)0 : 5);
    
    /* Nested sizeof with void */
    sizeof(sizeof((void)0));
    
    /* Invalid pointer arithmetic */
    void *ptr = 0;
    ptr = (void*)((char*)ptr + (void)0);  /* Adding void to pointer */
    
    /* Trigger 10: Using __builtin_choose_expr with invalid types */
    __builtin_choose_expr(1, (void)0, 5);
    
    return 0;
}

/* Additional test functions for variadic context */
void test_va_arg_errors(va_list ap) {
    /* Using va_arg with wrong type in variadic context */
    short s = __builtin_va_arg(ap, short);  /* short promotes to int */
    
    /* Array type in va_arg */
    int arr[5];
    /* int *arr_ptr = __builtin_va_arg(ap, int[5]); */  /* Array type decay */
    
    /* Function type in va_arg */
    /* void (*func)(void) = __builtin_va_arg(ap, void(void)); */
}

/* Function with invalid return type manipulation */
void* test_invalid_returns() {
    /* Returning address of local compound literal */
    return &(int){42};  /* Address of temporary */
}

/* Test with attribute that might affect expansion */
__attribute__((optimize("O0")))
void optimized_function() {
    /* Mix of errors that might behave differently at O0 */
    int x = (void)void_func();
    __builtin_va_arg((va_list){0}, double);
}

/* Large type that might cause expansion issues */
typedef struct {
    large_int data[1024];
} HugeType;

void test_huge_type() {
    HugeType h;
    /* Operations on huge type might fail during expansion */
    sizeof(h.data[0] = 1);  /* Assignment in sizeof */
}
