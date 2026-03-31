/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling boundaries */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile memory for scheduling barriers */
volatile int global_barrier = 0;
volatile int* volatile volatile_ptr;

/* Helper functions that won't be inlined */
NOINLINE int helper1(int a, int b) {
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + (a ^ b);
}

NOINLINE float helper2(float a, float b) {
    asm volatile ("" : "+x"(a), "+x"(b) : : "memory");
    return a * b - a / (b + 1.0f);
}

NOINLINE void helper3(__m128i* dst, const __m128i* src) {
    asm volatile (
        "movdqu %1, %%xmm0\n\t"
        "paddd %%xmm0, %%xmm0\n\t"
        "movdqu %%xmm0, %0\n\t"
        : "=m"(*dst)
        : "m"(*src)
        : "xmm0", "memory"
    );
}

/* Function with complex scheduling requirements */
NOINLINE int complex_scheduling_region(int seed) {
    int a = seed, b = seed * 2, c = seed * 3;
    float f1 = seed * 1.5f, f2 = seed * 2.5f;
    __m128i vec1, vec2, vec3;
    volatile int* p = &global_barrier;
    
    /* Mixed integer operations creating dependency chains */
    a = b + c;
    b = a * seed;
    c = b >> 3;
    a = c ^ (b & 0xFF);
    b = helper1(a, c);
    
    /* Volatile accesses create scheduling barriers */
    *p = *p + 1;
    volatile_ptr = p;
    int tmp = *volatile_ptr;
    
    /* Floating point with memory barriers */
    asm volatile ("" : : : "memory");
    f1 = helper2(f1, f2);
    f2 = f1 * 3.14159f;
    asm volatile ("" : : : "memory");
    
    /* Vector operations - triggers target-specific scheduling */
    vec1 = _mm_set_epi32(a, b, c, seed);
    vec2 = _mm_set_epi32(b, c, a, tmp);
    vec3 = _mm_add_epi32(vec1, vec2);
    helper3(&vec1, &vec3);
    
    /* More integer ops with builtins */
    int popcnt = __builtin_popcount(a | b | c);
    int ctz = __builtin_ctz((a & 0xFFFF) | 1);
    
    /* Complex conditional with many operations */
    int result = 0;
    for (int i = 0; i < 8; i++) {
        if (i & 1) {
            result += a * i + popcnt;
            asm volatile ("nop" : : : "memory");
        } else {
            result -= b / (i + 1) + ctz;
            asm volatile ("" : : : "eax", "memory");
        }
        
        /* Volatile access in loop */
        *p += i;
        
        /* More vector ops */
        if (i % 3 == 0) {
            vec2 = _mm_slli_epi32(vec1, i % 4);
            vec3 = _mm_add_epi32(vec3, vec2);
        }
    }
    
    /* Extract results from vector */
    int vec_results[4];
    _mm_storeu_si128((__m128i*)vec_results, vec3);
    
    return result + vec_results[0] + vec_results[1] + vec_results[2] + vec_results[3];
}

