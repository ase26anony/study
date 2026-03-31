/* Test case to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable function to stress the scheduler */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int barrier = 0;
    int arr1[32];
    int arr2[32];
    float farr1[16];
    float farr2[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    int counter = 0;
    
    /* Outer loop for sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Create register pressure with many live values */
            int a = arr1[i-1];
            int b = arr1[i];
            int c = arr1[i+1];
            int d = arr2[i-1];
            int e = arr2[i];
            int f = arr2[i+1];
            
            /* Mixed integer calculations with dependencies */
            int t1 = a * b + c;
            int t2 = d ^ e | f;
            int t3 = t1 - t2;
            int t4 = t3 << (i & 3);
            int t5 = t4 * 11467;
            
            /* Floating point calculations to use FP registers */
            float fa = farr1[i & 15];
            float fb = farr2[i & 15];
            float fc = fa * 1.73205f + fb;
            float fd = fb - fa * 0.57735f;
            
            /* Volatile read creates scheduling barrier */
            int vol = barrier;
            
            /* Conditional execution with side effects */
            if ((t5 ^ vol) & 0x100) {
                /* Branch 1: different calculations */
                t1 = t5 * 3;
                t2 = t4 / 2;
                arr1[i] = t1 + t2;
                fc = fc * 2.0f - 1.0f;
            } else {
                /* Branch 2: alternative calculations */
                t1 = t5 / 3;
                t2 = t4 * 2;
                arr1[i] = t1 | t2;
                fd = fd * 1.5f + 0.5f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" : : : "memory");
            
            /* Use values computed much earlier */
            int t6 = t3 + t5;
            float fe = fc + fd;
            
            /* More calculations extending live ranges */
            arr2[i] = (t6 ^ (int)fe) & 0xFF;
            farr1[i & 15] = fe * 0.86603f;
            farr2[i & 15] = fe * 0.5f;
            
            /* Accumulate checksum */
            sum += arr1[i] + arr2[i];
            counter++;
        }
        
        /* Modify barrier to affect condition */
        barrier = outer & 0xFF;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling complexity */
static int __attribute__((noinline,noipa))
complex_calculation(int seed) {
    int x = seed;
    int y = seed * 3;
    int z = seed * 7;
    
    for (int i = 0; i < 100; i++) {
        x = (x ^ y) + z;
        y = (y ^ z) + x;
        z = (z ^ x) + y;
        
        /* Conditional with arithmetic in both branches */
        if (x & 0x80) {
            x = x * 3 - y;
            asm volatile("" : : : "memory");
        } else {
            x = x / 2 + z;
        }
        
        z = z ^ (y << 2);
    }
    
    return x + y + z;
}

int main(void) {
    int result1, result2;
    int total = 0;
    
    /* Call stress function multiple times */
    for (int i = 0; i < 5; i++) {
        stress_sched(100, &result1);
        total += result1;
        
        result2 = complex_calculation(i * 17);
        total += result2;
    }
    
    printf("Result checksum: %d\n", total);
    
    /* Verify with known value for basic correctness */
    if (total == 0) {
        printf("Warning: Result is zero\n");
    }
    
    return 0;
}
