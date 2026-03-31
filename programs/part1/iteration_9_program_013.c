/* 
 * Complex scheduling test to trigger haifa-sched.cc free_sched_context
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o sched_test sched_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOPS 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Volatile-like calculation */
    volatile int inner_limit = (mode % 4) + 10;
    
    /* Volatile array for intermediate results */
    volatile int temp[16];
    for (int i = 0; i < 16; i++) temp[i] = i * iter;
    
    /* Outer loop with volatile limit */
    for (volatile int o = 0; o < outer_limit; o++) {
        /* Inner loop with complex operations */
        for (volatile int i = 0; i < inner_limit; i++) {
            int idx = (i + o) & 0xFF;
            int val1 = arr1[idx];
            int val2 = arr2[idx];
            
            /* Long dependency chain */
            int a = val1 * val2 + iter;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            int b = a ^ (val1 << 3);
            int c = b - (val2 >> 2);
            __asm__ volatile ("" : : : "memory");
            
            int d = c * 0x5A5A5A5A;
            int e = d ^ (a & 0x0F0F0F0F);
            
            /* Multiple basic blocks created by if-else chain */
            if (e & 0x10000000) {
                /* Branch 1: Arithmetic operations */
                e = e * 3 + 1;
                arr1[idx] = e ^ arr2[idx];
                __asm__ volatile ("" : : : "memory");
            } else if (e & 0x08000000) {
                /* Branch 2: Memory operations */
                arr2[idx] = arr1[idx] * 2 - e;
                e = arr2[idx] ^ 0x55555555;
            } else if (e & 0x04000000) {
                /* Branch 3: Function call in some cases */
                if ((e ^ iter) & 0x01000000) {
                    volatile int pid = getpid();
                    e ^= (pid & 0xFF);
                }
                e = e >> 4;
            } else if (e & 0x02000000) {
                /* Branch 4: More complex arithmetic */
                e = ((e << 16) | (e >> 16)) + 0x12345678;
                arr1[idx] = e;
                arr2[idx] = ~e;
            } else {
                /* Default branch: Mixed operations */
                e = e * e - val1 * val2;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Switch-like behavior using bit tests */
            for (int bit = 0; bit < 4; bit++) {
                if (e & (1 << (bit + 20))) {
                    temp[bit] += e * (bit + 1);
                    __asm__ volatile ("" : : : "memory");
                } else {
                    temp[bit + 4] -= e / ((bit + 1) | 1);
                }
            }
            
            /* Another dependency chain */
            int f = e + temp[i & 0xF];
            int g = f ^ (f << 13);
            int h = g * 0x9E3779B9;
            
            /* Store result with memory barrier */
            arr1[idx] = h;
            __asm__ volatile ("" : : : "memory");
            arr2[idx] = h ^ 0xDEADBEEF;
            
            result ^= h;
        }
        
        /* Additional scheduling barrier between outer loop iterations */
        __asm__ volatile ("" : : : "memory");
        
        /* Pseudo-random branch based on volatile calculation */
        volatile int branch_sel = (o * iter) % 7;
        if (branch_sel == 0) {
            /* Mode 0: Additional arithmetic */
            for (int j = 0; j < 4; j++) {
                temp[j] = temp[j] * 2 + 1;
            }
        } else if (branch_sel == 1) {
            /* Mode 1: Memory shuffle */
            volatile int swap = arr1[0];
            arr1[0] = arr1[ARRAY_SIZE-1];
            arr1[ARRAY_SIZE-1] = swap;
        } else if (branch_sel == 2) {
            /* Mode 2: Function call */
            volatile clock_t clk = clock();
            result ^= (clk & 0xFFFF);
        }
        
        /* Final barrier in outer loop */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
secondary_schedule(volatile int *arr, volatile int seed) {
    volatile int acc = seed;
    volatile int limit = (seed % 5) + 3;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Complex sequence of operations */
        int idx = (i * 17) % ARRAY_SIZE;
        int val = arr[idx];
        
        /* Multiple independent operations to fill instruction queue */
        int t1 = val * 0xCCCCCCCD;
        int t2 = val + 0x12345678;
        int t3 = val ^ 0xF0F0F0F0;
        int t4 = val >> 4;
        int t5 = val << 4;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Dependency mixing */
        t1 = t1 ^ t2;
        t3 = t3 + t4;
        t5 = t5 * t1;
        
        /* Conditional store */
        if ((t5 ^ seed) & 0x80000000) {
            arr[idx] = t3;
            __asm__ volatile ("" : : : "memory");
        } else {
            arr[idx] = t5;
        }
        
        acc ^= t1 + t3 + t5;
    }
    
    return acc;
}

int main() {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() ^ (i * 0x123456);
        array2[i] = rand() ^ (i * 0xABCDEF);
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < MAX_LOOPS; iter++) {
        /* Volatile mode variable to prevent static analysis */
        volatile int mode = (iter * 17) % 11;
        mode_switch ^= mode;
        
        /* Call complex scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Call secondary function to add more scheduling complexity */
        volatile int res2 = secondary_schedule(array1, iter);
        volatile int res3 = secondary_schedule(array2, iter + 1);
        
        checksum ^= res1 ^ res2 ^ res3;
        
        /* Occasionally modify arrays to create new scheduling patterns */
        if (iter & 1) {
            for (int i = 0; i < ARRAY_SIZE; i += 8) {
                array1[i] ^= checksum;
                array2[i] ^= ~checksum;
            }
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= array1[i] ^ array2[i];
    }
    
    printf("Checksum: %d, Final XOR: %d\n", checksum, final_sum);
    
    return 0;
}
