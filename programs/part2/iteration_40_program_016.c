/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force scheduling across function boundaries */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile ("" : "+r" (a), "+r" (b) : : "memory", "eax", "ebx");
    return a * b + barrier;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float barrier = 0.0f;
    asm volatile ("" : "+x" (a), "+x" (b) : : "memory", "xmm0", "xmm1");
    return a / (b + 1.0f) + barrier;
}

__attribute__((noinline)) static __m128i helper3(__m128i a, __m128i b) {
    volatile __m128i barrier = _mm_setzero_si128();
    asm volatile ("" : "+x" (a), "+x" (b) : : "memory", "xmm2", "xmm3");
    return _mm_add_epi32(a, b);
}

/* Force scheduling state with mixed operations */
static int complex_region_1(int* arr, volatile int* varr, int n) {
    int sum = 0;
    __m128i vec_sum = _mm_setzero_si128();
    
    /* Create instruction chains with dependencies */
    for (int i = 0; i < n; i++) {
        /* Multiple interdependent integer operations */
        int a = arr[i];
        int b = a + i;
        int c = b * 2;
        int d = c >> 3;
        int e = d ^ 0x55AA55AA;
        
        /* Volatile access creates scheduling barrier */
        *varr = *varr + 1;
        
        /* Inline assembly with clobbers */
        asm volatile ("# Complex operation %0 %1" : "+r" (e) : "r" (i) : "memory", "eax", "ebx", "ecx");
        
        /* Function call with side effects */
        e = helper1(e, i);
        
        /* SIMD operations for target-specific scheduling */
        if (i % 4 == 0) {
            __m128i v1 = _mm_set_epi32(e, i, a, b);
            __m128i v2 = _mm_set_epi32(c, d, e, i);
            vec_sum = _mm_add_epi32(vec_sum, v1);
            vec_sum = _mm_add_epi32(vec_sum, v2);
            
            /* Architecture-specific builtin */
            int popcnt = __builtin_popcount(e);
            e ^= popcnt;
        }
        
        /* Mixed floating point operations */
        float f = (float)e;
        f = helper2(f, f * 0.5f);
        
        /* More volatile accesses */
        volatile int* p = &arr[i];
        *p = (*p) + (int)f;
        
        sum += e;
    }
    
    /* Extract from vector */
    int vec_result[4];
    _mm_storeu_si128((__m128i*)vec_result, vec_sum);
    sum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    return sum;
}

/* Create instruction queues with nested control flow */
static int complex_region_2(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    /* Switch with multiple cases to create complex basic blocks */
    while (i < n) {
        switch (i % 7) {
            case 0: {
                /* Case with many independent instructions */
                int a = arr[i] + 1;
                int b = arr[i+1] * 2;
                int c = arr[i+2] & 0xFF;
                int d = arr[i+3] | 0x55;
                int e = arr[i+4] ^ arr[i+5];
                int f = a + b;
                int g = c - d;
                int h = e << 2;
                int j = f * g;
                int k = h / (j + 1);
                
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                
                result += a + b + c + d + e + f + g + h + j + k;
                i += 6;
                break;
            }
            case 1: {
                /* Vector operations */
                __m128i v1 = _mm_loadu_si128((__m128i*)&arr[i]);
                __m128i v2 = _mm_loadu_si128((__m128i*)&arr[i+4]);
                __m128i v3 = _mm_add_epi32(v1, v2);
                __m128i v4 = _mm_mullo_epi32(v3, _mm_set1_epi32(3));
                
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v4);
                result += temp[0] + temp[1] + temp[2] + temp[3];
                
                /* Profile-guided optimization hint */
                if (__builtin_expect(arr[i] > 100, 0)) {
                    result += __builtin_ctz(arr[i]);
                }
                i += 8;
                break;
            }
            case 2: {
                /* Nested loop with data-dependent exit */
                int j = 0;
                int local_sum = 0;
                while (j < 5 && (i + j) < n) {
                    local_sum += arr[i + j];
                    local_sum = helper1(local_sum, j);
                    j++;
                    
                    /* Volatile in inner loop */
                    volatile int barrier = j;
                    (void)barrier;
                }
                result += local_sum;
                i += j;
                break;
            }
            case 3: {
                /* Mixed operations with computed goto */
                static void* labels[] = { &&L1, &&L2, &&L3, &&L4 };
                goto *labels[i % 4];
                
                L1: {
                    result += arr[i] * 3;
                    i++;
                    break;
                }
                L2: {
                    result += arr[i] / 2;
                    i++;
                    break;
                }
                L3: {
                    result += arr[i] << 1;
                    i++;
                    break;
                }
                L4: {
                    result += arr[i] >> 1;
                    i++;
                    break;
                }
            }
            default: {
                /* Default case with many operations */
                for (int j = 0; j < 3 && (i + j) < n; j++) {
                    int val = arr[i + j];
                    val = (val * 7) + 13;
                    val = val ^ (val >> 16);
                    val = helper1(val, j);
                    
                    /* Custom register constraint */
                    register int r asm("ebx") = val;
                    asm volatile ("# Using ebx %0" : "+r" (r) : : "memory");
                    val = r;
                    
                    result += val;
                }
                i += 3;
                break;
            }
        }
    }
    
    return result;
}

/* Force front-end state restoration with optimization pragma */
#pragma GCC optimize ("O3")
static int complex_region_3(volatile int* varr, float* farr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Interleaved integer and float operations */
        int ival = *varr;
        float fval = farr[i];
        
        /* Multiple scheduling regions separated by barriers */
        asm volatile ("# Region A" : : : "memory");
        
        ival = ival * 2 + 1;
        fval = fval * 3.14159f;
        
        asm volatile ("# Region B" : : : "memory");
        
        /* Operations that can execute in parallel */
        int a = ival + i;
        int b = ival - i;
        float c = fval + i;
        float d = fval - i;
        
        /* Function calls create scheduling boundaries */
        a = helper1(a, b);
        c = helper2(c, d);
        
        /* More volatile accesses */
        volatile int tmp = a;
        *varr = tmp;
        
        sum += a + (int)c;
    }
    
    return sum;
}
#pragma GCC reset_options

/* Main function with multiple complex regions */
int main() {
    const int SIZE = 256;
    int arr[SIZE];
    volatile int varr[SIZE];
    float farr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3 + 7;
        varr[i] = i * 2 + 1;
        farr[i] = i * 0.5f;
    }
    
    int checksum = 0;
    
    /* Execute multiple complex regions to trigger state saving */
    checksum += complex_region_1(arr, varr, SIZE);
    checksum += complex_region_2(arr, SIZE);
    checksum += complex_region_3(varr, farr, SIZE);
    
    /* Additional complex region in main */
    __m128i vec_acc = _mm_setzero_si128();
    for (int i = 0; i < SIZE; i += 4) {
        __m128i v1 = _mm_loadu_si128((__m128i*)&arr[i]);
        __m128i v2 = _mm_loadu_si128((__m128i*)&varr[i]);
        __m128i v3 = helper3(v1, v2);
        
        /* Variable latency operation */
        if (i % 8 == 0) {
            /* Simulate cache miss pattern */
            for (int j = 0; j < 100; j++) {
                asm volatile ("# Cache stress %0" : "+r" (j) : : "memory");
            }
        }
        
        vec_acc = _mm_add_epi32(vec_acc, v3);
    }
    
    int vec_result[4];
    _mm_storeu_si128((__m128i*)vec_result, vec_acc);
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Final mixing */
    checksum = helper1(checksum, checksum ^ 0xDEADBEEF);
    
    return checksum & 0x7FFFFFFF; /* Return positive value */
}
