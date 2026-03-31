/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    
    /* Local variables for register pressure */
    int local_array[64];
    for (int i = 0; i < 64; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* --- BLOCK A: Create rematerialization candidates --- */
        /* Simple recomputable expressions (strong candidates) */
        int cand1 = arg1 + 10;           /* Constant offset from argument */
        int cand2 = arg2 * 2;            /* Simple arithmetic */
        int cand3 = arg3 & 0xFF;         /* Mask operation */
        
        /* Address calculation with constant offset */
        int *cand4 = &local_array[arg1 % 64];
        int *cand5 = &local_array[arg2 % 64 + 5];
        
        /* Immediate use of candidates in block A */
        result += cand1;
        result += cand2;
        result += cand3;
        result += *cand4;
        result += *cand5;
        
        /* --- Conditional jump to split live ranges --- */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* --- BLOCK B: High register pressure region --- */
            /* Many independent variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            int t3 = t2 - arg2;
            int t4 = t3 * 3;
            int t5 = t4 / 2;
            long t6 = t5 + 1000L;
            long t7 = t6 - 500L;
            long t8 = t7 * 2L;
            float t9 = (float)t8 * 1.5f;
            float t10 = t9 + 3.14f;
            double t11 = (double)t10 * 2.0;
            double t12 = t11 - 1.0;
            int t13 = (int)t12;
            int t14 = t13 ^ 0x55;
            int t15 = t14 << 2;
            int t16 = t15 >> 1;
            int t17 = t16 | 0xAA;
            int t18 = t17 & 0xF0;
            int t19 = t18 + iter;
            int t20 = t19 * 7;
            
            /* More variables to increase pressure */
            long t21 = t20 + 10000L;
            long t22 = t21 - 5000L;
            float t23 = (float)t22 / 2.0f;
            double t24 = (double)t23 * 3.14159;
            int t25 = (int)t24;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t7, t8};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int *vp = (int*)&v5;
            t25 += vp[0] + vp[1] + vp[2] + vp[3];
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to keep them alive */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            result += t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
            result += t21 + t22 + t23 + t24 + t25;
            
            /* --- BLOCK C: Use candidates again after high pressure --- */
            /* This forces reconsideration of rematerialization */
            result += cand1 * 2;
            result += cand2 / 2;
            result += cand3 | 0x80;
            result += *cand4 * 3;
            result += *cand5 - 5;
            
            /* More computations to ensure instructions aren't deleted */
            int final1 = cand1 + cand2;
            int final2 = cand3 + *cand4;
            result += final1 * final2;
        }
        
        /* Additional loop-invariant computation to create more candidates */
        int loop_cand = arg1 + arg2 + iter;
        result += loop_cand;
    }
    
    return result;
}

/* Second function with different pattern to increase LTO opportunities */
static volatile int __attribute__((noinline))
test_remat2(volatile int a, volatile int b) {
    volatile int sum = 0;
    int arr[32];
    
    for (int i = 0; i < 32; i++) {
        arr[i] = i + a;
    }
    
    for (volatile int j = 0; j < b; j++) {
        /* More rematerialization candidates */
        int rc1 = a + j * 4;
        int rc2 = b - j * 2;
        int *rc3 = &arr[j % 32];
        
        sum += rc1;
        sum += rc2;
        sum += *rc3;
        
        if (always_true) {
            /* High pressure block */
            int p1 = sum * 3, p2 = p1 + 1, p3 = p2 * 2, p4 = p3 - 1;
            int p5 = p4 / 2, p6 = p5 | 0xF, p7 = p6 << 1, p8 = p7 >> 1;
            long p9 = p8 + 100L, p10 = p9 * 2L;
            float p11 = (float)p10 * 1.1f;
            double p12 = (double)p11 * 2.2;
            
            asm volatile("" ::: "memory");
            
            sum += p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 + p11 + p12;
            
            /* Reuse candidates */
            sum += rc1 * rc2;
            sum += *rc3 + j;
        }
    }
    
    return sum;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test functions multiple times to give compiler more context */
    for (volatile int i = 0; i < iterations; i++) {
        total += test_remat(i, i+1, i+2, 5);
        total += test_remat2(i, 3);
        
        /* Alternate arguments to create different recomputable expressions */
        if (i % 2 == 0) {
            total += test_remat(i*2, i*3, i*4, 2);
        } else {
            total += test_remat2(i*5, 4);
        }
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Also use dump files if available */
    #ifdef __GNUC__
    asm volatile("" : "=r"(total) : "0"(total));
    #endif
    
    return total != 0 ? 0 : 1;
}
