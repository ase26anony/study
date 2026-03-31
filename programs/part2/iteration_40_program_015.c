/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline, target("no-sse"))) 
int helper1(int a, int b) {
    volatile int barrier = a + b;
    asm volatile ("" : "+r" (barrier) : : "memory");
    return barrier * 2;
}

__attribute__((noinline, target("default")))
float helper2(float a, float b) {
    volatile float v = a;
    asm volatile ("# FPU barrier" : "+t" (v) : : "memory", "st(1)", "st(2)");
    return v * b;
}

__attribute__((noinline))
void* helper3(void* ptr, int offset) {
    volatile char* p = (char*)ptr;
    asm volatile ("# Memory barrier" : : "r" (p) : "memory");
    return p + offset;
}

/* Complex SIMD operations */
__attribute__((target("sse4.2")))
__m128i simd_chain(__m128i a, __m128i b, __m128i c) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi32(t1, c);
    __m128i t3 = _mm_slli_epi32(t2, 3);
    __m128i t4 = _mm_xor_si128(t3, _mm_set1_epi32(0xFFFFFFFF));
    return _mm_srai_epi32(t4, 1);
}

/* Function with mixed operations causing complex scheduling */
uint64_t complex_scheduling_region(int mode, volatile int* mem, float* farr, 
                                   __m128i* vec, uint64_t seed) {
    uint64_t checksum = seed;
    int i, j, k;
    
    /* Multiple volatile accesses create scheduling barriers */
    volatile int* vptr = mem;
    *vptr = *vptr + 1;
    vptr++;
    *vptr = *vptr - 2;
    
    /* Switch with multiple cases, each creating different scheduling patterns */
    switch (mode & 7) {
        case 0: {
            /* Long dependency chain with integer ops */
            int a = (int)(checksum & 0xFFFF);
            int b = a * 3;
            int c = b >> 4;
            int d = c + a;
            int e = d * d;
            int f = __builtin_popcount(e);
            int g = f | 0xFF;
            int h = g & ~0x0F;
            int i = h << 2;
            int j = i - b;
            int k = j / 3;
            int l = k ^ 0xAA;
            int m = l % 17;
            int n = m + c;
            int o = n * 2;
            checksum += o;
            
            /* Inline assembly with explicit clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r" (checksum)
                : "r" (o)
                : "eax", "cc", "memory"
            );
            break;
        }
        
        case 1: {
            /* Floating point intensive */
            float f1 = farr[0];
            float f2 = farr[1];
            float f3 = f1 * f2;
            float f4 = f3 + f1;
            float f5 = f4 / f2;
            float f6 = f5 - f3;
            float f7 = helper2(f6, f1);
            float f8 = f7 * 2.0f;
            float f9 = f8 + f4;
            float f10 = f9 / f5;
            
            /* Force scheduling barrier */
            asm volatile ("# FP barrier" : : : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)");
            
            checksum += (uint64_t)(f10 * 1000.0f);
            break;
        }
            
        case 2: {
            /* SIMD operations */
            __m128i v1 = vec[0];
            __m128i v2 = vec[1];
            __m128i v3 = _mm_set1_epi32(checksum & 0xFFFFFFFF);
            
            for (int i = 0; i < 4; i++) {
                v1 = simd_chain(v1, v2, v3);
                v2 = _mm_slli_epi32(v1, i);
                v3 = _mm_add_epi32(v3, _mm_set1_epi32(1));
                
                /* Memory barrier between SIMD ops */
                asm volatile ("# SIMD barrier" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
            }
            
            int* vdata = (int*)&v1;
            checksum += vdata[0] + vdata[1] + vdata[2] + vdata[3];
            break;
        }
            
        case 3: {
            /* Mixed integer/pointer arithmetic */
            char* ptr = (char*)mem;
            for (int i = 0; i < 8; i++) {
                ptr = helper3(ptr, i * 4);
                *((volatile int*)ptr) = i;
                
                int val = *((volatile int*)ptr);
                checksum = checksum * 31 + val;
                
                /* Varying latency operations */
                if (i & 1) {
                    checksum = __builtin_bswap64(checksum);
                } else {
                    checksum = __builtin_rotateleft64(checksum, 17);
                }
            }
            break;
        }
            
        default: {
            /* Nested loops with data-dependent exits */
            int limit = (mode % 13) + 5;
            for (i = 0; i < limit; i++) {
                int inner_limit = (i % 7) + 3;
                for (j = 0; j < inner_limit; j++) {
                    int val = i * j;
                    val = helper1(val, j);
                    
                    for (k = 0; k < 2; k++) {
                        val = val ^ (k << 4);
                        val = val + (i & j);
                        
                        /* Conditional break with data dependency */
                        if ((val & 0xF) == (k & 0xF)) {
                            val = val >> 1;
                            break;
                        }
                    }
                    
                    checksum += val;
                    
                    /* Memory clobber */
                    asm volatile ("# Loop barrier" : : : "memory");
                }
                
                /* Function call creates scheduling boundary */
                if (i & 1) {
                    checksum = helper1(checksum & 0xFFFF, i);
                }
            }
            break;
        }
    }
    
    return checksum;
}

/* Main function with multiple complex scheduling regions */
int main() {
    uint64_t final_checksum = 0x123456789ABCDEF0ULL;
    
    /* Allocate and initialize various data types */
    volatile int* volatile_mem = (volatile int*)malloc(256 * sizeof(int));
    float* float_arr = (float*)malloc(128 * sizeof(float));
    __m128i* simd_arr = (__m128i*)malloc(64 * sizeof(__m128i));
    
    for (int i = 0; i < 256; i++) {
        volatile_mem[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        float_arr[i] = i * 1.5f;
    }
    for (int i = 0; i < 64; i++) {
        simd_arr[i] = _mm_set_epi32(i*4, i*3, i*2, i);
    }
    
    /* Create multiple scheduling regions with different characteristics */
    #pragma GCC optimize("O3")
    for (int region = 0; region < 5; region++) {
        /* Change optimization pragma to potentially trigger state saving */
        if (region & 1) {
            #pragma GCC optimize("Os")
        } else {
            #pragma GCC optimize("O3")
        }
        
        /* Each iteration creates a complex scheduling region */
        final_checksum = complex_scheduling_region(
            region, 
            volatile_mem + region * 16,
            float_arr + region * 8,
            simd_arr + region * 4,
            final_checksum
        );
        
        /* Additional independent instructions that can execute in parallel */
        int a = final_checksum & 0xFF;
        int b = (final_checksum >> 8) & 0xFF;
        int c = (final_checksum >> 16) & 0xFF;
        int d = (final_checksum >> 24) & 0xFF;
        
        /* Many independent operations */
        int e = a * b;
        int f = c + d;
        int g = a ^ c;
        int h = b | d;
        int i = e << 2;
        int j = f >> 1;
        int k = g * 3;
        int l = h + 7;
        int m = i & j;
        int n = k | l;
        int o = m ^ n;
        
        /* Memory barrier */
        asm volatile ("# Parallel ops barrier" : : : "memory");
        
        final_checksum = (final_checksum << 13) ^ o;
        
        /* Profile-guided optimization hints */
        if (__builtin_expect(region < 3, 1)) {
            /* Likely path */
            final_checksum = __builtin_ia32_crc32di(final_checksum, o);
        } else {
            /* Unlikely path - different scheduling pattern */
            final_checksum = ~final_checksum;
            asm volatile (
                "rorq $23, %0\n\t"
                : "+r" (final_checksum)
                :
                : "cc"
            );
        }
    }
    
    /* Final mixed operations */
    __m128i final_vec = _mm_set1_epi64x(final_checksum);
    final_vec = _mm_add_epi64(final_vec, _mm_set1_epi64x(0x1000));
    final_vec = _mm_mul_epu32(final_vec, _mm_set1_epi64x(0xABCD));
    
    int64_t* vec_data = (int64_t*)&final_vec;
    final_checksum = vec_data[0] + vec_data[1];
    
    /* Cleanup */
    free((void*)volatile_mem);
    free(float_arr);
    free(simd_arr);
    
    /* Return checksum to prevent dead code elimination */
    return (int)(final_checksum & 0x7FFFFFFF);
}
