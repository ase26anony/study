/* 
 * Complex scheduling test to trigger haifa-sched.cc free_sched_context logic
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Volatile limit to prevent optimization */
    volatile int inner_limit = (mode % 4) + 10;
    
    /* Outer loop with volatile bounds */
    for (volatile int o = 0; o < outer_limit; o++) {
        volatile int state = (o * 17 + iter) % ARRAY_SIZE;
        
        /* Inner loop with complex operations */
        for (volatile int i = 0; i < inner_limit; i++) {
            int idx = (state + i) % ARRAY_SIZE;
            volatile int branch_selector = (idx + iter + o) % 7;
            
            /* Create multiple basic blocks with if-else chains */
            if (branch_selector == 0) {
                /* Long dependency chain */
                volatile int a = arr1[idx] * 3 + arr2[idx];
                volatile int b = a ^ (idx * 7);
                volatile int c = b >> (iter % 5);
                volatile int d = c * 11 - arr1[(idx + 1) % ARRAY_SIZE];
                arr2[idx] = d ^ (arr2[(idx + 2) % ARRAY_SIZE] & 0xFF);
                
                /* Memory barrier to split scheduling regions */
                __asm__ volatile ("" : : : "memory");
                
                /* More arithmetic with dependencies */
                volatile int e = arr2[idx] + arr1[(idx + 3) % ARRAY_SIZE];
                volatile int f = e * 13 / (iter + 1);
                arr1[idx] = f | (arr2[(idx + 4) % ARRAY_SIZE] << 3);
                
            } else if (branch_selector == 1) {
                /* Different arithmetic pattern */
                volatile int x = arr2[idx] - arr1[idx];
                volatile int y = x * x + 5;
                volatile int z = y % 97;
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                arr1[idx] = z ^ (iter * 19);
                arr2[idx] = arr1[idx] + (x >> 2);
                
            } else if (branch_selector == 2) {
                /* Memory intensive operations */
                volatile int temp[4];
                temp[0] = arr1[(idx + 0) % ARRAY_SIZE];
                temp[1] = arr1[(idx + 1) % ARRAY_SIZE];
                temp[2] = arr2[(idx + 2) % ARRAY_SIZE];
                temp[3] = arr2[(idx + 3) % ARRAY_SIZE];
                
                volatile int sum = 0;
                for (int j = 0; j < 4; j++) {
                    sum += temp[j] * (j + 1);
                }
                
                arr1[idx] = sum;
                arr2[idx] = sum ^ temp[0];
                
                /* Barrier after memory ops */
                __asm__ volatile ("" : : : "memory");
                
            } else if (branch_selector == 3) {
                /* Bit manipulation chain */
                volatile int val = arr1[idx];
                val = (val << 3) | (val >> 5);
                val = val ^ (0x5A5A5A5A >> (iter % 16));
                val = val + (arr2[idx] * 3);
                val = (val & 0x0F0F0F0F) | ((~val) & 0xF0F0F0F0);
                
                arr2[idx] = val;
                arr1[idx] = val * 2 - arr2[(idx + 5) % ARRAY_SIZE];
                
            } else if (branch_selector == 4) {
                /* Conditional function call to add complexity */
                volatile int should_call = (iter + idx) % 11;
                if (should_call == 0) {
                    /* Dummy system call that can't be optimized away */
                    volatile int pid = getpid();
                    arr1[idx] ^= (pid & 0xFF);
                }
                
                /* Mixed operations */
                volatile int m = arr1[idx] + arr2[idx];
                volatile int n = m * m - m;
                arr2[idx] = n % 256;
                
            } else if (branch_selector == 5) {
                /* Complex multi-step computation */
                volatile int p = arr1[idx];
                volatile int q = arr2[idx];
                
                for (int step = 0; step < 3; step++) {
                    p = (p * 3 + q) % 1000;
                    q = (q * 5 - p) % 1000;
                    __asm__ volatile ("" : : : "memory");  /* Barrier in loop */
                }
                
                arr1[idx] = p;
                arr2[idx] = q;
                
            } else { /* branch_selector == 6 */
                /* Switch-like behavior using bit tests */
                volatile int bits = idx;
                volatile int acc = 0;
                
                for (int bit = 0; bit < 8; bit++) {
                    if (bits & (1 << bit)) {
                        acc += arr1[(idx + bit) % ARRAY_SIZE];
                    } else {
                        acc -= arr2[(idx + bit) % ARRAY_SIZE];
                    }
                }
                
                /* Multiple barriers to create scheduling regions */
                __asm__ volatile ("" : : : "memory");
                arr1[idx] = acc;
                __asm__ volatile ("" : : : "memory");
                arr2[idx] = acc ^ idx;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Final result accumulation with dependency */
            result ^= arr1[idx] + arr2[idx];
            
            /* Another barrier to potentially queue instructions */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Modify loop variables based on computation */
        if (o % 2 == 0) {
            inner_limit = (inner_limit + 1) % 8 + 5;
        }
    }
    
    return result;
}

/* Secondary scheduling function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                          volatile int seed) {
    volatile int total = 0;
    volatile int limit = (seed % 5) + 3;
    
    for (volatile int phase = 0; phase < limit; phase++) {
        /* Pseudo-random index calculation */
        volatile int base = (seed * 1103515245 + 12345) % ARRAY_SIZE;
        
        for (int i = 0; i < 32; i++) {
            int idx = (base + i * 7) % ARRAY_SIZE;
            
            /* Interleaved memory operations */
            volatile int a = arr1[idx];
            volatile int b = arr2[(idx + phase) % ARRAY_SIZE];
            
            /* Complex arithmetic with many intermediate values */
            volatile int t1 = a * b;
            volatile int t2 = t1 >> (phase % 8);
            volatile int t3 = t2 + (a ^ b);
            volatile int t4 = t3 * 7 - 13;
            
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            volatile int t5 = (t4 & 0xFFFF) | ((~t4) & 0xFFFF0000);
            volatile int t6 = t5 + (phase * 17);
            
            arr1[idx] = t6;
            arr2[(idx + phase) % ARRAY_SIZE] = t6 ^ (i * 19);
            
            total += t6;
        }
        
        /* Change pattern mid-loop */
        if (phase == limit / 2) {
            __asm__ volatile ("" : : : "memory");
            /* Force potential state save */
            for (int j = 0; j < 16; j++) {
                int idx = (base + j * 11) % ARRAY_SIZE;
                arr1[idx] = arr1[idx] * 2 - arr2[idx];
            }
        }
    }
    
    return total;
}

int main() {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int final_result = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (int iter = 0; iter < MAX_LOOP_ITER; iter++) {
        volatile int mode = (iter + mode_switch) % 4;
        
        /* Alternate between different scheduling patterns */
        if (iter % 2 == 0) {
            volatile int res = complex_schedule_loop(array1, array2, mode, iter);
            final_result ^= res;
        } else {
            volatile int res = alternate_schedule_pattern(array1, array2, iter);
            final_result ^= res;
        }
        
        /* Modify mode for next iteration */
        mode_switch = (mode_switch * 13 + 7) % 11;
        
        /* Occasionally reinitialize parts of arrays */
        if (iter % 3 == 0) {
            for (int i = 0; i < 64; i++) {
                int idx = (iter * 31 + i) % ARRAY_SIZE;
                array1[idx] = (array1[idx] + iter) % 1000;
                array2[idx] = (array2[idx] * 3 + i) % 1000;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    checksum ^= final_result;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}
