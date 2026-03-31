/* Test program to trigger selective scheduling RTL dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int barrier = 0;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, k;
    
    /* Initialize arrays with non-uniform patterns */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop - provides iteration count */
    for (k = 0; k < iterations; k++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Create register pressure with many live variables */
            int a = arr1[i-1];
            int b = arr2[i];
            int c = arr1[i+1];
            float fa = farr1[i-1];
            float fb = farr2[i];
            float fc = farr1[i+1];
            
            /* Chain of dependent integer operations */
            int t1 = a * b + k;
            int t2 = t1 ^ c;
            int t3 = t2 - (a << 2);
            int t4 = t3 * 7 + i;
            
            /* Chain of dependent floating-point operations */
            float ft1 = fa * fb + (float)k;
            float ft2 = ft1 / (fc + 1.0f);
            float ft3 = ft2 * 3.14f - fa;
            float ft4 = ft3 + (float)i * 0.5f;
            
            /* Conditional execution with side effects */
            if ((t4 & 0xF) > 8) {
                /* Branch 1: More complex operations */
                int t5 = t4 * t3 + (b >> 1);
                float ft5 = ft4 * ft3 + fb;
                
                /* Use inline assembly as scheduling barrier */
                asm volatile("" ::: "memory");
                
                /* Extended live range usage */
                arr1[i] = t5 + arr2[i-1];
                farr1[i] = ft5 + farr2[i-1];
                
                /* More operations after barrier */
                int t6 = arr1[i] ^ t5;
                float ft6 = farr1[i] * ft5;
                
                sum += t6;
                fsum += ft6;
            } else {
                /* Branch 2: Different operations */
                int t5 = (t4 + t2) | (c & 0xFF);
                float ft5 = (ft4 - ft2) * (fc * 0.25f);
                
                /* Another inline assembly barrier */
                asm volatile("" ::: "memory");
                
                /* Different array updates */
                arr2[i] = t5 - arr1[i+1];
                farr2[i] = ft5 - farr1[i+1];
                
                /* Operations with extended live ranges */
                int t6 = arr2[i] & t5;
                float ft6 = farr2[i] / (ft5 + 1.0f);
                
                sum += t6 * 2;
                fsum += ft6 * 2.0f;
            }
            
            /* Use values computed much earlier in the loop */
            /* This extends live ranges across the conditional */
            barrier = t1;  /* volatile write creates scheduling barrier */
            
            /* More operations using extended live ranges */
            int t7 = t4 + barrier;
            float ft7 = ft4 + (float)barrier;
            
            /* Final array updates with complex indexing */
            int idx = (i + k) & 0x1F;
            arr1[idx] = t7 + sum;
            farr1[idx] = ft7 + fsum;
            
            /* Additional arithmetic to increase pressure */
            for (j = 0; j < 4; j++) {
                arr2[(i + j) & 0x1F] += (t7 >> j);
                farr2[(i + j) & 0x1F] += ft7 / (j + 1);
            }
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = sum & 0xFF;
        farr1[0] = fsum * 0.01f;
    }
    
    /* Final computation mixing all results */
    int final_sum = 0;
    for (i = 0; i < 32; i++) {
        final_sum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    *result = final_sum + sum + (int)fsum;
}

/* Helper function to create additional scheduling context */
static int __attribute__((noinline))
process_data(int seed, int size) 
{
    int buffer[64];
    int i, total = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < size && i < 64; i++) {
        buffer[i] = seed * i + (i % 17);
    }
    
    /* Process with mixed operations */
    for (i = 1; i < size && i < 63; i++) {
        buffer[i] = buffer[i-1] * 3 + buffer[i+1] * 2;
        total += buffer[i];
        
        /* Occasional barrier */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return total;
}

int main(void) 
{
    int result1, result2, result3;
    int checksum = 0;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Multiple calls with different parameters */
    stress_sched(100, &result1);
    checksum ^= result1;
    
    result2 = process_data(result1, 48);
    checksum ^= result2;
    
    stress_sched(50, &result3);
    checksum ^= result3;
    
    /* Final mixed computation */
    checksum += process_data(checksum, 32);
    
    printf("Result checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
