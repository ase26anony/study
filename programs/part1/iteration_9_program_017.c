/* Complex scheduling test to trigger haifa-sched.cc free_sched_context logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11;
    volatile int result = 0;
    volatile int counter = 0;
    
    /* Outer loop with volatile limit to prevent compile-time simplification */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex dependency chains */
        for (int inner = 0; inner < 128; inner++) {
            /* Multiple basic blocks created by if-else chain */
            if (inner & 0x01) {
                /* Long dependency chain 1 */
                a = b * c + d;
                __asm__ volatile ("" : : : "memory");
                e = a ^ f;
                g = e >> (inner & 0x0F);
                h = g * arr1[inner & 0xFF];
                arr2[inner & 0xFF] = h + mode;
            } else if (inner & 0x02) {
                /* Long dependency chain 2 */
                i = j - k + outer;
                __asm__ volatile ("" : : : "memory");
                j = i * arr2[(inner + 1) & 0xFF];
                k = j ^ arr1[(inner + 2) & 0xFF];
                arr1[inner & 0xFF] = k - g;
            } else if (inner & 0x04) {
                /* Independent memory operations */
                arr1[inner & 0xFF] = arr2[(inner * 3) & 0xFF] + 1;
                arr2[inner & 0xFF] = arr1[(inner * 5) & 0xFF] - 1;
                __asm__ volatile ("" : : : "memory");
            } else if (inner & 0x08) {
                /* Complex arithmetic with multiple uses */
                volatile int t1 = arr1[inner & 0xFF];
                volatile int t2 = arr2[inner & 0xFF];
                a = t1 * t2 + (inner << 2);
                b = a ^ (t1 - t2);
                c = b >> (outer & 0x07);
                d = c * 137;
                arr1[inner & 0xFF] = d;
                arr2[inner & 0xFF] = d ^ 0x55AA55AA;
            } else {
                /* Default case with mixed operations */
                arr1[inner & 0xFF] += arr2[inner & 0xFF] * outer;
                arr2[inner & 0xFF] ^= inner * mode;
            }
            
            /* Pseudo-random branch to create varying control flow */
            switch (rand() % 5) {
                case 0:
                    a = b + c;
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 1:
                    d = e - f;
                    arr1[(inner + 3) & 0xFF] = d;
                    break;
                case 2:
                    /* Function call in one branch - adds call instruction */
                    if ((inner & 0x10) && (mode & 0x01)) {
                        volatile int pid = getpid();
                        arr2[inner & 0xFF] ^= pid & 0xFF;
                    }
                    break;
                case 3:
                    g = h * i;
                    j = g / (k + 1);
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 4:
                    arr1[inner & 0xFF] = arr2[inner & 0xFF] ^ 
                                        arr1[(inner + 1) & 0xFF];
                    break;
            }
            
            /* Additional memory barrier */
            if (inner % 16 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
            
            counter++;
        }
        
        /* Varying loop continuation condition */
        if (outer % 3 == 0) {
            volatile int temp = arr1[outer & 0xFF];
            result += temp;
            __asm__ volatile ("" : : : "memory");
        } else if (outer % 3 == 1) {
            volatile int temp = arr2[outer & 0xFF];
            result -= temp;
        } else {
            result ^= outer;
        }
    }
    
    return result;
}

/* Secondary complex function with different patterns */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                          volatile int iterations) {
    volatile int x = 1, y = 2, z = 3;
    volatile int acc = 0;
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Complex switch creating jump table */
        switch (iter & 0x07) {
            case 0:
                x = y * z + arr1[iter & 0xFF];
                __asm__ volatile ("" : : : "memory");
                break;
            case 1:
                y = x ^ z ^ arr2[(iter + 64) & 0xFF];
                break;
            case 2:
                z = (x << 3) | (y >> 2);
                arr1[iter & 0xFF] = z;
                __asm__ volatile ("" : : : "memory");
                break;
            case 3:
                x = arr1[iter & 0xFF] * arr2[iter & 0xFF];
                y = x + iter;
                break;
            case 4:
                /* Nested if-else chain */
                if (iter & 0x10) {
                    z = x - y;
                } else if (iter & 0x20) {
                    z = x + y;
                } else if (iter & 0x40) {
                    z = x * y;
                } else {
                    z = x ^ y;
                }
                __asm__ volatile ("" : : : "memory");
                break;
            case 5:
                arr2[iter & 0xFF] = arr1[(iter * 7) & 0xFF] ^ iter;
                break;
            case 6:
                x = y = z = iter;
                break;
            case 7:
                /* Clock call adds scheduling complexity */
                if ((iter % 19) == 0) {
                    volatile clock_t clk = clock();
                    arr1[iter & 0xFF] ^= clk & 0xFFFF;
                }
                __asm__ volatile ("" : : : "memory");
                break;
        }
        
        /* Dependency chain across iterations */
        acc = acc * 31 + x;
        acc = acc ^ (y << 8);
        acc = acc + (z * iter);
        
        if (iter % 8 == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int outer_limit = 8;  /* Volatile to prevent constant propagation */
    volatile int mode = 0;
    volatile int total_result = 0;
    
    /* Multiple calls to create scheduling contexts across different phases */
    for (int call_num = 0; call_num < 10; call_num++) {
        mode = call_num % 4;
        
        /* Vary loop limits to create different scheduling scenarios */
        volatile int current_limit = outer_limit + (call_num % 3);
        
        /* Call core scheduling function */
        volatile int result1 = complex_schedule_loop(array1, array2, 
                                                   current_limit, mode);
        
        /* Alternate between different scheduling patterns */
        if (call_num % 2 == 0) {
            volatile int result2 = alternate_schedule_pattern(array1, array2, 
                                                            current_limit * 2);
            total_result ^= result1 + result2;
        } else {
            total_result ^= result1 * 31;
        }
        
        /* Modify arrays between calls to create new scheduling contexts */
        for (int i = 0; i < 256; i += 8) {
            array1[i] ^= call_num;
            array2[i] += call_num * i;
        }
        
        /* Memory barrier between scheduling contexts */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= total_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
