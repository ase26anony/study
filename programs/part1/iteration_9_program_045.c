/* 
 * Complex scheduling test to trigger haifa-sched.cc free_sched_context
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fno-inline -fno-tree-loop-optimize -march=native
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int loop_limit = (iter % 10) + 15;  /* Volatile-like limit */
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < loop_limit; outer++) {
        /* Create multiple basic blocks with complex control flow */
        if (mode & (1 << 0)) {
            /* Branch 1: Arithmetic dependency chain */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            __asm__ volatile ("" : : : "memory");
            f = e >> (outer & 3);
            result += f;
            
            /* Memory access pattern */
            arr1[(a + outer) % ARRAY_SIZE] = b;
            arr2[(c + outer) % ARRAY_SIZE] = d;
        } 
        else if (mode & (1 << 1)) {
            /* Branch 2: Different arithmetic chain */
            b = c * d - e;
            __asm__ volatile ("" : : : "memory");
            c = b | f;
            __asm__ volatile ("" : : : "memory");
            d = c & 0xFF;
            result += d;
            
            /* More memory ops */
            volatile int idx = (iter * outer) % ARRAY_SIZE;
            arr1[idx] = arr2[idx] + 1;
            arr2[(idx + 1) % ARRAY_SIZE] = arr1[idx] * 2;
        }
        else if (mode & (1 << 2)) {
            /* Branch 3: Mixed operations with function call */
            if ((outer % 7) == 0) {
                /* Occasional function call to complicate scheduling */
                volatile int pid = getpid();
                f = (pid & 0xF) + outer;
            }
            
            e = (a * b) / (c + 1);
            __asm__ volatile ("" : : : "memory");
            d = e << (f % 4);
            __asm__ volatile ("" : : : "memory");
            result += d;
            
            /* Array operations */
            for (volatile int i = 0; i < 4; i++) {
                arr1[(outer + i) % ARRAY_SIZE] += i;
                __asm__ volatile ("" : : : "memory");
            }
        }
        else {
            /* Default branch: Complex computation */
            volatile int temp = 0;
            for (volatile int inner = 0; inner < 8; inner++) {
                /* Inner loop creates more scheduling pressure */
                temp += arr1[(outer + inner) % ARRAY_SIZE];
                temp ^= arr2[(outer + inner + 1) % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Long dependency chain */
            a = temp * 1103515245 + 12345;
            b = (a >> 16) & 0x7FFF;
            c = b * 16807 % 2147483647;
            d = c ^ 0x55555555;
            e = d + (temp % 100);
            f = e * 6364136223846793005ULL;
            
            result += (f & 0xFF);
        }
        
        /* Switch-like multi-way branch based on pseudo-random value */
        volatile int branch_selector = (result + outer) % 5;
        
        switch (branch_selector) {
            case 0:
                arr1[outer % ARRAY_SIZE] = result * 3;
                __asm__ volatile ("" : : : "memory");
                break;
            case 1:
                arr2[outer % ARRAY_SIZE] = result / 2;
                __asm__ volatile ("" : : : "memory");
                break;
            case 2:
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                arr1[outer % ARRAY_SIZE] ^= arr2[outer % ARRAY_SIZE];
                break;
            case 3:
                /* Complex operation with barrier */
                result = (result << 3) | (result >> 29);
                __asm__ volatile ("" : : : "memory");
                arr2[outer % ARRAY_SIZE] = result;
                break;
            default:
                /* Mixed operations */
                volatile int clock_val = clock() & 0xFF;
                result += clock_val;
                arr1[outer % ARRAY_SIZE] += clock_val;
                arr2[outer % ARRAY_SIZE] -= clock_val;
                __asm__ volatile ("" : : : "memory");
        }
        
        /* Additional independent instructions to fill instruction queue */
        volatile int x = arr1[(outer + 10) % ARRAY_SIZE];
        volatile int y = arr2[(outer + 20) % ARRAY_SIZE];
        volatile int z = x * y + result;
        volatile int w = z ^ (x << 2);
        
        /* Store results back with barriers */
        __asm__ volatile ("" : : : "memory");
        arr1[(outer + 5) % ARRAY_SIZE] = w;
        arr2[(outer + 15) % ARRAY_SIZE] = z;
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
another_scheduling_function(volatile int *arr, volatile int seed) {
    volatile int acc = seed;
    volatile int limit = (seed % 20) + 10;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Multiple parallel dependency chains */
        volatile int chain1 = arr[i % ARRAY_SIZE];
        volatile int chain2 = arr[(i + 64) % ARRAY_SIZE];
        volatile int chain3 = arr[(i + 128) % ARRAY_SIZE];
        
        chain1 = chain1 * 13 + 7;
        chain2 = chain2 ^ 0x9E3779B9;
        chain3 = chain3 >> (i % 5);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Cross-chain dependencies */
        chain1 ^= chain2;
        chain2 += chain3;
        chain3 |= chain1;
        
        /* Store with reordering opportunities */
        arr[i % ARRAY_SIZE] = chain1;
        arr[(i + 64) % ARRAY_SIZE] = chain2;
        arr[(i + 128) % ARRAY_SIZE] = chain3;
        
        /* Barrier every few iterations */
        if (i % 3 == 0) {
            __asm__ volatile ("" : : : "memory");
        }
        
        acc += chain1 + chain2 + chain3;
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
    
    volatile int checksum = 0;
    volatile int mode_switch = 1;
    
    /* Multiple calls to create different scheduling contexts */
    for (volatile int iteration = 0; iteration < 8; iteration++) {
        /* Vary the mode to create different control flow patterns */
        mode_switch = (mode_switch * 1103515245 + 12345) & 0x7;
        
        /* Call the core scheduling function */
        volatile int result = complex_schedule_loop(array1, array2, 
                                                   mode_switch, iteration);
        checksum ^= result;
        
        /* Call secondary function to add more scheduling complexity */
        if (iteration % 2 == 0) {
            volatile int sec_result = another_scheduling_function(array1, result);
            checksum += sec_result;
        }
        
        /* Modify arrays between calls to prevent pattern recognition */
        for (volatile int j = 0; j < 32; j++) {
            int idx = (iteration * 17 + j) % ARRAY_SIZE;
            array1[idx] = (array1[idx] * 3 + 1) & 0xFFF;
            array2[idx] ^= array1[idx];
        }
        
        __asm__ volatile ("" : : : "memory");  /* Barrier between iterations */
    }
    
    /* Final computation to use all results and prevent dead code elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array1[i];
        final_sum ^= array2[i];
    }
    
    final_sum ^= checksum;
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_sum);
    
    return 0;
}
