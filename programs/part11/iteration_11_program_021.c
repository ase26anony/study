/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Volatile arrays for memory operands */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct with volatile members */
struct volatile_floats {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int pattern_unordered(void) {
    volatile double d = g_nan;
    volatile float f = (float)g_nan;
    volatile long double ld = (long double)g_nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (d != d) {
        result |= 1;  /* unordered */
    }
    
    if (isunordered(f, f)) {
        result |= 2;
    }
    
    if (isunordered(ld, ld)) {
        result |= 4;
    }
    
    /* Complex expression with unordered */
    result |= (d != d) ? 8 : 0;
    result |= (isunordered(f, f) && (darr[0] < darr[1])) ? 16 : 0;
    
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int pattern_ordered(void) {
    volatile double d1 = g_one;
    volatile double d2 = g_two;
    volatile float f1 = farr[1];
    volatile float f2 = farr[2];
    int result = 0;
    
    /* Direct ordered checks */
    if (d1 == d1) {
        result |= 1;  /* ordered */
    }
    
    if (isordered(f1, f2)) {
        result |= 2;
    }
    
    /* Ordered with memory operands */
    struct volatile_floats vf = {farr[0], darr[1], ldarr[2]};
    if (vf.d == vf.d && vf.f == vf.f) {
        result |= 4;
    }
    
    /* Ternary with ordered */
    result |= (isordered(vf.ld, vf.ld)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int pattern_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile float d = farr[0];  /* Same value */
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is a == b or unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Another UNEQ pattern */
    if (!(c != d)) {
        result |= 2;
    }
    
    /* Complex UNEQ expression */
    result |= (!(darr[0] != darr[0])) ? 4 : 0;
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int pattern_unge(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile float f1 = farr[1];
    volatile float f2 = farr[2];
    int result = 0;
    
    /* UNGE: !(x < y) */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* With memory operand */
    if (!(f1 < f2)) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(x < y) || (x != x)) ? 4 : 0;
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int pattern_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile long double ld1 = ldarr[2];
    volatile long double ld2 = ldarr[1];
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With long double */
    if (!(ld1 <= ld2)) {
        result |= 2;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int pattern_unle(void) {
    volatile double p = g_one;
    volatile double q = g_two;
    volatile float r = farr[0];
    volatile float s = farr[3];
    int result = 0;
    
    /* UNLE: !(p > q) */
    if (!(p > q)) {
        result |= 1;
    }
    
    /* Another UNLE */
    if (!(r > s)) {
        result |= 2;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int pattern_unlt(void) {
    volatile double u = g_one;
    volatile double v = g_two;
    volatile float w = farr[0];
    volatile float x = farr[1];
    int result = 0;
    
    /* UNLT: !(u >= v) */
    if (!(u >= v)) {
        result |= 1;
    }
    
    /* With array access */
    if (!(w >= x)) {
        result |= 2;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int pattern_ltgt(void) {
    volatile double m = g_one;
    volatile double n = g_two;
    volatile float o = farr[1];
    volatile float p = farr[2];
    int result = 0;
    
    /* LTGT: (m < n) || (m > n) */
    if ((m < n) || (m > n)) {
        result |= 1;
    }
    
    /* Another LTGT */
    if ((o < p) || (o > p)) {
        result |= 2;
    }
    
    /* Complex LTGT with NaN */
    volatile double nan = g_nan;
    result |= ((m < n) || (m > n) || (nan != nan)) ? 4 : 0;
    
    sink(result);
    return result;
}

/* Functions using inline assembly to force condition code usage */
__attribute__((noinline))
int asm_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* Force unordered condition code with inline asm goto */
    if (a != a) {
        __asm__ goto (
            "j%c0 %l0"
            : /* no outputs */
            : "i" (0)  /* condition code */
            : /* no clobbers */
            : taken_label
        );
        return 0;
        
    taken_label:
        result = 1;
    }
    
    sink(result);
    return result;
}

__attribute__((noinline))
int asm_ordered(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    int result = 0;
    
    if (x == x) {
        __asm__ goto (
            "j%c0 %l0"
            : /* no outputs */
            : "i" (0)
            : /* no clobbers */
            : ordered_label
        );
        return 0;
        
    ordered_label:
        result = 1;
    }
    
    sink(result);
    return result;
}

__attribute__((noinline))
int asm_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;  /* Equal values */
    int result = 0;
    
    if (!(a != b)) {
        __asm__ goto (
            "j%c0 %l0"
            : /* no outputs */
            : "i" (0)
            : /* no clobbers */
            : uneq_label
        );
        return 0;
        
    uneq_label:
        result = 1;
    }
    
    sink(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    g_zero = 0.0;
    g_one = 1.0;
    g_two = 2.0;
    
    farr[0] = 0.0f;
    farr[1] = 1.0f;
    farr[2] = 2.0f;
    farr[3] = 3.0f;
    
    darr[0] = 0.0;
    darr[1] = 1.0;
    darr[2] = 2.0;
    darr[3] = 3.0;
    
    ldarr[0] = 0.0L;
    ldarr[1] = 1.0L;
    ldarr[2] = 2.0L;
    ldarr[3] = 3.0L;
    
    /* Call all pattern functions */
    checksum += pattern_unordered();
    checksum += pattern_ordered();
    checksum += pattern_uneq();
    checksum += pattern_unge();
    checksum += pattern_ungt();
    checksum += pattern_unle();
    checksum += pattern_unlt();
    checksum += pattern_ltgt();
    
    /* Call asm functions */
    checksum += asm_unordered();
    checksum += asm_ordered();
    checksum += asm_uneq();
    
    /* Additional complex expressions */
    {
        volatile double x = g_nan;
        volatile double y = g_one;
        volatile double z = g_two;
        
        /* Nested ternary with multiple condition codes */
        int r = (x != x) ? 1 : ((y < z) ? 2 : 3);
        r += (!(y >= z)) ? 4 : 0;
        r += ((y < z) || (y > z)) ? 8 : 0;
        
        checksum += r;
        sink(r);
    }
    
    /* Mixed types in complex expression */
    {
        volatile float fa = farr[0];
        volatile double db = darr[1];
        volatile long double ldc = ldarr[2];
        
        int r = 0;
        r |= (fa != fa) ? 1 : 0;
        r |= (!(db < db)) ? 2 : 0;
        r |= ((ldc < ldc) || (ldc > ldc)) ? 4 : 0;
        r |= (!(fa > fa)) ? 8 : 0;
        
        checksum += r;
        sink(r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int dummy;
    dummy = x;
}

void sink_ptr(void* p) {
    volatile static void* dummy;
    dummy = p;
}
