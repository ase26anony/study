/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

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

__attribute__((noinline, target("sse4.2")))
int helper2(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ 0x55AA55AA;
    }
    asm volatile ("cpuid" : : "a"(0) : "rbx", "rcx", "rdx", "memory");
    return sum;
}

__attribute__((noinline))
void helper3(volatile int *p, __m128i *vec) {
    *p = _mm_extract_epi32(*vec, 0);
    asm volatile ("mfence" ::: "memory");
}

/* Complex function with multiple scheduling regions */
__attribute__((optimize("O3")))
int compute_checksum(int seed) {
    volatile int v1 = seed;
    int arr1[256], arr2[256];
    float farr[128];
    __m128i vec_data[64];
    int *volatile volatile_ptr = &arr1[0];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        arr1[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        arr2[i] = arr1[i] ^ 0xAAAAAAAA;
    }
    for (int i = 0; i < 128; i++) {
        farr[i] = (float)arr1[i] * 0.5f;
    }
    for (int i = 0; i < 64; i++) {
        vec_data[i] = _mm_set_epi32(arr1[i*4+3], arr1[i*4+2], 
                                   arr1[i*4+1], arr1[i*4]);
    }
    
    int result = 0;
    int switch_var = seed % 8;
    
    /* Region 1: Complex switch with instruction chains */
    switch (switch_var) {
        case 0: {
            /* Long dependency chain */
            int a = arr1[0] + arr2[0];
            int b = a * arr1[1];
            int c = b >> (arr2[1] & 0xF);
            int d = c ^ arr1[2];
            int e = d * 3 + arr2[2];
            int f = e & 0xFFFF;
            int g = f | arr1[3];
            int h = g - arr2[3];
            int i = h * 7;
            int j = i / (arr1[4] | 1);
            int k = j << 2;
            int l = k ^ 0xDEADBEEF;
            int m = l + arr2[4];
            int n = m & 0x7FFFFFFF;
            int o = n * 1103515245;
            result += o;
            
            /* Vector operations mixed in */
            __m128i v1 = _mm_loadu_si128(&vec_data[0]);
            __m128i v2 = _mm_loadu_si128(&vec_data[1]);
            __m128i vsum = _mm_add_epi32(v1, v2);
            _mm_storeu_si128(&vec_data[2], vsum);
            
            /* Inline assembly with clobbers */
            asm volatile (
                "mov %1, %%eax\n\t"
                "imul %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r"(result)
                : "r"(result), "r"(arr1[5])
                : "eax", "memory"
            );
            break;
        }
        
        case 1:
        case 2: {
            /* Nested loops with data-dependent exits */
            for (int outer = 0; outer < 16; outer++) {
                int inner = 0;
                int limit = arr1[outer] & 0xFF;
                while (inner < limit) {
                    /* Mixed integer/float operations */
                    int idx = (inner * 17) & 0xFF;
                    arr2[idx] = arr1[idx] + inner;
                    farr[idx >> 1] = (float)arr2[idx] * 0.25f;
                    
                    /* Memory barrier */
                    asm volatile ("" ::: "memory");
                    
                    /* Conditional break with arithmetic */
                    if ((arr2[idx] & 0x7) == 0) {
                        arr2[idx] = helper1(arr2[idx], idx);
                        if (arr2[idx] > 1000) break;
                    }
                    inner++;
                }
                
                /* SIMD operation every 4 iterations */
                if ((outer & 3) == 0) {
                    __m128i v = _mm_set1_epi32(outer);
                    vec_data[outer >> 2] = _mm_add_epi32(vec_data[outer >> 2], v);
                }
            }
            result += helper2(arr2, 64);
            break;
        }
        
        default: {
            /* Mixed operations in tight loop */
            for (int i = 0; i < 128; i += 4) {
                /* Pointer arithmetic with volatile */
                volatile int *vp = volatile_ptr + i;
                *vp = *vp + arr1[i];
                
                /* Multiple independent operations */
                int t1 = arr1[i] * arr2[i];
                int t2 = arr1[i+1] | arr2[i+1];
                int t3 = arr1[i+2] ^ arr2[i+2];
                int t4 = arr1[i+3] - arr2[i+3];
                
                /* Chain them together */
                t1 = t1 + t2;
                t3 = t3 * t4;
                result += t1 ^ t3;
                
                /* Architecture-specific builtins */
                result += __builtin_popcount(t1);
                result += __builtin_ctz(t3 | 1);
                
                /* Function call with side effects */
                helper3(vp, &vec_data[i >> 2]);
            }
            break;
        }
    }
    
    /* Region 2: Another complex block with gotos */
    {
        int counter = 0;
        int sum = 0;
        
    restart_loop:
        for (int i = 0; i < 32; i++) {
            /* Data-dependent computation */
            int val = arr1[i] + (counter * i);
            
            /* Multiple if-else chains */
            if (val & 1) {
                val = val * 3 + 1;
                if (val > 1000000) {
                    val >>= 1;
                } else if (val < 100) {
                    val = val << 4;
                }
            } else {
                val = val / 2;
                if ((val & 0xF) == 0) {
                    val = val ^ 0xABCD;
                }
            }
            
            /* Mixed with floating point */
            float fval = (float)val;
            fval = fval * 0.333f;
            arr2[i] = (int)fval;
            
            sum += arr2[i];
            
            /* Create scheduling barrier periodically */
            if ((i & 7) == 0) {
                asm volatile ("lfence" ::: "memory");
            }
        }
        
        counter++;
        if (counter < 3) {
            /* Computed goto-like behavior */
            switch (sum & 3) {
                case 0: goto restart_loop;
                case 1: arr1[0] = sum; goto restart_loop;
                default: break;
            }
        }
        
        result += sum;
    }
    
    /* Region 3: Vector-intensive computations */
    {
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 64; i += 4) {
            __m128i v1 = _mm_loadu_si128(&vec_data[i]);
            __m128i v2 = _mm_loadu_si128(&vec_data[i+1]);
            __m128i v3 = _mm_loadu_si128(&vec_data[i+2]);
            __m128i v4 = _mm_loadu_si128(&vec_data[i+3]);
            
            /* Multiple vector operations */
            __m128i t1 = _mm_add_epi32(v1, v2);
            __m128i t2 = _mm_sub_epi32(v3, v4);
            __m128i t3 = _mm_xor_si128(t1, t2);
            __m128i t4 = _mm_slli_epi32(t3, 2);
            
            vsum = _mm_add_epi32(vsum, t4);
            
            /* Store with different alignments */
            if (i & 1) {
                _mm_storeu_si128(&vec_data[i], t4);
            } else {
                _mm_stream_si128(&vec_data[i], t4);
            }
        }
        
        /* Extract results from vector */
        int vresult[4];
        _mm_storeu_si128((__m128i*)vresult, vsum);
        result += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    /* Final mixing */
    result = result ^ (result >> 16);
    result = result * 0x85EBCA6B;
    result = result ^ (result >> 13);
    result = result * 0xC2B2AE35;
    result = result ^ (result >> 16);
    
    return result & 0x7FFFFFFF;
}

/* Main function with multiple optimization regions */
#pragma GCC optimize("O2")
int main() {
    int final_result = 0;
    
    /* Call compute_checksum multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        /* Change optimization pragma periodically */
        if (i % 25 == 0) {
            #pragma GCC optimize("O3", "fschedule-insns2")
        } else if (i % 25 == 12) {
            #pragma GCC optimize("O2", "fno-schedule-insns")
        }
        
        int seed = i * 1103515245 + 12345;
        final_result ^= compute_checksum(seed);
        
        /* Profile-guided optimization hint */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            /* Unlikely path with extra computation */
            volatile int unlikely_sum = 0;
            for (int j = 0; j < 16; j++) {
                unlikely_sum += (seed >> j) & 1;
            }
            final_result += unlikely_sum;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int output __attribute__((unused)) = final_result;
    
    return final_result & 0xFF;
}
