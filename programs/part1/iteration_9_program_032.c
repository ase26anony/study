/* haifa-sched-trigger.c
 * Program designed to trigger scheduling context creation and cleanup
 * in GCC's Haifa scheduler (haifa-sched.cc lines 4681-4691)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int i, j, k;
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int result = 0;
    volatile int temp_store[8];
    
    /* Outer loop with volatile limit to prevent optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Create instruction pressure with long dependency chains */
        a = arr1[i & 255] * b + c;
        __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
        
        b = a ^ arr2[(i + 1) & 255];
        c = b >> (i & 7);
        
        /* Multiple independent operations to fill instruction queue */
        d = arr1[(i * 3) & 255] + arr2[(i * 5) & 255];
        temp_store[0] = d * a;
        temp_store[1] = b + c;
        
        __asm__ volatile ("" : : : "memory");  /* Another barrier */
        
        /* Complex if-else chain creating multiple basic blocks */
        volatile int branch_selector = (i + mode_flag) & 7;
        
        if (branch_selector == 0) {
            /* Branch 0: Arithmetic chain */
            a = a * 2 - b;
            b = b + c * 3;
            c = c ^ d;
            result += a + b - c;
        } else if (branch_selector == 1) {
            /* Branch 1: Memory operations */
            temp_store[2] = arr1[(i + 2) & 255];
            temp_store[3] = arr2[(i + 3) & 255];
            a = temp_store[2] * temp_store[3];
            b = a >> 4;
            result += b;
        } else if (branch_selector == 2) {
            /* Branch 2: More complex arithmetic */
            for (k = 0; k < 3; k++) {
                a = (a << k) | (b >> k);
                b = b + arr1[(i + k) & 255];
            }
            result += a * b;
        } else if (branch_selector == 3) {
            /* Branch 3: Function call to add complexity */
            if ((i & 1) == 0) {
                volatile int pid = getpid();
                a = a ^ (pid & 0xFF);
                result += a;
            }
        } else if (branch_selector == 4) {
            /* Branch 4: Nested operations */
            d = arr1[i & 255] * arr2[i & 255];
            a = d - b;
            b = a * c;
            c = b / (d + 1);
            result += c;
        } else if (branch_selector == 5) {
            /* Branch 5: Bit manipulation chain */
            a = (a << 3) | (a >> 5);
            b = b ^ c ^ d;
            c = (c * 13) & 0xFF;
            result += a ^ b ^ c;
        } else if (branch_selector == 6) {
            /* Branch 6: Memory intensive */
            for (j = 0; j < 4; j++) {
                temp_store[j] = arr1[(i + j) & 255] + arr2[(i + j * 2) & 255];
            }
            a = temp_store[0] + temp_store[1] - temp_store[2] + temp_store[3];
            result += a;
        } else {
            /* Branch 7: Default with scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            a = b + c + d;
            b = a * 2;
            result += b;
        }
        
        /* Store results back to prevent elimination */
        arr1[i & 255] = a;
        arr2[i & 255] = b;
        
        /* Another scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Switch-like structure compiled to jump table */
        volatile int switch_val = (i * 7 + mode_flag) & 3;
        switch (switch_val) {
            case 0:
                c = a + b;
                arr1[(i + 64) & 255] = c;
                break;
            case 1:
                c = a - b;
                arr2[(i + 64) & 255] = c;
                break;
            case 2:
                c = a * b;
                temp_store[4] = c;
                break;
            case 3:
                c = (a ^ b) & 0xFF;
                result += c;
                break;
        }
        
        /* Final dependency chain */
        d = c * 2 + result;
        a = (d & 0xFFFF) | ((result << 16) & 0xFFFF0000);
        result = a ^ b ^ c ^ d;
    }
    
    return result;
}

/* Secondary function to increase scheduling complexity */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int size, volatile int iter) {
    volatile int sum = 0;
    volatile int i;
    
    for (i = 0; i < size; i++) {
        volatile int idx = (i + iter) & 255;
        volatile int val = arr[idx];
        
        /* Create instruction mix */
        if (val & 1) {
            val = (val << 3) | (val >> 5);
            __asm__ volatile ("" : : : "memory");
        } else if (val & 2) {
            val = val * 3 + 1;
        } else if (val & 4) {
            val = val ^ 0xAA;
        }
        
        /* Multiple independent operations */
        volatile int t1 = val + i;
        volatile int t2 = val * i;
        volatile int t3 = t1 ^ t2;
        
        sum += t3;
        arr[idx] = val + t3;
        
        /* Scheduling barrier every 8 iterations */
        if ((i & 7) == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return sum;
}

int main(void) {
    volatile int array1[256];
    volatile int array2[256];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < 256; i++) {
        array1[i] = rand() & 0xFFF;
        array2[i] = rand() & 0xFFF;
    }
    
    /* Main loop to trigger multiple scheduling contexts */
    for (j = 0; j < 8; j++) {
        volatile int outer_limit = (j % 3) + 8;  /* Varying limits */
        volatile int mode = j & 7;
        
        /* Call core scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, outer_limit, mode);
        
        /* Process arrays with secondary function */
        volatile int res2 = secondary_schedule_func(array1, 128 + (j * 2), j);
        volatile int res3 = secondary_schedule_func(array2, 128 + (j * 3), j + 1);
        
        checksum ^= res1 ^ res2 ^ res3;
        
        /* Additional scheduling barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute final checksum to prevent elimination */
    for (i = 0; i < 256; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
