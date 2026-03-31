/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Volatile globals to create memory barriers */
volatile int g_volatile_counter = 0;
volatile int* g_volatile_ptr = &g_volatile_counter;
volatile float g_volatile_float = 3.14159f;

/* Arrays for data dependencies */
int g_int_array[256];
float g_float_array[256];
__m128i g_vector_array[64];

/* Helper functions that won't be inlined */
NOINLINE int helper_complex_calc(int a, int b, int c) {
    /* Complex calculation chain */
    int t1 = a * b + c;
    int t2 = (t1 >> 3) & 0xFF;
    int t3 = t2 * t2 - t1;
    asm volatile ("nop" : : : "memory");  /* Scheduling barrier */
    return t3 ^ (t1 << 2);
}

NOINLINE float helper_float_ops(float a, float b, float c) {
    /* Floating point chain */
    float t1 = a * b + c;
    float t2 = t1 / (b + 1.0f);
    float t3 = t2 * t2 - t1;
    /* Force memory clobber */
    asm volatile ("" : : : "memory");
    return t3;
}

NOINLINE __m128i helper_vector_op(__m128i a, __m128i b) {
    /* Vector operations */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_slli_epi32(t1, 3);
    __m128i t3 = _mm_xor_si128(t1, t2);
    /* Explicit clobber of vector registers */
    asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return t3;
}

/* Always inline to create larger scheduling regions */
ALWAYS_INLINE int complex_scheduling_region(int seed) {
    int result = seed;
    volatile int* local_vol = g_volatile_ptr;
    
    /* Pattern 1: Multiple interdependent integer operations */
    int a = result + 1;
    int b = a * 2;
    int c = b >> 4;
    int d = c ^ a;
    int e = d * 3 + b;
    
    /* Pattern 2: Volatile memory accesses creating barriers */
    *local_vol = *local_vol + 1;
    int f = e + *local_vol;
    *local_vol = f & 0xFF;
    
    /* Pattern 3: Inline assembly with explicit clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (f)
        : "r" (f)
        : "eax", "memory"
    );
    
    /* Pattern 4: Mixed data types */
    float fa = (float)f * 1.5f;
    float fb = fa + g_volatile_float;
    int g = (int)fb ^ f;
    
    /* Pattern 5: Architecture-specific builtins */
    g += __builtin_popcount(f) * 2;
    g += __builtin_ctz(f | 1) * 3;
    
    return g;
}

/* Function with nested control flow */
NOINLINE int nested_control_flow(int base) {
    int result = base;
    
    /* Outer loop */
    for (int i = 0; i < 8; i++) {
        /* Inner loop with data-dependent exit */
        for (int j = 0; j < (result & 0x7) + 1; j++) {
            /* Switch inside loop */
            switch ((i + j) & 0x3) {
                case 0: {
                    /* Vector operations */
                    __m128i v1 = _mm_set_epi32(i, j, result, base);
                    __m128i v2 = _mm_set_epi32(j, i, base, result);
                    __m128i v3 = helper_vector_op(v1, v2);
                    int temp[4];
                    _mm_storeu_si128((__m128i*)temp, v3);
                    result += temp[0] + temp[1];
                    break;
                }
                case 1: {
                    /* Integer chain */
                    int t1 = result * 3;
                    int t2 = t1 >> (j & 0x3);
                    int t3 = t2 ^ (i * 7);
                    result = helper_complex_calc(result, t2, t3);
                    break;
                }
                case 2: {
                    /* Floating point operations */
                    float ft1 = (float)result * 0.5f;
                    float ft2 = helper_float_ops(ft1, (float)j, (float)i);
                    result += (int)ft2;
                    /* Memory barrier */
                    asm volatile ("" : : : "memory");
                    break;
                }
                case 3: {
                    /* Mixed operations with goto */
                    if (__builtin_expect((result & 0xFF) == 0, 0)) {
                        result += 1;
                    } else {
                        /* Create a small basic block with goto */
                        int tmp = result;
                        tmp *= 2;
                        tmp += i * j;
                        result = tmp;
                    }
                    break;
                }
            }
            
            /* Conditional break */
            if ((result & 0xF) == 0xF) {
                result ^= 0xAA;
                if (j > 2) break;
            }
        }
        
        /* More operations after inner loop */
        result = complex_scheduling_region(result);
        
        /* Array accesses with pointer arithmetic */
        g_int_array[i] = result;
        int* ptr = &g_int_array[i];
        ptr[1] = ptr[0] + i;
        ptr[2] = ptr[1] * result;
    }
    
    return result;
}

