/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Complex function with mixed operations */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Integer operations with data dependency */
        sum += val * i;
        sum ^= (val << 3) | (val >> 5);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        sum += cond_val;
        
        /* Floating point operations */
        fsum += (float)val * 0.12345f;
        
        /* Built-in function for complex RTL */
        sum += __builtin_popcount(val);
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Call to non-inline function */
        sum += helper_mul(val, i % 16);
        
        /* Another conditional with different types */
        if (i % 3 == 0) {
            fsum = helper_fmul(fsum, 1.01f);
            barrier = sum;  /* Volatile write */
        } else if (i % 3 == 1) {
            sum -= __builtin_clz(val | 1);
        } else {
            sum ^= ~val;
        }
        
        /* Array write with complex index */
        arr[(i + 1) % n] = sum & 0xFF;
    }
    
    /* Mix integer and float results */
    return sum + (int)fsum;
}

/* Function with nested loops for outer-loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* arr, int n) {
    int total = 0;
    
    /* Outer loop for pipelining */
    for (int i = 0; i < n; i += 4) {
        int block_sum = 0;
        
        /* Inner loop with unrollable pattern */
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            int idx = i + j;
            int val = arr[idx];
            
            /* Mixed-width operations */
            int64_t wide_val = (int64_t)val * val;
            block_sum += (int)wide_val + (int)(wide_val >> 32);
            
            /* Complex conditional */
            int pred = (val % 7) - 3;
            block_sum += (pred > 0) ? val : -val;
            
            /* Memory barrier */
            asm volatile("" : : : "memory");
        }
        
        total ^= block_sum * i;
        
        /* Function call in loop */
        total += helper_mul(block_sum, i % 8);
    }
    
    return total;
}

/* Function with pointer chasing and indirect calls */
__attribute__((noinline, optimize("O2")))
int test_function_3(int** ptr_arr, int n) {
    int result = 0;
    int* current = ptr_arr[0];
    
    for (int i = 0; i < n; i++) {
        /* Pointer dereference */
        int val = *current;
        
        /* Complex addressing mode */
        current = ptr_arr[(val + i) % n];
        
        /* Bit manipulation */
        result = ((result << 5) | (result >> 27)) ^ val;
        
        /* Volatile read for side effect */
        volatile int dummy = val;
        (void)dummy;
        
        /* Conditional with both paths used */
        if (val & 1) {
            result += __builtin_parity(val);
        } else {
            result -= __builtin_ctz(val | 1);
        }
    }
    
    return result;
}

/* Target-specific function for x86 */
#ifdef __x86_64__
__attribute__((target("arch=haswell"), noinline, optimize("O3")))
int test_function_x86(int* arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use 64-bit operations on x86_64 */
        int64_t val64 = arr[i];
        val64 = val64 * val64 * 1140671485 + 12820163;
        
        /* Mix with 32-bit operations */
        sum += (int)val64 + (int)(val64 >> 32);
        
        /* SIMD-like scalar operations */
        int a = arr[i] & 0xFF;
        int b = (arr[i] >> 8) & 0xFF;
        int c = (arr[i] >> 16) & 0xFF;
        sum += a * b + b * c + c * a;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}
#endif

/* Main test driver */
int main(void) {
    const int SIZE = 256;
    int* array = (int*)malloc(SIZE * sizeof(int));
    int** ptr_array = (int**)malloc(SIZE * sizeof(int*));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFF;
        ptr_array[i] = &array[(i * 13) % SIZE];
    }
    
    /* Call test functions with different patterns */
    int result1 = test_function_1(array, SIZE, 42);
    int result2 = test_function_2(array, SIZE);
    int result3 = test_function_3(ptr_array, SIZE);
    
    int total = result1 + result2 + result3;
    
    #ifdef __x86_64__
    total += test_function_x86(array, SIZE);
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Test result: %d\n", total);
    
    /* Verify with a simple checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array[i];
    }
    printf("Array checksum: %d\n", checksum);
    
    free(ptr_array);
    free(array);
    
    return (total > 0) ? 0 : 1;
}
