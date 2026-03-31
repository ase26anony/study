/* Test for selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

/* Helper functions to prevent optimization */
static volatile int sink;

/* Non-inline function to force call RTL */
__attribute__((noinline, optimize("O2")))
int helper_multiply(int a, int b) {
    return a * b + (a ^ b);
}

/* Non-inline function with mixed operations */
__attribute__((noinline, optimize("O2")))
float helper_float(float a, float b) {
    return a * b + (a / (b + 1.0f));
}

/* Test 1: Complex integer loop with data dependencies */
__attribute__((noinline, optimize("O2")))
int test_selective_sched_int(int* array, int size) {
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < size; i++) {
        /* Create ILP opportunities */
        int val = array[i];
        int squared = val * val;
        int shifted = val << (i & 3);
        
        /* Conditional operation - may generate cond_exec RTL */
        int cond_val = (val > 0) ? squared : shifted;
        
        /* Mix with helper call */
        int helper_result = helper_multiply(val, i);
        
        /* Complex computation chain */
        sum += cond_val + helper_result;
        prod *= (val & 0xFF) + 1;
        
        /* Memory barrier to create scheduling region */
        asm volatile ("" : : : "memory");
        
        /* Use builtin for complex RTL pattern */
        if (i % 16 == 0) {
            sum += __builtin_popcount(val);
        }
    }
    
    sink = prod; /* Prevent dead code elimination */
    return sum;
}

/* Test 2: Mixed int/float operations */
__attribute__((noinline, optimize("O2")))
float test_selective_sched_mixed(float* farr, int* iarr, int size) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Integer operations */
        int ival = iarr[i];
        int imod = ival % 256;
        int ishift = ival >> (imod & 7);
        
        /* Float operations */
        float fval = farr[i];
        float fsquared = fval * fval;
        float fscaled = fval * (float)i;
        
        /* Conditional float operation */
        float fcond = (fval > 0.0f) ? fsquared : fscaled;
        
        /* Call float helper */
        float fhelper = helper_float(fval, fcond);
        
        /* Mixed type computation */
        fsum += fhelper + (float)imod;
        isum += ishift + (int)(fval * 100.0f);
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Complex control flow */
        if (i % 8 == 0) {
            fsum *= 0.99f;
            isum ^= 0x55AA55AA;
        } else if (i % 3 == 0) {
            fsum -= fval;
            isum |= 0x00FF00FF;
        }
    }
    
    sink = isum;
    return fsum;
}

/* Test 3: Nested loops with array updates */
__attribute__((noinline, optimize("O2")))
int test_selective_sched_nested(int size) {
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return -1;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        array[i] = i * 3 + 1;
    }
    
    int total = 0;
    /* Outer loop */
    for (int i = 0; i < size - 1; i++) {
        /* Inner loop with data dependency */
        for (int j = i + 1; j < size; j++) {
            if (j - i < 10) {
                int diff = array[j] - array[i];
                int abs_diff = (diff > 0) ? diff : -diff;
                
                /* Complex expression with multiple operations */
                total += abs_diff * ((i * j) % 256);
                
                /* Update array element - creates store RTL */
                array[j] = (array[j] + array[i]) & 0xFFFF;
            }
        }
        
        /* Scheduling barrier between outer loop iterations */
        asm volatile ("" : : : "memory");
    }
    
    free(array);
    return total;
}

/* Test 4: Switch statement with different operations */
__attribute__((noinline, optimize("O2")))
int test_selective_sched_switch(int x, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        switch ((x + i) % 7) {
            case 0:
                result += i * 3;
                result ^= 0x12345678;
                break;
            case 1:
                result -= i * 5;
                result |= 0x87654321;
                break;
            case 2:
                result *= (i & 0xFF) + 1;
                result = (result > 1000000) ? result % 1000000 : result;
                break;
            case 3:
                result = helper_multiply(result, i);
                result = __builtin_bswap32(result);
                break;
            case 4:
                result = (result << 4) | (result >> 28);
                result += __builtin_popcount(i);
                break;
            case 5:
                result ^= result >> 16;
                result *= 1103515245;
                result += 12345;
                break;
            default:
                result = ~result;
                result += i * i;
                break;
        }
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    
    /* Test data */
    int int_array[SIZE];
    float float_array[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 7 - 13;
        float_array[i] = (float)i * 0.1f - 5.0f;
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_int(int_array, SIZE);
    float result2 = test_selective_sched_mixed(float_array, int_array, SIZE);
    int result3 = test_selective_sched_nested(100);
    int result4 = test_selective_sched_switch(42, 1000);
    
    /* Combine results */
    int final_result = result1 + (int)result2 + result3 + result4;
    
    printf("Selective scheduler test result: %d\n", final_result);
    printf("Result1: %d, Result2: %.2f, Result3: %d, Result4: %d\n",
           result1, result2, result3, result4);
    
    return 0;
}
