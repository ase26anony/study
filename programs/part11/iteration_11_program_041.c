/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Volatile arrays for memory operand testing */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[2] = {0.0L, 1.0L};

/* Struct with volatile members */
struct volatile_floats {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct volatile_floats vf = {0.0f, 0.0, 0.0L};

/* ========== Pattern 1: UNORDERED/ORDERED ========== */
__attribute__((noinline))
int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[1];
    
    int result = 0;
    
    /* UNORDERED patterns */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    if (a != a) {  /* NaN != NaN -> unordered */
        result |= 2;
    }
    
    /* ORDERED patterns */
    if (isordered(b, c)) {
        result |= 4;
    }
    
    if (b == b) {  /* Non-NaN == itself -> ordered */
        result |= 8;
    }
    
    /* Mixed ordered/unordered with memory operands */
    if (isunordered(darr[0], darr[1]) || isordered(farr[2], farr[3])) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* ========== Pattern 2: UNEQ (unordered or equal) ========== */
__attribute__((noinline))
int test_uneq(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_one;
    
    int result = 0;
    
    /* UNEQ: !(a > b) && !(a < b) */
    if (!(x > y) && !(x < y)) {  /* NaN compared to number */
        result |= 1;
    }
    
    /* Using volatile struct member */
    if (!(vf.d > darr[1]) && !(vf.d < darr[1])) {
        result |= 2;
    }
    
    /* Complex expression with ternary */
    result += (isunordered(x, y) || x == y) ? 4 : 0;
    
    sink(result);
    return result;
}

/* ========== Pattern 3: UNGE (not less than) ========== */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile float c = farr[1];
    volatile float d = farr[2];
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With memory operand */
    if (!(darr[0] < darr[3])) {
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "jnge %l0\n\t"
        : : : "cc", "st" : not_taken
    );
    
    result |= 4;
    goto after_label;
    
not_taken:
    result |= 8;
    
after_label:
    sink(result);
    return result;
}

/* ========== Pattern 4: UNGT (not less than or equal) ========== */
__attribute__((noinline))
int test_ungt(void) {
    volatile double x = g_two;
    volatile double y = g_one;
    volatile long double z = ldarr[0];
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(x <= y)) {
        result |= 1;
    }
    
    /* Complex expression */
    if ((x > y) || isunordered(x, y)) {
        result |= 2;
    }
    
    /* Inline assembly with condition code */
    volatile double m = darr[2];
    volatile double n = darr[1];
    
    __asm__ goto (
        "comisd %1, %0\n\t"
        "jnle %l0\n\t"
        : : "x"(m), "x"(n) : "cc" : gt_taken
    );
    
    result |= 4;
    goto after_gt;
    
gt_taken:
    result |= 8;
    
after_gt:
    sink(result);
    return result;
}

/* ========== Pattern 5: UNLE (unordered or less than or equal) ========== */
__attribute__((noinline))
int test_unle(void) {
    volatile float a = farr[0];
    volatile float b = farr[3];
    volatile double c = g_one;
    volatile double d = g_two;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(g_nan > c)) {
        result |= 2;
    }
    
    /* Nested in ternary */
    result += (!(darr[1] > darr[2])) ? 4 : 0;
    
    sink(result);
    return result;
}

/* ========== Pattern 6: UNLT (unordered or less than) ========== */
__attribute__((noinline))
int test_unlt(void) {
    volatile double p = g_zero;
    volatile double q = g_one;
    volatile long double r = ldarr[0];
    volatile long double s = ldarr[1];
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(p >= q)) {
        result |= 1;
    }
    
    /* With memory operand from struct */
    if (!(vf.d >= darr[1])) {
        result |= 2;
    }
    
    /* Inline assembly forcing condition code */
    __asm__ goto (
        "fcomip %%st(1), %%st\n\t"
        "jnlt %l0\n\t"
        : : : "cc", "st" : lt_not_taken
    );
    
    result |= 4;
    goto after_lt;
    
lt_not_taken:
    result |= 8;
    
after_lt:
    sink(result);
    return result;
}

/* ========== Pattern 7: LTGT (less than or greater than) ========== */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double u = g_one;
    volatile double v = g_two;
    volatile float w = farr[2];
    volatile float x = farr[1];
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((u < v) || (u > v)) {
        result |= 1;
    }
    
    /* With NaN (should be false for NaN) */
    if ((g_nan < u) || (g_nan > u)) {
        result |= 2;
    }
    
    /* Complex expression with memory */
    result += ((darr[0] < darr[3]) || (darr[0] > darr[3])) ? 4 : 0;
    
    /* Inline assembly to force condition code materialization */
    volatile double m1 = darr[1];
    volatile double m2 = darr[2];
    
    __asm__ goto (
        "ucomisd %1, %0\n\t"
        "jne %l0\n\t"
        : : "x"(m1), "x"(m2) : "cc" : ne_taken
    );
    
    result |= 8;
    goto after_ne;
    
ne_taken:
    result |= 16;
    
after_ne:
    sink(result);
    return result;
}

/* ========== Pattern 8: Mixed condition codes in complex expression ========== */
__attribute__((noinline))
int test_mixed_complex(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile float d = farr[0];
    volatile float e = farr[3];
    
    int result = 0;
    
    /* Complex nested ternary with multiple condition codes */
    result = (a != a) ? 1 : 
             ((b < c) ? 2 : 
             ((!(d > e)) ? 3 : 
             ((isunordered(b, a) || isordered(c, b)) ? 4 : 5)));
    
    /* Logical AND/OR combination */
    if ((!(a >= b)) && ((c < b) || (c > b))) {
        result += 10;
    }
    
    /* Memory operands with array indexing */
    for (int i = 0; i < 3; i++) {
        if (!(darr[i] >= darr[i+1]) && !isunordered(darr[i], darr[i+1])) {
            result += (1 << i);
        }
    }
    
    sink(result);
    return result;
}

/* ========== Pattern 9: Function pointer to force codegen ========== */
typedef int (*test_func_t)(void);

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    sink_ptr((void*)&seed);
    
    /* Call each pattern function */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_complex();
    
    /* Array of function pointers to prevent dead code elimination */
    test_func_t funcs[] = {
        test_unordered_ordered,
        test_uneq,
        test_unge,
        test_ungt,
        test_unle,
        test_unlt,
        test_ltgt,
        test_mixed_complex
    };
    
    /* Call through function pointers */
    for (int i = 0; i < 8; i++) {
        checksum ^= funcs[i]();
    }
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int sink_var;
    sink_var = x;
}

void sink_ptr(void* p) {
    volatile static void* sink_ptr_var;
    sink_ptr_var = p;
}
