/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-schedule-insns -fselective-scheduling2 -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Noinline functions to force scheduling boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a + b;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float result = a * b;
    asm volatile ("" : "+x" (result) : : "memory");
    return result;
}

__attribute__((noinline)) void helper3(volatile int* p, int val) {
    *p = (*p & 0xFF) | (val << 8);
    asm volatile ("mfence" : : : "memory");
}

/* Function with complex scheduling requirements */
__attribute__((noinline)) uint64_t complex_scheduling_region(int mode) {
    volatile int v1 = 1, v2 = 2, v3 = 3;
    volatile float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    uint64_t checksum = 0;
    
    /* Mixed data types and operations */
    int a = v1 + v2;
    float b = f1 * f2;
    int c = a * (int)b;
    float d = b + f3;
    
    /* Vector/SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(v1, v2, v3, a);
    __m128i vec2 = _mm_set_epi32(c, (int)d, v1 * 2, v2 * 3);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    /* Extract results with inline assembly using specific registers */
    int vec_results[4];
    _mm_storeu_si128((__m128i*)vec_results, vec3);
    
    /* Complex instruction chain with dependencies */
    for (int i = 0; i < 4; i++) {
        vec_results[i] = helper1(vec_results[i], i);
        if (__builtin_expect(vec_results[i] > 100, 0)) {
            vec_results[i] >>= 2;
        } else {
            vec_results[i] <<= 1;
        }
    }
    
    /* Switch statement creating multiple basic blocks */
    switch (mode & 0x7) {
        case 0: {
            /* Case with many independent instructions */
            int t1 = vec_results[0] + vec_results[1];
            int t2 = vec_results[2] * vec_results[3];
            float t3 = helper2(f1, f2);
            int t4 = t1 ^ t2;
            float t5 = t3 + f3;
            int t6 = (int)t5 * t4;
            checksum += t6;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            /* More operations */
            t1 = t1 >> 1;
            t2 = t2 << 2;
            t4 = t4 | 0xFF;
            checksum += t1 + t2 + t4;
            break;
        }
        case 1: {
            /* Case with volatile accesses and inline assembly */
            volatile int* p = &v1;
            for (int j = 0; j < 3; j++) {
                *p = *p + 1;
                p = &v2;
                asm volatile ("nop; nop; nop" : : : "eax", "memory");
            }
            checksum += *p;
            break;
        }
        case 2: {
            /* Case with nested loops and conditional breaks */
            int sum = 0;
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < i; j++) {
                    sum += i * j;
                    if (sum > 1000) {
                        sum >>= 2;
                        if (j > 5) break;
                    }
                }
                /* Function call creates scheduling barrier */
                helper3(&v3, i);
            }
            checksum += sum;
            break;
        }
        case 3: {
            /* Case with architecture-specific builtins */
            checksum += __builtin_popcount(vec_results[0]);
            checksum += __builtin_ctz(vec_results[1] | 1);
            checksum += __builtin_clz(vec_results[2]);
            break;
        }
        default: {
            /* Default case with mixed operations */
            checksum = (checksum * 1103515245 + 12345) & 0x7FFFFFFF;
            break;
        }
    }
    
    return checksum;
}

/* Second complex region with different patterns */
__attribute__((noinline)) uint64_t another_scheduling_region(int seed) {
    volatile double d1 = 1.234, d2 = 5.678;
    volatile int arr[16];
    uint64_t checksum = seed;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        arr[i] = i * seed;
    }
    
    /* Multiple independent instruction chains */
    int chain1 = arr[0] + arr[1];
    int chain2 = arr[2] * arr[3];
    double chain3 = d1 * d2;
    int chain4 = arr[4] ^ arr[5];
    
    /* Interdependent operations */
    chain1 = helper1(chain1, chain2);
    chain2 = chain2 * (int)chain3;
    chain3 = helper2((float)chain3, (float)chain4);
    chain4 = chain4 | chain1;
    
    /* Create instruction-level parallelism */
    for (int i = 6; i < 12; i += 2) {
        int tmp1 = arr[i] + arr[i+1];
        int tmp2 = arr[i] * arr[i+1];
        checksum += tmp1 * tmp2;
        
        /* Memory barrier every few iterations */
        if (i % 4 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Complex expression with many operations */
    checksum = (checksum * 6364136223846793005ULL + 1442695040888963407ULL);
    checksum ^= (chain1 << 32) | chain2;
    checksum += (uint64_t)(chain3 * 1000.0);
    checksum ^= chain4;
    
    return checksum;
}

/* Main function with multiple scheduling regions */
int main() {
    uint64_t final_checksum = 0;
    
    /* Create multiple complex scheduling regions */
    for (int region = 0; region < 5; region++) {
        /* Each region uses different modes to exercise different paths */
        for (int mode = 0; mode < 8; mode++) {
            /* Force scheduling state saving with complex regions */
            uint64_t result = complex_scheduling_region(mode + region);
            final_checksum ^= result;
            
            /* Add another layer of scheduling complexity */
            result = another_scheduling_region(result & 0xFF);
            final_checksum += result;
            
            /* Insert scheduling barrier between iterations */
            asm volatile ("# Scheduling Barrier" : : : "memory");
        }
        
        /* Additional complex block with goto for control flow */
        int counter = region * 100;
        compute_label:
            counter = helper1(counter, final_checksum & 0xFF);
            if (counter < 1000) {
                final_checksum += counter;
                counter += 150;
                goto compute_label;
            }
    }
    
    /* Final mixing */
    final_checksum = (final_checksum >> 32) ^ (final_checksum << 32);
    final_checksum = __builtin_bswap64(final_checksum);
    
    /* Prevent dead code elimination */
    volatile uint64_t output = final_checksum;
    return (int)(output & 0xFFFFFFFF);
}