/* Another complex region with different patterns */
NOINLINE int switch_based_scheduling(int mode) {
    int result = 0;
    
    switch (mode % 5) {
        case 0: {
            /* Many independent instructions */
            int a1 = helper1(mode, 1);
            int a2 = helper1(mode, 2);
            int a3 = helper1(mode, 3);
            int a4 = helper1(mode, 4);
            asm volatile ("" : : : "memory");
            result = a1 + a2 + a3 + a4;
            
            /* Memory operations with different latencies */
            volatile int v1 = a1;
            volatile int v2 = a2;
            result += v1 * v2;
            break;
        }
        case 1: {
            /* Floating point intensive */
            float f = mode;
            for (int i = 0; i < 12; i++) {
                f = helper2(f, i + 1.0f);
                if (i % 2 == 0) {
                    asm volatile ("nop\n\tnop" : : : "memory");
                }
            }
            result = (int)f;
            break;
        }
        case 2: {
            /* Vector operations with inline assembly constraints */
            __m128i v = _mm_set1_epi32(mode);
            __m128i accum = _mm_setzero_si128();
            
            for (int i = 0; i < 16; i++) {
                v = _mm_add_epi32(v, _mm_set1_epi32(i));
                if (i % 4 == 0) {
                    accum = _mm_add_epi32(accum, v);
                    asm volatile ("" : : : "xmm0", "xmm1", "memory");
                }
            }
            
            int r[4];
            _mm_storeu_si128((__m128i*)r, accum);
            result = r[0] + r[1] + r[2] + r[3];
            break;
        }
        case 3: {
            /* Mixed operations with memory barriers */
            result = mode;
            for (int i = 0; i < 20; i++) {
                result = helper1(result, i);
                if (i == 10) {
                    asm volatile ("" : : : "memory");
                    global_barrier++;
                }
            }
            break;
        }
        case 4: {
            /* Complex dependency chain */
            int x = mode, y = mode * 2, z = mode * 3;
            for (int i = 0; i < 15; i++) {
                x = (x * y + z) >> 1;
                y = (y * z + x) & 0xFFF;
                z = (z * x + y) ^ 0xABCD;
                asm volatile ("" : "+r"(x), "+r"(y), "+r"(z) : : "memory");
            }
            result = x + y + z;
            break;
        }
    }
    
    return result;
}

/* Main function with multiple scheduling regions */
int main() {
    int checksum = 0;
    
    /* Initialize volatile pointer */
    volatile_ptr = &global_barrier;
    
    /* Multiple complex scheduling regions */
    for (int i = 0; i < 100; i++) {
        /* Region 1: Complex integer and vector ops */
        checksum ^= complex_scheduling_region(i);
        
        /* Region 2: Switch-based with different patterns */
        checksum += switch_based_scheduling(checksum);
        
        /* Region 3: Inline with mixed operations */
        {
            int a = checksum, b = i, c = global_barrier;
            float f = a * 0.5f;
            __m128i v = _mm_set_epi32(a, b, c, checksum ^ i);
            
            /* Dependency chain */
            for (int j = 0; j < 8; j++) {
                a = helper1(a, j);
                b = (b << (j % 4)) | (a >> (8 - j % 4));
                c = __builtin_popcount(a ^ b) + c;
                f = helper2(f, j + 1.0f);
                
                /* Vector operation every iteration */
                v = _mm_add_epi32(v, _mm_set1_epi32(j));
                
                /* Memory barrier every 3 iterations */
                if (j % 3 == 0) {
                    asm volatile ("" : : : "memory");
                    *volatile_ptr += j;
                }
            }
            
            int vsum[4];
            _mm_storeu_si128((__m128i*)vsum, v);
            checksum += a + b + c + (int)f + vsum[0] + vsum[1];
        }
        
        /* Region 4: Nested loops with conditionals */
        if (i % 3 == 0) {
            int x = 0;
            for (int j = 0; j < 25; j++) {
                for (int k = 0; k < 5; k++) {
                    x += helper1(j, k);
                    if (k == 2) {
                        asm volatile ("nop" : : : "memory");
                        x += *volatile_ptr;
                    }
                }
                
                /* Vector ops in outer loop */
                if (j % 4 == 0) {
                    __m128i v = _mm_set1_epi32(x);
                    __m128i v2 = _mm_slli_epi32(v, j % 8);
                    int tmp[4];
                    _mm_storeu_si128((__m128i*)tmp, v2);
                    x += tmp[0];
                }
            }
            checksum ^= x;
        }
    }
    
    /* Final computation using all accumulated state */
    checksum += __builtin_popcount(checksum);
    checksum ^= global_barrier;
    
    /* Prevent dead code elimination */
    asm volatile ("" : "+r"(checksum) : : "memory");
    
    return checksum & 0xFF;
}
