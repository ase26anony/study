/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling barriers */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + 37;
    asm volatile ("" : "+r" (result) : : "memory", "eax");
    return result;
}

__attribute__((noinline)) int helper2(int a, int b) {
    volatile int result = a ^ b;
    asm volatile ("nop; nop" : : : "memory", "ebx", "ecx");
    return result;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float result = a * b;
    asm volatile ("" : "+x" (result) : : "memory", "xmm0", "xmm1");
    return result;
}

/* Complex SIMD operations to trigger target-specific scheduling */
__attribute__((noinline)) __m128i simd_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_slli_epi32(t1, 3);
    __m128i t3 = _mm_xor_si128(t2, _mm_set1_epi32(0x55555555));
    asm volatile ("" : "+x" (t3) : : "memory", "xmm2", "xmm3", "xmm4");
    return t3;
}

/* Function with mixed operations to create complex scheduling regions */
int complex_scheduling_region(int *arr, float *farr, volatile int *varr, 
                              __m128i *simd_arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    __m128i simd_sum = _mm_setzero_si128();
    
    /* Multiple independent instruction chains */
    int chain1 = arr[0];
    int chain2 = arr[1];
    float fchain = farr[0];
    
    /* Create instruction-level parallelism */
    for (int i = 0; i < n; i++) {
        /* Pattern 1: Multiple interdependent integer operations */
        chain1 = chain1 + arr[i];
        chain2 = chain2 * (arr[i] + 1);
        int temp = chain1 ^ chain2;
        temp = temp >> (i & 3);
        temp = temp * 7 - 13;
        
        /* Pattern 2: Volatile memory accesses creating barriers */
        *varr = *varr + 1;
        volatile int vtemp = *varr;
        chain1 = chain1 + vtemp;
        
        /* Pattern 3: Inline assembly with explicit clobbers */
        asm volatile (
            "addl %%eax, %%ecx\n\t"
            "imull %%edx, %%ecx\n\t"
            : "=c" (temp)
            : "a" (chain1), "c" (temp), "d" (chain2)
            : "memory", "cc"
        );
        
        /* Pattern 4: Function calls within tight loops */
        if (i % 3 == 0) {
            temp = helper1(temp, i);
        } else if (i % 3 == 1) {
            temp = helper2(temp, chain1);
        }
        
        /* Pattern 5: Mixed floating point operations */
        fchain = fchain * farr[i % 8] + 1.5f;
        if (i % 5 == 0) {
            fchain = helper3(fchain, farr[(i + 1) % 8]);
        }
        
        /* Pattern 6: SIMD operations for target-specific scheduling */
        if (i % 4 == 0 && i + 1 < n) {
            __m128i v1 = simd_arr[i % 4];
            __m128i v2 = simd_arr[(i + 1) % 4];
            __m128i v3 = simd_op(v1, v2);
            simd_sum = _mm_add_epi32(simd_sum, v3);
            
            /* Extract results to scalar */
            int simd_vals[4];
            _mm_storeu_si128((__m128i*)simd_vals, v3);
            temp += simd_vals[0] + simd_vals[1];
        }
        
        /* Pattern 7: Builtins with specialized scheduling */
        if (temp > 0) {
            int bits = __builtin_popcount(temp);
            int lz = __builtin_clz(temp);
            temp = (temp << (bits & 3)) | (temp >> (lz & 3));
        }
        
        sum += temp;
        fsum += fchain;
        
        /* Memory barrier to split scheduling regions */
        if (i % 8 == 7) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final reduction */
    int simd_final[4];
    _mm_storeu_si128((__m128i*)simd_final, simd_sum);
    sum += simd_final[0] + simd_final[1] + simd_final[2] + simd_final[3];
    
    return sum + (int)fsum;
}

/* Second complex region with different patterns */
int another_scheduling_region(int *arr, int n) {
    int result = 0;
    
    /* Switch statement with multiple cases creating different flows */
    for (int i = 0; i < n; i++) {
        switch (i % 6) {
            case 0: {
                int a = arr[i] + 1;
                int b = a * 2;
                int c = b >> 1;
                result += c ^ arr[i];
                asm volatile ("nop" : : : "memory", "eax");
                break;
            }
            case 1: {
                volatile int v = arr[i];
                int t = v * v;
                t = helper1(t, i);
                result += t % 256;
                break;
            }
            case 2: {
                int t = arr[i];
                for (int j = 0; j < 3; j++) {
                    t = t * 3 + j;
                    if (t > 1000) break;
                }
                result += t;
                break;
            }
            case 3: {
                int t = arr[i];
                /* Nested loop with data-dependent exit */
                int k = 0;
                while (k < 5 && t < 10000) {
                    t = t * 2 + k;
                    k++;
                }
                result += t;
                break;
            }
            case 4: {
                /* Mixed pointer arithmetic */
                int *p = &arr[i];
                int t = *p + *(p + (i % 2));
                t = t | (t << 16);
                result += t;
                break;
            }
            case 5: {
                /* Complex chain with memory barriers */
                int t = arr[i];
                asm volatile ("" : "+r" (t) : : "memory");
                t = t * 7;
                asm volatile ("" : "+r" (t) : : "memory");
                t = t - 13;
                asm volatile ("" : "+r" (t) : : "memory");
                result += t;
                break;
            }
        }
        
        /* Profile-guided optimization hint */
        if (__builtin_expect(i % 16 == 0, 0)) {
            result += helper2(result, i);
        }
    }
    
    return result;
}

/* Function with goto and computed jumps */
int goto_scheduling_region(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    loop_start:
    if (i >= n) goto loop_end;
    
    int val = arr[i];
    
    /* Computed goto-like structure */
    if (val % 4 == 0) goto block0;
    if (val % 4 == 1) goto block1;
    if (val % 4 == 2) goto block2;
    goto block3;
    
    block0: {
        val = val * 3 + 1;
        sum += val;
        i++;
        goto loop_start;
    }
    
    block1: {
        volatile int v = val;
        val = v ^ 0xAAAA;
        sum += val;
        i++;
        goto loop_start;
    }
    
    block2: {
        val = helper1(val, i);
        sum += val;
        i++;
        goto loop_start;
    }
    
    block3: {
        val = val >> 2;
        sum += val;
        i++;
        goto loop_start;
    }
    
    loop_end:
    return sum;
}

int main() {
    const int N = 256;
    
    /* Initialize arrays with different data types */
    int *int_arr = (int*)malloc(N * sizeof(int));
    float *float_arr = (float*)malloc(8 * sizeof(float));
    volatile int *volatile_arr = (volatile int*)malloc(sizeof(int));
    __m128i *simd_arr = (__m128i*)malloc(4 * sizeof(__m128i));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i * 3 + 7;
    }
    
    for (int i = 0; i < 8; i++) {
        float_arr[i] = i * 1.5f + 2.3f;
    }
    
    *volatile_arr = 42;
    
    for (int i = 0; i < 4; i++) {
        simd_arr[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
    }
    
    /* Execute multiple complex scheduling regions */
    int result = 0;
    
    /* Region 1: Main complex scheduling */
    #pragma GCC optimize("O3")
    result += complex_scheduling_region(int_arr, float_arr, volatile_arr, 
                                       simd_arr, N);
    
    /* Region 2: Switch-based scheduling */
    #pragma GCC optimize("O2")
    result += another_scheduling_region(int_arr, N);
    
    /* Region 3: Goto-based scheduling */
    #pragma GCC optimize("O3")
    result += goto_scheduling_region(int_arr, N);
    
    /* Region 4: Additional mixed operations */
    #pragma GCC optimize("Os")
    {
        int temp = 0;
        for (int i = 0; i < N; i++) {
            /* Many independent instructions */
            int a = int_arr[i] + 1;
            int b = int_arr[(i + 1) % N] * 2;
            int c = int_arr[(i + 2) % N] & 0xFF;
            int d = a ^ b;
            int e = c | d;
            float f = float_arr[i % 8];
            f = f * 2.0f - 1.0f;
            
            /* Memory barrier every 16 iterations */
            if (i % 16 == 0) {
                asm volatile ("" : : : "memory");
            }
            
            temp += e + (int)f;
        }
        result += temp;
    }
    
    /* Region 5: SIMD-intensive region */
    #pragma GCC optimize("O3")
    {
        __m128i sum = _mm_setzero_si128();
        for (int i = 0; i < N / 4; i++) {
            __m128i v1 = _mm_loadu_si128((__m128i*)&int_arr[i*4]);
            __m128i v2 = _mm_slli_epi32(v1, 2);
            __m128i v3 = _mm_add_epi32(v1, v2);
            __m128i v4 = _mm_xor_si128(v3, _mm_set1_epi32(0x33333333));
            
            /* Custom assembly with register constraints */
            asm volatile (
                "paddd %1, %0\n\t"
                "pslld $1, %0\n\t"
                : "+x" (v4)
                : "xm" (sum)
                : "memory", "xmm5", "xmm6"
            );
            
            sum = v4;
        }
        
        int simd_result[4];
        _mm_storeu_si128((__m128i*)simd_result, sum);
        result += simd_result[0] + simd_result[1] + 
                 simd_result[2] + simd_result[3];
    }
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free((void*)volatile_arr);
    free(simd_arr);
    
    return result != 0 ? 0 : 1;
}
