/* Test to trigger selective scheduling RTL dumps in sel-sched-dump.cc */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 12345;  /* volatile to create scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < iterations; j++) {
        int temp1 = seed;
        int temp2 = j * 7;
        float ftemp1 = j * 1.1f;
        float ftemp2 = j * 2.2f;
        
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int idx1 = (temp1 + i) & 31;
            int idx2 = (temp2 + i * 3) & 31;
            int idx3 = (temp1 * 2 + i) & 31;
            
            /* Multiple dependent calculations extending live ranges */
            int calc1 = arr1[idx1] * 3 + arr2[idx2];
            int calc2 = arr1[idx2] / 2 - arr2[idx1];
            int calc3 = calc1 * calc2 + temp1;
            int calc4 = calc3 ^ (calc1 + calc2);
            
            /* Floating point calculations mixed in */
            int fidx = i & 15;
            float fcalc1 = farr1[fidx] * 3.14f + ftemp1;
            float fcalc2 = farr2[fidx] * 2.71f - ftemp2;
            float fcalc3 = fcalc1 * fcalc2;
            
            /* Conditional with side effects in both branches */
            if (calc3 > calc4 * 2) {
                /* Branch 1: different calculations */
                arr1[idx3] = calc4 + (calc1 >> 2);
                arr2[idx1] = calc3 - (calc2 << 1);
                farr1[fidx] = fcalc3 * 0.5f;
                
                /* Additional computation only in this branch */
                temp1 = (temp1 * 1103515245 + 12345) & 0x7fffffff;
            } else {
                /* Branch 2: alternative calculations */
                arr1[idx1] = calc3 ^ calc4;
                arr2[idx3] = calc1 + calc2 * 3;
                farr2[fidx] = fcalc1 + fcalc2;
                
                /* Different computation in this branch */
                temp2 = (temp2 * 1664525 + 1013904223) & 0x7fffffff;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            if (i > 15) {
                /* Cross-iteration dependency */
                arr1[i-15] += calc3;
                arr2[i-15] ^= calc4;
                
                /* More floating point ops */
                fcalc3 = farr1[(i-8)&15] * farr2[(i-12)&15];
                ftemp1 = fcalc3 * 0.25f;
            }
            
            /* Additional dependent calculations after the barrier */
            int final_calc = arr1[i] + arr2[i] + calc3;
            arr1[i] = final_calc ^ temp1;
            arr2[i] = final_calc & temp2;
            
            /* Another volatile read to create scheduling barrier */
            seed = seed + 1;
        }
        
        /* Update seed for next outer iteration */
        seed = temp1 ^ temp2;
    }
    
    /* Compute checksum result */
    int sum = 0;
    for (i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
    }
    for (i = 0; i < 16; i++) {
        sum += (int)farr1[i] + (int)farr2[i];
    }
    *result = sum;
}

/* Another complex function with different pattern */
static void __attribute__((noinline))
complex_calculation(int n, int *output)
{
    int matrix[8][8];
    int i, j, k;
    
    /* Initialize matrix */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Nested loops with computations */
    for (k = 0; k < n; k++) {
        for (i = 1; i < 7; i++) {
            for (j = 1; j < 7; j++) {
                /* Stencil computation with multiple dependencies */
                int up = matrix[i-1][j];
                int down = matrix[i+1][j];
                int left = matrix[i][j-1];
                int right = matrix[i][j+1];
                
                /* Complex conditional */
                if ((up + down) > (left + right)) {
                    matrix[i][j] = (up * 3 - down * 2) ^ 
                                   (left + right * 5);
                    /* Inline asm barrier */
                    asm volatile("" ::: "memory");
                } else {
                    matrix[i][j] = (down * 7 + up * 11) | 
                                   (left * 13 - right * 17);
                }
                
                /* Post-calculation */
                matrix[i][j] += k * (i + j);
            }
        }
    }
    
    /* Compute result */
    int total = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            total ^= matrix[i][j];
        }
    }
    *output = total;
}

int main(void)
{
    int result1, result2;
    int i;
    
    /* Call stress functions multiple times */
    for (i = 0; i < 3; i++) {
        stress_sched(100 + i * 50, &result1);
        complex_calculation(50 + i * 25, &result2);
        
        /* Use results to prevent optimization */
        printf("Iteration %d: results = %d, %d\n", 
               i, result1, result2);
    }
    
    return 0;
}
