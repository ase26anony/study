/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline, optimize("O3")))
stress_sched(int iterations, int *result) {
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32];
    int arr2[32];
    float farr1[16];
    float farr2[16];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 32; i++) {
        arr1[i] = (seed * i) & 0xFF;
        arr2[i] = (seed + i * 3) & 0xFF;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = (seed * i) / 7.0f;
        farr2[i] = (seed + i * 5) / 11.0f;
    }
    
    int sum = 0;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Create long dependency chain with mixed operations */
            int t1 = arr1[i-1] * 3;
            int t2 = arr2[i+1] + 7;
            float ft1 = farr1[i % 16] * 2.5f;
            float ft2 = farr2[i % 16] / 1.7f;
            
            /* Volatile read to create scheduling barrier */
            volatile int barrier = seed;
            
            /* Complex conditional with side effects in both branches */
            if ((t1 ^ t2) > (barrier & 0x3F)) {
                /* Branch 1: Integer-heavy operations */
                int t3 = t1 * t2 - (i << 3);
                int t4 = (t3 >> 2) + (t1 & t2);
                float ft3 = ft1 + ft2 * (i & 7);
                
                arr1[i] = t3 ^ t4;
                arr2[i] = (t4 * 2) - (t3 / 3);
                farr1[i % 16] = ft3 * 0.9f;
                
                /* Inline assembly as scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* Use values computed much earlier */
                sum += arr1[i-2] + t4;
            } else {
                /* Branch 2: Different operations to challenge scheduler */
                int t3 = (t1 + t2) | (i * 5);
                int t4 = (t1 - t2) ^ (i << 2);
                float ft3 = ft2 - ft1 / (i & 3 + 1);
                
                arr1[i] = t3 & t4;
                arr2[i] = (t4 << 1) | (t3 >> 1);
                farr2[i % 16] = ft3 * 1.1f;
                
                /* Another inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Extended live range usage */
                sum += arr2[i+1] - t3;
            }
            
            /* Cross-iteration dependencies */
            arr1[0] += (arr1[i] & 1);
            arr2[31] ^= (arr2[i] >> 4);
            
            /* Floating-point operations mixed with integer */
            float ft4 = farr1[(i+1) % 16] + farr2[(i-1) % 16];
            farr1[i % 16] = ft4 * (i & 1 ? 0.8f : 1.2f);
            
            /* More volatile operations */
            volatile float fbarrier = ft4;
            farr2[i % 16] += fbarrier;
        }
        
        /* Loop-carried dependency */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        arr1[0] ^= seed;
        arr2[0] += seed >> 16;
    }
    
    /* Final computation using all arrays */
    for (int i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
    }
    for (int i = 0; i < 16; i++) {
        sum += (int)(farr1[i] + farr2[i]);
    }
    
    *result = sum;
}

/* Helper to prevent optimization */
static int __attribute__((noinline))
compute_checksum(int iter) {
    int result1, result2;
    
    /* Call with different parameters to create varied scheduling contexts */
    stress_sched(iter, &result1);
    stress_sched(iter / 2 + 1, &result2);
    
    return result1 ^ result2;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    int checksum = compute_checksum(iterations);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Also print to stderr to potentially interact with dump output */
    fprintf(stderr, "Checksum: %08x\n", checksum);
    
    return 0;
}
