/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int result = 0;
    volatile int counter = 0;
    volatile int dep_chain = mode_flag;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with complex dependency chains */
        for (int i = 0; i < 128; i++) {
            /* Create multiple basic blocks with if-else chains */
            if ((i & 0x1F) == 0) {
                /* Branch 1: Long dependency chain with arithmetic */
                dep_chain = arr1[i] * dep_chain + arr2[255 - i];
                dep_chain = (dep_chain ^ (dep_chain >> 3)) + i;
                dep_chain = dep_chain * 1103515245 + 12345;
                
                /* Memory barrier to split scheduling regions */
                __asm__ volatile ("" : : : "memory");
                
                /* More arithmetic with memory access */
                arr2[i] = (arr1[i] ^ dep_chain) + (arr2[i] * 3);
                result += dep_chain & 0xFF;
            } 
            else if ((i & 0x0F) == 0) {
                /* Branch 2: Different operations with memory loads */
                volatile int temp = arr2[i] - arr1[255 - i];
                temp = (temp << 2) | (temp >> 30);
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                
                dep_chain = temp ^ dep_chain;
                arr1[i] = dep_chain + (i * 7);
                result ^= temp;
            }
            else if ((i & 0x07) == 0) {
                /* Branch 3: Mix of operations */
                volatile int a = arr1[i];
                volatile int b = arr2[i];
                volatile int c = a * b + dep_chain;
                
                /* Complex dependency chain */
                for (int j = 0; j < 3; j++) {
                    c = (c * 1664525 + 1013904223) & 0x7FFFFFFF;
                    __asm__ volatile ("" : : : "memory");
                }
                
                dep_chain = c ^ (a + b);
                arr2[255 - i] = dep_chain;
                result += c;
            }
            else {
                /* Branch 4: Simple arithmetic but with function call possibility */
                if ((mode_flag & 0x1) && (i == 63)) {
                    /* Occasionally introduce a function call */
                    counter += getpid() & 0xFF;
                }
                
                /* Independent parallel operations to fill instruction queue */
                volatile int x = arr1[i] + i;
                volatile int y = arr2[i] - i;
                volatile int z = x * y;
                volatile int w = x ^ y ^ z;
                
                /* Multiple memory barriers to create scheduling boundaries */
                __asm__ volatile ("" : : : "memory");
                arr1[i] = z + w;
                __asm__ volatile ("" : : : "memory");
                arr2[i] = z - w;
                __asm__ volatile ("" : : : "memory");
                
                dep_chain = (dep_chain + z) ^ w;
                result = result * 13 + dep_chain;
            }
            
            /* Switch-like behavior based on pseudo-random pattern */
            switch (i & 0x3) {
                case 0:
                    dep_chain = dep_chain + (result << 1);
                    break;
                case 1:
                    dep_chain = dep_chain ^ (result >> 2);
                    break;
                case 2:
                    dep_chain = dep_chain * 3 - result;
                    break;
                case 3:
                    dep_chain = (dep_chain & result) | 0x5555;
                    break;
            }
            
            /* Final memory barrier in the loop */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Inter-loop operations with volatile accesses */
        if (outer & 0x1) {
            volatile int mix = 0;
            for (int k = 0; k < 16; k++) {
                mix = mix ^ arr1[k * 8] ^ arr2[k * 8 + 4];
                __asm__ volatile ("" : : : "memory");
            }
            result += mix;
        }
    }
    
    return result + counter;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int iterations) {
    volatile int acc = 0;
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Multi-way branching with bit tests */
        for (int i = 0; i < 64; i++) {
            int pattern = arr1[i] ^ arr2[63 - i];
            
            /* Series of if-else-if creating many basic blocks */
            if (pattern & 0x00000001) {
                acc += arr1[i] * 2;
                __asm__ volatile ("" : : : "memory");
            }
            if (pattern & 0x00000002) {
                acc -= arr2[i] / 3;
            }
            if (pattern & 0x00000004) {
                acc ^= (arr1[i] + arr2[i]);
                __asm__ volatile ("" : : : "memory");
            }
            if (pattern & 0x00000008) {
                acc = (acc << 1) | (acc >> 31);
            }
            if (pattern & 0x00000010) {
                volatile int temp = clock() & 0xFF;
                acc += temp;
            }
            if (pattern & 0x00000020) {
                acc = acc * 1103515245 + 12345;
            }
            if (pattern & 0x00000040) {
                __asm__ volatile ("" : : : "memory");
                arr1[i] = acc ^ i;
            }
            if (pattern & 0x00000080) {
                arr2[63 - i] = acc + i;
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() & 0xFFFF;
        array2[i] = rand() & 0xFFFF;
    }
    
    volatile int checksum = 0;
    volatile int outer_limit = 3;  /* Volatile to prevent constant propagation */
    volatile int mode_switch = 0;
    
    /* Multiple calls to create different scheduling contexts */
    for (int call_num = 0; call_num < 8; call_num++) {
        mode_switch = call_num & 0x3;
        
        /* Vary loop bounds slightly */
        outer_limit = 2 + (call_num & 0x1);
        
        /* Call the complex scheduling function */
        volatile int result1 = complex_schedule_loop(array1, array2, 
                                                    outer_limit, mode_switch);
        
        /* Alternate between different scheduling patterns */
        if (call_num & 0x1) {
            volatile int result2 = alternate_schedule_pattern(array1, array2, 
                                                             1 + (call_num & 0x3));
            checksum ^= result1 + result2;
        } else {
            checksum ^= result1 * 31;
        }
        
        /* Modify array contents between calls */
        for (int i = 0; i < 256; i += 8) {
            array1[i] = (array1[i] * 13 + call_num) & 0xFFFF;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum ^= array1[i] ^ array2[i];
        __asm__ volatile ("" : : : "memory");
    }
    
    checksum ^= final_sum;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", checksum);
    
    return 0;
}