/* Another scheduling region with different pattern */
NOINLINE int second_scheduling_region(int start) {
    int acc = start;
    
    /* Unrolled loop with many independent instructions */
    #pragma GCC unroll 4
    for (int i = 0; i < 16; i++) {
        /* Independent computations that can be parallelized */
        int t1 = acc + i;
        int t2 = acc * i;
        int t3 = acc ^ i;
        float f1 = (float)acc * 0.25f;
        float f2 = f1 + (float)i;
        
        /* Force them to be used together */
        acc = (t1 + t2) ^ t3;
        acc += (int)(f1 * f2);
        
        /* Memory operation every few iterations */
        if ((i & 0x3) == 0) {
            g_volatile_counter = acc;
            asm volatile ("" : : : "memory");
        }
    }
    
    /* SIMD operations */
    __m128i vec_acc = _mm_set1_epi32(acc);
    for (int i = 0; i < 4; i++) {
        __m128i vec1 = _mm_loadu_si128((__m128i*)&g_int_array[i*4]);
        __m128i vec2 = _mm_add_epi32(vec_acc, vec1);
        __m128i vec3 = _mm_mullo_epi32(vec2, _mm_set1_epi32(i+1));
        _mm_storeu_si128((__m128i*)&g_int_array[i*4], vec3);
        
        /* Clobber vector registers */
        asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    return acc;
}

/* Main function with multiple scheduling regions */
int main() {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        g_int_array[i] = i;
        g_float_array[i] = (float)i * 0.1f;
        if (i < 64) {
            g_vector_array[i] = _mm_set_epi32(i, i*2, i*3, i*4);
        }
    }
    
    int result = 0x12345678;
    
    /* Region 1: Complex nested control flow */
    result = nested_control_flow(result);
    
    /* Region 2: Independent instruction parallelism */
    result ^= second_scheduling_region(result);
    
    /* Region 3: Switch with many cases */
    for (int iter = 0; iter < 3; iter++) {
        switch (result & 0x7) {
            case 0: {
                result = helper_complex_calc(result, iter, result >> 8);
                int t = result;
                t = t * 3 + 1;
                t = t >> 2;
                t = t ^ 0x55AA;
                result = t;
                break;
            }
            case 1: {
                float f = helper_float_ops((float)result, (float)iter, 0.5f);
                result += (int)f;
                result *= 2;
                break;
            }
            case 2: {
                __m128i v = helper_vector_op(
                    _mm_set1_epi32(result),
                    _mm_set1_epi32(iter)
                );
                int temp;
                _mm_store_ss((float*)&temp, _mm_castsi128_ps(v));
                result ^= temp;
                break;
            }
            case 3: {
                /* Long chain of operations */
                int x = result;
                for (int k = 0; k < 5; k++) {
                    x = (x << 3) | (x >> 29);
                    x ^= k * 0x9E3779B9;
                    x += g_int_array[k];
                }
                result = x;
                break;
            }
            case 4: {
                /* Memory intensive */
                for (int k = 0; k < 8; k++) {
                    g_int_array[k] = result + k;
                    g_float_array[k] = (float)result * 0.01f * k;
                    result += g_int_array[k] + (int)g_float_array[k];
                }
                break;
            }
            case 5: {
                /* Builtin functions */
                result += __builtin_popcount(result) * 17;
                result += __builtin_ctz(result | 1) * 23;
                result += __builtin_parity(result) * 31;
                break;
            }
            case 6: {
                /* Inline assembly with multiple constraints */
                int old_result = result;
                asm volatile (
                    "movl %1, %%ecx\n\t"
                    "leal (%%ecx, %%ecx, 2), %%eax\n\t"
                    "addl $0x4567, %%eax\n\t"
                    "rorl $5, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "=r" (result)
                    : "r" (old_result)
                    : "eax", "ecx", "memory"
                );
                break;
            }
            case 7: {
                /* Mixed operations */
                result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
                float f = (float)result / 65536.0f;
                result = (int)(f * 1000.0f) ^ result;
                break;
            }
        }
        
        /* Volatile access between cases */
        g_volatile_counter = result;
    }
    
    /* Region 4: Final computations */
    result = complex_scheduling_region(result);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    /* Additional scheduling region in cleanup-like code */
    {
        int cleanup = result;
        for (int i = 0; i < 32; i++) {
            cleanup ^= g_int_array[i & 0xFF];
            cleanup = (cleanup << 1) | (cleanup >> 31);
            if ((i & 0x7) == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        final_result += cleanup;
    }
    
    return final_result & 0x7FFFFFFF;  /* Ensure positive return */
}
