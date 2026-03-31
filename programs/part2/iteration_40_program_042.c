/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory", "eax", "ebx");
    return a * b + barrier;
}

__attribute__((noinline)) float helper2(float x, float y) {
    volatile float mem = 3.14f;
    asm volatile ("# FPU barrier" : "+t"(x), "+u"(y) : : "memory", "st", "st(1)");
    return x * y + mem;
}

__attribute__((noinline)) __m128i helper3(__m128i a, __m128i b) {
    volatile __m128i v = _mm_set1_epi32(0xDEADBEEF);
    asm volatile ("# SIMD barrier" : "+x"(a), "+x"(b) : : "memory", "xmm0", "xmm1");
    return _mm_add_epi32(_mm_xor_si128(a, b), v);
}

/* Force front-end state saving with always_inline */
__attribute__((always_inline)) static inline int complex_region(int *arr, float *farr, volatile int *varr, int n) {
    int sum = 0;
    __m128i vec_acc = _mm_setzero_si128();
    
    /* Mixed operations to create instruction queue */
    for (int i = 0; i < n; i++) {
        /* Volatile access creates scheduling barrier */
        int v = varr[i];
        
        /* Integer chain with dependencies */
        int a = arr[i] + v;
        int b = a * 3;
        int c = b >> 2;
        int d = c ^ 0x5555;
        
        /* Floating point interleaved */
        float f = farr[i];
        f = f * 2.0f + (float)d;
        
        /* Inline assembly with clobbers */
        asm volatile ("# Mixed ops %0, %1" : "+r"(d), "+t"(f) : : "memory", "eax", "st");
        
        /* Function call creates scheduling boundary */
        d = helper1(d, i);
        f = helper2(f, 1.5f);
        
        /* SIMD operations for target-specific scheduling */
        __m128i vi = _mm_set_epi32(d, i, arr[i], v);
        __m128i vj = _mm_set1_epi32(0x12345678);
        vec_acc = helper3(vec_acc, _mm_mullo_epi16(vi, vj));
        
        /* Switch with multiple cases to create complex BB */
        switch (i & 0x3) {
            case 0:
                sum += d + (int)f;
                asm volatile ("nop" : : : "memory");
                break;
            case 1:
                sum += d * 2 - (int)f;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 2:
                sum += __builtin_popcount(d) ^ (int)f;
                /* Force register pressure */
                asm volatile ("# case2" : : : "eax", "ebx", "ecx", "edx");
                break;
            default:
                sum += __builtin_ctz(d | 1) + (int)f;
                /* Vector operation in default case */
                vec_acc = _mm_slli_epi32(vec_acc, 2);
                break;
        }
        
        /* Conditional break with data dependency */
        if (__builtin_expect((sum & 0xFF) == 0, 0)) {
            /* Unlikely path with more operations */
            volatile int *p = &varr[i];
            *p = *p + sum;
            asm volatile ("# unlikely" : : : "memory", "eax", "ebx", "ecx");
            if (i > n/2) break;
        }
    }
    
    /* Reduce vector to scalar */
    int vec_sum[4] __attribute__((aligned(16)));
    _mm_store_si128((__m128i*)vec_sum, vec_acc);
    sum += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    
    return sum;
}

/* Another scheduling region with different pattern */
__attribute__((noinline)) int second_region(int *arr, int n) {
    int total = 0;
    
    /* Nested loops with data-dependent exit */
    for (int i = 0; i < n; i++) {
        int acc = arr[i];
        for (int j = 0; j < 8; j++) {
            /* Long dependency chain */
            acc = acc * 3 + 1;
            acc = acc ^ (acc >> 1);
            acc = __builtin_bswap32(acc);
            acc = acc + j;
            
            /* Memory access with varying addresses */
            volatile int *vp = &arr[(i + j) % n];
            *vp = *vp + acc;
            
            /* Architecture-specific builtin */
            acc += __builtin_ia32_crc32si(acc, j);
            
            if (acc % 1000 == 0) {
                /* Early exit from inner loop */
                asm volatile ("# inner_break" : : : "memory");
                break;
            }
        }
        
        /* Compute jumps via goto */
        if (acc & 1) goto label1;
        if (acc & 2) goto label2;
        if (acc & 4) goto label3;
        
        total += acc;
        continue;
        
    label1:
        total += acc * 2;
        asm volatile ("# label1" : : : "memory");
        continue;
        
    label2:
        total += acc / 2;
        /* Force FPU ops in integer path */
        float ftmp = (float)acc;
        asm volatile ("fwait" : : : "st");
        total += (int)ftmp;
        continue;
        
    label3:
        total += acc ^ 0xAAAA;
        /* SIMD in computed jump path */
        __m128i v = _mm_set_epi32(acc, total, i, n);
        v = _mm_srai_epi32(v, 3);
        total += _mm_extract_epi32(v, 0);
        continue;
    }
    
    return total;
}

int main() {
    /* Large arrays to prevent optimization */
    static int array[256] __attribute__((aligned(64)));
    static float farray[256] __attribute__((aligned(64)));
    static volatile int varray[256] __attribute__((aligned(64)));
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
        farray[i] = (float)i * 0.5f;
        varray[i] = i ^ 0x55;
    }
    
    int result = 0;
    
    /* Multiple scheduling regions with different optimization hints */
    #pragma GCC optimize("O2")
    result += complex_region(array, farray, varray, 128);
    
    #pragma GCC optimize("O3")
    result += second_region(array, 128);
    
    /* Third region with manual scheduling barrier */
    asm volatile ("# Region3_start" : : : "memory");
    {
        int tmp = 0;
        for (int i = 0; i < 64; i++) {
            /* Many independent instructions */
            int a = array[i] + 1;
            int b = array[i+64] * 2;
            int c = array[i+128] & 0xFF;
            int d = array[i+192] | 0xAA;
            
            /* All can execute in parallel */
            tmp += a + b + c + d;
            
            /* Memory barrier every 8 iterations */
            if ((i & 7) == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        result += tmp;
    }
    asm volatile ("# Region3_end" : : : "memory");
    
    /* Fourth region with vector operations only */
    __m128i vsum = _mm_setzero_si128();
    for (int i = 0; i < 256; i += 4) {
        __m128i v1 = _mm_load_si128((__m128i*)&array[i]);
        __m128i v2 = _mm_load_si128((__m128i*)&array[i+128]);
        
        /* Various SIMD operations */
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi16(v1, v2);
        __m128i v5 = _mm_sll_epi32(v3, v4);
        
        /* Target-specific intrinsic */
        v5 = _mm_xor_si128(v5, _mm_set1_epi32(0xFFFFFFFF));
        
        vsum = _mm_add_epi32(vsum, v5);
    }
    
    /* Reduce final vector */
    int vtmp[4];
    _mm_store_si128((__m128i*)vtmp, vsum);
    result += vtmp[0] + vtmp[1] + vtmp[2] + vtmp[3];
    
    /* Use result to prevent elimination */
    volatile int sink = result;
    return sink & 0x7FFFFFFF; /* Ensure positive return */
}
