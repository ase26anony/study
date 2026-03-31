/* test_sel_sched.c - Test for GCC selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline,noipa))
static int stress_sched(int iterations) {
    volatile int barrier = 0;  /* Create scheduling barrier */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides enough iterations */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 1; j < 31; j++) {
            /* Chain of dependent arithmetic operations */
            int t1 = arr1[j-1] + arr2[j+1];
            int t2 = t1 * arr1[j];
            float ft1 = farr1[j-1] * farr2[j+1];
            float ft2 = ft1 + farr1[j];
            
            /* Volatile read creates scheduling barrier */
            barrier = arr1[j];
            
            /* Conditional with side effects in both branches */
            if ((t2 + (int)ft2) % 7 > 3) {
                /* Branch 1: complex operations */
                arr1[j] = t2 ^ (arr2[j] << 2);
                farr1[j] = ft2 * 1.732f - farr2[j];
                
                /* More dependent calculations */
                int t3 = arr1[j] * 3 - arr2[j-1];
                float ft3 = farr1[j] / 2.0f + farr2[j+1];
                
                /* Inline assembly as scheduling boundary */
                asm volatile("" : : : "memory");
                
                /* Use values computed earlier */
                arr2[j] = t3 + (int)(ft3 * 100);
                farr2[j] = ft3 * 0.707f;
            } else {
                /* Branch 2: different complex operations */
                arr1[j] = t2 | (arr2[j] >> 1);
                farr1[j] = ft2 / 1.414f + farr2[j];
                
                /* Alternative dependent calculations */
                int t3 = arr1[j] + arr2[j+1] * 2;
                float ft3 = farr1[j] * 3.141f - farr2[j-1];
                
                /* Another inline assembly barrier */
                asm volatile("" : : : "memory");
                
                /* Different usage pattern */
                arr2[j] = t3 - (int)(ft3 * 50);
                farr2[j] = ft3 / 2.718f;
            }
            
            /* Extended live range usage - values used much later */
            int final_calc = (arr1[j] * 2 + arr2[j]) / 3;
            float final_fcalc = (farr1[j] + farr2[j]) * 0.5f;
            
            /* Mix integer and float operations */
            sum += final_calc + (int)final_fcalc;
            
            /* More arithmetic to increase pressure */
            arr1[j] = (arr1[j] + 1) & 0xFF;
            arr2[j] = (arr2[j] - 1) | 0x1;
            farr1[j] = farr1[j] * 0.99f;
            farr2[j] = farr2[j] * 1.01f;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = arr1[31];
        arr2[0] = arr2[31];
        farr1[0] = farr1[31];
        farr2[0] = farr2[31];
    }
    
    return sum;
}

/* Second complex function to increase scheduling complexity */
__attribute__((noinline,noipa))
static int secondary_sched(int n) {
    int matrix[8][8];
    int i, j, total = 0;
    
    /* Initialize matrix */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Complex nested loops with data dependencies */
    for (i = 1; i < 7; i++) {
        for (j = 1; j < 7; j++) {
            /* Multiple dependent calculations */
            int a = matrix[i-1][j] * 3;
            int b = matrix[i][j-1] * 2;
            int c = matrix[i+1][j] / 2;
            int d = matrix[i][j+1] / 3;
            
            /* Conditional with arithmetic in both paths */
            if ((a + b) > (c + d)) {
                matrix[i][j] = (a << 2) | (b & 0xF);
                asm volatile("" : : : "memory");
                total += matrix[i][j] * i;
            } else {
                matrix[i][j] = (c >> 1) ^ (d * 3);
                asm volatile("" : : : "memory");
                total += matrix[i][j] * j;
            }
            
            /* More operations to extend live ranges */
            matrix[i][j] += matrix[i-1][j-1] + matrix[i+1][j+1];
        }
    }
    
    return total;
}

int main(void) {
    int result1, result2, final_sum;
    
    printf("Starting selective scheduling test...\n");
    
    /* Call the complex function multiple times */
    result1 = stress_sched(100);
    result2 = secondary_sched(50);
    
    /* Combine results to prevent optimization */
    final_sum = result1 + result2;
    
    /* Use result to ensure execution */
    printf("Result checksum: %d\n", final_sum);
    
    /* Additional test with different parameters */
    result1 = stress_sched(50);
    result2 = secondary_sched(25);
    final_sum += result1 + result2;
    
    printf("Final checksum: %d\n", final_sum);
    
    return (final_sum > 0) ? 0 : 1;
}
