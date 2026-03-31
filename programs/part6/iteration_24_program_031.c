/* test_sched_context.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - creates scheduling barriers */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Helper with memory dependencies */
static int helper_mem(int *arr, int idx) {
    int t = arr[idx];
    arr[idx] = t * 2 + 1;
    return arr[idx] ^ t;
}

/* Function with dense independent arithmetic operations */
static int dense_arithmetic(volatile int seed) {
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed | 0xFF00;
    
    /* Create many independent operations to fill instruction queue */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t2 - t1;
    int t5 = t3 & t4;
    int t6 = t4 | t3;
    int t7 = t5 * t6;
    int t8 = t6 + t5;
    int t9 = t7 ^ t8;
    int t10 = t8 - t7;
    int t11 = t9 & t10;
    int t12 = t10 | t9;
    int t13 = t11 * t12;
    int t14 = t12 + t11;
    int t15 = t13 ^ t14;
    int t16 = t14 - t13;
    int t17 = t15 & t16;
    int t18 = t16 | t15;
    int t19 = t17 * t18;
    int t20 = t18 + t17;
    
    /* Mix with memory operations */
    volatile int mem[4];
    mem[0] = t1; mem[1] = t2; mem[2] = t3; mem[3] = t4;
    int m1 = mem[0] + mem[1];
    int m2 = mem[2] * mem[3];
    
    return t20 + m1 + m2;
}

/* Function with SSE/MMX intrinsics to trigger target-specific scheduling */
static __m128i vector_operations(volatile int seed1, volatile int seed2) {
    /* Create vector data */
    int arr1[4] = {seed1, seed1 + 1, seed1 + 2, seed1 + 3};
    int arr2[4] = {seed2, seed2 * 2, seed2 * 3, seed2 * 4};
    
    __m128i v1 = _mm_loadu_si128((__m128i*)arr1);
    __m128i v2 = _mm_loadu_si128((__m128i*)arr2);
    
    /* Multiple vector operations to create scheduling complexity */
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_mullo_epi16(v1, v2);  /* Note: requires SSE4.1 */
    __m128i v5 = _mm_xor_si128(v3, v4);
    __m128i v6 = _mm_slli_epi32(v5, 3);
    __m128i v7 = _mm_srli_epi32(v6, 1);
    
    /* Mix with scalar operations */
    int s1 = seed1 ^ seed2;
    int s2 = seed1 & seed2;
    
    /* Inline assembly with memory clobber to create scheduling barriers */
    asm volatile ("" : : "r"(s1), "r"(s2) : "memory");
    
    __m128i v8 = _mm_add_epi32(v7, _mm_set1_epi32(s1));
    
    return v8;
}

/* Function with complex control flow and inline assembly barriers */
static int control_flow_mix(volatile int seed, volatile int iter) {
    int result = 0;
    
    /* Loop with volatile iteration count to prevent optimization */
    for (int i = 0; i < iter; ++i) {
        int branch_cond = seed ^ i;
        
        /* Inline assembly barrier - scheduler may try to move instructions across this */
        asm volatile ("# Assembly Barrier" : : : "memory");
        
        if (branch_cond & 1) {
            /* Branch 1: Arithmetic heavy */
            int a = seed + i;
            int b = seed * i;
            
            /* Inline assembly with dependencies */
            int c, d;
            asm volatile ("add %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));
            asm volatile ("and %0, %1, %2" : "=r"(d) : "r"(c), "r"(seed));
            
            result += helper_pure(c, d);
            
            /* Another barrier */
            asm volatile ("# Mid-block Barrier" : : : "memory");
            
            /* More operations */
            result ^= (a * b) + (c ^ d);
        } else {
            /* Branch 2: Memory heavy */
            volatile int mem[8];
            for (int j = 0; j < 8; ++j) {
                mem[j] = seed + i + j;
            }
            
            int sum = 0;
            for (int j = 0; j < 8; ++j) {
                sum += mem[j] * (j + 1);
            }
            
            result += sum;
            
            /* Call helper with memory operations */
            int tmp_arr[4] = {sum, sum + 1, sum + 2, sum + 3};
            result ^= helper_mem(tmp_arr, i & 3);
        }
        
        /* Additional computation with data dependencies */
        int t = result;
        asm volatile ("ror %0, %0, #3" : "+r"(t));  /* Rotate right */
        result = t ^ (seed * i);
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int iter_count = argc > 4 ? atoi(argv[4]) : 100;
    
    int total_result = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 10; ++outer) {
        volatile int loop_seed = seed1 + outer;
        
        /* Pattern 1: Dense arithmetic operations */
        total_result += dense_arithmetic(loop_seed);
        
        /* Pattern 2: Vector operations (triggers target-specific scheduling) */
        __m128i vec_result = vector_operations(loop_seed, seed2);
        int vec_arr[4];
        _mm_storeu_si128((__m128i*)vec_arr, vec_result);
        total_result += vec_arr[0] + vec_arr[1] + vec_arr[2] + vec_arr[3];
        
        /* Pattern 3: Complex control flow with barriers */
        total_result += control_flow_mix(loop_seed, iter_count);
        
        /* Pattern 4: Mixed operations in conditional blocks */
        volatile int cond = seed3 ^ outer;
        if (cond & 0x100) {
            /* More arithmetic density */
            int a = loop_seed;
            int b = seed2;
            for (int i = 0; i < 5; ++i) {
                int t1, t2, t3;
                asm volatile ("mul %0, %1, %2" : "=r"(t1) : "r"(a), "r"(b));
                asm volatile ("add %0, %1, %2" : "=r"(t2) : "r"(t1), "r"(i));
                asm volatile ("eor %0, %1, %2" : "=r"(t3) : "r"(t2), "r"(cond));
                total_result += t3;
                a = t3;
                b = t2;
            }
        }
        
        /* Memory operations to create load/store dependencies */
        volatile int mem_buffer[16];
        for (int i = 0; i < 16; ++i) {
            mem_buffer[i] = total_result + i;
        }
        
        int mem_sum = 0;
        for (int i = 0; i < 16; ++i) {
            mem_sum += mem_buffer[i] * (i + 1);
        }
        
        total_result ^= mem_sum;
        
        /* Function call with volatile args creates scheduling context */
        total_result += helper_pure(loop_seed, mem_sum & 0xFF);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return total_result & 0xFF;
}
