/* sel-sched-test.c - Test program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function to prevent optimization */
static int __attribute__((noinline)) use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Non-inline function to generate call RTL */
static int __attribute__((noinline)) helper_compute(int a, int b) {
    return (a * b) ^ (a + b);
}

/* Test 1: Complex integer loop with mixed operations */
__attribute__((noinline, optimize("O3")))
int test_complex_loop(int* array, int size) {
    int sum = 0;
    int prod = 1;
    float fsum = 0.0f;
    
    for (int i = 0; i < size; i++) {
        /* Various integer operations */
        int val = array[i];
        sum += val * i;
        prod *= (val > 0) ? val : 1;
        
        /* Conditional move pattern */
        int max_val = (val > sum) ? val : sum;
        
        /* Mixed float/int operations */
        fsum += (float)val * 0.5f;
        
        /* Memory barrier to create scheduling region */
        asm volatile("" : : : "memory");
        
        /* Builtin function for complex RTL */
        int bits = __builtin_popcount(val);
        sum ^= bits;
        
        /* Function call */
        sum += helper_compute(val, i);
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            sum -= val;
        } else if (i % 3 == 1) {
            sum |= val;
        } else {
            sum &= ~val;
        }
    }
    
    /* Mix results to prevent dead code elimination */
    return sum + (int)fsum + prod;
}

/* Test 2: Nested loops with pointer arithmetic */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(short* data, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        short* row_ptr = data + i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Pointer arithmetic and memory access */
            short val = row_ptr[j];
            
            /* Mixed 32/64-bit operations */
            int64_t big_val = (int64_t)val * j;
            row_sum += (int)(big_val & 0xFFFFFFFF);
            
            /* Complex conditional */
            int cond_val = (val > 100) ? val * 2 : 
                          (val < -100) ? val / 2 : val;
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
            
            /* Use builtin */
            row_sum += __builtin_ffs(val & 0xFF);
            
            /* Another branch for control flow */
            if ((i + j) % 4 == 0) {
                row_sum <<= 2;
            }
        }
        
        total += row_sum;
        
        /* Outer loop barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Test 3: Floating point intensive with conversions */
__attribute__((noinline, optimize("O3")))
float test_float_ops(float* fa, double* da, int n) {
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Float/double mix */
        float fval = fa[i];
        double dval = da[i];
        
        /* Various FP operations */
        fsum += fval * 1.5f;
        dsum += dval * 2.5;
        
        /* Conversions */
        fsum += (float)dval;
        dsum += (double)fval;
        
        /* Conditional FP operation */
        float fcond = (fval > 0.0f) ? fval : -fval;
        fsum += fcond;
        
        /* Integer mixed in */
        int ival = (int)fval;
        fsum += (float)(ival % 10);
        
        /* Barrier */
        asm volatile("" : : : "memory");
        
        /* Complex expression */
        fsum = fsum * 0.99f + fval * 0.01f;
    }
    
    /* Prevent optimization */
    volatile float fsink = fsum;
    volatile double dsink = dsum;
    
    return fsum + (float)dsum;
}

/* Test 4: Switch statement with various operations */
__attribute__((noinline, optimize("O3")))
int test_switch_pattern(int* data, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int val = data[i] % 7;
        
        switch (val) {
            case 0:
                result += i * 2;
                asm volatile("" : : : "memory");
                break;
            case 1:
                result -= i;
                result = __builtin_bswap32(result);
                break;
            case 2:
                result ^= 0xAAAA;
                result *= 3;
                break;
            case 3:
                result |= 0xF0F0;
                result = (result >> 4) | (result << 28);
                break;
            case 4:
                result &= 0x0F0F0F0F;
                result += helper_compute(i, val);
                break;
            case 5:
                result = ~result;
                asm volatile("" : : : "memory");
                break;
            default:
                result += val;
                result = (result < 0) ? -result : result;
                break;
        }
        
        /* Another barrier */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return result;
}

/* Main driver */
int main(void) {
    const int SIZE = 256;
    int* int_array = malloc(SIZE * sizeof(int));
    short* short_array = malloc(SIZE * 2 * sizeof(short));
    float* float_array = malloc(SIZE * sizeof(float));
    double* double_array = malloc(SIZE * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
        short_array[i * 2] = (short)(i * 3);
        short_array[i * 2 + 1] = (short)(i * 5);
        float_array[i] = (float)(i * 0.1);
        double_array[i] = (double)(i * 0.01);
    }
    
    /* Run all tests */
    int result1 = test_complex_loop(int_array, SIZE);
    int result2 = test_nested_loops(short_array, 16, 32);
    float result3 = test_float_ops(float_array, double_array, SIZE);
    int result4 = test_switch_pattern(int_array, SIZE);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + (int)result3 + result4;
    
    /* Use the result */
    final_result = use_result(final_result);
    
    printf("Test completed. Checksum: %d\n", final_result);
    
    free(int_array);
    free(short_array);
    free(float_array);
    free(double_array);
    
    return 0;
}
