/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa_test haifa-sched-trigger.c
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
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int result = 0;
    volatile int counter = 0;
    
    /* Volatile loop bound to prevent compile-time simplification */
    volatile int outer_limit = (mode % 5) + 3;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with complex operations */
        for (volatile int idx = 0; idx < MAX_LOOP_ITER; idx++) {
            /* Long dependency chain 1 */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            g = e >> (h % 8);
            
            /* Independent memory operations */
            volatile int temp1 = arr1[(idx + iter) % ARRAY_SIZE];
            volatile int temp2 = arr2[(idx * 3) % ARRAY_SIZE];
            
            /* Long dependency chain 2 */
            i = j * k - l;
            __asm__ volatile ("" : : : "memory");
            j = i ^ temp1;
            k = j << (temp2 % 8);
            
            /* Complex conditional structure creating multiple basic blocks */
            if (idx & 0x01) {
                /* Branch 1: Arithmetic operations */
                result += a * b - c + d;
                __asm__ volatile ("" : : : "memory");
                arr1[idx % ARRAY_SIZE] = result ^ temp1;
            } else if (idx & 0x02) {
                /* Branch 2: Bitwise operations */
                result ^= e | f | g;
                __asm__ volatile ("" : : : "memory");
                arr2[idx % ARRAY_SIZE] = result & temp2;
            } else if (idx & 0x04) {
                /* Branch 3: Shift operations */
                result = (result << 3) | (result >> 29);
                __asm__ volatile ("" : : : "memory");
                arr1[(idx + 1) % ARRAY_SIZE] = result;
            } else if (idx & 0x08) {
                /* Branch 4: Mixed operations with function call */
                if ((idx % 17) == 0) {
                    /* Volatile condition to prevent call elimination */
                    volatile int pid = getpid();
                    result ^= pid & 0xFF;
                    __asm__ volatile ("" : : : "memory");
                }
                arr2[(idx + 2) % ARRAY_SIZE] = result + temp1;
            } else {
                /* Default branch: Complex arithmetic */
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                __asm__ volatile ("" : : : "memory");
                arr1[(idx + 3) % ARRAY_SIZE] = result % 1000;
            }
            
            /* Additional multi-way branch based on pseudo-random value */
            int branch_selector = (idx * 13 + iter * 17) % 10;
            switch (branch_selector) {
                case 0:
                    a = b + c;
                    b = c - d;
                    break;
                case 1:
                    c = d * e;
                    d = e / (f + 1);
                    break;
                case 2:
                    e = f ^ g;
                    f = g | h;
                    break;
                case 3:
                    g = h << 2;
                    h = i >> 1;
                    break;
                case 4:
                    i = j + k;
                    j = k - l;
                    break;
                case 5:
                    k = l * a;
                    l = a / (b + 1);
                    break;
                case 6:
                    /* Memory barrier and operation */
                    __asm__ volatile ("" : : : "memory");
                    result += arr1[(idx + 5) % ARRAY_SIZE];
                    break;
                case 7:
                    result -= arr2[(idx + 7) % ARRAY_SIZE];
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 8:
                    result ^= (temp1 * temp2) & 0xFF;
                    break;
                case 9:
                    /* Another scheduling barrier */
                    __asm__ volatile ("" : : : "memory");
                    result = ~result;
                    break;
            }
            
            counter++;
        }
        
        /* Vary operations based on outer loop iteration */
        if (outer & 0x01) {
            /* Swap and rotate values */
            volatile int tmp = a;
            a = b; b = c; c = d; d = tmp;
            __asm__ volatile ("" : : : "memory");
        } else {
            /* Complex transformation */
            a = (a * b) >> 2;
            b = (b * c) >> 2;
            c = (c * d) >> 2;
            d = (d * a) >> 2;
        }
    }
    
    return result + counter;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
another_schedule_function(volatile int *arr, volatile int seed) {
    volatile int x = seed, y = seed * 2, z = seed * 3;
    volatile int sum = 0;
    
    for (volatile int i = 0; i < (seed % 20) + 10; i++) {
        /* Complex dependency web */
        x = (x * 13 + y * 17) ^ z;
        __asm__ volatile ("" : : : "memory");
        y = (y * 19 + z * 23) ^ x;
        z = (z * 29 + x * 31) ^ y;
        
        /* Memory operations with barriers */
        arr[i % ARRAY_SIZE] = x;
        __asm__ volatile ("" : : : "memory");
        arr[(i + 64) % ARRAY_SIZE] = y;
        arr[(i + 128) % ARRAY_SIZE] = z;
        
        /* Conditional with multiple paths */
        if (i & 0x1) {
            sum += x;
        } else if (i & 0x2) {
            sum += y;
            __asm__ volatile ("" : : : "memory");
        } else if (i & 0x4) {
            sum += z;
        } else {
            sum += x + y + z;
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Switch for jump table generation */
        switch (i % 7) {
            case 0: x = x >> 1; break;
            case 1: y = y << 1; break;
            case 2: z = z ^ 0x55AA; break;
            case 3: x = x + y; break;
            case 4: y = y + z; break;
            case 5: z = z + x; break;
            case 6: x = x * y * z; break;
        }
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic behavior */
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
    volatile int mode_switch = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (volatile int iteration = 0; iteration < 8; iteration++) {
        /* Vary parameters to create different scheduling scenarios */
        volatile int mode = (iteration * 7) % 11;
        volatile int param = iteration * 13;
        
        /* Call complex scheduling function multiple times */
        volatile int result1 = complex_schedule_loop(array1, array2, mode, param);
        __asm__ volatile ("" : : : "memory");
        
        volatile int result2 = another_schedule_function(array1, param + 1);
        __asm__ volatile ("" : : : "memory");
        
        volatile int result3 = complex_schedule_loop(array2, array1, mode + 1, param + 2);
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= result1 ^ result2 ^ result3;
        
        /* Modify mode switch for next iteration */
        mode_switch = (mode_switch * 3 + 1) % 5;
    }
    
    /* Compute final checksum from arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    /* Print result to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
