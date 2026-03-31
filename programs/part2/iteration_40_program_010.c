/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure 
   -fsched-spec-load -fsched-spec-load-dangerous -fselective-scheduling2 
   -march=native -mtune=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))
#define VOLATILE_MEMORY volatile

/* Non-inlinable functions to create scheduling barriers */
NOINLINE int helper1(int a, int b) {
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + (a ^ b);
}

NOINLINE float helper2(float a, float b) {
    asm volatile ("" : "+x"(a), "+x"(b) : : "memory");
    return a * b - a / (b + 1.0f);
}

NOINLINE void helper3(VOLATILE_MEMORY int* p) {
    *p = (*p << 3) | (*p >> 5);
    asm volatile ("mfence" ::: "memory");
}

/* Always inline to create complex basic blocks */
ALWAYS_INLINE int complex_block1(int seed) {
    int a = seed;
    int b = a + 0x12345678;
    int c = b * 0x89ABCDEF;
    VOLATILE_MEMORY int* vptr = &a;
    
    /* Mixed operations with volatile access */
    *vptr = *vptr + 1;
    int d = c ^ (*vptr);
    
    /* Inline assembly with clobbers */
    asm volatile ("addl %%ebx, %%eax\n\t"
                  "rorl $7, %%eax"
                  : "=a"(d) : "a"(d), "b"(c) : "cc", "memory");
    
    /* Function call creates scheduling barrier */
    d = helper1(d, a);
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(d, c, b, a);
    __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    
    int result;
    _mm_storeu_si128((__m128i*)&result, vec3);
    return result;
}

ALWAYS_INLINE float complex_block2(float seed) {
    float a = seed;
    float b = a * 3.14159f;
    float c = b / 2.71828f;
    
    /* Memory barrier */
    asm volatile ("" ::: "memory");
    
    /* Multiple independent FP operations */
    float d = b + c;
    float e = b - c;
    float f = b * c;
    float g = b / (c + 1.0f);
    
    /* Function call */
    d = helper2(d, e);
    
    /* Mixed integer/float operations */
    int id = (int)d;
    id = __builtin_popcount(id) + __builtin_ctz(id | 1);
    
    /* More volatile operations */
    VOLATILE_MEMORY float* vf = &a;
    *vf = *vf + id;
    
    return a + d + e + f + g;
}

/* Complex switch-case structure */
ALWAYS_INLINE int switch_block(int val) {
    int result = 0;
    
    switch (val & 7) {
        case 0: {
            /* Vector operations */
            __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v2 = _mm_set_ps(val, val+1, val+2, val+3);
            __m128 v3 = _mm_add_ps(v1, v2);
            float f[4];
            _mm_storeu_ps(f, v3);
            result = (int)(f[0] + f[1] + f[2] + f[3]);
            break;
        }
        case 1: {
            /* Integer chain */
            int a = val;
            for (int i = 0; i < 8; i++) {
                a = (a * 1103515245 + 12345) & 0x7fffffff;
                a = __builtin_bswap32(a);
            }
            result = a;
            break;
        }
        case 2: {
            /* Mixed operations with inline asm */
            int x = val;
            asm volatile ("imull %%ecx, %%eax\n\t"
                          "shrl $3, %%eax"
                          : "+a"(x) : "c"(val) : "cc");
            result = x ^ (val << 4);
            break;
        }
        case 3: {
            /* Memory intensive */
            VOLATILE_MEMORY int arr[16];
            for (int i = 0; i < 16; i++) {
                arr[i] = val + i;
                asm volatile ("" ::: "memory");
            }
            for (int i = 0; i < 16; i++) {
                result ^= arr[i];
            }
            break;
        }
        default: {
            /* Complex arithmetic chain */
            result = val;
            for (int i = 0; i < 12; i++) {
                result = (result * 13 + 17) % 7919;
                if (result & 1) {
                    result ^= 0x55555555;
                }
            }
            break;
        }
    }
    
    return result;
}

/* Nested loops with data-dependent exit */
ALWAYS_INLINE int nested_loop_block(int limit) {
    int sum = 0;
    int outer = limit;
    
    while (outer-- > 0) {
        int inner = (outer * 97) % 23 + 5;
        VOLATILE_MEMORY int counter = inner;
        
        do {
            /* Multiple independent operations */
            int a = sum + outer;
            int b = sum - outer;
            int c = sum * outer;
            int d = sum ^ outer;
            
            /* Inline asm with register constraints */
            asm volatile ("addl %%ebx, %%eax\n\t"
                          "subl %%ecx, %%eax\n\t"
                          "xorl %%edx, %%eax"
                          : "+a"(sum) : "b"(a), "c"(b), "d"(d) : "cc");
            
            /* Function call */
            helper3(&counter);
            
            counter--;
        } while (counter > 0 && (sum & 0xFFF) != 0);
        
        /* SIMD operation every few iterations */
        if (outer % 4 == 0) {
            __m128i vsum = _mm_set1_epi32(sum);
            __m128i vinc = _mm_set1_epi32(outer);
            vsum = _mm_add_epi32(vsum, vinc);
            sum = _mm_cvtsi128_si32(vsum);
        }
    }
    
    return sum;
}

/* Main function with multiple scheduling regions */
int main() {
    int checksum = 0xDEADBEEF;
    float fchecksum = 3.14159f;
    
    /* Region 1: Complex integer block */
    #pragma GCC optimize("O3")
    for (int i = 0; i < 100; i++) {
        checksum ^= complex_block1(checksum + i);
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    /* Region 2: Floating point intensive */
    #pragma GCC optimize("Os")
    for (int i = 0; i < 50; i++) {
        fchecksum += complex_block2(fchecksum + i);
        if (__builtin_expect((i & 15) == 0, 0)) {
            checksum += (int)fchecksum;
        }
    }
    checksum += (int)(fchecksum * 1000);
    
    /* Region 3: Switch-case patterns */
    #pragma GCC optimize("O2")
    for (int i = 0; i < 200; i++) {
        checksum += switch_block(checksum + i);
        /* Memory barrier between iterations */
        asm volatile ("" ::: "memory");
    }
    
    /* Region 4: Nested loops */
    #pragma GCC optimize("O3")
    checksum += nested_loop_block(25);
    
    /* Region 5: Mixed vector operations */
    {
        __m256i vec1 = _mm256_set_epi32(checksum, checksum+1, checksum+2, 
                                       checksum+3, checksum+4, checksum+5,
                                       checksum+6, checksum+7);
        __m256i vec2 = _mm256_set1_epi32(0x12345678);
        __m256i vec3 = _mm256_xor_si256(vec1, vec2);
        
        int results[8];
        _mm256_storeu_si256((__m256i*)results, vec3);
        
        for (int i = 0; i < 8; i++) {
            checksum ^= results[i];
            checksum = __builtin_bswap32(checksum);
        }
    }
    
    /* Final mixing */
    checksum = (checksum * 0xCC9E2D51) ^ (checksum >> 16);
    checksum = (checksum * 0x1B873593) ^ (checksum >> 13);
    
    /* Prevent dead code elimination */
    VOLATILE_MEMORY int* volatile_output = (VOLATILE_MEMORY int*)0x1000;
    *volatile_output = checksum;
    
    return checksum & 0x7FFFFFFF;
}
