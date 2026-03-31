/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int* volatile volatile_ptr;
volatile int mem_barrier = 0;

/* Noinline functions to force calls */
__attribute__((noinline)) int helper1(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + (a ^ b);
}

__attribute__((noinline)) int helper2(int* p, int val) {
    *p = (*p & val) | (val << 3);
    asm volatile ("" : : : "memory");
    return *p;
}

__attribute__((noinline)) float helper3(float a, float b, float c) {
    float t = a * b;
    asm volatile ("nop; nop; nop" : : : "memory");
    return t + c - (a / b);
}

/* Always inline with complex operations */
__attribute__((always_inline)) static inline int complex_chain(int x) {
    int a = x * 2;
    int b = a ^ 0x55AA55AA;
    int c = b + (x >> 3);
    int d = c * 7;
    asm volatile ("" : "+r"(d) : : "cc", "memory");
    return d;
}

/* Vector operations */
__attribute__((always_inline)) static inline __m128i vector_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_mullo_epi16(a, b);
    __m128i t3 = _mm_slli_epi32(t1, 3);
    asm volatile ("" : : : "memory");
    return _mm_xor_si128(t2, t3);
}

/* Function with scheduling barriers */
__attribute__((noinline)) int scheduling_region_1(int seed) {
    int result = seed;
    volatile int* p = &mem_barrier;
    
    /* Mixed operations creating dependencies */
    result = helper1(result, 17);
    *p = *p + 1;  /* Volatile access */
    
    int a = result * 3;
    int b = a ^ 0x12345678;
    int c = b + (result >> 2);
    
    asm volatile ("mfence" : : : "memory");  /* Strong barrier */
    
    c = helper2(&c, a);
    result = c + (a & b);
    
    /* SIMD operations */
    __m128i v1 = _mm_set_epi32(result, a, b, c);
    __m128i v2 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v3 = vector_op(v1, v2);
    
    int varr[4];
    _mm_storeu_si128((__m128i*)varr, v3);
    result += varr[0] + varr[1] + varr[2] + varr[3];
    
    return result;
}

/* Region with nested control flow */
__attribute__((noinline)) int scheduling_region_2(int base) {
    int sum = 0;
    volatile int barrier = 0;
    
    /* Nested loops with data-dependent exits */
    for (int i = 0; i < 10; i++) {
        int inner = base + i;
        barrier = barrier + 1;  /* Volatile */
        
        for (int j = 0; j < 5; j++) {
            if (__builtin_expect((inner & 3) == 0, 0)) {
                /* Unlikely path */
                inner = complex_chain(inner);
                asm volatile ("nop; nop" : : : "memory");
            } else {
                /* Likely path with many independent ops */
                int t1 = inner * j;
                int t2 = inner + j;
                int t3 = inner ^ j;
                int t4 = inner - j;
                
                asm volatile ("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4) : "memory");
                
                sum += t1 + t2 + t3 + t4;
                inner = helper1(t1, t2);
            }
            
            /* Memory barrier every few iterations */
            if ((j & 1) == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Switch with multiple cases */
        switch (i & 3) {
            case 0: {
                int x = sum * 3;
                int y = x ^ 0xFF;
                sum = y + (x >> 1);
                asm volatile ("# case0" : : : "memory");
                break;
            }
            case 1: {
                float f1 = sum * 0.5f;
                float f2 = helper3(f1, 2.0f, 3.0f);
                sum += (int)f2;
                asm volatile ("# case1" : : : "memory", "xmm0", "xmm1");
                break;
            }
            case 2: {
                /* Vector operations */
                __m128i v = _mm_set1_epi32(sum);
                v = _mm_slli_epi32(v, 2);
                v = _mm_add_epi32(v, _mm_set1_epi32(1));
                sum += _mm_extract_epi32(v, 0);
                break;
            }
            default: {
                /* Complex chain */
                sum = complex_chain(sum);
                asm volatile ("# default" : : : "cc", "memory");
                break;
            }
        }
    }
    
    return sum;
}

/* Region with computed goto */
__attribute__((noinline)) int scheduling_region_3(int init) {
    int val = init;
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    /* Multiple independent instruction streams */
    int a = val + 1;
    int b = val * 2;
    int c = val ^ 0xAA;
    int d = val >> 3;
    
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    
    /* Computed goto to create interesting control flow */
    int idx = val & 3;
    goto *labels[idx];
    
L0:
    a = helper1(a, b);
    b = helper2(&b, c);
    asm volatile ("# L0" : : : "memory");
    goto join;
    
L1:
    c = complex_chain(c);
    d = d * 7 + 3;
    asm volatile ("# L1" : : : "memory");
    goto join;
    
L2:
    {
        __m128i v1 = _mm_set_epi32(a, b, c, d);
        __m128i v2 = _mm_set1_epi32(0x55);
        __m128i v3 = _mm_xor_si128(v1, v2);
        int arr[4];
        _mm_storeu_si128((__m128i*)arr, v3);
        a = arr[0]; b = arr[1]; c = arr[2]; d = arr[3];
    }
    goto join;
    
L3:
    a = __builtin_popcount(a);
    b = __builtin_ctz(b | 1);
    c = __builtin_clz(c | 1);
    asm volatile ("# L3" : : : "memory");
    goto join;
    
L4:
    /* Fall through to join */
    
join:
    val = a + b + c + d;
    
    /* Another scheduling barrier */
    asm volatile ("mfence" : : : "memory");
    
    return val;
}

/* Main function with multiple scheduling regions */
int main() {
    int result = 0;
    volatile_ptr = &global_counter;
    
    /* Region 1: Basic complex scheduling */
    result = scheduling_region_1(42);
    
    /* Region 2: Nested control flow */
    result ^= scheduling_region_2(result);
    
    /* Region 3: Computed goto and vector ops */
    result += scheduling_region_3(result);
    
    /* Region 4: Inline complex operations */
    for (int i = 0; i < 100; i++) {
        int t = result;
        
        /* Many independent operations */
        int x1 = t * 3;
        int x2 = t + 17;
        int x3 = t ^ 0x1234;
        int x4 = t >> 2;
        int x5 = __builtin_bswap32(t);
        int x6 = complex_chain(t);
        
        /* Volatile access */
        *volatile_ptr = *volatile_ptr + 1;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Mix results */
        result = x1 + x2 + x3 + x4 + x5 + x6;
        
        /* Every 10 iterations, add more complexity */
        if ((i % 10) == 0) {
            __m128i v = _mm_set1_epi32(result);
            v = _mm_slli_epi32(v, (i & 3) + 1);
            v = _mm_add_epi32(v, _mm_set1_epi32(i));
            result = _mm_extract_epi32(v, 0);
            
            /* Call helper with side effects */
            result = helper2(&result, i);
        }
        
        /* Scheduling barrier every 3 iterations */
        if ((i % 3) == 0) {
            asm volatile ("# barrier" : : : "memory", "eax", "ebx", "ecx", "edx");
        }
    }
    
    /* Final mixing */
    result = result ^ (result >> 16);
    result = result * 0x5BD1E995;
    result = result ^ (result >> 15);
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result & 0x7FFFFFFF;  /* Return positive value */
}
