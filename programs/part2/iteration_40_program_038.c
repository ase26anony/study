/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-schedule-insns -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline, target("no-sse"))) 
int helper1(int a, int b) {
    volatile int result = a + b;
    asm volatile ("" : "+r" (result) : : "memory", "eax", "ebx");
    return result;
}

__attribute__((noinline, target("no-avx")))
float helper2(float x, float y) {
    volatile float tmp = x * y;
    asm volatile ("fadd %%st(1), %%st(0)" : "+t" (tmp) : : "st(1)", "memory");
    return tmp;
}

__attribute__((noinline))
void* helper3(void* ptr, int offset) {
    volatile char* p = (char*)ptr;
    asm volatile ("lock addl $1, %0" : "+m" (*p) : : "memory", "cc");
    return p + offset;
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    volatile int barrier = seed;
    int result = 0;
    
    /* Region 1: Integer operations with dependencies */
    int a = barrier + 1;
    int b = a * 3;
    int c = b >> 2;
    int d = c ^ 0x55AA55AA;
    int e = d & 0x00FF00FF;
    int f = e | 0xFF00FF00;
    
    /* Volatile memory access creating scheduling barrier */
    volatile int* volatile_ptr = &barrier;
    *volatile_ptr = *volatile_ptr + f;
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "addl $0x12345678, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (f)
        : "eax", "memory", "cc"
    );
    
    /* SIMD operations for target-specific scheduling */
#ifdef __SSE2__
    __m128i vec1 = _mm_set_epi32(result, f, e, d);
    __m128i vec2 = _mm_set_epi32(c, b, a, seed);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, _mm_set1_epi32(7));
    
    int simd_results[4];
    _mm_storeu_si128((__m128i*)simd_results, vec4);
    result += simd_results[0] + simd_results[3];
#endif
    
    return result;
}

/* Function with nested control flow creating instruction queues */
int nested_control_flow(int iterations) {
    int sum = 0;
    volatile int counter = iterations;
    
    /* Outer loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        switch (i % 5) {
            case 0: {
                /* Case with many independent instructions */
                int t1 = i * 2;
                int t2 = i + 1;
                int t3 = i ^ 0xFF;
                int t4 = i & 0x0F;
                int t5 = i | 0xF0;
                sum += t1 + t2 + t3 + t4 + t5;
                
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            }
            case 1: {
                /* Case with function calls */
                sum += helper1(i, sum);
                sum += helper2(i * 1.5f, sum * 0.5f);
                break;
            }
            case 2: {
                /* Case with computed goto (indirect jump) */
                static void* labels[] = { &&L0, &&L1, &&L2, &&L3 };
                goto *labels[i % 4];
                
                L0: sum += i * 3; goto end_case;
                L1: sum += i * 5; goto end_case;
                L2: sum += i * 7; goto end_case;
                L3: sum += i * 11; goto end_case;
                end_case:
                break;
            }
            case 3: {
                /* Case with inner loop */
                for (int j = 0; j < (i % 10) + 1; j++) {
                    sum += (i * j) ^ sum;
                    if (j % 3 == 0) {
                        asm volatile ("pause" : : : "memory");
                    }
                }
                break;
            }
            case 4: {
                /* Case with mixed operations */
                float fsum = sum;
                for (int k = 0; k < 3; k++) {
                    fsum = fsum * 1.1f + k * 0.5f;
                    sum += (int)fsum;
                }
                break;
            }
        }
        
        /* Conditional break based on data-dependent condition */
        if (sum > 1000000) {
            asm volatile ("" : : : "memory");
            break;
        }
        
        /* Update volatile counter */
        counter--;
    }
    
    return sum;
}

/* Main function with multiple scheduling regions */
int main() {
    int final_result = 0;
    volatile int seed = 42;
    
    /* Initialize arrays with different data types */
    int int_array[256];
    float float_array[256];
    volatile int volatile_array[256];
    
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 1.5f;
        volatile_array[i] = i;
    }
    
    /* Region 1: Complex scheduling with SIMD */
    final_result += complex_scheduling_region(seed);
    
    /* Region 2: Nested control flow */
    final_result += nested_control_flow(50);
    
    /* Region 3: Pointer arithmetic and memory operations */
    char* ptr = (char*)int_array;
    for (int i = 0; i < 100; i++) {
        ptr = helper3(ptr, (i % 16) + 1);
        final_result += *ptr;
        
        /* Architecture-specific builtins */
        final_result += __builtin_popcount(final_result);
        final_result += __builtin_ctz(final_result | 1);
    }
    
    /* Region 4: Mixed vector and scalar operations */
#ifdef __SSE2__
    for (int i = 0; i < 64; i += 4) {
        __m128i v1 = _mm_loadu_si128((__m128i*)&int_array[i]);
        __m128i v2 = _mm_loadu_si128((__m128i*)&int_array[i + 64]);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, _mm_set1_epi32(final_result & 0xFF));
        
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, v4);
        final_result += temp[0] + temp[1] + temp[2] + temp[3];
    }
#endif
    
    /* Region 5: More complex control flow with profile hints */
    for (int i = 0; i < 1000; i++) {
        if (__builtin_expect(i < 900, 1)) {
            /* Hot path: many independent operations */
            int t1 = int_array[i % 256] * 2;
            int t2 = float_array[i % 256] * 3;
            int t3 = volatile_array[i % 256] ^ 0xAA;
            final_result += t1 + t2 + t3;
        } else {
            /* Cold path: function calls and barriers */
            final_result = helper1(final_result, i);
            asm volatile ("mfence" : : : "memory");
        }
        
        /* Create scheduling pressure with memory barriers */
        if (i % 100 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final computation with all patterns mixed */
    int checksum = final_result;
    checksum = complex_scheduling_region(checksum);
    checksum = nested_control_flow(20);
    
    /* Use checksum to prevent dead code elimination */
    volatile int output = checksum;
    return output & 0x7FFFFFFF;  /* Ensure positive return */
}
