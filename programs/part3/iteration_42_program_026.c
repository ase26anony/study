/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant propagation */
volatile double vd1 = 1.0, vd2 = 2.0, vd3 = NAN, vd4 = INFINITY;
volatile long double vld1 = 1.0L, vld2 = 2.0L, vld3 = NAN, vld4 = INFINITY;
volatile int vi1 = 0, vi2 = 1, vi3 = 2;

/* UNORDERED condition code test with x87 */
static __attribute__((noinline)) 
int test_unordered_x87(long double a, long double b) {
    int result;
    /* x87 floating compare with unordered condition */
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "m"(a), "m"(b), "u"(UNORDERED)
                  : "cc", "st");
    return result;
}

/* ORDERED condition code test with SSE */
static __attribute__((noinline))
int test_ordered_sse(double a, double b) {
    int result;
    /* SSE2 compare with ordered condition */
    asm volatile ("comisd %2, %1\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(ORDERED)
                  : "cc");
    return result;
}

/* UNEQ condition code test */
static __attribute__((noinline))
int test_uneq_mixed(double a, long double b) {
    int result;
    /* Mixed comparison - unordered or equal */
    asm volatile ("fldt %2\n\t"
                  "fldl %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "m"(a), "m"(b), "u"(UNEQ)
                  : "cc", "st");
    return result;
}

/* UNGE condition code test (not less than) */
static __attribute__((noinline))
int test_unge_sse(double a, double b) {
    int result;
    asm volatile ("comisd %2, %1\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNGE)
                  : "cc");
    return result;
}

/* UNGT condition code test (not less or equal) */
static __attribute__((noinline))
int test_ungt_x87(long double a, long double b) {
    int result;
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "m"(a), "m"(b), "u"(UNGT)
                  : "cc", "st");
    return result;
}

/* UNLE condition code test */
static __attribute__((noinline))
int test_unle_mixed(long double a, double b) {
    int result;
    asm volatile ("fldl %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "m"(a), "m"(b), "u"(UNLE)
                  : "cc", "st");
    return result;
}

/* UNLT condition code test */
static __attribute__((noinline))
int test_unlt_sse(double a, double b) {
    int result;
    asm volatile ("comisd %2, %1\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNLT)
                  : "cc");
    return result;
}

/* LTGT condition code test (unordered or not equal) */
static __attribute__((noinline))
int test_ltgt_x87(long double a, long double b) {
    int result;
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "m"(a), "m"(b), "u"(LTGT)
                  : "cc", "st");
    return result;
}

/* Helper function that uses switch to select condition code */
static __attribute__((noinline))
int test_condition_switch(int cond_code, double a, double b) {
    int result = 0;
    
    /* This switch may cause the compiler to generate 
       condition code operands in RTL */
    switch (cond_code) {
        case 0:  /* UNORDERED */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNORDERED)
                          : "cc");
            break;
        case 1:  /* ORDERED */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(ORDERED)
                          : "cc");
            break;
        case 2:  /* UNEQ */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNEQ)
                          : "cc");
            break;
        case 3:  /* UNGE */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGE)
                          : "cc");
            break;
        case 4:  /* UNGT */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGT)
                          : "cc");
            break;
        case 5:  /* UNLE */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLE)
                          : "cc");
            break;
        case 6:  /* UNLT */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLT)
                          : "cc");
            break;
        case 7:  /* LTGT */
            asm volatile ("comisd %2, %1\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(LTGT)
                          : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage if 
               an invalid condition code somehow reaches here */
            asm volatile ("# Invalid condition code %c0"
                          : : "u"(cond_code));
            result = -1;
    }
    return result;
}

/* Function with complex control flow to obscure optimization */
static __attribute__((noinline))
int test_complex_flow(double *arr, int n, volatile int *cond_selector) {
    int total = 0;
    long double ld_acc = 0.0L;
    double d_acc = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile read to prevent loop invariant motion */
        int selector = *cond_selector;
        
        /* Mix regular C comparisons with inline assembly */
        if (arr[i] != arr[(i + 1) % n]) {
            total += test_unordered_x87(ld_acc, (long double)arr[i]);
        }
        
        if (arr[i] >= arr[(i + 2) % n]) {
            total += test_ordered_sse(d_acc, arr[(i + 1) % n]);
        }
        
        /* Use selector to choose different condition codes */
        switch (selector & 7) {
            case 0:
                total += test_uneq_mixed(d_acc, ld_acc);
                break;
            case 1:
                total += test_unge_sse(arr[i], d_acc);
                break;
            case 2:
                total += test_ungt_x87(ld_acc, ld_acc + 1.0L);
                break;
            case 3:
                total += test_unle_mixed(ld_acc, d_acc);
                break;
            case 4:
                total += test_unlt_sse(arr[i], arr[(i + 1) % n]);
                break;
            case 5:
                total += test_ltgt_x87(ld_acc, (long double)arr[i]);
                break;
            default:
                total += test_condition_switch(selector, arr[i], d_acc);
        }
        
        /* Update accumulators with volatile writes */
        ld_acc += (long double)arr[i];
        d_acc += arr[i];
        
        /* Force memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int loop_count = 100;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Create array with mix of normal and special values */
    double values[8];
    values[0] = vd1;          /* 1.0 */
    values[1] = vd2;          /* 2.0 */
    values[2] = vd3;          /* NAN */
    values[3] = vd4;          /* INFINITY */
    values[4] = -vd4;         /* -INFINITY */
    values[5] = 0.0;
    values[6] = -0.0;
    values[7] = 3.141592653589793;
    
    /* Volatile selector to prevent constant propagation */
    volatile int cond_selector = vi1;
    
    printf("Testing x86 condition code printing logic...\n");
    
    /* Main test loop */
    for (int iter = 0; iter < loop_count; iter++) {
        /* Update selector with non-trivial pattern */
        cond_selector = (cond_selector * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Test all condition code functions directly */
        accumulator += test_unordered_x87(vld1, vld2);
        accumulator += test_ordered_sse(vd1, vd2);
        accumulator += test_uneq_mixed(vd3, vld3);  /* NAN vs NAN */
        accumulator += test_unge_sse(vd1, vd4);     /* 1.0 vs INF */
        accumulator += test_ungt_x87(vld4, vld1);   /* INF vs 1.0 */
        accumulator += test_unle_mixed(vld2, vd1);  /* 2.0L vs 1.0 */
        accumulator += test_unlt_sse(vd2, vd1);     /* 2.0 vs 1.0 */
        accumulator += test_ltgt_x87(vld3, vld1);   /* NAN vs 1.0 */
        
        /* Test with switch-based selection */
        accumulator += test_condition_switch(iter & 7, 
                                           values[iter % 8], 
                                           values[(iter + 1) % 8]);
        
        /* Test complex control flow */
        accumulator += test_complex_flow(values, 8, &cond_selector);
        
        /* Occasionally use out-of-range selector to potentially 
           trigger default case in printing logic */
        if ((iter % 13) == 0) {
            int temp = test_condition_switch(8, values[0], values[1]);
            accumulator += temp;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulator result: %d\n", accumulator);
    
    /* Additional test with inline assembly that might trigger 
       the printing logic during compilation */
    asm volatile ("# Condition code test block\n\t"
                  "# %c0 %c1 %c2 %c3 %c4 %c5 %c6 %c7"
                  : 
                  : "u"(UNORDERED), "u"(ORDERED), "u"(UNEQ),
                    "u"(UNGE), "u"(UNGT), "u"(UNLE),
                    "u"(UNLT), "u"(LTGT));
    
    return 0;
}
