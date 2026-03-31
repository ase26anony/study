/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o coverage_test coverage_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile variables to create memory barriers */
volatile int global_barrier = 0;
volatile int* volatile volatile_ptr;

/* Non-inlinable functions to force scheduling boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + (a ^ b);
}

__attribute__((noinline)) int helper2(int* p, int v) {
    volatile int tmp = *p;
    *p = tmp + v;
    asm volatile ("mfence" : : : "memory");
    return *p;
}

__attribute__((noinline)) float helper3(float a, float b) {
    asm volatile ("" : "+x"(a), "+x"(b) : : "memory");
    return a * b - a / (b + 1.0f);
}

/* Function with complex scheduling requirements */
__attribute__((noinline, optimize("O3"))) 
int complex_scheduling_region(int seed) {
    int a = seed, b = seed * 2, c = seed + 1;
    float f1 = seed * 0.5f, f2 = seed * 1.5f;
    volatile int v1 = 0, v2 = 0;
    
    /* Mixed integer operations creating dependency chains */
    a = b + c;
    b = a * seed;
    c = b >> 3;
    a = c ^ (b & 0xFF);
    b = __builtin_popcount(a) + c;
    c = __builtin_ctz(b | 1) * a;
    
    /* Volatile memory accesses creating barriers */
    v1 = a;
    v2 = b;
    global_barrier = c;
    volatile_ptr = &v1;
    
    /* SIMD operations for target-specific scheduling */
    __m128i vec1 = _mm_set_epi32(a, b, c, seed);
    __m128i vec2 = _mm_set_epi32(b, c, a, seed * 3);
    __m128i vec3 = _mm_add_epi32(vec1, vec2);
    __m128i vec4 = _mm_mullo_epi32(vec3, vec1);
    
    /* Extract results with inline assembly clobbers */
    int simd_result;
    asm volatile (
        "movd %1, %%eax\n\t"
        "pextrd $1, %1, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(simd_result)
        : "x"(vec4)
        : "eax", "ebx", "memory"
    );
    
    /* Floating point operations */
    f1 = helper3(f1, f2);
    f2 = f1 * 2.0f - f2 / 3.0f;
    
    /* More volatile operations */
    v1 = helper2(&v1, simd_result);
    v2 = helper2(&v2, a + b);
    
    return a + b + c + simd_result + (int)f1 + v1 + v2;
}

