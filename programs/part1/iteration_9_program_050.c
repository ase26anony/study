/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fno-inline -o haifa-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 50

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int i, j, k;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile limit to prevent optimization */
    volatile int inner_limit = (iter % 5) + 10;
    
    /* Volatile control variables to force dynamic scheduling decisions */
    volatile int branch_selector = mode ^ iter;
    volatile int phase = 0;
    
    /* Outer loop with volatile bound - encourages scheduler state management */
    for (i = 0; i < outer_limit; i++) {
        /* Create scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Complex inner loop with multiple dependency chains */
        for (j = 0; j < inner_limit; j++) {
            /* Multiple basic blocks created by if-else chain */
            if (branch_selector & (1 << 0)) {
                /* Long dependency chain 1 */
                volatile int a = arr1[(i + j) % ARRAY_SIZE];
                volatile int b = arr2[(i * j) % ARRAY_SIZE];
                volatile int c = a * b + phase;
                volatile int d = c ^ (a << 3);
                volatile int e = d >> (b % 8);
                result += e;
                
                /* Memory access pattern 1 */
                arr1[(result + i) % ARRAY_SIZE] = e;
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector & (1 << 1)) {
                /* Long dependency chain 2 */
                volatile int x = arr2[(i - j + ARRAY_SIZE) % ARRAY_SIZE];
                volatile int y = arr1[(j * 3) % ARRAY_SIZE];
                volatile int z = (x * y) - (phase << 2);
                volatile int w = z | (x & y);
                volatile int v = w ^ (y % 16);
                result -= v;
                
                /* Memory access pattern 2 */
                arr2[(result + j) % ARRAY_SIZE] = v;
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector & (1 << 2)) {
                /* Mixed operations with function call */
                volatile int m = arr1[(i * 4 + j) % ARRAY_SIZE];
                volatile int n = arr2[(j * 5) % ARRAY_SIZE];
                
                /* Function call in one branch - adds call instruction to scheduling */
                if ((mode + iter + j) % 17 == 0) {
                    volatile int pid = getpid();
                    m ^= (pid & 0xFF);
                }
                
                volatile int p = (m * n) / ((phase % 8) + 1);
                volatile int q = p + (m << (n % 4));
                volatile int r = q ^ (n >> 2);
                result ^= r;
                
                arr1[(i + result) % ARRAY_SIZE] = r;
                __asm__ volatile ("" : : : "memory");
            }
            else {
                /* Default path with independent operations */
                volatile int s = arr1[(i + phase) % ARRAY_SIZE];
                volatile int t = arr2[(j + phase) % ARRAY_SIZE];
                volatile int u = (s + t) * (phase + 1);
                volatile int v = u - (s & t);
                volatile int w = v | (s ^ t);
                result |= w;
                
                arr2[(j + result) % ARRAY_SIZE] = w;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Additional scheduling barrier in the middle of loop */
            __asm__ volatile ("" : : : "memory");
            
            /* Switch-like behavior using bit tests */
            volatile int bit_test = (i * j + mode) & 0xF;
            if (bit_test & 1) {
                volatile int tmp = arr1[bit_test % ARRAY_SIZE];
                arr2[bit_test % ARRAY_SIZE] = tmp + result;
            }
            if (bit_test & 2) {
                volatile int tmp = arr2[(bit_test * 2) % ARRAY_SIZE];
                arr1[(bit_test * 2) % ARRAY_SIZE] = tmp - result;
            }
            if (bit_test & 4) {
                volatile int tmp = result * phase;
                arr1[(bit_test * 3) % ARRAY_SIZE] = tmp;
            }
            if (bit_test & 8) {
                volatile int tmp = result / ((phase % 4) + 1);
                arr2[(bit_test * 4) % ARRAY_SIZE] = tmp;
            }
            
            /* Update phase with pseudo-random but volatile calculation */
            phase = (phase * 1103515245 + 12345) & 0x7FFFFFFF;
            phase = phase % 100;
        }
        
        /* Change branch selector dynamically */
        branch_selector = (branch_selector * 13 + i) & 0xFF;
        
        /* Another scheduling barrier between outer loop iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed, volatile int rounds) {
    volatile int acc = seed;
    volatile int i, j;
    
    for (i = 0; i < rounds; i++) {
        volatile int limit = (seed + i) % 15 + 5;
        
        for (j = 0; j < limit; j++) {
            /* Multiple independent operations to fill instruction queue */
            volatile int a = arr1[(i * 3 + j) % ARRAY_SIZE];
            volatile int b = arr2[(i * 5 - j + ARRAY_SIZE) % ARRAY_SIZE];
            volatile int c = arr1[(j * 7) % ARRAY_SIZE];
            volatile int d = arr2[(i * 11 + j * 3) % ARRAY_SIZE];
            
            /* Parallel dependency chains */
            volatile int chain1 = a * b + c;
            volatile int chain2 = (a ^ b) | c;
            volatile int chain3 = (b << (a % 4)) - d;
            volatile int chain4 = (c >> (d % 4)) ^ a;
            
            /* Cross-chain dependencies */
            volatile int mix1 = chain1 + chain2;
            volatile int mix2 = chain3 - chain4;
            volatile int mix3 = mix1 * mix2;
            volatile int mix4 = mix1 ^ mix2;
            
            /* Store results back */
            arr1[(i + j * 2) % ARRAY_SIZE] = mix3;
            arr2[(i * 2 + j) % ARRAY_SIZE] = mix4;
            
            acc += mix3 - mix4;
            
            /* Frequent scheduling barriers */
            if (j % 3 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Function call occasionally */
        if (i % 7 == 0) {
            volatile int clock_val = clock() & 0xFF;
            acc ^= clock_val;
        }
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int final_result = 0;
    volatile int mode_switch = 0;
    
    /* Main loop that calls scheduling functions multiple times */
    for (int iter = 0; iter < 8; iter++) {
        volatile int mode = (iter * 17) % 13;
        volatile int bound = (iter % 5) + 3;
        
        /* Call first complex scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Update mode for variability */
        mode_switch = (mode_switch + res1) & 0xFF;
        
        /* Call alternate scheduling pattern */
        volatile int res2 = alternate_schedule_pattern(array1, array2, mode_switch, bound);
        
        final_result ^= res1;
        final_result += res2;
        
        /* Modify arrays between iterations to create new scheduling contexts */
        for (int i = 0; i < ARRAY_SIZE; i += 7) {
            array1[i] = (array1[i] * 3 + iter) % 1000;
            array2[i] = (array2[i] * 5 - iter + 1000) % 1000;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= final_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
