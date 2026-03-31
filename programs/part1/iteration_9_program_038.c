/* haifa-sched-trigger.c
 * Program designed to trigger scheduling context save/restore in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 50

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int limit = (iter % 7) + 5;  /* Volatile-like calculation */
    volatile int result = 0;
    volatile int temp_regs[8] = {0};
    
    /* Outer loop with volatile bound to prevent optimization */
    for (i = 0; i < limit; i++) {
        volatile int inner_limit = (mode + i) % 10 + 15;
        volatile int branch_selector = (iter * 31 + i * 17) % 13;
        
        /* Complex inner loop with multiple dependency chains */
        for (j = 0; j < inner_limit; j++) {
            volatile int idx = (i * 19 + j * 23) % ARRAY_SIZE;
            volatile int op_type = (idx + iter) % 8;
            
            /* Long dependency chain 1 */
            temp_regs[0] = arr1[idx] * arr2[idx] + iter;
            temp_regs[1] = temp_regs[0] ^ (arr1[(idx + 1) % ARRAY_SIZE]);
            temp_regs[2] = temp_regs[1] >> (j % 5);
            temp_regs[3] = temp_regs[2] * temp_regs[0] - temp_regs[1];
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Long dependency chain 2 (independent) */
            temp_regs[4] = arr2[(idx + 3) % ARRAY_SIZE] + arr1[(idx + 5) % ARRAY_SIZE];
            temp_regs[5] = temp_regs[4] * temp_regs[4] / (j + 1);
            temp_regs[6] = temp_regs[5] | temp_regs[2];
            temp_regs[7] = temp_regs[6] & 0xFFFF;
            
            /* Multiple basic blocks created by if-else chain */
            if (op_type == 0) {
                arr1[idx] = temp_regs[3] + temp_regs[7];
                /* Function call in one branch */
                if ((j & 1) && (mode & 1)) {
                    volatile int pid = getpid();
                    arr2[idx] = temp_regs[7] ^ (pid & 0xFF);
                }
            } else if (op_type == 1) {
                arr1[idx] = temp_regs[3] - temp_regs[7];
                arr2[idx] = temp_regs[5] * 2;
            } else if (op_type == 2) {
                arr1[idx] = temp_regs[3] | temp_regs[7];
                arr2[idx] = temp_regs[6] >> 1;
            } else if (op_type == 3) {
                arr1[idx] = temp_regs[3] & temp_regs[7];
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                arr2[idx] = temp_regs[4] + iter;
            } else if (op_type == 4) {
                arr1[idx] = temp_regs[3] ^ temp_regs[7];
                arr2[idx] = temp_regs[2] - j;
            } else if (op_type == 5) {
                arr1[idx] = temp_regs[7] * temp_regs[1];
                /* Complex bit manipulation chain */
                volatile int bit_var = temp_regs[0];
                for (k = 0; k < 4; k++) {
                    bit_var = (bit_var << 2) | ((bit_var >> 6) & 0x3);
                }
                arr2[idx] = bit_var;
            } else if (op_type == 6) {
                arr1[idx] = temp_regs[3] + temp_regs[5] + temp_regs[7];
                /* Switch-like behavior using bit tests */
                volatile int bits = temp_regs[6];
                if (bits & (1 << 0)) arr2[idx] += 1;
                if (bits & (1 << 1)) arr2[idx] *= 2;
                if (bits & (1 << 2)) arr2[idx] ^= 0xAA;
                if (bits & (1 << 3)) arr2[idx] = ~arr2[idx];
                if (bits & (1 << 4)) {
                    volatile clock_t clk = clock();
                    arr2[idx] ^= (clk & 0xFF);
                }
            } else { /* op_type == 7 */
                arr1[idx] = temp_regs[3] / (temp_regs[7] + 1);
                arr2[idx] = temp_regs[0] % (temp_regs[4] + 1);
            }
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Cross-iteration dependency */
            if (j > 0) {
                arr1[(idx + 2) % ARRAY_SIZE] += arr1[idx] % 17;
            }
            
            /* Multi-way branch based on branch_selector */
            switch (branch_selector % 4) {
                case 0:
                    result += arr1[idx];
                    break;
                case 1:
                    result -= arr2[idx];
                    break;
                case 2:
                    result ^= arr1[idx] * arr2[idx];
                    break;
                case 3:
                    result = (result << 3) | ((result >> 5) & 0x7);
                    /* Final memory barrier in this path */
                    __asm__ volatile ("" : : : "memory");
                    break;
            }
        }
        
        /* Outer loop dependency */
        if (i > 0) {
            volatile int outer_idx = (i * 11) % ARRAY_SIZE;
            arr2[outer_idx] = arr1[outer_idx] + result;
        }
    }
    
    return result;
}

