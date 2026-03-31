/* Test to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) {
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32];
    int arr2[32];
    float farr1[16];
    float farr2[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    
    /* Outer loop for sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent arithmetic operations */
            int a = arr1[i-1] + arr2[i+1];
            int b = a * seed;
            int c = b ^ (i << 3);
            int d = c - arr1[i];
            
            /* Floating point calculations mixed in */
            float fa = farr1[i % 16];
            float fb = farr2[i % 16];
            float fc = fa * fb + (float)d;
            
            /* Conditional execution with side effects */
            if ((d & 0xF) > 8) {
                /* Branch 1: complex operations */
                int e = d * 3 + (a >> 2);
                float fd = fc * 2.0f - fa;
                arr1[i] = e ^ (int)fd;
                farr1[i % 16] = fd * 0.5f;
                
                /* More arithmetic chain */
                int g = e * 7 - (b % 17);
                arr2[i] = g ^ (i * 11);
            } else {
                /* Branch 2: different operations */
                int e = d / 2 + (c << 1);
                float fd = fc / 1.5f + fb;
                arr1[i] = e | (int)fd;
                farr2[i % 16] = fd * 1.25f;
                
                /* Alternative arithmetic chain */
                int g = e * 3 + (d % 23);
                arr2[i] = g & (i * 13);
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Extended live range usage */
            int late_use = arr1[i-1] + arr2[i+1] + d;
            float late_float = fc * 2.0f + farr1[i % 16];
            
            /* More operations using values computed earlier */
            int h = late_use * 3 + (int)(late_float * 10.0f);
            arr1[(i + 1) % 32] = h ^ seed;
            
            /* Volatile read to create scheduling barrier */
            int volatile_read = seed;
            arr2[(i + 2) % 32] = h + volatile_read;
            
            /* Accumulate checksum */
            sum += arr1[i] + arr2[i] + (int)farr1[i % 16];
        }
        
        /* Modify seed to vary pattern */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    *result = sum;
}

/* Another complex function to increase scheduling opportunities */
static int __attribute__((noinline))
complex_calculation(int *data, int size) {
    int acc = 0;
    volatile int barrier = 1;
    
    for (int i = 1; i < size - 1; i++) {
        /* Data-dependent chain */
        int x = data[i-1] * 3;
        int y = data[i] * 5;
        int z = data[i+1] * 7;
        
        /* Complex condition */
        if ((x ^ y) > (z & 0xFF)) {
            int t = x * y - z;
            data[i] = t >> 2;
            acc += t * 2;
            
            /* Inline asm barrier */
            asm volatile("" ::: "memory");
            
            /* More operations */
            int u = data[i] * 11 + barrier;
            data[i+1] = u % 997;
        } else {
            int t = y * z + x;
            data[i] = t << 1;
            acc -= t / 3;
            
            /* Different computation path */
            int u = data[i] * 13 - barrier;
            data[i-1] = u % 991;
        }
        
        /* Cross-iteration dependency */
        data[i] += acc & 0xFF;
    }
    
    return acc;
}

int main(void) {
    int result1, result2;
    int data[100];
    
    /* Initialize data array */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Call stress function multiple times */
    stress_sched(1000, &result1);
    
    /* Call complex calculation */
    result2 = complex_calculation(data, 100);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional check to use all data */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check ^= data[i];
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
