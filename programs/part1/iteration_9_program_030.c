/* Complex scheduling test to trigger haifa-sched.cc free_sched_context logic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_QUEUE_PRESSURE 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int result = 0;
    volatile int outer_limit = (mode % 5) + 3;  /* Non-constant limit */
    volatile int inner_limit = (iter % 7) + 4;  /* Varying inner bounds */
    
    /* Force scheduler to handle multiple basic blocks */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Memory barrier to split scheduling region */
        __asm__ volatile ("" : : : "memory");
        
        for (volatile int inner = 0; inner < inner_limit; inner++) {
            /* Create long dependency chains */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");
            e = a ^ f;
            g = e >> (h & 0x3);
            __asm__ volatile ("" : : : "memory");
            
            /* Independent memory operations to fill instruction queue */
            arr1[(a + inner) % ARRAY_SIZE] = b + c;
            arr2[(b + outer) % ARRAY_SIZE] = d * e;
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way branch creating multiple basic blocks */
            volatile int branch_selector = (a + b + c + inner) % 8;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic chain */
                i = j * k - l;
                j = i ^ (g << 2);
                k = j + arr1[(i + 1) % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector == 1) {
                /* Branch 1: Different operations */
                l = (h & i) | (j ^ k);
                m = l >> 1;
                arr2[(m + 2) % ARRAY_SIZE] = l * m;
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 2) {
                /* Branch 2: Memory intensive */
                for (volatile int t = 0; t < 3; t++) {
                    arr1[(inner + t) % ARRAY_SIZE] += arr2[(outer + t) % ARRAY_SIZE];
                    __asm__ volatile ("" : : : "memory");
                }
            }
            else if (branch_selector == 3) {
                /* Branch 3: Function call to complicate scheduling */
                if ((mode & 0x1) && (iter > 2)) {
                    volatile int pid = getpid();
                    arr1[pid % ARRAY_SIZE] = pid ^ inner;
                    __asm__ volatile ("" : : : "memory");
                }
            }
            else if (branch_selector == 4) {
                /* Branch 4: Complex bit operations */
                volatile int x = (a << 3) | (b >> 2);
                volatile int y = (c ^ d) & (e | f);
                volatile int z = x * y + g;
                arr2[z % ARRAY_SIZE] = x ^ y ^ z;
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 5) {
                /* Branch 5: Nested conditionals */
                if (outer & 0x1) {
                    a = b + c;
                    if (inner & 0x2) {
                        d = e * f;
                        __asm__ volatile ("" : : : "memory");
                    } else {
                        g = h ^ i;
                    }
                } else {
                    j = k - l;
                }
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 6) {
                /* Branch 6: Register pressure */
                volatile int r1 = a + b, r2 = c + d, r3 = e + f;
                volatile int r4 = g + h, r5 = i + j, r6 = k + l;
                volatile int r7 = r1 * r2, r8 = r3 ^ r4, r9 = r5 | r6;
                result += r7 + r8 + r9;
                __asm__ volatile ("" : : : "memory");
            }
            else { /* branch_selector == 7 */
                /* Branch 7: Mixed operations with barrier */
                volatile int tmp = clock() % 1000;
                if (tmp > 500) {
                    a = b * c + arr1[tmp % ARRAY_SIZE];
                    __asm__ volatile ("" : : : "memory");
                }
                d = e ^ f ^ tmp;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Additional independent operations to maximize queue pressure */
            for (volatile int q = 0; q < MAX_QUEUE_PRESSURE; q++) {
                volatile int idx = (inner + q) % ARRAY_SIZE;
                volatile int val1 = arr1[idx];
                volatile int val2 = arr2[idx];
                arr1[idx] = val1 + val2 + q;
                arr2[idx] = val1 ^ val2 ^ q;
            }
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Switch-like structure with computed goto simulation */
        volatile int switch_val = (outer + mode) % 4;
        if (switch_val == 0) {
            a = b + c * d;
            __asm__ volatile ("" : : : "memory");
        } else if (switch_val == 1) {
            e = f ^ g | h;
            __asm__ volatile ("" : : : "memory");
        } else if (switch_val == 2) {
            i = j - k + l;
            __asm__ volatile ("" : : : "memory");
        } else {
            m = n * o / p;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result + a + b + c + d;
}

/* Secondary complex function with different patterns */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed, volatile int depth) {
    volatile int x = seed, y = seed + 1, z = seed + 2;
    volatile int limit = (depth % 3) + 2;
    
    for (volatile int i = 0; i < limit; i++) {
        __asm__ volatile ("" : : : "memory");
        
        /* Pattern 1: Interleaved dependencies */
        x = y * z + arr1[i % ARRAY_SIZE];
        __asm__ volatile ("" : : : "memory");
        y = x ^ arr2[(i + 1) % ARRAY_SIZE];
        z = y >> (i & 0x3);
        __asm__ volatile ("" : : : "memory");
        
        /* Pattern 2: Parallel chains */
        volatile int chain1 = x + y;
        volatile int chain2 = y * z;
        volatile int chain3 = z ^ x;
        __asm__ volatile ("" : : : "memory");
        
        arr1[(chain1 + i) % ARRAY_SIZE] = chain2;
        arr2[(chain2 + i) % ARRAY_SIZE] = chain3;
        arr1[(chain3 + i) % ARRAY_SIZE] = chain1;
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional with function call */
        if ((seed + i) % 5 == 0) {
            volatile int t = time(NULL) % 100;
            arr2[t % ARRAY_SIZE] = t ^ i;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return x + y + z;
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
    
    volatile int mode = 1;
    volatile int total_result = 0;
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int iter = 0; iter < 8; iter++) {
        __asm__ volatile ("" : : : "memory");
        
        /* Call core scheduling function with varying parameters */
        volatile int r1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Alternate between different scheduling patterns */
        if (iter % 2 == 0) {
            volatile int r2 = alternate_schedule_pattern(array1, array2, iter, iter % 4);
            total_result += r1 + r2;
        } else {
            total_result += r1 * 2;
        }
        
        /* Modify mode to change scheduling behavior */
        mode = (mode * 3 + 7) % 11;
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= total_result;
    
    printf("Result: %d (checksum: %d)\n", total_result, checksum);
    
    return 0;
}
