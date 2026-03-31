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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int result = 0;
    volatile int counter = 0;
    
    /* Volatile outer loop limit - prevents compile-time optimization */
    volatile int outer_limit = (mode % 5) + 3;
    
    /* Create multiple basic blocks with complex control flow */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with volatile limit */
        volatile int inner_limit = (iter % 8) + 4;
        
        for (volatile int inner = 0; inner < inner_limit; inner++) {
            /* Long dependency chain 1 */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            g = e >> (h & 3);
            
            /* Independent memory operations */
            arr1[(inner + outer) & 0xFF] = g + arr2[inner & 0xFF];
            __asm__ volatile ("" : : : "memory");
            
            /* Long dependency chain 2 */
            i = j * k - l;
            b = i & 0x7FFF;
            c = b ^ (d << 2);
            
            /* Multi-way branch creating multiple basic blocks */
            volatile int branch_selector = (inner + outer + mode) & 7;
            
            if (branch_selector == 0) {
                /* Basic block 1: Arithmetic operations */
                f = (a * b) / (c + 1);
                arr2[inner & 0xFF] = f ^ arr1[(inner + 1) & 0xFF];
            } else if (branch_selector == 1) {
                /* Basic block 2: Bit manipulation */
                h = (g << 3) | (e >> 2);
                j = h ^ 0xABCD;
                arr1[(inner + 2) & 0xFF] = j;
            } else if (branch_selector == 2) {
                /* Basic block 3: Memory intensive */
                k = arr1[inner & 0xFF] * arr2[(inner + 3) & 0xFF];
                l = k + (mode << 4);
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 3) {
                /* Basic block 4: Function call (scheduling complexity) */
                if ((iter & 1) && (outer & 1)) {
                    volatile int pid = getpid();
                    d = pid & 0xFF;
                }
            } else if (branch_selector == 4) {
                /* Basic block 5: Complex arithmetic chain */
                volatile int t1 = a * b + c * d;
                volatile int t2 = e * f - g * h;
                volatile int t3 = (t1 ^ t2) + (i * j);
                result += t3 & 0xFF;
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 5) {
                /* Basic block 6: Shift operations */
                a = (b << (c & 3)) | (d >> (e & 3));
                f = (g << 1) ^ (h << 2) ^ (i << 3);
            } else {
                /* Basic block 7: Default operations */
                arr2[(inner + outer) & 0xFF] = 
                    (arr1[inner & 0xFF] + mode) ^ iter;
                counter++;
            }
            
            /* More independent operations to fill instruction queue */
            volatile int x = arr1[(inner * 3) & 0xFF];
            volatile int y = arr2[(inner * 5) & 0xFF];
            volatile int z = x * y + (inner << 2);
            arr1[(inner * 7) & 0xFF] = z;
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Switch-like structure using bit tests (compiles to multiple branches) */
            volatile int switch_val = (inner + mode) & 0xF;
            for (int bit = 0; bit < 4; bit++) {
                if (switch_val & (1 << bit)) {
                    /* Each creates a separate basic block */
                    volatile int temp = bit * 17 + outer;
                    arr2[(inner + bit) & 0xFF] = temp;
                    if (bit == 2) {
                        /* Nested condition */
                        __asm__ volatile ("" : : : "memory");
                        result ^= temp;
                    }
                }
            }
        }
        
        /* Inter-loop operations with memory barriers */
        __asm__ volatile ("" : : : "memory");
        arr1[outer & 0xFF] = (arr1[outer & 0xFF] + result) ^ counter;
        arr2[outer & 0xFF] = (arr2[outer & 0xFF] * (outer + 1)) & 0xFFFF;
    }
    
    return result + counter;
}

/* Secondary scheduling-intensive function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int acc = 0;
    volatile int limit = (seed % 7) + 2;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Create instruction pressure with unrolled-style operations */
        volatile int x0 = arr1[(i * 0) & 0xFF];
        volatile int x1 = arr1[(i * 1) & 0xFF];
        volatile int x2 = arr1[(i * 2) & 0xFF];
        volatile int x3 = arr1[(i * 3) & 0xFF];
        
        volatile int y0 = arr2[(i * 4) & 0xFF];
        volatile int y1 = arr2[(i * 5) & 0xFF];
        volatile int y2 = arr2[(i * 6) & 0xFF];
        volatile int y3 = arr2[(i * 7) & 0xFF];
        
        /* Parallel dependency chains */
        volatile int chain1 = x0 * y0 + x1 * y1;
        volatile int chain2 = x2 ^ y2 | x3 & y3;
        volatile int chain3 = (x0 << 2) + (x1 >> 1) - (x2 & 0xF);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Complex conditional network */
        if ((chain1 & 1) && !(chain2 & 2)) {
            arr1[i & 0xFF] = chain3 + seed;
            if (chain3 > 1000) {
                volatile int t = clock() & 0xFF;
                arr2[i & 0xFF] ^= t;
            }
        } else if ((chain2 & 4) || (chain3 < 500)) {
            arr2[i & 0xFF] = chain1 - chain2;
            __asm__ volatile ("" : : : "memory");
        } else {
            arr1[i & 0xFF] = (chain1 + chain2 + chain3) & 0xFF;
        }
        
        acc += arr1[i & 0xFF] + arr2[i & 0xFF];
    }
    
    return acc;
}

int main() {
    /* Seed for deterministic but complex behavior */
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
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int iteration = 0; iteration < 8; iteration++) {
        volatile int mode = iteration & 3;
        
        /* Call core scheduling function multiple times */
        volatile int result1 = complex_schedule_loop(array1, array2, mode, iteration);
        
        /* Alternate between different scheduling patterns */
        if (iteration & 1) {
            volatile int result2 = alternate_schedule_pattern(array1, array2, iteration);
            checksum ^= result2;
        }
        
        checksum ^= result1;
        
        /* Modify array contents for next iteration */
        for (int i = 0; i < 256; i += 8) {
            array1[i] = (array1[i] * 1103515245 + 12345) & 0x7FFFFFFF;
            array2[i] = (array2[i] ^ iteration) + i;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
