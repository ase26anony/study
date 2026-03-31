/* test_sel_sched_dump.c
 * Test to trigger selective scheduling RTL dumps in GCC.
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable function to force selective scheduling analysis */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int barrier;  /* Creates scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with pattern to prevent dead code elimination */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    barrier = 0;  /* First scheduling barrier */
    
    /* Outer loop - provides enough iterations for scheduling analysis */
    for (i = 0; i < iterations; i++) {
        int temp1 = arr1[i & 31];
        int temp2 = arr2[i & 31];
        float ftemp1 = farr1[i & 31];
        float ftemp2 = farr2[i & 31];
        
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 128; j++) {
            /* Chain of dependent arithmetic operations creating register pressure */
            int idx = (j + i) & 31;
            int a = arr1[idx] * 3;
            int b = arr2[idx] * 7;
            int c = a + b;
            int d = c * 11;
            int e = d - arr1[(idx + 1) & 31];
            int f = e ^ arr2[(idx + 2) & 31];
            
            float fa = farr1[idx] * 1.7f;
            float fb = farr2[idx] * 3.3f;
            float fc = fa + fb;
            float fd = fc * 2.1f;
            float fe = fd - farr1[(idx + 1) & 31];
            
            /* Conditional execution with side effects */
            if ((f & 1) != 0) {
                /* Branch 1: Integer-heavy operations */
                int g = f * 13;
                int h = g >> 2;
                int k = h | 0x5555;
                arr1[idx] = k + temp1;
                temp1 = (temp1 * 3) & 0xFFF;
                
                /* More FP ops in this branch */
                float ff = fe * 1.3f;
                farr1[idx] = ff + ftemp1;
                ftemp1 = ftemp1 * 1.1f;
            } else {
                /* Branch 2: Different arithmetic pattern */
                int g = f * 17;
                int h = g << 1;
                int k = h & 0xAAAA;
                arr2[idx] = k + temp2;
                temp2 = (temp2 * 5) & 0xFFF;
                
                /* Different FP ops */
                float ff = fe * 1.7f;
                farr2[idx] = ff + ftemp2;
                ftemp2 = ftemp2 * 1.3f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed earlier after the barrier */
            arr1[(idx + 3) & 31] = f + d;
            arr2[(idx + 4) & 31] = e ^ c;
            
            /* More FP operations extending live ranges */
            farr1[(idx + 3) & 31] = fe + fd;
            farr2[(idx + 4) & 31] = fc * fa;
            
            /* Additional dependent operations creating complex DAG */
            int m = arr1[idx] + arr2[idx];
            float fm = farr1[idx] + farr2[idx];
            arr1[(idx + 5) & 31] = m * 2;
            farr1[(idx + 5) & 31] = fm * 1.5f;
            
            /* Volatile read creates another scheduling barrier */
            barrier = arr1[(idx + 6) & 31];
        }
        
        /* Cross-iteration dependencies */
        arr1[i & 31] = temp1;
        arr2[i & 31] = temp2;
        farr1[i & 31] = ftemp1;
        farr2[i & 31] = ftemp2;
    }
    
    /* Compute checksum result */
    int sum = 0;
    float fsum = 0.0f;
    for (i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
        fsum += farr1[i] + farr2[i];
    }
    *result = sum + (int)fsum;
}

/* Secondary complex function to increase scheduling opportunities */
static void __attribute__((noinline,noipa))
another_sched_stress(int n, int *out) {
    int matrix[8][8];
    int i, j, k;
    
    /* Initialize matrix */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Matrix-like operations with dependencies */
    for (k = 0; k < n; k++) {
        for (i = 0; i < 8; i++) {
            int row_sum = 0;
            for (j = 0; j < 8; j++) {
                /* Complex addressing calculations */
                int idx = (i + j + k) & 7;
                int val = matrix[i][idx] * matrix[idx][j];
                val = (val + matrix[j][i]) * 3;
                val = val ^ (1 << (j & 3));
                
                /* Conditional with both branches having side effects */
                if (val > 1000) {
                    matrix[i][j] = val >> 2;
                    row_sum += val & 0xFF;
                } else {
                    matrix[i][j] = val << 1;
                    row_sum += val | 0x80;
                }
                
                /* Scheduling barrier */
                asm volatile("" ::: "memory");
            }
            matrix[i][i] += row_sum;
        }
    }
    
    /* Final reduction */
    int total = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            total += matrix[i][j];
        }
    }
    *out = total;
}

int main(void) {
    int result1, result2;
    int i;
    
    /* Call stress functions multiple times to ensure execution */
    for (i = 0; i < 3; i++) {
        stress_sched(100, &result1);
        another_sched_stress(50, &result2);
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    return 0;
}
