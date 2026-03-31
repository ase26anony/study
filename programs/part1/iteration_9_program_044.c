#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode) {
    volatile int result = 0;
    volatile int i, j, k;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Inner loop with complex dependency chains */
        for (j = 0; j < 128; j++) {
            /* Multiple basic blocks created by if-else chain */
            if (mode & 0x1) {
                /* Long dependency chain 1 */
                temp1 = arr1[j] * arr2[j] + i;
                temp2 = temp1 ^ (arr1[j] >> 2);
                temp3 = temp2 * 0x5A827999;
                
                /* Memory barrier to split scheduling region */
                __asm__ volatile ("" : : : "memory");
                
                /* Use result in next computation */
                arr1[j] = temp3 + (temp2 << 3);
                result ^= arr1[j];
                
                /* Another barrier */
                __asm__ volatile ("" : : : "memory");
            }
            
            if (mode & 0x2) {
                /* Independent memory operations */
                temp1 = arr2[255 - j] - arr1[j];
                temp2 = temp1 * temp1;
                
                /* Complex arithmetic with multiple uses */
                arr2[j] = (temp2 >> 4) | (temp2 << 28);
                result += arr2[j];
                
                /* Barrier to force instruction queuing */
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Multi-way branch using bit tests */
            for (k = 0; k < 4; k++) {
                if (mode & (1 << (k + 2))) {
                    switch (k) {
                        case 0:
                            /* More arithmetic chains */
                            temp1 = arr1[j] + arr2[j];
                            temp2 = temp1 * 3;
                            arr1[j] = temp2 ^ result;
                            break;
                        case 1:
                            /* Different operations */
                            temp1 = arr1[j] - arr2[j];
                            temp2 = temp1 / 2;
                            arr2[j] = temp2 | result;
                            break;
                        case 2:
                            /* Include a potential function call */
                            if ((j % 16) == 0) {
                                volatile int pid = getpid();
                                arr1[j] ^= pid;
                            }
                            break;
                        case 3:
                            /* Memory-intensive operations */
                            temp1 = arr1[j] * arr2[j];
                            temp2 = arr2[255 - j];
                            arr1[j] = temp1 + temp2;
                            arr2[j] = temp1 - temp2;
                            break;
                    }
                    /* Barrier in each branch */
                    __asm__ volatile ("" : : : "memory");
                }
            }
            
            /* Final computation mixing everything */
            if (j % 3 == 0) {
                temp1 = arr1[j] + arr2[j] + result;
                temp2 = temp1 * 0x9E3779B9;
                result = temp2 ^ (temp1 >> 5);
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Vary the mode pseudo-randomly within the loop */
        mode = (mode * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int iterations) {
    volatile int sum = 0;
    volatile int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Different pattern to create varied scheduling contexts */
        for (j = 0; j < 64; j++) {
            /* Multiple independent chains */
            volatile int chain1 = arr1[j * 2] * arr2[j * 2];
            volatile int chain2 = arr1[j * 2 + 1] + arr2[j * 2 + 1];
            
            __asm__ volatile ("" : : : "memory");
            
            /* Cross-chain dependencies */
            chain1 = chain1 ^ chain2;
            chain2 = chain2 - chain1;
            
            /* Store results back with barriers */
            arr1[j * 2] = chain1;
            __asm__ volatile ("" : : : "memory");
            arr2[j * 2 + 1] = chain2;
            
            /* Complex condition with side effects */
            if ((chain1 + chain2) > 0) {
                sum += chain1;
                __asm__ volatile ("" : : : "memory");
                sum -= chain2;
            } else {
                sum ^= chain1;
                __asm__ volatile ("" : : : "memory");
                sum |= chain2;
            }
        }
        
        /* Reorder elements */
        for (j = 0; j < 32; j++) {
            volatile int tmp = arr1[j];
            arr1[j] = arr2[63 - j];
            arr2[63 - j] = tmp;
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
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Volatile loop bounds to prevent optimization */
    volatile int outer_loop_limit = 5;
    volatile int alt_iterations = 3;
    
    /* Multiple calls to create different scheduling contexts */
    for (i = 0; i < outer_loop_limit; i++) {
        volatile int mode = (i * 17) % 31;
        volatile int result;
        
        /* Alternate between two complex functions */
        if (i % 2 == 0) {
            result = complex_schedule_loop(array1, array2, 
                                          (i % 3) + 2, mode);
        } else {
            result = alternate_schedule_pattern(array1, array2, 
                                               (i % 2) + 1);
        }
        
        checksum ^= result;
        
        /* Modify arrays between calls to prevent pattern recognition */
        for (j = 0; j < 256; j += 8) {
            array1[j] ^= result;
            array2[j] += i;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final computation to use all results */
    for (i = 0; i < 256; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    /* Print to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
