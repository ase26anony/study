/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-sched-trigger
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
    volatile int i, j, k;
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0;
    volatile int temp1, temp2, temp3;
    volatile int loop_limit = (iter % 5) + 3;  /* Volatile-like variation */
    
    /* Outer loop with volatile limit */
    for (i = 0; i < loop_limit; i++) {
        /* Create multiple basic blocks with if-else chains */
        if (mode & 0x01) {
            /* Branch 1: Arithmetic dependency chain */
            a = arr1[i] * 3 + arr2[i];
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            b = a ^ (arr1[i] >> 2);
            c = b + (arr2[i] * 7);
            d = c - (a & 0xFF);
            e = d | (b ^ 0x55);
            
            /* Store results back with memory barrier */
            arr1[i] = e;
            __asm__ volatile ("" : : : "memory");
        } 
        else if (mode & 0x02) {
            /* Branch 2: Different operations */
            a = arr1[i] + arr2[i];
            b = arr1[i] - arr2[i];
            c = a * b;
            __asm__ volatile ("" : : : "memory");
            
            /* Complex calculation with multiple dependencies */
            for (k = 0; k < 3; k++) {
                d = (c << k) + (a >> k);
                e = d ^ (b & (0xFF >> k));
                arr2[i + k] = e;
            }
        } 
        else if (mode & 0x04) {
            /* Branch 3: Memory intensive */
            temp1 = arr1[i];
            temp2 = arr2[i];
            temp3 = arr1[temp1 % ARRAY_SIZE];
            
            __asm__ volatile ("" : : : "memory");
            
            /* Long dependency chain */
            for (j = 0; j < 4; j++) {
                a = temp1 + temp2 + temp3;
                b = a * (j + 1);
                c = b ^ (temp1 << j);
                d = c - (temp2 >> j);
                e = d | (temp3 & j);
                
                temp1 = b;
                temp2 = c;
                temp3 = d;
            }
            
            arr1[i] = e;
            __asm__ volatile ("" : : : "memory");
        } 
        else {
            /* Branch 4: Mixed operations with function call */
            if ((iter + i) % 7 == 0) {
                /* Volatile condition for function call */
                volatile int pid = getpid();
                a = pid & 0xFF;
            } else {
                a = clock() & 0xFF;
            }
            
            b = arr1[i] * a;
            c = arr2[i] + b;
            d = (c << 2) | (b >> 2);
            
            __asm__ volatile ("" : : : "memory");
            
            /* Another dependency chain */
            for (j = 0; j < 2; j++) {
                e = (d + j) ^ (a * j);
                arr1[(i + j) % ARRAY_SIZE] = e;
                arr2[(i + j) % ARRAY_SIZE] = e + j;
            }
        }
        
        /* Switch-like multi-way branch using bit operations */
        volatile int branch_selector = (iter + i) % 8;
        
        if (branch_selector & 0x01) {
            temp1 = arr1[i] * 2;
            __asm__ volatile ("" : : : "memory");
            temp2 = arr2[i] / 3;
            temp3 = temp1 + temp2;
        }
        
        if (branch_selector & 0x02) {
            temp1 = arr1[i] ^ arr2[i];
            temp2 = temp1 << 1;
            temp3 = temp2 | 0x1;
        }
        
        if (branch_selector & 0x04) {
            /* More arithmetic with barriers */
            a = arr1[i] + 1;
            b = arr2[i] - 1;
            __asm__ volatile ("" : : : "memory");
            c = a * b;
            d = c % 256;
            e = d ^ 0xAA;
            
            arr1[i] = e;
            arr2[i] = e + i;
        }
        
        /* Final memory barrier in loop */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Return volatile result to prevent elimination */
    return a + b + c + d + e + loop_limit;
}

int main() {
    volatile int arr1[ARRAY_SIZE];
    volatile int arr2[ARRAY_SIZE];
    volatile int i, j;
    volatile int result = 0;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    /* Multiple calls with varying parameters to create scheduling contexts */
    for (j = 0; j < MAX_LOOP_ITER; j++) {
        volatile int mode = (j % 4) * 2 + 1;  /* Varying modes */
        
        /* Call complex function multiple times */
        for (i = 0; i < 3; i++) {
            result = complex_schedule_loop(arr1, arr2, mode + i, j + i);
            
            /* Use result to prevent dead code elimination */
            checksum ^= result;
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Modify array between calls */
        if (j % 2 == 0) {
            for (i = 0; i < ARRAY_SIZE / 4; i++) {
                arr1[i * 4] = (arr1[i * 4] * 3) % 1000;
                arr2[i * 4] = (arr2[i * 4] + 7) % 1000;
            }
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Compute final checksum to prevent optimization */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 1;
}
