/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa_test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling-intensive function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int e = 5, f = 6, g = 7, h = 8;
    volatile int result = 0;
    
    /* Volatile outer loop limit - prevents compile-time simplification */
    volatile int outer_limit = (mode % 3) + 3;
    
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Create scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with volatile limit */
        volatile int inner_limit = (iter % 8) + 12;
        for (volatile int inner = 0; inner < inner_limit; inner++) {
            /* Complex dependency chain 1 */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");
            e = a ^ f;
            g = e >> (inner & 7);
            h = g * arr1[inner % 256];
            
            /* Memory access pattern */
            arr2[(inner + outer) % 256] = h;
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Complex dependency chain 2 */
            b = c + d * e;
            f = g ^ h;
            d = f - a;
            c = b >> (outer & 3);
            
            /* Multi-way branch to create multiple basic blocks */
            int branch_selector = (inner + outer + mode) & 0xF;
            
            if (branch_selector < 4) {
                /* Branch 1: Arithmetic operations */
                a = b * c + arr1[(inner + 1) % 256];
                e = d ^ f;
                __asm__ volatile ("" : : : "memory");
                result += a * e;
            } else if (branch_selector < 8) {
                /* Branch 2: Shift/rotate operations */
                g = (h << 3) | (h >> 29);
                arr2[inner % 256] = g ^ arr1[(inner + 2) % 256];
                result ^= g;
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector < 12) {
                /* Branch 3: Mixed operations with memory barrier */
                volatile int temp = arr1[(inner + 3) % 256];
                a = temp * b + c;
                __asm__ volatile ("" : : : "memory");
                d = a ^ temp;
                result |= d;
            } else {
                /* Branch 4: Potential function call (guarded by volatile) */
                if ((mode & 1) && (iter % 7 == 0)) {
                    /* Introduce function call to complicate scheduling */
                    volatile int pid = getpid();
                    arr2[(inner + pid) % 256] = pid & 0xFF;
                }
                __asm__ volatile ("" : : : "memory");
                result = result * 1103515245 + 12345;
            }
            
            /* Switch-like structure using bit tests */
            int switch_val = inner & 7;
            if (switch_val & 1) {
                a = b + c;
                __asm__ volatile ("" : : : "memory");
            }
            if (switch_val & 2) {
                d = e - f;
            }
            if (switch_val & 4) {
                g = h * arr1[(inner + 4) % 256];
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Final memory store with barrier */
            arr1[inner % 256] = result ^ (a + b + c + d + e + f + g + h);
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Additional scheduling complexity between outer loop iterations */
        if (outer % 2 == 0) {
            volatile int temp_sum = 0;
            for (int i = 0; i < 16; i++) {
                temp_sum += arr1[(outer * 16 + i) % 256];
                __asm__ volatile ("" : : : "memory");
            }
            result ^= temp_sum;
        } else {
            volatile int temp_prod = 1;
            for (int i = 0; i < 8; i++) {
                temp_prod *= arr2[(outer * 8 + i) % 256] | 1;
                __asm__ volatile ("" : : : "memory");
            }
            result += temp_prod;
        }
    }
    
    return result;
}

/* Secondary function to create different scheduling patterns */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr, volatile int seed) {
    volatile int x = seed, y = seed * 3, z = seed * 7;
    volatile int limit = (seed % 5) + 10;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Long dependency chain */
        x = y * z + arr[i % 256];
        __asm__ volatile ("" : : : "memory");
        y = x ^ (z << 3);
        z = y - arr[(i + 1) % 256];
        
        /* Conditional with memory access */
        if (i & 1) {
            arr[i % 256] = x + y;
            __asm__ volatile ("" : : : "memory");
        } else {
            arr[i % 256] = z - y;
        }
        
        /* Nested conditionals for basic block creation */
        switch (i & 3) {
            case 0:
                x = x * 1664525 + 1013904223;
                break;
            case 1:
                y = y * 1103515245 + 12345;
                __asm__ volatile ("" : : : "memory");
                break;
            case 2:
                z = (z >> 16) ^ (z & 0xFFFF);
                break;
            case 3:
                arr[(i + 64) % 256] = x ^ y ^ z;
                __asm__ volatile ("" : : : "memory");
                break;
        }
    }
    
    return x + y + z;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to repeatedly trigger scheduling contexts */
    for (int main_iter = 0; main_iter < 8; main_iter++) {
        /* Volatile variables for loop bounds */
        volatile int outer_bound = (main_iter % 4) + 2;
        
        for (volatile int outer = 0; outer < outer_bound; outer++) {
            /* Call core scheduling function multiple times */
            volatile int res1 = complex_schedule_loop(array1, array2, 
                                                     mode_switch, main_iter);
            
            __asm__ volatile ("" : : : "memory");
            
            /* Alternate between different scheduling patterns */
            if (main_iter & 1) {
                volatile int res2 = alternate_schedule_pattern(array1, res1);
                checksum ^= res2;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Modify mode for next iteration */
            mode_switch = (mode_switch + 1) % 7;
            
            /* Additional memory operations */
            for (int i = 0; i < 32; i++) {
                int idx = (outer * 32 + i) % 256;
                array2[idx] = array1[idx] ^ checksum;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Compute intermediate checksum to prevent elimination */
        volatile int temp_sum = 0;
        for (int i = 0; i < 128; i++) {
            temp_sum += array1[i] ^ array2[255 - i];
            __asm__ volatile ("" : : : "memory");
        }
        checksum += temp_sum;
    }
    
    /* Final checksum computation and output */
    volatile int final_checksum = 0;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= array1[i];
        final_checksum += array2[i];
        __asm__ volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
