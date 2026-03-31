/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c -o test */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3, vol_arg4;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function with complex control flow to create old remats */
static volatile int test_remat(volatile int a, volatile int b, 
                               volatile int c, volatile int d) {
    /* Local variables for register pressure */
    int local_array[256];
    volatile int result = 0;
    
    /* Initialize local array */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to encourage rematerialization */
    for (int iter = 0; iter < 100; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Simple recomputable expressions - strong rematerialization candidates */
        int cand1 = a + 5;              /* arg + constant */
        int cand2 = b * 2;              /* arg * constant */
        int cand3 = &local_array[c] - local_array;  /* address calculation */
        int cand4 = d << 3;             /* shift operation */
        
        /* Use candidates immediately in BLOCK A */
        result += cand1;
        result -= cand2;
        result ^= cand3;
        result |= cand4;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many distinct local variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + a;
            int t3 = t2 - b;
            int t4 = t3 * c;
            int t5 = t4 / (d + 1);
            long t6 = t5 * 123456789L;
            long t7 = t6 + 987654321L;
            long t8 = t7 - 555555555L;
            float f1 = t8 * 0.5f;
            float f2 = f1 + 1.234f;
            float f3 = f2 * 3.14159f;
            double d1 = f3 * 2.71828;
            double d2 = d1 / 1.41421;
            double d3 = d2 + 0.57721;
            
            /* More variables for additional pressure */
            int t9 = t1 ^ t2;
            int t10 = t3 | t4;
            int t11 = t5 & t9;
            int t12 = t10 + t11;
            long t13 = t12 * 13579L;
            float f4 = t13 * 0.25f;
            double d4 = f4 * 1.61803;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t9, t10, t11};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf vf3 = vf1 * vf2;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += t1 + t2 + t3 + t4 + t5 + t12;
            result += (int)(t6 >> 32) + (int)(t7 >> 32) + (int)(t8 >> 32);
            result += (int)f1 + (int)f2 + (int)f3;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
            
            #ifdef __SSE2__
            result += v3[0] + v3[1] + v3[2] + v3[3];
            result += (int)vf3[0] + (int)vf3[1] + (int)vf3[2] + (int)vf3[3];
            #endif
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces compiler to reconsider rematerialization */
        result += cand1 * 3;
        result -= cand2 / 2;
        result ^= cand3 << 1;
        result |= cand4 >> 1;
        
        /* Additional use with different computation */
        int cand1_alt = a + 5;      /* Same recomputation */
        int cand2_alt = b * 2;
        result += cand1_alt - cand2_alt;
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize volatile arguments */
    vol_arg1 = argc > 1 ? atoi(argv[1]) : 123;
    vol_arg2 = argc > 2 ? atoi(argv[2]) : 456;
    vol_arg3 = argc > 3 ? atoi(argv[3]) : 789;
    vol_arg4 = argc > 4 ? atoi(argv[4]) : 1011;
    
    /* Loop to increase optimization opportunities */
    volatile int total = 0;
    int iterations = argc > 5 ? atoi(argv[5]) : 1000;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify arguments slightly each iteration */
        vol_arg1 += i & 1;
        vol_arg2 += i & 2;
        vol_arg3 += i & 4;
        vol_arg4 += i & 8;
        
        total += test_remat(vol_arg1, vol_arg2, vol_arg3, vol_arg4);
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Additional test with different patterns */
    {
        /* Test with address calculations */
        int array[100];
        volatile int sum = 0;
        
        for (int i = 0; i < 100; i++) {
            array[i] = i * i;
        }
        
        for (int i = 0; i < 50; i++) {
            /* Create rematerialization candidate with address */
            int *ptr = &array[i + vol_arg1];
            sum += *ptr;
            
            /* High pressure block */
            if (always_true) {
                int x1 = sum * 2, x2 = x1 + 3, x3 = x2 * 4, x4 = x3 - 5;
                int x5 = x4 / 6, x6 = x5 ^ 7, x7 = x6 | 8, x8 = x7 & 9;
                float fx1 = x8 * 1.1f, fx2 = fx1 + 2.2f, fx3 = fx2 * 3.3f;
                double dx1 = fx3 * 4.4, dx2 = dx1 / 5.5, dx3 = dx2 + 6.6;
                asm volatile("" ::: "memory");
                sum += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
                sum += (int)fx1 + (int)fx2 + (int)fx3;
                sum += (int)dx1 + (int)dx2 + (int)dx3;
            }
            
            /* Use address candidate again */
            sum += *(ptr + 1);
        }
        
        printf("Array sum: %d\n", sum);
    }
    
    return 0;
}
