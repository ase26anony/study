/* test_sel_sched_dump.c
 * Designed to trigger selective scheduling RTL dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force function not to be inlined to maintain complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) 
{
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with complex patterns */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3 + seed;
        arr2[i] = i * 5 - seed;
        farr1[i] = i * 1.5f + seed;
        farr2[i] = i * 2.5f - seed;
    }
    
    /* Outer loop - provides enough iterations for scheduling analysis */
    for (j = 0; j < iterations; j++) {
        int temp1 = arr1[j & 31];
        int temp2 = arr2[j & 31];
        float ftemp1 = farr1[j & 31];
        float ftemp2 = farr2[j & 31];
        
        /* Complex inner loop with high register pressure */
        for (i = 0; i < 32; i++) {
            /* Chain of dependent integer operations */
            int a = temp1 + i;
            int b = a * temp2;
            int c = b - arr1[i];
            int d = c ^ arr2[(i + 1) & 31];
            int e = d << (i & 3);
            int f = e + (temp1 >> 2);
            
            /* Chain of dependent floating-point operations */
            float fa = ftemp1 * i;
            float fb = fa + ftemp2;
            float fc = fb / (farr1[i] + 1.0f);
            float fd = fc - farr2[(i + 2) & 31];
            float fe = fd * 1.125f;
            float ff = fe + (ftemp1 * 0.5f);
            
            /* Conditional execution with side effects */
            if ((f & 7) > (i & 3)) {
                /* Branch 1: Different arithmetic pattern */
                arr1[i] = f + (d >> 1);
                arr2[i] = e - (a * 2);
                farr1[i] = ff * 0.75f;
                farr2[i] = fe - fa;
                
                /* Additional computation in this branch */
                temp1 = (temp1 + arr1[(i + 3) & 31]) & 0xFFF;
                ftemp1 = ftemp1 * 0.9f + farr1[(i + 2) & 31];
            } else {
                /* Branch 2: Alternative computation pattern */
                arr1[i] = e + (f >> 2);
                arr2[i] = d - (b / 4);
                farr1[i] = fe * 1.25f;
                farr2[i] = ff + fb;
                
                /* Different computation in else branch */
                temp2 = (temp2 ^ arr2[(i + 4) & 31]) | 1;
                ftemp2 = ftemp2 * 1.1f - farr2[(i + 1) & 31];
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier - extends live ranges */
            if (i > 16) {
                arr1[i] += a;  /* Use 'a' computed at start of loop */
                farr1[i] += fa; /* Use 'fa' computed at start of loop */
            }
            
            /* More computations using mixed types */
            int g = arr1[i] + arr2[(i + 8) & 31];
            float fg = farr1[i] * farr2[(i + 8) & 31];
            
            /* Store results with complex indexing */
            arr1[(i + 16) & 31] = g + (j & 0xFF);
            farr1[(i + 16) & 31] = fg * (1.0f + (j % 100) * 0.01f);
        }
        
        /* Update result with checksum */
        *result += arr1[j & 31] + arr2[j & 31] + (int)farr1[j & 31] + (int)farr2[j & 31];
    }
}

int main(void) 
{
    int result = 0;
    int i;
    
    /* Call stress function multiple times to ensure execution */
    for (i = 0; i < 5; i++) {
        stress_sched(100, &result);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Additional complexity to prevent optimization */
    if (result > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
