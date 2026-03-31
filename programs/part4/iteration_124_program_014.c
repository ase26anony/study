/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_loop(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer operations */
        int val = arr[i];
        int squared = val * val;
        int shifted = squared >> 3;
        
        /* Conditional operation */
        int cond_val = (val > 100) ? val : shifted;
        
        /* Function call */
        int multiplied = helper_mul(cond_val, i);
        
        /* Builtin function */
        int popcnt = __builtin_popcount(multiplied);
        
        /* Memory access with barrier */
        arr[i] = popcnt + i;
        asm volatile("" : : : "memory");
        
        /* Floating point operations */
        float fval = (float)val;
        float fsquared = fval * fval;
        float fscaled = helper_fmul(fsquared, 1.5f);
        
        /* Mixed-type accumulation */
        sum += multiplied + (int)fscaled;
        fsum += fscaled;
        
        /* Branch to create control flow */
        if (i % 7 == 0) {
            sum -= popcnt;
            asm volatile("" : : : "memory");
        } else if (i % 13 == 0) {
            fsum *= 0.99f;
        }
    }
    
    /* Final computation with volatile to prevent optimization */
    volatile int final_sum = sum + (int)fsum;
    return final_sum;
}

/* Test with 64-bit operations - Test 2 */
__attribute__((noinline, optimize("O3")))
long long test_64bit_ops(long long* arr, int n) {
    long long total = 0;
    long long product = 1;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit operations */
        long long val = arr[i];
        long long squared = val * val;
        long long shifted = squared << (i & 7);
        
        /* Conditional move pattern */
        long long selected = (squared > shifted) ? squared : shifted;
        
        /* Mixed 32/64 bit operations */
        int low_part = (int)(selected & 0xFFFFFFFF);
        int high_part = (int)(selected >> 32);
        int mixed = low_part * high_part;
        
        /* Memory store with complex addressing */
        arr[(i * 3) % n] = selected + mixed;
        
        /* Accumulate with dependency */
        total += selected;
        product *= (mixed + 1);
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another branch for control flow */
        if (total > product) {
            total -= product % 256;
        }
    }
    
    return total ^ product;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_pointer_chasing(int** ptr_arr, int n) {
    int sum = 0;
    int* current = ptr_arr[0];
    
    for (int i = 0; i < n; i++) {
        /* Pointer dereference */
        int val = *current;
        
        /* Complex computation */
        int transformed = ((val << 4) | (val >> 28)) ^ 0x5A5A5A5A;
        
        /* Builtin for complex RTL */
        int leading_zeros = __builtin_clz(transformed);
        int trailing_zeros = __builtin_ctz(transformed | 1);
        
        /* Conditional store */
        if (transformed > 0) {
            *current = transformed;
        }
        
        /* Update pointer with bounds check */
        int next_idx = (i * 17 + 1) % n;
        current = ptr_arr[next_idx];
        
        /* Accumulate with memory barrier */
        sum += leading_zeros - trailing_zeros;
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Vector-like operations - Test 4 */
__attribute__((noinline, optimize("O3")))
int test_vector_ops(short* data, int n) {
    int sums[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < n - 3; i += 4) {
        /* Load multiple values */
        short v0 = data[i];
        short v1 = data[i + 1];
        short v2 = data[i + 2];
        short v3 = data[i + 3];
        
        /* SIMD-like operations */
        int ext0 = (int)v0 * v0;
        int ext1 = (int)v1 * v1;
        int ext2 = (int)v2 * v2;
        int ext3 = (int)v3 * v3;
        
        /* Reductions */
        sums[0] += ext0;
        sums[1] += ext1;
        sums[2] += ext2;
        sums[3] += ext3;
        
        /* Cross-element operations */
        sums[0] += (ext1 >> 4);
        sums[1] += (ext2 >> 4);
        sums[2] += (ext3 >> 4);
        sums[3] += (ext0 >> 4);
        
        /* Conditional update */
        if (sums[0] > sums[1]) {
            data[i] = (short)(sums[0] & 0xFFFF);
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final reduction */
    return sums[0] + sums[1] + sums[2] + sums[3];
}

/* Main driver */
int main() {
    const int SIZE = 256;
    
    /* Allocate and initialize test data */
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    long long* ll_arr = (long long*)malloc(SIZE * sizeof(long long));
    int** ptr_arr = (int**)malloc(SIZE * sizeof(int*));
    short* short_arr = (short*)malloc(SIZE * sizeof(short));
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 37 + 123) & 0xFFF;
        ll_arr[i] = (long long)i * i * 7919;
        ptr_arr[i] = &int_arr[i];
        short_arr[i] = (short)(i * 97);
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_loop(int_arr, SIZE);
    long long result2 = test_64bit_ops(ll_arr, SIZE / 4);
    int result3 = test_pointer_chasing(ptr_arr, SIZE / 8);
    int result4 = test_vector_ops(short_arr, SIZE);
    
    /* Combine results with volatile to ensure execution */
    volatile int final_result = result1 + (int)result2 + result3 + result4;
    
    printf("Test results: %d, %lld, %d, %d\n", result1, result2, result3, result4);
    printf("Final combined: %d\n", final_result);
    
    /* Cleanup */
    free(int_arr);
    free(ll_arr);
    free(ptr_arr);
    free(short_arr);
    
    return 0;
}
