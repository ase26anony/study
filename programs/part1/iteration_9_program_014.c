/* 
 * Complex scheduling test to trigger haifa-sched.cc free_sched_context
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o sched_test sched_test.c
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
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int e = 5, f = 6, g = 7, h = 8;
    volatile int result = 0;
    volatile int i, j;
    
    /* Volatile loop limit to prevent compile-time optimization */
    volatile int outer_limit = (mode % 5) + 3;
    
    /* Create multiple basic blocks with complex control flow */
    for (i = 0; i < outer_limit; i++) {
        /* Inner loop with volatile limit */
        volatile int inner_limit = (iter % 8) + 4;
        
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain 1 */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            g = e >> (h & 0x7);
            
            /* Memory access pattern 1 */
            arr1[(a + j) % ARRAY_SIZE] = g + arr2[(b + i) % ARRAY_SIZE];
            
            /* Independent operations to fill instruction queue */
            volatile int temp1 = arr2[(c + j * 3) % ARRAY_SIZE];
            volatile int temp2 = arr1[(d + i * 2) % ARRAY_SIZE];
            
            /* Complex multi-way branch creating multiple basic blocks */
            int branch_selector = (a + b + c + iter + j) % 10;
            
            if (branch_selector < 3) {
                /* Branch 1: Arithmetic operations */
                b = (temp1 * 0x5A5A5A5A) ^ temp2;
                c = (b << 3) | (b >> 29);
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector < 6) {
                /* Branch 2: Different operations with memory barrier */
                d = temp1 + (temp2 * 0x12345678);
                f = d ^ 0xDEADBEEF;
                __asm__ volatile ("" : : : "memory");
                /* Add a potential function call in one branch */
                if ((iter & 0x1) && (j & 0x2)) {
                    volatile int pid = getpid();
                    arr1[(pid + j) % ARRAY_SIZE] += pid & 0xFF;
                }
            }
            else if (branch_selector < 8) {
                /* Branch 3: More complex operations */
                h = (temp1 * temp2) - (temp1 / (temp2 ? temp2 : 1));
                g = h ^ (h << 16);
                __asm__ volatile ("" : : : "memory");
            }
            else {
                /* Branch 4: Bit manipulation chain */
                volatile int chain = temp1;
                chain = (chain * 0xCCCCCCCD) >> 3;
                chain = chain ^ (chain << 7);
                chain = chain * 0x9E3779B9;
                arr2[(chain + i) % ARRAY_SIZE] = chain;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Another dependency chain */
            volatile int x = arr1[(a + g) % ARRAY_SIZE];
            volatile int y = arr2[(b + h) % ARRAY_SIZE];
            volatile int z = x * y + (x ^ y);
            
            /* Switch-like structure compiled to jump table */
            int switch_val = z & 0x7;
            switch (switch_val) {
                case 0:
                    result += z * 2;
                    break;
                case 1:
                    result ^= z;
                    arr1[(result + j) % ARRAY_SIZE] = result;
                    break;
                case 2:
                    result = (result << 1) | (result >> 31);
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 3:
                    result -= z;
                    if ((iter + j) & 1) {
                        volatile int clock_val = clock();
                        result ^= clock_val & 0xFFFF;
                    }
                    break;
                case 4:
                    result = result * 0x9E3779B9;
                    break;
                case 5:
                    result = (result + z) ^ 0x55555555;
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 6:
                    result = ~result;
                    arr2[(result + i) % ARRAY_SIZE] = result;
                    break;
                case 7:
                    result = result / ((z & 0xFF) + 1);
                    break;
            }
            
            /* Final memory store with barrier */
            arr1[(i * 17 + j * 13) % ARRAY_SIZE] = result;
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Outer loop operations */
        if (i & 1) {
            volatile int mix = arr1[(i * 7) % ARRAY_SIZE] ^ arr2[(i * 11) % ARRAY_SIZE];
            arr2[(i * 19) % ARRAY_SIZE] = mix * iter;
        }
    }
    
    return result;
}

/* Secondary complex function with different patterns */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed, volatile int rounds) {
    volatile int acc = seed;
    volatile int i, j;
    
    for (i = 0; i < rounds; i++) {
        volatile int inner = (seed + i) % 7 + 2;
        
        for (j = 0; j < inner; j++) {
            /* Create instruction pressure with many independent operations */
            volatile int op1 = arr1[(i * 31 + j * 29) % ARRAY_SIZE];
            volatile int op2 = arr2[(i * 23 + j * 19) % ARRAY_SIZE];
            volatile int op3 = arr1[(i * 17 + j * 13) % ARRAY_SIZE];
            volatile int op4 = arr2[(i * 11 + j * 7) % ARRAY_SIZE];
            
            /* Parallel computation chains */
            volatile int chain1 = (op1 * op2) + (op1 ^ op2);
            volatile int chain2 = (op3 | op4) * 0x12345678;
            volatile int chain3 = (op1 + op3) ^ (op2 + op4);
            
            __asm__ volatile ("" : : : "memory");
            
            /* Complex conditional network */
            if ((chain1 + chain2) & 0x100) {
                acc += chain1 * 3;
                arr1[(acc + i) % ARRAY_SIZE] = chain2;
            } else if (chain3 & 0x80) {
                acc ^= chain2;
                arr2[(acc + j) % ARRAY_SIZE] = chain3;
                if (j & 1) {
                    volatile int tmp = getpid();
                    acc ^= tmp & 0xFF;
                }
            } else {
                acc = (acc << 5) | (acc >> 27);
                __asm__ volatile ("" : : : "memory");
            }
            
            /* More operations to increase instruction count */
            volatile int temp = arr1[(chain1 + chain3) % ARRAY_SIZE];
            arr2[(chain2 + temp) % ARRAY_SIZE] = acc;
            
            /* Bit manipulation sequence */
            for (int k = 0; k < 3; k++) {
                volatile int bits = (acc >> (k * 8)) & 0xFF;
                if (bits & (1 << (k % 8))) {
                    acc ^= (1 << (bits % 32));
                }
            }
            
            __asm__ volatile ("" : : : "memory");
        }
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
        array1[i] = rand() ^ (i * 0x9E3779B9);
        array2[i] = rand() ^ (i * 0x6A09E667);
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        volatile int outer_mode = (iter * 17) % 11;
        
        /* Alternate between two complex scheduling patterns */
        if (iter & 1) {
            volatile int result1 = complex_schedule_loop(array1, array2, 
                                                        outer_mode, iter);
            checksum ^= result1;
            
            /* Modify mode for next call */
            mode_switch = (mode_switch + result1) & 0xFF;
            volatile int result2 = complex_schedule_loop(array2, array1,
                                                        mode_switch, iter + 1);
            checksum ^= result2;
        } else {
            volatile int rounds = (iter % 5) + 3;
            volatile int result3 = alternate_schedule_pattern(array1, array2,
                                                             iter, rounds);
            checksum ^= result3;
            
            volatile int result4 = alternate_schedule_pattern(array2, array1,
                                                             result3, rounds + 1);
            checksum ^= result4;
        }
        
        /* Cross-mix arrays to create data dependencies */
        for (int i = 0; i < ARRAY_SIZE / 4; i++) {
            volatile int idx = (iter * 13 + i * 7) % ARRAY_SIZE;
            array1[idx] ^= array2[(idx + 17) % ARRAY_SIZE];
            array2[idx] ^= array1[(idx + 23) % ARRAY_SIZE];
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final checksum computation to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
