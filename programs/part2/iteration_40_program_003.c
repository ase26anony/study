/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o coverage_test coverage_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Force no-inline to create scheduling boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = a;
    asm volatile ("" : "+r" (barrier) : : "memory");
    return barrier + b * 3;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float v = a;
    asm volatile ("" : "+x" (v) : : "memory");
    return v * b + 1.0f;
}

__attribute__((noinline)) static void helper3(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + 1;
        asm volatile ("nop" : : : "memory");
    }
}

/* Complex SIMD operations to trigger target-specific scheduling */
__attribute__((noinline)) static __m128i simd_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi32(t1, _mm_set1_epi32(7));
    __m128i t3 = _mm_slli_epi32(t2, 3);
    __m128i t4 = _mm_xor_si128(t3, _mm_set1_epi32(0xAAAAAAAA));
    return _mm_and_si128(t4, _mm_set1_epi32(0x7FFFFFFF));
}

/* Function with mixed operations to create complex scheduling regions */
__attribute__((noinline)) static int complex_region(int seed, int* results) {
    volatile int* volatile_ptr = &seed;
    int checksum = 0;
    
    /* Region 1: Integer operations with dependencies */
    int a = seed + 1;
    int b = a * 3;
    int c = b >> 2;
    int d = c ^ 0x55AA55AA;
    int e = d * 7 + 1;
    *volatile_ptr = e;
    checksum += e;
    
    /* Region 2: Mixed float/int with barriers */
    float f1 = (float)seed * 1.5f;
    float f2 = helper2(f1, 2.0f);
    int i1 = (int)f2;
    checksum += i1;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull $137, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (i1)
        : "r" (checksum)
        : "%eax", "memory"
    );
    
    /* Region 3: SIMD operations */
    __m128i v1 = _mm_set_epi32(seed, seed+1, seed+2, seed+3);
    __m128i v2 = _mm_set_epi32(seed+4, seed+5, seed+6, seed+7);
    __m128i v3 = simd_op(v1, v2);
    int simd_results[4];
    _mm_storeu_si128((__m128i*)simd_results, v3);
    checksum += simd_results[0] + simd_results[1];
    
    /* Region 4: Loop with data-dependent exit */
    int loop_var = seed & 0xF;
    while (loop_var > 0) {
        int temp = helper1(loop_var, checksum);
        checksum ^= temp;
        loop_var -= (temp & 3);
        asm volatile ("" : : : "memory");
    }
    
    results[0] = checksum;
    return checksum;
}

/* Another complex region with switch statement */
__attribute__((noinline)) static int switch_region(int mode, int input) {
    int result = input;
    
    switch (mode & 7) {
        case 0: {
            /* Case with many independent instructions */
            int a = input + 1;
            int b = input * 2;
            int c = input & 0xFF;
            int d = input | 0xAA;
            int e = input ^ 0x55;
            result = a + b + c + d + e;
            asm volatile ("nop" : : : "memory");
            break;
        }
        case 1: {
            /* Mixed operations */
            float f = (float)input;
            f = f * 1.618f;
            result = (int)f;
            volatile int barrier = result;
            result = barrier * 3;
            break;
        }
        case 2: {
            /* SIMD case */
            __m128i v = _mm_set1_epi32(input);
            v = _mm_slli_epi32(v, 2);
            v = _mm_add_epi32(v, _mm_set1_epi32(1));
            result = _mm_extract_epi32(v, 0);
            break;
        }
        case 3: {
            /* Function call chain */
            result = helper1(input, input * 2);
            result = helper1(result, result >> 1);
            break;
        }
        default: {
            /* Complex default with nested operations */
            result = input;
            for (int i = 0; i < 4; i++) {
                result = (result * 3 + 1) & 0x7FFF;
                asm volatile ("" : : : "memory");
            }
            break;
        }
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    volatile int seed = 42;
    int results[256];
    int final_checksum = 0;
    
    /* Force multiple scheduling contexts */
    #pragma GCC optimize("O2")
    {
        /* Region A: Complex integer operations */
        int a = seed + 1;
        int b = a * 3;
        int c = b >> 2;
        int d = c ^ 0x55AA55AA;
        int e = d * 7 + 1;
        int f = e & 0x7FFFFFFF;
        int g = f | 0x80000000;
        int h = g ^ 0x12345678;
        int i = h * 13;
        int j = i / 3;
        int k = j % 17;
        int l = k << 4;
        int m = l >> 1;
        int n = m + 1;
        int o = n - 2;
        results[0] = o;
        final_checksum ^= o;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    #pragma GCC optimize("O3")
    {
        /* Region B: Mixed operations with SIMD */
        __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
        __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, _mm_set1_epi32(3));
        int simd_res[4];
        _mm_storeu_si128((__m128i*)simd_res, v4);
        
        for (int idx = 0; idx < 4; idx++) {
            results[idx + 1] = simd_res[idx];
            final_checksum += simd_res[idx];
        }
        
        /* Call to complex region */
        final_checksum ^= complex_region(seed, &results[10]);
    }
    
    #pragma GCC optimize("Os")
    {
        /* Region C: Switch-based region */
        for (int mode = 0; mode < 8; mode++) {
            results[20 + mode] = switch_region(mode, seed + mode);
            final_checksum += results[20 + mode];
        }
        
        /* Array operations with helper */
        helper3(&results[30], 16);
        for (int i = 0; i < 16; i++) {
            final_checksum ^= results[30 + i];
        }
    }
    
    #pragma GCC optimize("O3")
    {
        /* Region D: Nested loops with volatile */
        volatile int counter = 0;
        for (int outer = 0; outer < 4; outer++) {
            for (int inner = 0; inner < 8; inner++) {
                int val = seed + outer * 8 + inner;
                val = helper1(val, inner);
                val = (val * 3) & 0xFF;
                
                /* Conditional break with data dependency */
                if (val > 200) {
                    counter++;
                    if (counter > 2) break;
                }
                
                results[50 + outer * 8 + inner] = val;
                final_checksum += val;
                
                /* Memory barrier every few iterations */
                if ((inner & 3) == 0) {
                    asm volatile ("" : : : "memory");
                }
            }
        }
        
        /* Final SIMD reduction */
        __m128i sum_vec = _mm_setzero_si128();
        for (int i = 0; i < 64; i += 4) {
            __m128i data = _mm_loadu_si128((__m128i*)&results[i]);
            sum_vec = _mm_add_epi32(sum_vec, data);
        }
        int sum_arr[4];
        _mm_storeu_si128((__m128i*)sum_arr, sum_vec);
        final_checksum += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];
    }
    
    /* Use builtins that may have special scheduling */
    final_checksum += __builtin_popcount(final_checksum);
    final_checksum += __builtin_ctz(final_checksum | 1);
    
    /* Prevent dead code elimination */
    volatile int output = final_checksum;
    return output;
}