/* Main function with multiple scheduling regions */
int main() {
    int result = 0;
    int array1[256];
    float array2[256];
    volatile int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 0.1f;
    }
    
    volatile_ptr = array1;
    
    /* Region 1: Nested loops with data-dependent exits */
    for (int i = 0; i < 100; i++) {
        int j = 0;
        while (j < 50) {
            int k = array1[j] & 0xF;
            switch (k) {
                case 0: array1[j] = helper1(array1[j], i); break;
                case 1: array2[j] = helper3(array2[j], i * 0.5f); break;
                case 2: array1[j] = __builtin_popcount(array1[j]); break;
                case 3: array1[j] = __builtin_ctz(array1[j] | 1); break;
                default: array1[j] = complex_scheduling_region(array1[j]);
            }
            
            /* Inline assembly barrier */
            asm volatile ("" : : : "memory");
            
            j += (array1[j] % 7) + 1;
            if (j > 100) break;
        }
        
        /* Memory fence */
        asm volatile ("mfence" : : : "memory");
    }
    
    /* Region 2: Switch with computed goto-like flow */
    for (int i = 0; i < 50; i++) {
        int val = array1[i];
        
        switch (val % 8) {
            case 0: {
                __m128i v1 = _mm_set1_epi32(val);
                __m128i v2 = _mm_set1_epi32(i);
                __m128i v3 = _mm_add_epi32(v1, v2);
                int temp;
                _mm_store_ss((float*)&temp, _mm_castsi128_ps(v3));
                array1[i] = temp;
                /* fall through */
            }
            case 1:
                array1[i] = helper2(&array1[i], val);
                array2[i] = array2[i] * 2.0f - 1.0f;
                break;
            case 2: {
                volatile int* p = &array1[i];
                *p = *p + 1;
                asm volatile ("lock addl $0, (%0)" : : "r"(p) : "memory");
                break;
            }
            case 3:
                array1[i] = complex_scheduling_region(array1[i]);
                break;
            case 4:
                array1[i] = (array1[i] << 3) | (array1[i] >> 29);
                array2[i] = helper3(array2[i], array2[i]);
                break;
            case 5:
                array1[i] = __builtin_bswap32(array1[i]);
                break;
            case 6:
                array1[i] = array1[i] ^ 0xAAAAAAAA;
                array2[i] = array2[i] + array2[(i + 1) % 256];
                break;
            case 7:
                array1[i] = helper1(array1[i], array1[(i + 127) % 256]);
                break;
        }
        
        /* Create instruction-level parallelism */
        int t1 = array1[i] + 1;
        int t2 = array1[i] * 2;
        int t3 = array1[i] & 0xFF;
        float f1 = array2[i] * 0.5f;
        float f2 = array2[i] * 1.5f;
        
        asm volatile ("" : "+r"(t1), "+r"(t2), "+r"(t3) : : "memory");
        
        array1[i] = t1 + t2 - t3;
        array2[i] = f1 + f2;
    }
    
    /* Region 3: Tight loop with function calls and barriers */
    for (int i = 0; i < 1000; i++) {
        int idx = i % 256;
        
        /* Independent operations that can be scheduled in parallel */
        int op1 = array1[idx] + global_barrier;
        int op2 = array1[(idx + 1) % 256] * 2;
        float op3 = array2[idx] * 3.14f;
        int op4 = __builtin_popcount(array1[(idx + 2) % 256]);
        
        /* Memory barrier between independent operations */
        asm volatile ("" : : : "memory");
        
        array1[idx] = helper1(op1, op2);
        array2[idx] = helper3(op3, array2[(idx + 3) % 256]);
        
        /* Another barrier */
        asm volatile ("mfence" : : : "memory");
        
        /* Complex operation with SIMD */
        if (i % 17 == 0) {
            __m128i v = _mm_set_epi32(op1, op2, op4, idx);
            v = _mm_slli_epi32(v, 2);
            v = _mm_xor_si128(v, _mm_set1_epi32(0x55555555));
            
            int temp;
            asm volatile (
                "movd %1, %0\n\t"
                "pextrd $1, %1, %%eax\n\t"
                "addl %%eax, %0"
                : "=r"(temp) : "x"(v) : "eax", "memory"
            );
            
            array1[idx] = temp;
        }
        
        /* Profile-guided optimization hint */
        if (__builtin_expect((i % 23) == 0, 0)) {
            array1[idx] = complex_scheduling_region(array1[idx]);
        }
    }
    
    /* Region 4: Mixed operations in single basic block */
    for (int i = 0; i < 128; i++) {
        /* Create a long dependency chain */
        int chain = i;
        chain = chain * 3 + 1;
        chain = chain ^ (chain >> 1);
        chain = __builtin_popcount(chain) * 7;
        chain = chain & 0xFF;
        chain = chain | (chain << 8);
        chain = helper1(chain, i);
        chain = helper2(&chain, global_barrier);
        chain = complex_scheduling_region(chain);
        chain = chain >> 4;
        chain = chain * 11;
        chain = __builtin_ctz(chain | 1) + chain;
        chain = chain ^ 0x12345678;
        
        /* SIMD operation in the middle */
        __m128i v = _mm_set1_epi32(chain);
        v = _mm_add_epi32(v, _mm_set1_epi32(i));
        v = _mm_mul_epu32(v, _mm_set1_epi32(3));
        
        int temp;
        _mm_store_ss((float*)&temp, _mm_castsi128_ps(v));
        
        /* Final computation */
        array1[i] = temp + chain;
        checksum += array1[i];
        
        /* Force scheduling state with volatile */
        global_barrier = checksum;
    }
    
    /* Compute final result */
    for (int i = 0; i < 256; i++) {
        result += array1[i];
        result ^= (int)array2[i];
    }
    
    result += checksum;
    result += global_barrier;
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    asm volatile ("" : : "r"(final_result) : "memory");
    
    return result & 0x7FFFFFFF;  /* Return positive value */
}
