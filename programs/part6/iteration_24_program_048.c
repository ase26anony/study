/* test_sched_context.c - Trigger free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
volatile int seed1, seed2, seed3;

/* Helper with function calls and volatile args - encourages state save/restore */
static int helper_func(int a, int b) {
    volatile int v = a;
    int r = v * b + 1;
    /* Create scheduling barrier */
    asm volatile ("" : : : "memory");
    return r ^ (b << 3);
}

/* Pattern 1: Dense arithmetic sequence to fill instruction queue */
int dense_arithmetic(int base) {
    int a = base + seed1;
    int b = seed2 ^ 0x1234;
    int c = seed3 * 2;
    int d = a + b;
    int e = c * d;
    int f = a ^ b ^ c;
    int g = d * e + f;
    int h = g << 3;
    int i = h ^ e;
    int j = i * a + b;
    int k = j - c;
    int l = k ^ d;
    int m = l * e;
    int n = m + f;
    int o = n ^ g;
    int p = o * h;
    int q = p + i;
    int r = q ^ j;
    int s = r * k;
    int t = s + l;
    int u = t ^ m;
    int v = u * n;
    int w = v + o;
    int x = w ^ p;
    int y = x * q;
    int z = y + r;
    
    /* Mix in memory operations */
    volatile int mem1 = z;
    int* ptr = (int*)&mem1;
    int mem2 = *ptr;
    
    return mem2 + s + t + u;
}

/* Pattern 2: Vector operations for target-specific scheduling hooks */
#ifdef __SSE2__
__m128i vector_ops(int iter) {
    __m128i v1 = _mm_set1_epi32(seed1);
    __m128i v2 = _mm_set1_epi32(seed2);
    __m128i v3 = _mm_set1_epi32(seed3);
    __m128i acc = _mm_setzero_si128();
    
    for (int i = 0; i < (iter & 7); ++i) {
        /* Create dependency chain with vector ops */
        __m128i t1 = _mm_add_epi32(v1, v2);
        __m128i t2 = _mm_mullo_epi16(t1, v3);
        __m128i t3 = _mm_xor_si128(t2, v1);
        __m128i t4 = _mm_slli_epi32(t3, 2);
        acc = _mm_add_epi32(acc, t4);
        
        /* Rotate values */
        v1 = v2;
        v2 = v3;
        v3 = t1;
    }
    
    /* Inline assembly with specific constraints */
    int result[4];
    _mm_storeu_si128((__m128i*)result, acc);
    
    asm volatile (
        "movdqa %0, %%xmm0\n\t"
        "pshufd $0x1B, %%xmm0, %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, %0"
        : "+m" (result)
        : 
        : "xmm0", "xmm1", "memory"
    );
    
    return _mm_loadu_si128((__m128i*)result);
}
#endif

/* Pattern 3: Complex control flow with inline assembly barriers */
int control_flow_pattern(int limit) {
    int sum = 0;
    volatile int cond = seed1;
    
    /* Loop with unpredictable trip count */
    for (int i = 0; i < (limit & 0xFF); ++i) {
        int x = i * seed2;
        int y = x ^ seed3;
        
        /* Artificial scheduling barrier */
        asm volatile ("# Scheduling Barrier" : : : "memory");
        
        if (cond & (1 << (i & 7))) {
            /* Branch with dependent operations */
            int t1 = x + y;
            int t2 = t1 * helper_func(x, y);
            int t3 = t2 ^ (y << 2);
            sum += t3;
            
            /* Another barrier */
            asm volatile ("" : : : "memory");
        } else {
            /* Alternative path */
            int t1 = x - y;
            int t2 = helper_func(y, x);
            int t3 = t1 * t2;
            sum -= t3;
        }
        
        /* Modify condition unpredictably */
        cond ^= (x << 3) | (y >> 2);
    }
    
    return sum;
}

/* Pattern 4: Mixed integer/float operations */
float mixed_operations(int base) {
    float f1 = (float)(base + seed1) * 1.5f;
    float f2 = (float)(seed2 ^ 0x5678) * 0.75f;
    int i1 = seed3 * 3;
    
    /* Interleave float and int ops */
    float acc = f1;
    for (int i = 0; i < 8; ++i) {
        int t_int = i1 * (i + 1);
        float t_float = acc * f2;
        
        /* Dependency between float and int */
        acc = t_float + (float)t_int;
        i1 ^= (int)acc;
        
        /* Memory op */
        volatile float mem_float = acc;
        acc = mem_float * 1.1f;
    }
    
    return acc;
}

/* Main driver that creates multiple scheduling contexts */
int main(int argc, char** argv) {
    /* Initialize volatile seeds from argv */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 100; ++iter) {
        volatile int loop_var = iter;
        
        /* Pattern 1: Dense arithmetic (fills instruction queue) */
        int r1 = dense_arithmetic(loop_var);
        
        /* Pattern 2: Vector operations (triggers target hooks) */
        #ifdef __SSE2__
        __m128i vres = vector_ops(loop_var);
        int vsum[4];
        _mm_storeu_si128((__m128i*)vsum, vres);
        int r2 = vsum[0] + vsum[1] + vsum[2] + vsum[3];
        #else
        int r2 = loop_var * 7;
        #endif
        
        /* Pattern 3: Control flow with barriers */
        int r3 = control_flow_pattern(loop_var);
        
        /* Pattern 4: Mixed operations */
        float r4_f = mixed_operations(loop_var);
        int r4 = (int)r4_f;
        
        /* Combine results with complex expression */
        total += (r1 ^ r2) + (r3 * r4) - helper_func(r1, r2);
        
        /* Occasionally create deeper nesting */
        if ((iter & 15) == 0) {
            volatile int inner_seed = total;
            for (int j = 0; j < 3; ++j) {
                int inner = dense_arithmetic(inner_seed + j);
                total ^= inner;
                
                /* Force scheduler to consider alternative schedules */
                asm volatile (
                    "addl %1, %0\n\t"
                    "xorl %2, %0"
                    : "+r" (total)
                    : "r" (seed1), "r" (seed2)
                    : "cc"
                );
            }
        }
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
