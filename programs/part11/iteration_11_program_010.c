/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile double g_dnan = 0.0/0.0;
volatile double g_dinf = 1.0/0.0;
volatile double g_dneg = -1.0;
volatile double g_dpos = 1.0;
volatile float g_fnan = 0.0f/0.0f;
volatile float g_finf = 1.0f/0.0f;
volatile long double g_ldnan = 0.0L/0.0L;

/* Volatile arrays for memory operands */
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile long double ldarr[2] = {0.0L, 1.0L};

/* Struct with volatile members for complex addressing */
struct FloatPair {
    volatile double a;
    volatile double b;
};
volatile struct FloatPair fp = {0.0, 1.0};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int pattern_unordered(void) {
    volatile double d1 = g_dnan;
    volatile double d2 = g_dpos;
    volatile float f1 = g_fnan;
    int result = 0;
    
    /* Using isunordered() */
    if (isunordered(d1, d2)) {
        result |= 1;
    }
    
    /* Direct NaN comparison */
    if (d1 != d1) {  /* Should be true for NaN */
        result |= 2;
    }
    
    /* Complex expression with unordered */
    result |= (isunordered(f1, f1) && (d2 == d2)) ? 4 : 0;
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : unordered_label
    );
    
    result |= 8;
    goto skip_unordered;
    
unordered_label:
    result |= 16;
    
skip_unordered:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int pattern_ordered(void) {
    volatile double d1 = g_dpos;
    volatile double d2 = g_dneg;
    volatile float f1 = g_finf;
    int result = 0;
    
    /* Using isordered() */
    if (isordered(d1, d2)) {
        result |= 1;
    }
    
    /* Direct ordered check */
    if (d1 == d1 && d2 == d2) {  /* Both non-NaN */
        result |= 2;
    }
    
    /* Complex expression */
    result |= (isordered(f1, f1) || (d1 < d2)) ? 4 : 0;
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ordered_label
    );
    
    result |= 8;
    goto skip_ordered;
    
ordered_label:
    result |= 16;
    
skip_ordered:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int pattern_uneq(void) {
    volatile double a = darr[0];
    volatile double b = darr[1];
    volatile double nan = g_dnan;
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is (a == b) including NaN case */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(nan != nan)) {  /* NaN != NaN is true, so !(true) is false */
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(fp.a != fp.b) && (a < b)) ? 4 : 0;
    
    /* Memory operands */
    volatile double* ptr1 = &darr[2];
    volatile double* ptr2 = &darr[3];
    if (!(*ptr1 != *ptr2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) = !(a < b) */
__attribute__((noinline))
int pattern_unge(void) {
    volatile float a = farr[0];
    volatile float b = farr[1];
    volatile double x = g_dpos;
    volatile double y = g_dneg;
    int result = 0;
    
    /* Direct UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With different types */
    if (!(x < y)) {
        result |= 2;
    }
    
    /* Nested in ternary */
    result |= (!(fp.a < fp.b) ? 4 : 0);
    
    /* Inline assembly forcing condition code */
    volatile double cmp1 = 2.0;
    volatile double cmp2 = 1.0;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : unge_label
    );
    
    result |= 8;
    goto skip_unge;
    
unge_label:
    result |= 16;
    
skip_unge:
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) = !(a <= b) */
__attribute__((noinline))
int pattern_ungt(void) {
    volatile long double a = ldarr[0];
    volatile long double b = ldarr[1];
    volatile double x = 3.0;
    volatile double y = 2.0;
    int result = 0;
    
    /* Direct UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Complex logical expression */
    if (!(x <= y) && (x != y)) {
        result |= 2;
    }
    
    /* With memory operand */
    if (!(g_dpos <= g_dneg)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int pattern_unle(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = g_dnan;
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan > b)) {
        result |= 2;
    }
    
    /* Complex: (a <= b) || (a != a) */
    if ((a <= b) || (a != a)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : unle_label
    );
    
    result |= 8;
    goto skip_unle;
    
unle_label:
    result |= 16;
    
skip_unle:
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int pattern_unlt(void) {
    volatile float a = 1.0f;
    volatile float b = 2.0f;
    volatile float nan = g_fnan;
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(nan >= b)) {
        result |= 2;
    }
    
    /* Complex: (a < b) || (a != a) */
    if ((a < b) || (a != a)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than, ordered) */
__attribute__((noinline))
int pattern_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 1.0;
    volatile double nan = g_dnan;
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal values should not trigger */
    if ((c < a) || (c > a)) {
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((nan < b) || (nan > b)) {
        result |= 4;
    }
    
    /* Complex expression */
    result |= (((fp.a < fp.b) || (fp.a > fp.b)) ? 8 : 0);
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ltgt_label
    );
    
    result |= 16;
    goto skip_ltgt;
    
ltgt_label:
    result |= 32;
    
skip_ltgt:
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
__attribute__((noinline))
int pattern_mixed(void) {
    volatile double x = darr[0];
    volatile double y = darr[1];
    volatile double z = g_dnan;
    int result = 0;
    
    /* Complex nested ternary with multiple condition codes */
    result = (x != x) ? 1 : 
             (!(y > z)) ? 2 :
             ((x < y) || (x > y)) ? 3 :
             (!(x >= y)) ? 4 :
             (!(x <= y)) ? 5 :
             (!(x != y)) ? 6 : 7;
    
    /* Logical combination */
    if ((isunordered(x, z) && !(y < x)) || ((x > y) && isordered(x, y))) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Memory-intensive pattern */
__attribute__((noinline))
int pattern_memory(void) {
    volatile struct {
        volatile double d1;
        volatile double d2;
        volatile float f1;
        volatile float f2;
    } s = {1.0, 2.0, 3.0f, 4.0f};
    
    int result = 0;
    
    /* Multiple memory comparisons */
    if (!(s.d1 < s.d2)) result |= 1;    /* UNGE */
    if (!(s.f1 > s.f2)) result |= 2;    /* UNLE */
    if ((s.d1 < s.d2) || (s.d1 > s.d2)) result |= 4;  /* LTGT */
    if (!(s.d1 != s.d2)) result |= 8;   /* UNEQ */
    
    /* Array accesses with different indices */
    volatile int idx = 1;
    if (!(darr[idx] < darr[idx+1])) result |= 16;
    if ((farr[0] < farr[2]) || (farr[0] > farr[2])) result |= 32;
    
    sink(result);
    return result;
}

/* Dummy sink function to prevent optimization */
void sink(int x) {
    static volatile int buffer;
    buffer = x;
}

void sink_ptr(void* p) {
    static volatile void* buffer;
    buffer = p;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile values */
    sink_ptr(&g_dnan);
    sink_ptr(&g_dinf);
    sink_ptr(&g_fnan);
    sink_ptr(&fp);
    
    /* Call each pattern function */
    checksum += pattern_unordered();
    checksum += pattern_ordered();
    checksum += pattern_uneq();
    checksum += pattern_unge();
    checksum += pattern_ungt();
    checksum += pattern_unle();
    checksum += pattern_unlt();
    checksum += pattern_ltgt();
    checksum += pattern_mixed();
    checksum += pattern_memory();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