/* Secondary complex function with different patterns */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2,
                          volatile int seed, volatile int rounds) {
    volatile int i, j;
    volatile int acc = seed;
    
    for (i = 0; i < rounds; i++) {
        volatile int pattern = (seed + i * 97) % 5;
        volatile int steps = (i % 3) + 8;
        
        for (j = 0; j < steps; j++) {
            volatile int idx = (i * 29 + j * 43) % ARRAY_SIZE;
            
            /* Different arithmetic patterns */
            switch (pattern) {
                case 0:
                    arr1[idx] = (arr1[idx] * 3 + arr2[idx]) ^ acc;
                    arr2[idx] = (arr2[idx] << 1) | ((arr2[idx] >> 7) & 1);
                    break;
                case 1:
                    arr1[idx] = arr1[idx] + (arr2[idx] * arr2[idx]);
                    arr2[idx] = arr1[idx] - arr2[idx];
                    break;
                case 2:
                    arr1[idx] = arr1[idx] | (arr2[idx] & acc);
                    arr2[idx] = arr2[idx] ^ (arr1[idx] * j);
                    break;
                case 3:
                    arr1[idx] = (arr1[idx] % 31) * (arr2[idx] % 17);
                    arr2[idx] = arr2[idx] + (i * j);
                    /* Scheduling barrier */
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 4:
                    arr1[idx] = ~(arr1[idx] & arr2[idx]);
                    arr2[idx] = arr2[idx] + getpid() % 127;
                    break;
            }
            
            acc = (acc * 31 + arr1[idx]) % 1048576;
            
            /* Conditional function call */
            if ((idx % 7) == 0) {
                volatile int dummy = clock();
                acc ^= (dummy & 0xFFF);
            }
        }
        
        /* Memory barrier between outer loop iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    return acc;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create different scheduling contexts */
    for (i = 0; i < 8; i++) {
        volatile int mode = i % 4;
        volatile int iter_limit = (i * 3 + 5) % MAX_LOOP_ITER;
        
        /* Call main scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, i);
        
        /* Call alternate pattern function */
        volatile int res2 = alternate_schedule_pattern(array1, array2, res1, iter_limit);
        
        /* Use results to prevent elimination */
        checksum ^= res1;
        checksum ^= res2;
        
        /* Modify mode for next iteration */
        mode = (mode + res1) % 8;
    }
    
    /* Final computation to use all array elements */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
        
        /* Additional complex operation */
        if (i % 3 == 0) {
            array1[i] = (array1[i] * array2[i]) ^ checksum;
        } else if (i % 3 == 1) {
            array2[i] = (array2[i] + array1[i]) | checksum;
        } else {
            volatile int temp = array1[i] * 3 - array2[i];
            array1[i] = temp ^ (checksum >> (i % 8));
            array2[i] = temp & (checksum << (i % 8));
        }
    }
    
    /* Final checksum computation */
    volatile int final_checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array1[i];
        final_checksum ^= array2[i];
        final_checksum = (final_checksum << 1) | ((final_checksum >> 31) & 1);
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
