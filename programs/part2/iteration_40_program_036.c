/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o coverage_test coverage_test.c */

#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    *p = *p + (*p >> 3);
    asm volatile ("nop" : : : "memory", "eax");
    return arr[idx];
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile ("" : : "r"(res) : "memory");
    return res;
}

/* Vector operations to trigger target-specific scheduling */
__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(a, b);
    __m128i t3 = _mm_mullo_epi32(t1, t2);
    __m128i t4 = _mm_slli_epi32(t3, 2);
    asm volatile ("" : : "x"(t4) : "memory");
    return t4;
}

/* Complex scheduling region 1 */
int region1(int seed) {
    int a = seed, b = seed * 2, c = seed + 3;
    volatile int* mem = (volatile int*)&seed;
    
    /* Chain of dependent operations */
    a = b + c;
    b = a * seed;
    c = b >> 2;
    a = c ^ b;
    b = a + *mem;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Mixed operations */
    *mem = *mem + 1;
    a = helper1(a, b);
    b = helper2(&seed, 0);
    
    /* SIMD operations */
    __m128i v1 = _mm_set_epi32(a, b, c, seed);
    __m128i v2 = _mm_set_epi32(b, c, a, seed * 2);
    __m128i v3 = vector_op(v1, v2);
    
    int arr[4];
    _mm_storeu_si128((__m128i*)arr, v3);
    
    return a + b + c + arr[0] + arr[1];
}

/* Complex scheduling region 2 with switch */
int region2(int val) {
    int result = 0;
    
    switch (val % 5) {
        case 0: {
            /* Long chain in case block */
            int t1 = val + 1;
            int t2 = t1 * 2;
            int t3 = t2 - val;
            int t4 = t3 >> 1;
            int t5 = t4 ^ t3;
            int t6 = helper1(t5, t4);
            int t7 = __builtin_popcount(t6);
            int t8 = t7 * t6;
            result = t8;
            asm volatile ("nop" : : : "memory", "ebx");
            break;
        }
        case 1: {
            volatile int* p = &val;
            *p = *p * 3;
            int t = *p;
            result = helper2(&t, 0) + __builtin_ctz(t | 1);
            asm volatile ("" : : "r"(result) : "memory");
            break;
        }
        case 2: {
            float f1 = val * 1.5f;
            float f2 = val * 0.75f;
            float f3 = helper3(f1, f2);
            result = (int)(f1 + f2 + f3);
            /* Force register pressure */
            asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
            break;
        }
        case 3: {
            /* Many independent instructions */
            int a = val + 1;
            int b = val * 2;
            int c = val - 3;
            int d = val ^ 0xFF;
            int e = val << 2;
            int f = val >> 1;
            int g = a + b;
            int h = c * d;
            int i = e & f;
            int j = g | h;
            int k = i ^ j;
            result = a + b + c + d + e + f + g + h + i + j + k;
            break;
        }
        default: {
            /* Nested loops creating instruction queue */
            int sum = 0;
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 5; j++) {
                    if ((i * j) % 3 == 0) {
                        sum += helper1(i, j);
                        asm volatile ("" : : : "memory");
                    } else {
                        sum += helper2(&sum, 0);
                    }
                }
            }
            result = sum;
            break;
        }
    }
    
    return result;
}

/* Complex scheduling region 3 with computed goto */
int region3(int base) {
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    int idx = base % 5;
    int r = base;
    
    goto *labels[idx];
    
L0:
    r = r * 2 + 1;
    r = helper1(r, base);
    r = r ^ (r >> 3);
    asm volatile ("nop" : : : "memory", "eax", "ebx");
    goto end;
    
L1:
    r = __builtin_popcount(r);
    r = r << (r % 8);
    volatile int* vp = &r;
    *vp = *vp + helper2(&r, 0);
    goto end;
    
L2:
    /* Mixed float/int operations */
    float f = r * 1.234f;
    f = helper3(f, f * 0.5f);
    r = (int)f + r;
    asm volatile ("" : : "r"(r), "x"(_mm_set1_ps(f)) : "memory");
    goto end;
    
L3:
    /* Vector operations with different latencies */
    __m128i v = _mm_set1_epi32(r);
    for (int i = 0; i < 4; i++) {
        v = _mm_add_epi32(v, _mm_set1_epi32(i));
        v = _mm_mullo_epi32(v, _mm_set1_epi32(r + i));
        asm volatile ("" : : "x"(v) : "memory");
    }
    int arr[4];
    _mm_storeu_si128((__m128i*)arr, v);
    r = arr[0] + arr[1] + arr[2] + arr[3];
    goto end;
    
L4:
    /* Instruction-level parallelism test */
    int a = r + 1, b = r * 2, c = r - 3, d = r ^ 0xAA;
    asm volatile ("" : : : "memory");  /* Barrier */
    int e = a * b, f = c + d, g = a ^ c, h = b & d;
    asm volatile ("" : : : "memory");  /* Another barrier */
    r = e + f + g + h + helper1(e, f) + helper2(&g, 0);
    goto end;
    
end:
    return r;
}

/* Main function with multiple scheduling regions */
int main() {
    int result = 0;
    volatile int counter = 0;
    
    /* Force multiple scheduling contexts */
    #pragma GCC optimize("O2")
    for (int i = 0; i < 100; i++) {
        result ^= region1(i + result);
        counter++;
    }
    
    #pragma GCC optimize("O3")
    for (int i = 0; i < 50; i++) {
        result += region2(i + result);
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    #pragma GCC optimize("Os")
    for (int i = 0; i < 75; i++) {
        result = region3(result + i);
        if (i % 10 == 0) {
            /* Function call in loop with volatile */
            volatile int* p = &result;
            *p = helper1(*p, i);
        }
    }
    
    /* Final mixed region */
    int arr[256];
    for (int i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Complex loop with data-dependent exit */
    int idx = 0;
    while (1) {
        arr[idx] = helper1(arr[idx], arr[(idx + 1) % 256]);
        idx = (idx * 13 + 7) % 256;
        if (idx == 0) break;
        
        /* Every 8 iterations, do something different */
        if ((idx & 7) == 0) {
            __m128i v = _mm_loadu_si128((__m128i*)&arr[idx]);
            v = vector_op(v, _mm_set1_epi32(idx));
            _mm_storeu_si128((__m128i*)&arr[idx], v);
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < 256; i++) {
        result ^= arr[i];
        result = (result << 1) | (result >> 31);  /* Rotate */
    }
    
    return result & 0x7FFFFFFF;  /* Ensure positive result */
}
