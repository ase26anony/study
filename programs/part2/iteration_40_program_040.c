/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline, target("no-sse"))) 
int helper1(int a, int b) {
    volatile int barrier = a + b;
    asm volatile ("" : : "r"(barrier) : "memory");
    return barrier * 2;
}

__attribute__((noinline, target("sse2")))
__m128i helper2(__m128i a, __m128i b) {
    __m128i result = _mm_add_epi32(a, b);
    asm volatile ("# SSE barrier" : : : "memory", "xmm0", "xmm1", "xmm2");
    return _mm_xor_si128(result, _mm_set1_epi32(0x55555555));
}

__attribute__((noinline))
float helper3(float* arr, int idx) {
    volatile float* vptr = &arr[idx];
    float val = *vptr;
    asm volatile ("mfence" : : : "memory");
    return val * 3.14159f;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int* volatile_ptr = (volatile int*)&seed;
    int result = 0;
    int i, j, k;
    
    /* Region 1: Mixed operations with volatile and inline asm */
    {
        int a = seed;
        int b = *volatile_ptr + 1;
        int c = a * b;
        
        /* Force scheduling barrier */
        asm volatile ("# Region 1 start" : : : "memory", "eax", "ebx", "ecx");
        
        /* Chain of dependent operations */
        for (i = 0; i < 8; i++) {
            a = (a << 1) | (a >> 31);  /* Rotate */
            b = helper1(b, c);
            c = a ^ b ^ c;
            *volatile_ptr = c;  /* Volatile write */
        }
        
        /* SIMD operations if available */
        __m128i vec1 = _mm_set_epi32(a, b, c, seed);
        __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec3 = helper2(vec1, vec2);
        
        int vec_data[4];
        _mm_storeu_si128((__m128i*)vec_data, vec3);
        result += vec_data[0] + vec_data[1] + vec_data[2] + vec_data[3];
    }
    
    /* Region 2: Nested loops with data-dependent exits */
    {
        int arr[16];
        for (i = 0; i < 16; i++) {
            arr[i] = i * seed;
        }
        
        int sum = 0;
        int limit = (*volatile_ptr & 0xF) + 5;
        
        for (i = 0; i < limit; i++) {
            for (j = i; j < 16; j++) {
                int temp = arr[j];
                /* Complex arithmetic chain */
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                temp = __builtin_popcount(temp);
                temp = temp * temp - temp;
                
                /* Inline asm with register constraints */
                asm volatile ("imull %%ecx, %%edx\n\t"
                             "addl %%edx, %%eax"
                             : "+a"(sum), "+d"(temp)
                             : "c"(j)
                             : "cc");
                
                if (temp > 1000) break;
            }
            
            /* Memory barrier */
            asm volatile ("# Loop barrier %0" : : "r"(i) : "memory");
        }
        result ^= sum;
    }
    
    /* Region 3: Switch statement with multiple cases */
    {
        int selector = result & 0x7;
        int switch_result = 0;
        
        switch (selector) {
            case 0: {
                /* Vector operations case */
                __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
                __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
                for (k = 0; k < 4; k++) {
                    v1 = _mm_add_epi32(v1, v2);
                    v2 = _mm_slli_epi32(v2, 1);
                }
                int vdata[4];
                _mm_storeu_si128((__m128i*)vdata, v1);
                switch_result = vdata[0] + vdata[3];
                break;
            }
            case 1: {
                /* Integer arithmetic chain */
                int x = seed;
                for (i = 0; i < 12; i++) {
                    x = (x * 13 + 7) & 0xFF;
                    x = __builtin_ctz(x | 1);  /* Count trailing zeros */
                    x = x * x + i;
                }
                switch_result = x;
                break;
            }
            case 2: {
                /* Mixed float/int operations */
                float farr[8];
                for (i = 0; i < 8; i++) {
                    farr[i] = (float)(i * seed) / 3.0f;
                }
                float fsum = 0.0f;
                for (i = 0; i < 8; i += 2) {
                    fsum += helper3(farr, i);
                }
                switch_result = (int)fsum;
                break;
            }
            case 3: {
                /* Pointer arithmetic */
                int* ptr = (int*)&seed;
                for (i = 0; i < 4; i++) {
                    ptr = (int*)((uintptr_t)ptr + sizeof(int));
                    switch_result += *(volatile int*)ptr;
                }
                break;
            }
            default: {
                /* Complex default case */
                int a = seed, b = result, c = selector;
                for (j = 0; j < 6; j++) {
                    asm volatile ("# Default case ops %0, %1, %2"
                                 : "+r"(a), "+r"(b), "+r"(c)
                                 : 
                                 : "memory", "cc");
                    a = a * b + c;
                    b = b ^ a ^ c;
                    c = c + (a >> 3);
                }
                switch_result = a + b + c;
                break;
            }
        }
        
        result += switch_result;
    }
    
    /* Region 4: Parallel independent operations */
    {
        int ops[8];
        volatile int sync = 0;
        
        /* Independent computations */
        ops[0] = result * 3;
        ops[1] = __builtin_popcount(result);
        ops[2] = result ^ 0xAAAAAAAA;
        ops[3] = result >> 4;
        ops[4] = helper1(ops[0], ops[1]);
        ops[5] = ops[2] | ops[3];
        
        /* Memory synchronization */
        sync = 1;
        asm volatile ("lock; addl $0, 0(%%esp)" : : : "memory", "cc");
        
        ops[6] = ops[4] * ops[5];
        ops[7] = __builtin_ctz(ops[6] | 1);
        
        /* Combine results */
        for (i = 0; i < 8; i++) {
            result = (result * 31 + ops[i]) & 0x7FFFFFFF;
        }
    }
    
    return result;
}

/* Main function with multiple complex regions */
int main() {
    int final_result = 0;
    int iteration;
    
    /* Execute multiple times to increase scheduling opportunities */
    for (iteration = 0; iteration < 100; iteration++) {
        int seed = iteration * 1234567;
        
        /* Profile-guided optimization hint */
        if (__builtin_expect((iteration & 1) == 0, 1)) {
            /* Likely path with complex scheduling */
            final_result ^= complex_scheduling_region(seed);
        } else {
            /* Unlikely but still complex path */
            volatile int temp = seed;
            for (int i = 0; i < 20; i++) {
                temp = temp * 1103515245 + 12345;
                asm volatile ("# Alternate path" : : "r"(temp) : "memory");
            }
            final_result += temp;
        }
        
        /* Periodic scheduling barrier */
        if (iteration % 10 == 0) {
            asm volatile ("mfence" : : : "memory");
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    return output & 0xFF;
}
