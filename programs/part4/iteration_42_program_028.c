/* test_sel_sched_dump.c
 * Designed to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity is preserved */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int seed = 42;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop to provide sufficient work */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create register pressure with many live variables */
            int a = arr1[i-1];
            int b = arr1[i];
            int c = arr1[i+1];
            int d = arr2[i-1];
            int e = arr2[i];
            int f = arr2[i+1];
            
            /* Mix integer and floating point operations */
            float fa = farr1[i % 16];
            float fb = farr1[(i+1) % 16];
            float fc = farr2[i % 16];
            float fd = farr2[(i+1) % 16];
            
            /* Chain of dependent arithmetic operations */
            int t1 = a * b + c;
            int t2 = d - e * f;
            int t3 = t1 ^ t2;
            int t4 = t3 << (i & 3);
            int t5 = t4 + seed;  /* volatile read creates barrier */
            
            float ft1 = fa * fb;
            float ft2 = fc / fd;
            float ft3 = ft1 + ft2;
            float ft4 = ft3 - fa;
            
            /* Conditional execution with side effects */
            if ((t5 & 7) > (i & 3)) {
                /* Branch 1: different operations */
                arr1[i] = t5 + (int)(ft4 * 100.0f);
                arr2[i] = t4 ^ (int)ft3;
                farr1[i % 16] = ft1 * 2.0f;
            } else {
                /* Branch 2: distinct operations */
                arr1[i] = t5 - (int)(ft4 * 50.0f);
                arr2[i] = t4 | (int)ft2;
                farr2[i % 16] = ft2 / 2.0f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            int late_use = t1 + t2 + (int)ft3;
            sum += late_use ^ (arr1[i] & 0xFF);
            
            /* More operations to increase pressure */
            for (j = 0; j < 2; j++) {
                int idx = (i + j) & 31;
                arr1[idx] += (sum & 0xF);
                arr2[idx] ^= (late_use >> (j * 4)) & 0xF;
            }
            
            /* Another volatile access */
            seed = sum & 0xFF;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] ^= sum;
        arr2[0] += sum;
    }
    
    *result = sum;
}

/* Another complex function to increase overall compilation complexity */
static int __attribute__((noinline))
helper_func(int x, int y) {
    int z = x * y;
    z = (z << 4) | (z >> 28);
    z ^= 0x5A5A5A5A;
    
    /* More complex control flow */
    if (z > 0) {
        z = z * 3 + 1;
    } else {
        z = (z + 1) * 2;
    }
    
    asm volatile("" : "+r"(z) : : "memory");
    return z;
}

int main(void) {
    int result1, result2, result3;
    int i;
    
    /* Call target function multiple times with different parameters */
    stress_sched(100, &result1);
    
    /* Additional computation to keep compiler busy */
    result2 = 0;
    for (i = 0; i < 1000; i++) {
        result2 += helper_func(i, result1);
    }
    
    stress_sched(50, &result3);
    
    /* Final result to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    printf("Result: %d\n", final_result);
    
    return 0;
}
