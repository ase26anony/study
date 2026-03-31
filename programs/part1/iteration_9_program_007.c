/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fno-inline -o haifa-trigger haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 32

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int i, j;
    volatile int outer_limit = (mode % 8) + 4;  /* Volatile to prevent optimization */
    volatile int inner_limit = (iter % 16) + 8; /* Volatile loop bounds */
    
    /* Volatile array indices to prevent constant propagation */
    volatile int idx_a = mode * 3;
    volatile int idx_b = iter * 5;
    volatile int idx_c = (mode ^ iter) & 0xFF;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (i * 17 + mode * 13 + iter * 7) & 0xF;
        volatile int temp_acc = 0;
        
        /* Inner loop creating instruction pressure */
        for (j = 0; j < inner_limit; j++) {
            volatile int op_type = (j + i * 3) % 6;
            volatile int arr_idx = (idx_a + j) % ARRAY_SIZE;
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way branch creating multiple basic blocks */
            if (op_type == 0) {
                /* Integer arithmetic chain */
                volatile int t1 = arr1[arr_idx] * 3;
                volatile int t2 = arr2[(arr_idx + 1) % ARRAY_SIZE] + 7;
                temp_acc = t1 ^ t2;
                arr1[arr_idx] = temp_acc >> 1;
                
                /* Another barrier */
                __asm__ volatile ("" : : : "memory");
                
            } else if (op_type == 1) {
                /* Different arithmetic pattern */
                volatile int t1 = arr1[(arr_idx + 2) % ARRAY_SIZE];
                volatile int t2 = arr2[(arr_idx + 3) % ARRAY_SIZE];
                temp_acc = (t1 * t2) + (t1 << 2);
                arr2[arr_idx] = temp_acc & 0x7FFF;
                
            } else if (op_type == 2) {
                /* Memory intensive operations */
                volatile int load1 = arr1[(idx_b + j) % ARRAY_SIZE];
                volatile int load2 = arr2[(idx_c + j * 2) % ARRAY_SIZE];
                temp_acc = load1 - load2;
                arr1[(arr_idx + 4) % ARRAY_SIZE] = temp_acc * 3;
                
                /* Barrier in middle of computation */
                __asm__ volatile ("" : : : "memory");
                
                temp_acc = temp_acc ^ (load1 * load2);
                arr2[(arr_idx + 5) % ARRAY_SIZE] = temp_acc;
                
            } else if (op_type == 3) {
                /* Complex dependency chain */
                volatile int chain1 = arr1[arr_idx];
                volatile int chain2 = chain1 * 2 + 1;
                volatile int chain3 = chain2 ^ arr2[arr_idx];
                volatile int chain4 = chain3 << (j % 4);
                volatile int chain5 = chain4 - (chain1 >> 2);
                
                arr1[arr_idx] = chain5;
                arr2[(arr_idx + j) % ARRAY_SIZE] = chain5 * chain3;
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                
            } else if (op_type == 4) {
                /* Branch with potential function call */
                if ((branch_selector & 0x3) == 0) {
                    /* Dummy system call to add call instruction */
                    volatile int pid = getpid();
                    temp_acc = (pid & 0xFF) * arr1[arr_idx];
                    arr2[arr_idx] = temp_acc;
                } else {
                    /* Alternative computation path */
                    temp_acc = arr1[arr_idx] + arr2[arr_idx];
                    arr1[arr_idx] = temp_acc * 2;
                }
                
            } else { /* op_type == 5 */
                /* Mixed operations with barrier */
                volatile int mix1 = arr1[(idx_a + i) % ARRAY_SIZE];
                volatile int mix2 = arr2[(idx_b + j) % ARRAY_SIZE];
                
                __asm__ volatile ("" : : : "memory");
                
                volatile int mix3 = (mix1 * mix2) + (mix1 >> 3);
                volatile int mix4 = mix3 ^ (mix2 * 3);
                
                arr1[(arr_idx + i) % ARRAY_SIZE] = mix4;
                arr2[(arr_idx + j) % ARRAY_SIZE] = mix4 - mix1;
                
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Additional independent operations to fill instruction queue */
            volatile int indep1 = arr1[(j + 1) % ARRAY_SIZE];
            volatile int indep2 = arr2[(j + 2) % ARRAY_SIZE];
            volatile int indep3 = indep1 * indep2 + (j % 31);
            
            /* Store result with volatile to prevent elimination */
            arr1[(j + 3) % ARRAY_SIZE] = indep3;
            
            /* Update volatile accumulator */
            result ^= temp_acc;
            result += indep3;
        }
        
        /* Switch-like behavior based on branch selector */
        switch (branch_selector & 0x7) {
            case 0:
                result += arr1[idx_a % ARRAY_SIZE];
                break;
            case 1:
                result -= arr2[idx_b % ARRAY_SIZE];
                break;
            case 2:
                result ^= (arr1[idx_c % ARRAY_SIZE] * 3);
                break;
            case 3:
                result = result >> 1;
                break;
            case 4:
                result = result * 2 + 1;
                break;
            case 5:
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                result = result & 0xFFFF;
                break;
            case 6:
                result = result | 0x1000;
                break;
            case 7:
                result = result ^ result;
                result += 1;
                break;
        }
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                          volatile int seed) {
    volatile int total = 0;
    volatile int cycles = (seed % 5) + 3;  /* Volatile loop count */
    
    for (volatile int c = 0; c < cycles; c++) {
        volatile int base = (seed + c * 19) % ARRAY_SIZE;
        
        /* Unrolled inner loop segment */
        for (volatile int k = 0; k < 8; k++) {
            volatile int idx = (base + k * 7) % ARRAY_SIZE;
            
            /* Multiple memory barriers */
            __asm__ volatile ("" : : : "memory");
            
            /* Parallel dependency chains */
            volatile int chain_a = arr1[idx];
            volatile int chain_b = arr2[(idx + 1) % ARRAY_SIZE];
            
            chain_a = chain_a * 5 + chain_b;
            chain_b = chain_b ^ (chain_a >> 2);
            
            __asm__ volatile ("" : : : "memory");
            
            volatile int chain_c = chain_a + chain_b * 3;
            volatile int chain_d = chain_c - (chain_b << 1);
            
            arr1[idx] = chain_c;
            arr2[(idx + 2) % ARRAY_SIZE] = chain_d;
            
            total += chain_c + chain_d;
        }
        
        /* Conditional with volatile check */
        volatile int check = seed ^ c;
        if (check & 0x4) {
            /* Function call in one path */
            volatile clock_t t = clock();
            total += (t & 0xFF);
        }
    }
    
    return total;
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
    
    /* Main loop calling scheduling functions multiple times */
    for (volatile int iteration = 0; iteration < 8; iteration++) {
        volatile int mode = (iteration * 13) % 7;
        
        /* Call core scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iteration);
        
        /* Alternate between different scheduling patterns */
        if (mode_switch & 0x1) {
            volatile int res2 = alternate_schedule_pattern(array1, array2, iteration);
            final_result ^= res2;
        }
        
        final_result += res1;
        mode_switch ^= iteration;
        
        /* Modify array contents between iterations */
        for (volatile int j = 0; j < ARRAY_SIZE / 4; j++) {
            volatile int idx = (iteration * 29 + j * 11) % ARRAY_SIZE;
            array1[idx] = (array1[idx] * 3 + 1) & 0x7FF;
            array2[idx] = (array2[idx] ^ array1[(idx + 1) % ARRAY_SIZE]) & 0x7FF;
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
