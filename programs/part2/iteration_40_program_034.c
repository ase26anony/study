/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o coverage_test coverage_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory", "eax", "ebx");
    return a * b + barrier;
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float v = 1.0f;
    asm volatile ("# FP barrier" : "+x"(a), "+x"(b) : : "memory", "st", "st(1)");
    return a * b + v;
}

__attribute__((noinline)) void helper3(volatile int* p, int n) {
    for (int i = 0; i < n; i++) {
        *p = *p + i;
        asm volatile ("mfence" : : : "memory");
    }
}

/* Force front-end state saving with PGO hints */
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Complex scheduling region 1: Mixed operations with SIMD */
int region1(int* arr, float* farr, volatile int* varr) {
    int sum = 0;
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    /* Store SIMD result to force scheduling context */
    _mm_storeu_si128((__m128i*)arr, vec3);
    
    /* Long dependency chain */
    int a = arr[0] + arr[1];
    int b = a * arr[2];
    int c = b >> arr[3];
    int d = c ^ a;
    int e = __builtin_popcount(d);
    
    /* Volatile access creates scheduling barrier */
    *varr = *varr + e;
    
    /* Floating point chain */
    float f = farr[0] * 1.5f;
    float g = f + farr[1];
    float h = g / 2.0f;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax"
        : "=a"(sum)
        : "a"(e), "b"(arr[4]), "c"((int)h)
        : "cc", "memory"
    );
    
    return sum;
}

/* Complex scheduling region 2: Nested control flow */
int region2(int* arr, volatile int* varr) {
    int result = 0;
    int i = 0;
    
    /* Nested loop with data-dependent exit */
    while (i < 16) {
        int j = 0;
        volatile int inner_barrier = arr[i];
        
        do {
            /* Multiple independent instructions for ready list */
            int t1 = arr[j] + 1;
            int t2 = arr[j+1] * 2;
            int t3 = arr[j+2] & 0xFF;
            int t4 = arr[j+3] | 0x80;
            
            /* Create instruction-level parallelism */
            result += t1 + t2 + t3 + t4;
            
            /* Memory barrier splits scheduling region */
            asm volatile ("" : : : "memory");
            
            j += helper1(j, 1);
            
            /* Conditional with PGO hint */
            if (UNLIKELY(j > 100)) break;
        } while (j < 8);
        
        *varr = *varr ^ result;
        i++;
    }
    
    return result;
}

/* Complex scheduling region 3: Switch with multiple cases */
int region3(int x, volatile int* varr) {
    int val = 0;
    
    switch (x % 7) {
        case 0: {
            /* Vector operations in case */
            __m128i v = _mm_set1_epi32(x);
            int arr[4];
            _mm_storeu_si128((__m128i*)arr, v);
            val = arr[0] + arr[1] - arr[2] * arr[3];
            /* Fall through */
        }
        case 1:
            val = __builtin_ctz(val | 1);
            val = helper1(val, x);
            break;
        case 2: {
            /* Mixed float/int operations */
            float f = (float)x * 3.14159f;
            int i = (int)f;
            val = i ^ x;
            /* Memory barrier */
            asm volatile ("# case2" : : : "memory", "eax", "ebx");
            break;
        }
        case 3:
            /* Long chain */
            val = x;
            for (int k = 0; k < 5; k++) {
                val = val * 2 + 1;
                val = val ^ (val >> 3);
            }
            break;
        case 4:
            val = __builtin_popcount(x);
            val = val << 4;
            break;
        case 5:
            /* Function call in switch */
            val = helper1(x, x+1);
            val = helper1(val, x+2);
            break;
        case 6:
        default:
            val = x * x - x;
            *varr = *varr + val;
            break;
    }
    
    return val;
}

/* Complex scheduling region 4: Multiple scheduling passes */
#pragma GCC optimize("O2")
int region4(int* arr, float* farr) {
    /* This region will have different optimization level */
    int sum = 0;
    
    /* Many independent instructions */
    int a1 = arr[0] + 1;
    int a2 = arr[1] * 2;
    int a3 = arr[2] & 0xFF;
    int a4 = arr[3] | 0x80;
    int a5 = arr[4] ^ 0x55;
    int a6 = arr[5] << 2;
    int a7 = arr[6] >> 1;
    int a8 = arr[7] + arr[8];
    
    /* Use all in expression */
    sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    
    /* Floating point parallel chain */
    float f1 = farr[0] * 1.1f;
    float f2 = farr[1] + 2.2f;
    float f3 = farr[2] / 3.3f;
    float f4 = farr[3] - 4.4f;
    
    /* Force register pressure */
    sum += (int)(f1 + f2 + f3 + f4);
    
    return sum;
}
#pragma GCC reset_options

/* Main function with multiple complex regions */
int main() {
    /* Initialize data arrays */
    int int_arr[256];
    float float_arr[256];
    volatile int volatile_var = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i * 3 + 1;
        float_arr[i] = (float)i * 0.5f;
    }
    
    int checksum = 0;
    
    /* Execute region 1 multiple times */
    for (int iter = 0; iter < 3; iter++) {
        checksum ^= region1(int_arr, float_arr, &volatile_var);
        
        /* Call helper to create scheduling context */
        helper3(&volatile_var, 4);
        
        /* Change data to affect scheduling */
        int_arr[iter * 10] = checksum;
    }
    
    /* Execute region 2 with nested loops */
    checksum += region2(int_arr, &volatile_var);
    
    /* Execute region 3 with switch */
    for (int i = 0; i < 10; i++) {
        checksum += region3(i + checksum, &volatile_var);
        
        /* Mix with floating point helper */
        float f = helper2(float_arr[i], float_arr[i+1]);
        checksum += (int)f;
    }
    
    /* Execute region 4 with different optimization */
    checksum ^= region4(int_arr, float_arr);
    
    /* Final complex region inline */
    {
        /* SIMD operations */
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 64; i += 4) {
            __m128i v = _mm_loadu_si128((__m128i*)&int_arr[i]);
            vsum = _mm_add_epi32(vsum, v);
        }
        
        /* Reduce vector */
        int arr[4];
        _mm_storeu_si128((__m128i*)arr, vsum);
        checksum += arr[0] + arr[1] + arr[2] + arr[3];
        
        /* Final volatile sync */
        asm volatile ("lock; addl $1, %0" : "+m"(volatile_var) : : "memory", "cc");
    }
    
    /* Use checksum to prevent dead code elimination */
    return checksum & 0xFF;
}
