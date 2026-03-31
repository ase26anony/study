/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    *p = *p * 2 + 1;
    return *p;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float temp = a;
    for (int i = 0; i < 3; i++) {
        temp = temp * b - i;
    }
    return temp;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    int result = seed;
    volatile int barrier = 0;
    
    /* Region 1: Mixed operations with volatile and inline asm */
    int a = seed * 2;
    int b = seed + 7;
    volatile int* vptr = &barrier;
    
    /* Force scheduling barriers */
    asm volatile ("nop; nop; nop" : : : "memory", "eax", "ebx");
    
    int c = helper1(a, b);
    *vptr = *vptr + c;
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(a, b, c, seed);
    __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    /* Extract results with inline asm constraints */
    int vec_result;
    asm volatile ("movd %1, %0" : "=r"(vec_result) : "x"(vec3) : "memory");
    
    /* Region 2: Nested control flow with data-dependent exits */
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * seed;
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        if (__builtin_expect(i % 3 == 0, 0)) {
            sum += helper2(arr, i);
            asm volatile ("" : : : "memory");
        } else {
            sum -= arr[i] >> 2;
        }
        
        /* Complex exit condition */
        if (sum > 1000) {
            for (int j = 0; j < 4; j++) {
                sum += (arr[j] * j) / (seed + 1);
                barrier++;
            }
            break;
        }
    }
    
    /* Region 3: Switch with multiple cases */
    switch (seed % 5) {
        case 0: {
            float f1 = seed * 0.5f;
            float f2 = helper3(f1, 2.0f);
            result += (int)(f1 + f2);
            /* More operations to create large basic block */
            result ^= (a * b) | c;
            result += __builtin_popcount(seed);
            result -= vec_result;
            break;
        }
        case 1:
            result = helper1(result, sum);
            result += __builtin_ctz(seed | 1);
            /* Memory barrier */
            asm volatile ("mfence" : : : "memory");
            break;
        case 2: {
            /* Vector operations */
            __m128 vf1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 vf2 = _mm_set_ps(seed * 0.1f, seed * 0.2f, seed * 0.3f, seed * 0.4f);
            __m128 vf3 = _mm_add_ps(vf1, vf2);
            float farr[4];
            _mm_store_ps(farr, vf3);
            result += (int)(farr[0] + farr[1] + farr[2] + farr[3]);
            break;
        }
        case 3:
            /* Many independent instructions for parallel scheduling */
            int t1 = result * 3;
            int t2 = sum / 2;
            int t3 = barrier ^ seed;
            int t4 = t1 + t2;
            int t5 = t3 * t4;
            int t6 = helper1(t4, t5);
            int t7 = t6 >> 4;
            int t8 = t7 & 0xFF;
            result = t8 + helper2(arr, t8 % 16);
            break;
        case 4:
            /* Loop with volatile and function calls */
            for (int i = 0; i < 8; i++) {
                volatile int counter = i;
                result += helper1(result, counter);
                if (i % 2 == 0) {
                    result -= helper2(arr, i);
                } else {
                    result ^= arr[i];
                }
                asm volatile ("" : : "r"(result), "r"(counter) : "memory");
            }
            break;
    }
    
    /* Region 4: Computed goto to create complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    goto *labels[result % 4];
    
label1:
    result = helper1(result, 42);
    goto join;
label2:
    result = helper2(&result, 0);
    goto join;
label3:
    result += __builtin_popcount(result);
    goto join;
label4:
    result ^= 0xDEADBEEF;
    /* fall through */
join:
    
    /* Final mixing */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

/* Second complex function with different patterns */
int another_scheduling_region(int base) {
    int acc = base;
    volatile int sync = 0;
    
    /* Mixed integer/float operations */
    for (int i = 0; i < 20; i++) {
        float f = i * 0.5f;
        acc += (int)f;
        
        if (i % 4 == 0) {
            acc = helper1(acc, i);
            sync++;
        } else if (i % 4 == 1) {
            acc ^= __builtin_ctz(i | 1);
            asm volatile ("nop" : : : "memory");
        } else if (i % 4 == 2) {
            /* Vector operation */
            __m128i v = _mm_set1_epi32(acc);
            __m128i v2 = _mm_slli_epi32(v, 2);
            int temp;
            _mm_store_ss((float*)&temp, _mm_castsi128_ps(v2));
            acc += temp;
        } else {
            acc = helper2(&acc, 0);
        }
        
        /* Memory barrier every 5 iterations */
        if (i % 5 == 0) {
            asm volatile ("mfence" : : : "memory");
        }
    }
    
    /* Switch with fall-through cases */
    switch (acc % 8) {
        case 0: acc += 1;
        case 1: acc *= 2;
        case 2: acc ^= 0x1234;
        case 3: acc = helper1(acc, 3);
        case 4: acc -= sync;
        case 5: acc |= 0xFF00;
        case 6: acc = helper2(&acc, 0);
        case 7: acc = __builtin_bswap32(acc);
    }
    
    return acc;
}

int main() {
    int final_result = 0;
    
    /* Multiple calls to create different scheduling contexts */
    for (int i = 0; i < 100; i++) {
        /* Vary optimization hints */
        if (i % 3 == 0) {
            #pragma GCC optimize("O3")
            final_result ^= complex_scheduling_region(i + final_result);
        } else if (i % 3 == 1) {
            #pragma GCC optimize("O2")
            final_result += another_scheduling_region(i ^ final_result);
        } else {
            #pragma GCC optimize("Os")
            final_result = helper1(final_result, i);
            final_result = helper2(&final_result, 0);
        }
        
        /* Occasionally add memory barriers */
        if (i % 7 == 0) {
            asm volatile ("lock; addl $0, 0(%%rsp)" : : : "memory", "cc");
        }
    }
    
    /* Final computation with SIMD */
    __m128i vsum = _mm_setzero_si128();
    int temp_arr[4] = {final_result, final_result >> 8, final_result >> 16, final_result >> 24};
    __m128i v = _mm_loadu_si128((__m128i*)temp_arr);
    vsum = _mm_add_epi32(vsum, v);
    
    int final_arr[4];
    _mm_storeu_si128((__m128i*)final_arr, vsum);
    
    for (int i = 0; i < 4; i++) {
        final_result += final_arr[i];
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    return final_result & 0xFF;
}
