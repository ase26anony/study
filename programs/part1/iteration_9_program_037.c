/* Complex scheduling test to trigger haifa-sched.cc free_sched_context logic */
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
    volatile int result = 0;
    volatile int i, j;
    volatile int outer_limit = (iter % 7) + 3;  /* Non-constant, volatile limit */
    volatile int inner_limit = (mode % 5) + 10; /* Another volatile limit */
    
    /* Volatile array for intermediate computations */
    volatile int temp[16];
    for (i = 0; i < 16; i++) temp[i] = arr1[i] ^ arr2[i];
    
    /* Outer loop with volatile bound - encourages scheduler state management */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (iter + i) % 8;
        volatile int accum = arr1[i % ARRAY_SIZE];
        
        /* Inner loop creating many instructions for scheduling */
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain with arithmetic operations */
            volatile int a = accum * 1103515245 + 12345;
            volatile int b = a ^ (arr2[j % ARRAY_SIZE] << 3);
            volatile int c = b + (j * 997);
            volatile int d = c ^ (c >> 16);
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Complex conditional structure creating multiple basic blocks */
            if (branch_selector & 0x1) {
                /* Branch 1: Integer arithmetic chain */
                accum = (accum * 3 + d) & 0x7FFFFFFF;
                accum = accum ^ (accum << 13);
                accum = accum ^ (accum >> 17);
                accum = accum ^ (accum << 5);
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Independent memory operations */
                temp[(i + j) % 16] = accum;
                arr1[(i + j) % ARRAY_SIZE] = accum ^ arr2[(i + j) % ARRAY_SIZE];
            } 
            else if (branch_selector & 0x2) {
                /* Branch 2: Different arithmetic pattern */
                accum = (accum << 4) | (accum >> 28);
                accum = accum + d * 16807;
                accum = accum - (accum / 127773) * 127773;
                
                /* Memory access pattern */
                arr2[(i * j) % ARRAY_SIZE] = accum;
                temp[j % 16] = temp[j % 16] ^ accum;
            }
            else if (branch_selector & 0x4) {
                /* Branch 3: More complex operations with function call */
                volatile int r = rand() % 1000;  /* Function call in scheduling region */
                accum = accum * 48271 + r;
                accum = (accum & 0x7FFF) * 2 - 0x7FFF;
                
                /* Multiple independent operations */
                volatile int t1 = accum * 3;
                volatile int t2 = accum / 5;
                volatile int t3 = accum ^ 0x55555555;
                
                __asm__ volatile ("" : : : "memory");
                
                temp[(accum >> 10) % 16] = t1 + t2 + t3;
            }
            else {
                /* Default branch: Mixed operations */
                accum = (accum + d) * 1664525 + 1013904223;
                accum = accum ^ (accum >> 15);
                
                /* Memory operations with barrier */
                __asm__ volatile ("" : : : "memory");
                
                arr1[(accum >> 20) % ARRAY_SIZE] = accum;
                arr2[(accum >> 18) % ARRAY_SIZE] = ~accum;
            }
            
            /* Switch-like structure using bit tests (creates jump table) */
            volatile int switch_var = (accum ^ iter) & 0x7;
            if (switch_var == 0) {
                accum = accum + 1;
                __asm__ volatile ("" : : : "memory");
            } else if (switch_var == 1) {
                accum = accum * 2;
            } else if (switch_var == 2) {
                accum = accum - 3;
            } else if (switch_var == 3) {
                accum = accum ^ 0xAAAAAAAA;
            } else if (switch_var == 4) {
                accum = accum | 0x55555555;
            } else if (switch_var == 5) {
                /* Function call in one path */
                volatile int pid = getpid();
                accum = accum ^ (pid & 0xFFFF);
            } else if (switch_var == 6) {
                accum = (accum << 1) | (accum >> 31);
            } else {
                accum = ~accum;
            }
            
            /* Final barrier in loop body */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Store result back to array with complex indexing */
        result ^= accum;
        arr1[(i * 17) % ARRAY_SIZE] = accum;
        arr2[(i * 13) % ARRAY_SIZE] = result;
    }
    
    /* Additional scheduling region after loops */
    __asm__ volatile ("" : : : "memory");
    
    /* Final computation mixing all temp values */
    for (i = 0; i < 16; i++) {
        result = result * 31 + temp[i];
    }
    
    return result;
}

/* Secondary scheduling function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int sum = 0;
    volatile int i, j;
    volatile int limit1 = (seed % 5) + 8;
    volatile int limit2 = ((seed * 3) % 7) + 6;
    
    for (i = 0; i < limit1; i++) {
        volatile int base = arr1[i % ARRAY_SIZE];
        
        for (j = 0; j < limit2; j++) {
            /* Different arithmetic pattern */
            volatile int x = base * 6364136223846793005ULL;
            volatile int y = x ^ (x >> 18);
            volatile int z = y * 277803737;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way conditional */
            volatile int cond = (i + j + seed) & 0x3;
            if (cond == 0) {
                base = (base + z) & 0xFFFFFFFF;
                arr1[(i * j) % ARRAY_SIZE] = base;
            } else if (cond == 1) {
                base = base ^ z;
                arr2[(i + j * 3) % ARRAY_SIZE] = base;
            } else if (cond == 2) {
                base = base * 3 - z;
                /* Function call in this path */
                volatile int t = clock();
                base = base ^ (t & 0xFFF);
            } else {
                base = (base << 4) ^ (base >> 28) ^ z;
            }
            
            __asm__ volatile ("" : : : "memory");
            
            /* Additional operations to increase instruction count */
            volatile int t1 = base * 5;
            volatile int t2 = base / 3;
            volatile int t3 = base ^ 0x12345678;
            base = t1 + t2 + t3;
        }
        
        sum += base;
        __asm__ volatile ("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand() ^ 0x5A5A5A5A;
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop calling scheduling functions multiple times */
    for (volatile int iter = 0; iter < 8; iter++) {
        volatile int result;
        
        /* Alternate between two different scheduling patterns */
        if (mode_switch) {
            result = complex_schedule_loop(array1, array2, iter % 4, iter);
        } else {
            result = alternate_schedule_pattern(array1, array2, iter);
        }
        
        mode_switch = !mode_switch;
        checksum ^= result;
        
        /* Modify arrays between calls to create different scheduling contexts */
        for (volatile int i = 0; i < ARRAY_SIZE / 4; i++) {
            array1[(iter * 31 + i) % ARRAY_SIZE] ^= result;
            array2[(iter * 37 + i) % ARRAY_SIZE] += iter;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final processing to ensure all code is used */
    volatile int final_sum = 0;
    for (volatile int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= array1[i];
        final_sum += array2[i];
    }
    
    checksum ^= final_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", (int)checksum);
    
    return 0;
}
