/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    *p = *p * 2 + 1;
    asm volatile("" : : : "memory");
    return arr[idx];
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile("" : : : "memory", "xmm0", "xmm1");
    return res;
}

/* Complex SIMD operations */
__attribute__((noinline)) __m128i simd_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi16(a, b);
    __m128i t3 = _mm_slli_epi32(t1, 3);
    __m128i t4 = _mm_xor_si128(t2, t3);
    asm volatile("" : : "x" (t4) : "xmm0", "xmm1", "xmm2", "xmm3");
    return t4;
}

/* Force register constraints */
__attribute__((noinline)) int constrained_asm(int a, int b) {
    int result;
    /* Tie to specific registers */
    asm volatile ("addl %%ebx, %%eax\n\t"
                  "imull %%ecx, %%eax\n\t"
                  : "=a" (result)
                  : "a" (a), "b" (b), "c" (37)
                  : "cc", "memory");
    return result;
}

int main() {
    volatile int checksum = 0;
    int* int_arr = (int*)malloc(1024 * sizeof(int));
    float* float_arr = (float*)malloc(512 * sizeof(float));
    volatile int* volatile_ptr = (volatile int*)malloc(256 * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        int_arr[i] = i * 3 + 1;
        if (i < 512) float_arr[i] = i * 0.5f;
        if (i < 256) volatile_ptr[i] = i * 2;
    }
    
    /* REGION 1: Complex integer operations with volatile and inline asm */
    {
        int a = int_arr[0];
        int b = int_arr[1];
        int c = int_arr[2];
        int d = int_arr[3];
        
        /* Create long dependency chain */
        a = a + b * c;
        asm volatile("" : : : "memory");  /* Scheduling barrier */
        b = (a >> 3) | (c << 5);
        c = helper1(a, b);
        d = constrained_asm(b, c);
        
        /* Volatile access chain */
        volatile_ptr[0] = volatile_ptr[0] + a;
        volatile_ptr[1] = volatile_ptr[1] * b;
        volatile_ptr[2] = volatile_ptr[2] ^ c;
        
        /* More operations to fill instruction queue */
        int e = __builtin_popcount(a);
        int f = __builtin_ctz(b);
        int g = e * f + d;
        
        checksum += a + b + c + d + e + f + g;
    }
    
    /* REGION 2: Mixed integer/float with SIMD */
    {
        __m128i vec_a = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec_b = _mm_set_epi32(5, 6, 7, 8);
        
        /* Multiple SIMD operations */
        __m128i r1 = simd_op(vec_a, vec_b);
        __m128i r2 = _mm_srai_epi32(r1, 2);
        __m128i r3 = _mm_and_si128(r1, r2);
        
        /* Extract and use results */
        int simd_results[4];
        _mm_storeu_si128((__m128i*)simd_results, r3);
        
        /* Interleave with float ops */
        float f1 = float_arr[0];
        float f2 = float_arr[1];
        for (int i = 0; i < 4; i++) {
            f1 = helper3(f1, f2 + i);
            f2 = f2 * 0.9f - simd_results[i] * 0.01f;
            asm volatile("" : : : "memory", "xmm4", "xmm5");
        }
        
        checksum += simd_results[0] + simd_results[1] + 
                   simd_results[2] + simd_results[3] + (int)f1 + (int)f2;
    }
    
    /* REGION 3: Nested loops with switch */
    {
        int total = 0;
        for (int i = 0; i < 100; i++) {
            switch (i % 7) {
                case 0: {
                    int t = int_arr[i] * 3;
                    t = t ^ (t >> 1);
                    total += helper2(int_arr, t % 1024);
                    asm volatile("nop" : : : "memory", "eax");
                    break;
                }
                case 1: {
                    float f = float_arr[i % 512];
                    f = f * f - f;
                    total += (int)f;
                    volatile_ptr[i % 256] += total;
                    break;
                }
                case 2: {
                    /* Complex asm with clobbers */
                    int x = i * 11;
                    asm volatile ("movl %1, %%eax\n\t"
                                  "roll $3, %%eax\n\t"
                                  "movl %%eax, %0\n\t"
                                  : "=r" (x)
                                  : "r" (x)
                                  : "eax", "cc", "memory");
                    total += x;
                    break;
                }
                case 3:
                case 4:
                case 5: {
                    /* Multiple independent operations */
                    int a = int_arr[i] + 1;
                    int b = int_arr[i+1] * 2;
                    int c = int_arr[i+2] & 0xFF;
                    int d = a * b - c;
                    asm volatile("" : : : "memory");
                    total += d;
                    break;
                }
                default: {
                    total += __builtin_popcount(i);
                    volatile_ptr[i % 256] = total;
                    break;
                }
            }
            
            /* Conditional break to create complex control flow */
            if (total > 1000000) break;
            if (i % 13 == 0) continue;
            if (i % 17 == 0) goto compute_more;
        }
        
        compute_more:
        checksum += total;
    }
    
    /* REGION 4: Instruction-level parallelism stress test */
    {
        /* Many independent integer operations */
        int v1 = int_arr[100] + 1;
        int v2 = int_arr[101] * 2;
        int v3 = int_arr[102] & 0xFF;
        int v4 = int_arr[103] | 0x55;
        int v5 = int_arr[104] ^ v1;
        int v6 = v2 << 3;
        int v7 = v3 >> 1;
        int v8 = v4 * v5;
        int v9 = v6 + v7;
        int v10 = v8 - v9;
        
        /* Memory barriers between independent groups */
        asm volatile("" : : : "memory");
        
        /* More independent ops */
        float f1 = float_arr[100] * 1.5f;
        float f2 = float_arr[101] / 2.0f;
        float f3 = f1 + f2;
        float f4 = f1 * f2 - f3;
        
        /* Volatile accesses mixed in */
        volatile_ptr[10] = v10;
        volatile_ptr[11] = (int)f4;
        
        /* Function calls with side effects */
        v1 = helper1(v1, v2);
        v3 = helper2(int_arr, v3 % 1024);
        f1 = helper3(f1, f2);
        
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    }
    
    /* REGION 5: SIMD intensive with data-dependent control flow */
    {
        __m128i accum = _mm_setzero_si128();
        for (int i = 0; i < 64; i += 4) {
            __m128i data = _mm_loadu_si128((__m128i*)&int_arr[i]);
            __m128i mask = _mm_cmpgt_epi32(data, _mm_set1_epi32(100));
            
            /* Data-dependent operation */
            __m128i scaled = _mm_mullo_epi16(data, _mm_set1_epi32(3));
            __m128i masked = _mm_and_si128(scaled, mask);
            accum = _mm_add_epi32(accum, masked);
            
            /* Conditional inline asm based on mask */
            int mask_bits = _mm_movemask_ps(_mm_castsi128_ps(mask));
            if (mask_bits & 1) {
                asm volatile ("cpuid" : : "a"(0) : "ebx", "ecx", "edx", "memory");
            }
            
            /* Mix with float operations */
            if (i % 8 == 0) {
                float f = float_arr[i % 512];
                f = helper3(f, f * 0.5f);
                volatile_ptr[i % 256] += (int)f;
            }
        }
        
        /* Extract SIMD result */
        int simd_sum[4];
        _mm_storeu_si128((__m128i*)simd_sum, accum);
        checksum += simd_sum[0] + simd_sum[1] + simd_sum[2] + simd_sum[3];
    }
    
    free(int_arr);
    free(float_arr);
    free((void*)volatile_ptr);
    
    return checksum & 0xFF;  /* Prevent overflow in return */
}
