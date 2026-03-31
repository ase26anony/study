#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force frame pointer usage by taking addresses */
#define USE_FP __attribute__((noinline, optimize("no-omit-frame-pointer")))

/* Various helper functions with different calling conventions */
static int __attribute__((noinline)) helper1(int a, int b, int c, int d, 
                                            int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    /* Use alloca to force frame pointer */
    char *buf = (char*)alloca(64);
    for (int i = 0; i < 64; i++) buf[i] = (char)(result + i);
    return result;
}

static float __attribute__((noinline)) helper2(float a, float b, float c, 
                                              float d, float e, float f) {
    volatile float sum = a + b + c + d + e + f;
    /* Force spill by taking address */
    float *ptr = &sum;
    *ptr += 1.0f;
    return sum;
}

USE_FP void __attribute__((noinline)) helper3(int *restrict a, float *restrict b,
                                             double *restrict c, int n) {
    volatile double acc = 0.0;
    for (int i = 0; i < n; i++) {
        acc += a[i] * b[i] + c[i];
        /* Inline asm to clobber registers */
        __asm__ volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
    }
    *c = acc;
}

static double __attribute__((noinline)) helper4(double a, double b, double c,
                                               double d, double e, double f,
                                               double g, double h) {
    /* Many live values across calls */
    volatile double t1 = a * b;
    volatile double t2 = c * d;
    volatile double t3 = e * f;
    volatile double t4 = g * h;
    
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7");
    
    return t1 + t2 - t3 + t4;
}

/* Function with mixed arguments to test different ABIs */
int __attribute__((noinline)) mixed_args(int a, float b, double c, int d,
                                        float e, double f, int g, float h) {
    volatile int vi = a + d + g;
    volatile float vf = b + e + h;
    volatile double vd = c + f;
    
    /* Force register pressure with many temporaries */
    int t1 = vi * 2;
    float t2 = vf * 3.0f;
    double t3 = vd * 4.0;
    int t4 = t1 + (int)t2;
    double t5 = t3 + t4;
    
    __asm__ volatile ("" : : : "rax", "rdx", "rcx", "r8", "r9", "r10",
                      "xmm0", "xmm1", "xmm2", "xmm3");
    
    return (int)(t5 + vi + vf + vd);
}

/* Create complex control flow with calls inside basic blocks */
static void complex_flow(int iter, volatile int *result) {
    /* Many local variables to create register pressure */
    int a = iter * 1;
    int b = iter * 2;
    int c = iter * 3;
    int d = iter * 4;
    int e = iter * 5;
    int f = iter * 6;
    int g = iter * 7;
    int h = iter * 8;
    int i = iter * 9;
    int j = iter * 10;
    float fa = iter * 1.1f;
    float fb = iter * 2.2f;
    float fc = iter * 3.3f;
    float fd = iter * 4.4f;
    double da = iter * 1.11;
    double db = iter * 2.22;
    
    /* First call - keep values live in registers */
    int r1 = helper1(a, b, c, d, e, f, g, h);
    
    /* Inline asm that clobbers call-clobbered registers */
    __asm__ volatile ("# Force clobber" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Use values after asm - they must be saved/restored */
    float r2 = helper2(fa + r1, fb, fc, fd, fa * 2.0f, fb * 3.0f);
    
    /* Conditional with calls on both paths */
    if (iter % 3 == 0) {
        /* Path with multiple calls in same basic block */
        double r3 = helper4(da, db, da * 2.0, db * 3.0,
                           da * 4.0, db * 5.0, da * 6.0, db * 7.0);
        
        /* More computations between calls */
        int t1 = i + j + (int)r2;
        float t2 = fa + fb + (float)r3;
        
        __asm__ volatile ("# Middle clobber" : : : 
            "rax", "rdx", "xmm0", "xmm1", "xmm2");
        
        int r4 = mixed_args(t1, t2, r3, a, b, (double)c, d, e);
        
        /* Store to volatile to prevent optimization */
        *result += r1 + (int)r2 + (int)r3 + r4;
    } else if (iter % 3 == 1) {
        /* Different path with array operations */
        int arr1[8] = {a, b, c, d, e, f, g, h};
        float arr2[8] = {fa, fb, fc, fd, fa*2, fb*2, fc*2, fd*2};
        double arr3[8] = {da, db, da*2, db*2, da*3, db*3, da*4, db*4};
        
        helper3(arr1, arr2, arr3, 8);
        
        /* Compute with results */
        double sum = 0.0;
        for (int k = 0; k < 8; k++) {
            sum += arr1[k] + arr2[k] + arr3[k];
        }
        
        __asm__ volatile ("# Path2 clobber" : : : "rcx", "rsi", "rdi", "xmm4", "xmm5");
        
        *result += (int)sum + i + j;
    } else {
        /* Loop with calls inside */
        int acc = 0;
        for (int k = 0; k < 4; k++) {
            /* Call inside loop - creates save/restore in loop body */
            acc += helper1(a + k, b + k, c + k, d + k, 
                          e + k, f + k, g + k, h + k);
            
            /* Clobber between loop iterations */
            if (k % 2 == 0) {
                __asm__ volatile ("# Loop clobber" : : : 
                    "rax", "rdx", "xmm0", "xmm1");
            }
        }
        *result += acc;
    }
}

/* Main driver with varying control flow */
int main(void) {
    volatile int final_result = 0;
    volatile int checksum = 0;
    
    /* Multiple iterations with different register pressure patterns */
    for (int iter = 0; iter < 100; iter++) {
        int old_result = final_result;
        
        /* Create basic block with internal calls */
        complex_flow(iter, &final_result);
        
        /* Verify computation wasn't optimized away */
        if (final_result == old_result) {
            checksum += iter;
        } else {
            checksum += final_result - old_result;
        }
        
        /* Periodic inline asm to clobber everything */
        if (iter % 7 == 0) {
            __asm__ volatile ("# Periodic full clobber" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        }
    }
    
    /* Use alloca in main to force frame pointer */
    volatile int *dynamic = (int*)alloca(sizeof(int) * 16);
    for (int i = 0; i < 16; i++) {
        dynamic[i] = final_result + i;
        checksum += dynamic[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Result: %d\n", final_result);
    
    return (checksum != 0) ? 0 : 1;
}
