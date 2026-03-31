/* test_expr_error.c - Trigger error_mark_node in expr.cc */

/* 1. Invalid void expressions in value contexts */
void void_func(void) {}

/* 2. Misusing va_arg builtins */
#include <stdarg.h>

/* 3. Malformed compound literals */
struct BadStruct {
    int valid_field;
};

/* 4. Target-specific expansion failures */
typedef float v4sf __attribute__((vector_size(16)));

/* 5. Transaction Memory constructs */
#ifdef __TM_FEATURE_AVAILABLE
#error "TM available - might not trigger error"
#endif

/* Helper for variadic abuse */
int misuse_va_arg(va_list ap) {
    /* Wrong type - float instead of double for variadic promotion */
    return (int)__builtin_va_arg(ap, float);
}

/* Complex nested error contexts */
int nested_errors(int x) {
    /* Invalid void in conditional */
    return x ? (void)0, 1 : sizeof((void)void_func());
}

/* Address of non-lvalue in complex context */
int* address_of_bitfield(void) {
    struct {
        unsigned int bitfield : 4;
    } s = {0};
    
    /* Taking address of bit-field - invalid */
    return &s.bitfield;  /* Should fail during expansion */
}

/* Main container for erroneous constructs */
int main(void) {
    int result = 0;
    
    /* Group 1: Basic void expression errors */
    /* These should all trigger expansion failures */
    
    /* 1a: void function as value */
    result = (void)void_func();
    
    /* 1b: void in comma operator */
    result = (printf("test"), 5);
    
    /* 1c: void in sizeof (valid syntax but might cause issues) */
    result = sizeof((void)0);
    
    /* Group 2: Builtin misuse */
    
    /* 2a: va_arg without proper context */
    {
        va_list ap;
        /* Uninitialized va_list with wrong type */
        result += __builtin_va_arg(ap, float);
    }
    
    /* 2b: Overflow builtins with unsupported types */
    {
        _Complex double c1 = 1.0 + 2.0i, c2 = 3.0 + 4.0i;
        _Complex double c3;
        /* Complex overflow check - likely unsupported */
        result += __builtin_add_overflow_p(c1, c2, c3);
    }
    
    /* Group 3: Compound literal errors */
    
    /* 3a: Non-existent field designator */
    int *p = &(struct BadStruct){ .non_existent = 1 };
    
    /* 3b: Compound literal in invalid context */
    &(int){ result };
    
    /* Group 4: Target-specific failures */
    
    /* 4a: Vector operations on potentially unsupported target */
    v4sf a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf c = a + b;  /* Might fail expansion on non-vector targets */
    
    /* 4b: Assume aligned with misaligned pointer */
    char buffer[10];
    int *misaligned = (int*)(buffer + 1);
    int *aligned = __builtin_assume_aligned(misaligned, 16);
    result += *aligned;
    
    /* Group 5: Transaction Memory (if compiled with -fgnu-tm) */
    
    /* 5a: Transaction block */
    __transaction_atomic {
        result++;
    }
    
    /* Group 6: Nested/complex errors */
    
    /* 6a: Invalid void in builtin constant check */
    result += __builtin_constant_p((void)0);
    
    /* 6b: Recursive error in conditional */
    result = nested_errors(result);
    
    /* 6c: Address of bitfield attempt */
    int *ptr = address_of_bitfield();
    result += (ptr != 0);
    
    /* 6d: Invalid pointer arithmetic with void */
    void *vp = &result;
    vp = vp + (void)0;  /* Adding void to pointer */
    
    /* 6e: Using __builtin_choose_expr with invalid types */
    result = __builtin_choose_expr(1, (void)0, 5);
    
    /* Force use of all variables to prevent elimination */
    asm volatile("" : : "r"(result), "r"(p), "r"(c));
    
    return result;
}

/* Additional global scope errors */
/* Invalid static initialization with void expression */
static int global_error = (void)void_func(), 42;

/* Invalid array designator */
int arr[5] = { [6] = 1 };  /* Out of bounds designator */

/* Function with invalid return type expression */
int invalid_return(void) {
    return (void)0;
}

/* Try to trigger errors in function prologue expansion */
void __attribute__((naked)) naked_func(void) {
    /* Naked function with body - might cause expansion issues */
    (void)0;
}

/* Variable with invalid alignment */
int __attribute__((aligned(257))) over_aligned;  /* Non-power-of-2 alignment */
