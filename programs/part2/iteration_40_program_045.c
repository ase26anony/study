/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int barrier = 0;
    barrier = a + b;
    asm volatile("" : : : "memory");
    return barrier * 2;
}

__attribute__((noinline)) int helper2(int a, int b) {
    asm volatile("" : : : "eax", "ebx", "ecx", "edx");
    return (a ^ b) + (a & b);
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float f = a;
    f = f * b + 1.0f;
    return f;
}

__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i r1 = _mm_add_epi32(a, b);
    __m128i r2 = _mm_mullo_epi16(a, b);
    __m128i r3 = _mm_slli_epi32(r1, 3);
    return _mm_xor_si128(r2, r3);
}

/* Force scheduling state saving by creating complex basic blocks */
int main() {
    volatile int seed = 42;
    int result = 0;
    int i, j, k;
    
    /* Mixed data types to prevent optimization */
    int arr_int[256];
    float arr_float[256];
    volatile int* volatile_ptr = &seed;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr_int[i] = i * 3;
        arr_float[i] = i * 0.5f;
    }
    
    /* REGION 1: Complex integer operations with volatile and inline asm */
    for (i = 0; i < 100; i++) {
        int a = arr_int[i];
        int b = arr_int[i + 1];
        int c = arr_int[i + 2];
        
        /* Chain of dependent operations */
        a = a + b * 3;
        asm volatile("nop" : : : "memory");
        b = (a >> 4) | (c << 3);
        c = helper1(a, b);
        
        /* Volatile access creates scheduling barrier */
        *volatile_ptr = *volatile_ptr + 1;
        
        /* More operations */
        a = a ^ b ^ c;
        b = __builtin_popcount(a) + __builtin_ctz(b);
        c = helper2(a, b);
        
        /* Inline assembly with clobbers */
        asm volatile(
            "addl %%ebx, %%eax\n\t"
            "imull %%ecx, %%eax"
            : "=a"(a)
            : "a"(a), "b"(b), "c"(c)
            : "cc"
        );
        
        result += a + b + c;
    }
    
    /* REGION 2: Vector/SIMD operations for target-specific scheduling */
    __m128i vec_a = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_b = _mm_set_epi32(5, 6, 7, 8);
    
    for (i = 0; i < 50; i++) {
        /* Multiple vector operations */
        __m128i r1 = _mm_add_epi32(vec_a, vec_b);
        __m128i r2 = _mm_sub_epi32(vec_a, vec_b);
        __m128i r3 = _mm_madd_epi16(vec_a, vec_b);
        
        /* Mixed with scalar operations */
        int* r1_ptr = (int*)&r1;
        for (j = 0; j < 4; j++) {
            result += r1_ptr[j] + i;
        }
        
        /* More vector ops */
        vec_a = vector_op(vec_a, vec_b);
        vec_b = vector_op(vec_b, r1);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* REGION 3: Nested loops with switch for instruction queue buildup */
    for (i = 0; i < 20; i++) {
        switch (i % 5) {
            case 0: {
                /* Many independent instructions */
                int t1 = arr_int[i] + 1;
                int t2 = arr_int[i + 10] * 2;
                float f1 = arr_float[i] * 3.14f;
                float f2 = helper3(f1, arr_float[i + 5]);
                int t3 = t1 | t2;
                int t4 = t1 & t2;
                int t5 = t3 ^ t4;
                result += t5 + (int)f2;
                break;
            }
            case 1: {
                /* Another block of independent ops */
                for (j = 0; j < 3; j++) {
                    int x = arr_int[i + j];
                    int y = arr_int[i + j + 3];
                    x = x * y + j;
                    y = (x << j) | (x >> (32 - j));
                    result += helper1(x, y);
                }
                break;
            }
            case 2: {
                /* Mixed float/int operations */
                float sum = 0.0f;
                for (k = 0; k < 8; k++) {
                    sum += arr_float[i + k];
                    arr_int[i + k] = (int)(sum * 100.0f);
                }
                result += (int)sum;
                break;
            }
            case 3: {
                /* Complex pointer arithmetic */
                int* ptr = arr_int + i;
                for (j = 0; j < 4; j++) {
                    ptr[j] = ptr[j] * 2 + ptr[j + 1];
                    result += ptr[j];
                }
                break;
            }
            case 4: {
                /* All of the above mixed together */
                int a = arr_int[i];
                float b = arr_float[i];
                __m128i v = _mm_set1_epi32(a);
                a = helper2(a, result);
                b = helper3(b, b * 2.0f);
                v = vector_op(v, _mm_set1_epi32((int)b));
                int* vp = (int*)&v;
                result += a + (int)b + vp[0] + vp[1];
                break;
            }
        }
        
        /* Conditional break to create data-dependent exit */
        if (result > 1000000) {
            /* Force early exit path */
            for (j = 0; j < 10; j++) {
                result -= arr_int[j];
            }
            if (result < 0) break;
        }
    }
    
    /* REGION 4: Unrolled loop with many operations */
    #pragma GCC unroll 4
    for (i = 0; i < 32; i += 4) {
        /* Independent operations that can be scheduled in parallel */
        int r0 = arr_int[i] * 3 + 1;
        int r1 = arr_int[i + 1] * 5 - 2;
        int r2 = arr_int[i + 2] / 2 + 7;
        int r3 = arr_int[i + 3] ^ 0xFF;
        
        /* More operations creating dependencies */
        r0 = r0 | r1;
        r1 = r1 & r2;
        r2 = r2 ^ r3;
        r3 = r3 + r0;
        
        /* Function calls mixed in */
        r0 = helper1(r0, r1);
        r1 = helper2(r1, r2);
        
        /* Memory operations */
        arr_int[i] = r0;
        arr_int[i + 1] = r1;
        
        result += r0 + r1 + r2 + r3;
    }
    
    /* REGION 5: Computed goto for complex control flow */
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    for (i = 0; i < 10; i++) {
        goto *labels[i % 5];
        
        L0: {
            int x = result * 3;
            float y = x * 0.25f;
            result = (int)(y + helper3(y, 2.0f));
            continue;
        }
        L1: {
            __m128i v = _mm_set1_epi32(result);
            v = _mm_slli_epi32(v, 2);
            int* vp = (int*)&v;
            result = vp[0] + vp[1];
            continue;
        }
        L2: {
            result = helper2(result, i);
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            continue;
        }
        L3: {
            volatile int tmp = result;
            for (j = 0; j < 3; j++) {
                tmp = tmp * 2 + arr_int[j];
            }
            result = tmp;
            continue;
        }
        L4: {
            result = __builtin_popcount(result) + 
                    __builtin_ctz(result | 1) +
                    __builtin_clz(result | 1);
            continue;
        }
    }
    
    /* Final mixing */
    result = result ^ (result >> 16);
    result = result * 0x45d9f3b;
    result = result ^ (result >> 16);
    
    return result & 0x7FFFFFFF; /* Ensure positive result */
}
