/* test_sel_sched.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * when compiled with flags like -fsel-sched-dump or -fdump-rtl-sched*.
 * The uncovered lines in sel-sched-dump.cc are hit during compilation,
 * not during execution of this program.
 */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent control flow and operations
 * that create interesting scheduling opportunities */
static unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int state = seed; /* volatile to prevent optimization */
    unsigned int result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        unsigned int temp = state;
        
        /* Inner loop with varying iteration count */
        for (j = 0; j < (i % 8) + 1; j++) {
            /* Mixed arithmetic operations creating dependencies */
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            temp = temp ^ (temp >> 16);
            temp = temp + (temp << 3);
            temp = temp - (temp >> 5);
            
            /* Bitwise operations */
            temp = (temp & 0x55555555) << 1 | (temp & 0xAAAAAAAA) >> 1;
            temp = temp ^ (i * j);
            
            /* Conditional operation based on computed value */
            if (temp % 7 == 0) {
                temp = temp / 3;
            } else if (temp % 5 == 0) {
                temp = temp * 2;
            } else {
                temp = temp | 1;
            }
            
            /* Memory barrier via inline asm to constrain scheduling */
            asm volatile("" ::: "memory");
        }
        
        /* Data-dependent branching */
        if (temp % 3 == 0) {
            result += temp;
        } else if (temp % 4 == 0) {
            result -= temp;
        } else {
            result ^= temp;
        }
        
        /* Complex update with division (expensive operation) */
        state = (state + temp) % 1000007;
        
        /* Early exit condition based on computation */
        if (result > 0xffff0000) {
            break;
        }
    }
    
    return result;
}

/* Another function with different pattern to increase scheduling complexity */
static long process_array(int *arr, int n) {
    volatile long sum = 0;
    volatile long product = 1;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Switch statement for control flow variety */
        switch (arr[i] % 6) {
            case 0:
                sum += arr[i] * 2;
                product *= (arr[i] % 100) + 1;
                break;
            case 1:
                sum -= arr[i] / 3;
                product /= (arr[i] % 50) + 1;
                break;
            case 2:
                sum ^= arr[i];
                product = product << (arr[i] % 4);
                break;
            case 3:
                sum = sum >> 1;
                product = product | arr[i];
                break;
            case 4:
                sum = sum * 3 - arr[i];
                product = product & 0x0f0f0f0f;
                break;
            default:
                sum = sum + (sum << 2);
                product = product % 1000;
                break;
        }
        
        /* Nested conditional */
        if (i % 10 == 0) {
            for (int k = 0; k < 3; k++) {
                arr[i] = (arr[i] + k) * 7;
            }
        }
    }
    
    /* Final computation mixing both values */
    return sum + product;
}

int main(int argc, char **argv) {
    unsigned int seed = 12345;
    int i;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Create array with data dependencies */
    int array[100];
    for (i = 0; i < 100; i++) {
        array[i] = (i * seed + 7) % 1000;
    }
    
    /* Call complex functions multiple times with different parameters */
    unsigned int total = 0;
    for (i = 0; i < 5; i++) {
        total += complex_loop(seed + i * 100, 50 + i * 10);
    }
    
    long array_result = process_array(array, 100);
    
    /* Mix results in non-trivial way */
    unsigned int final_result = (total % 100000) + (array_result % 100000);
    
    /* Output to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return 0;
}
