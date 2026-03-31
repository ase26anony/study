/* test_sel_sched_dump.c
 * Designed to trigger selective scheduling RTL dumps in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops 
 *               -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity isn't reduced */
static void __attribute__((noinline)) stress_sched(int iterations) {
    /* Local arrays to create register pressure and memory dependencies */
    volatile int arr_a[32];
    volatile int arr_b[32];
    volatile float arr_f[32];
    int temp[32];
    
    /* Initialize arrays with pattern to prevent dead code elimination */
    for (int i = 0; i < 32; i++) {
        arr_a[i] = i * 3;
        arr_b[i] = i * 7;
        arr_f[i] = i * 1.5f;
        temp[i] = 0;
    }
    
    /* Outer loop - provides multiple iterations for scheduler to analyze */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent arithmetic operations creating register pressure */
            int t1 = arr_a[i-1] * 3 + outer;
            int t2 = arr_b[i] * 7 - t1;
            float f1 = arr_f[i] * 2.0f + (float)t1;
            float f2 = arr_f[i+1] * 3.0f - (float)t2;
            
            /* Volatile reads/writes create scheduling barriers */
            volatile int barrier = arr_a[i];
            (void)barrier;  /* Use barrier to prevent optimization */
            
            /* Conditional execution with side effects in both branches */
            if ((t1 * t2 + (int)f1) % 7 > 3) {
                /* Branch 1: Complex arithmetic chain */
                int t3 = t1 * 2 - t2;
                float f3 = f1 * 1.5f + f2;
                arr_a[i] = t3 + (int)f3;
                arr_f[i] = f3 * 0.75f;
                
                /* More operations to extend live ranges */
                temp[i] = t3 * 3 + (int)(f3 * 2.0f);
            } else {
                /* Branch 2: Different arithmetic pattern */
                int t3 = t2 * 3 + t1;
                float f3 = f2 * 2.0f - f1;
                arr_b[i] = t3 - (int)f3;
                arr_f[i] = f3 * 1.25f;
                
                /* Different operations to challenge scheduler */
                temp[i] = t3 * 2 - (int)(f3 * 3.0f);
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier in the loop */
            /* This extends live ranges and creates complex dependencies */
            if (i > 10) {
                int delayed_use = temp[i-10] * 2 + arr_a[i-5];
                arr_b[i-5] = delayed_use % 256;
                
                /* More arithmetic to increase pressure */
                float f_delayed = arr_f[i-8] * 1.1f;
                arr_f[i] = arr_f[i] + f_delayed;
            }
            
            /* Additional arithmetic to create more ILP opportunities */
            int t4 = arr_a[i] * arr_b[i];
            float f4 = arr_f[i] * arr_f[i-1];
            temp[i] = temp[i] + t4 + (int)f4;
            
            /* Another volatile barrier */
            volatile int barrier2 = arr_b[i];
            (void)barrier2;
        }
        
        /* Cross-iteration dependencies to prevent loop unrolling from simplifying too much */
        arr_a[0] = arr_a[31] * 2;
        arr_b[0] = arr_b[1] + outer;
        arr_f[0] = arr_f[31] * 0.9f;
    }
}

/* Helper function to create additional scheduling complexity */
static int __attribute__((noinline)) checksum_array(volatile int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Complex indexing to prevent simple analysis */
        int idx = (i * 13) % size;
        sum = sum * 31 + arr[idx];
        
        /* Conditional with arithmetic */
        if (sum % 2 == 0) {
            sum = sum / 2;
        } else {
            sum = sum * 3 + 1;
        }
    }
    return sum;
}

int main() {
    int total_checksum = 0;
    
    /* Multiple calls with different iteration counts */
    for (int run = 0; run < 3; run++) {
        stress_sched(50 + run * 10);
        
        /* Create arrays for checksum calculation */
        volatile int check_arr[32];
        for (int i = 0; i < 32; i++) {
            check_arr[i] = (i * 17 + run) % 256;
        }
        
        int run_checksum = checksum_array(check_arr, 32);
        total_checksum = total_checksum * 59 + run_checksum;
        
        printf("Run %d checksum: %d\n", run, run_checksum);
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use result to prevent dead code elimination */
    if (total_checksum > 1000000) {
        printf("Result is large\n");
    }
    
    return total_checksum % 256;
}
