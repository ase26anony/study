/* test_sel_sched_dump.c
 * Test to trigger selective scheduling RTL dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable function to ensure it gets scheduled independently */
static void __attribute__((noinline,noipa))
stress_sched(int iterations, int *result) {
    volatile int barrier;  /* Creates scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j, k;
    
    /* Initialize arrays with pattern to prevent dead code elimination */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    barrier = 0;  /* Volatile write creates scheduling boundary */
    
    /* Outer loop - provides enough iterations for scheduler to work with */
    for (i = 0; i < iterations; i++) {
        int temp_sum = 0;
        
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 32; j++) {
            int idx = (j + i) & 31;  /* Non-trivial index calculation */
            float fcalc;
            int icalc;
            
            /* Chain of dependent arithmetic operations creating register pressure */
            icalc = arr1[idx] * 3 + arr2[idx] * 7;
            fcalc = farr1[idx] * 1.7f + farr2[idx] * 3.2f;
            
            /* Volatile read creates scheduling barrier */
            barrier = arr1[(idx + 1) & 31];
            
            /* More arithmetic with mixed types */
            icalc += barrier * 2;
            fcalc += barrier * 0.5f;
            
            /* Conditional execution with side effects in both branches */
            if (icalc > 100) {
                /* Branch 1: complex calculations */
                arr1[idx] = icalc / 3 + (int)(fcalc * 2.0f);
                farr1[idx] = fcalc * 1.1f + icalc * 0.3f;
                
                /* Inline assembly as scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* More arithmetic after assembly barrier */
                arr2[idx] = arr1[idx] * 2 - icalc;
            } else {
                /* Branch 2: different calculations */
                arr1[idx] = icalc * 2 - (int)(fcalc * 1.5f);
                farr1[idx] = fcalc * 0.9f - icalc * 0.2f;
                
                /* Different inline assembly usage */
                asm volatile("nop" ::: "memory");
                
                /* Different arithmetic pattern */
                arr2[idx] = arr1[idx] + icalc * 3;
            }
            
            /* Live range extension: use values computed much earlier */
            farr2[idx] = fcalc * 0.8f + arr1[(idx + 3) & 31] * 0.1f;
            
            /* More arithmetic mixing all values */
            temp_sum += icalc + (int)fcalc + arr1[idx] + arr2[idx];
            
            /* Another volatile operation */
            barrier = temp_sum & 255;
            
            /* Final calculation using extended live ranges */
            arr2[(idx + 2) & 31] = (arr1[idx] + arr2[idx] + barrier) * 3;
        }
        
        sum += temp_sum;
        
        /* Cross-iteration dependency to prevent loop unrolling from simplifying too much */
        arr1[0] = sum & 65535;
    }
    
    *result = sum;
}

/* Second complex function to increase scheduling opportunities */
static void __attribute__((noinline,noipa))
complex_calculation(int *data, int size, int *out) {
    volatile int sync;
    int i, j;
    int accum = 0;
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Nested loop with complex addressing */
        for (j = 0; j < 8; j++) {
            int idx = (i * 8 + j) % size;
            
            /* Multiple dependent operations */
            val = val * 3 + data[idx];
            val = (val << 3) | (val >> 29);  /* Rotation */
            val ^= 0x5A5A5A5A;
            
            /* Conditional with arithmetic in both paths */
            if (val & 1) {
                val += data[(idx + 1) % size] * 7;
                asm volatile("" ::: "memory");
            } else {
                val -= data[(idx + 2) % size] * 3;
            }
            
            /* Volatile for scheduling barrier */
            sync = val;
            val += sync & 0xFF;
        }
        
        accum += val;
        data[i] = val;
    }
    
    *out = accum;
}

int main(void) {
    int result1, result2;
    int test_data[64];
    int i;
    
    /* Initialize test data */
    for (i = 0; i < 64; i++) {
        test_data[i] = i * 13 + 7;
    }
    
    /* Call the scheduling-stress function multiple times */
    stress_sched(100, &result1);
    
    /* Call second complex function */
    complex_calculation(test_data, 64, &result2);
    
    /* Final calculation to use both results */
    int final_result = result1 + result2;
    
    /* Print to prevent optimization */
    printf("Result1: %d, Result2: %d, Final: %d\n", result1, result2, final_result);
    
    return final_result != 0 ? 0 : 1;
}
