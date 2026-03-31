/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining to force function call scheduling barriers */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

__attribute__((noinline)) int helper2(int* arr, int idx) {
    volatile int* p = &arr[idx];
    int val = *p;
    *p = val + (idx * 7);
    asm volatile("" : : : "memory", "eax", "ebx");
    return val;
}

__attribute__((noinline)) float helper3(float a, float b) {
    volatile float res = a * b - a / (b + 1.0f);
    asm volatile("" : "+x"(res) : : "memory");
    return res;
}

/* Force vector operations with mixed types */
__attribute__((noinline)) __m128i vector_op(__m128i a, __m128i b) {
    __m128i add = _mm_add_epi32(a, b);
    __m128i mul = _mm_mullo_epi16(a, b);
    __m128i xor = _mm_xor_si128(add, mul);
    
    /* Inline assembly with explicit clobbers */
    asm volatile (
        "pshufd $0x1B, %1, %0\n\t"
        : "=x"(xor)
        : "x"(xor)
        : "memory"
    );
    return xor;
}

/* Complex scheduling region with mixed operations */
__attribute__((noinline, optimize("O3"))) 
int complex_region1(int* arr, float* farr, volatile int* varr, int n) {
    int sum = 0;
    int i, j;
    
    /* Mixed integer operations creating dependency chains */
    for (i = 0; i < n; i++) {
        int a = arr[i];
        int b = arr[(i * 3) % n];
        int c = a + b;
        int d = c * (i + 1);
        int e = d >> (i & 3);
        int f = e ^ (a * b);
        
        /* Volatile access creating scheduling barrier */
        volatile int* vp = &varr[i];
        *vp = *vp + f;
        
        /* Function call with side effects */
        f += helper1(a, b);
        
        /* Inline assembly with clobbers */
        asm volatile (
            "imul %1, %0\n\t"
            "add $0x7, %0\n\t"
            : "+r"(f)
            : "r"(c)
            : "cc", "memory"
        );
        
        sum += f;
    }
    return sum;
}

/* Another complex region with different patterns */
__attribute__((noinline, optimize("O2")))
int complex_region2(int* arr, float* farr, __m128i* vec_arr, int n) {
    int total = 0;
    int i = 0;
    
    /* Switch with multiple cases creating different scheduling paths */
    switch (n % 5) {
        case 0: {
            /* Vector operations */
            __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
            __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
            __m128i vres = vector_op(v1, v2);
            
            int vals[4];
            _mm_storeu_si128((__m128i*)vals, vres);
            total += vals[0] + vals[3];
            
            /* Many independent instructions */
            int t1 = arr[0] + 1;
            int t2 = arr[1] * 2;
            int t3 = arr[2] & 0xFF;
            int t4 = arr[3] | 0x80;
            int t5 = __builtin_popcount(arr[4]);
            int t6 = __builtin_ctz(arr[5] | 1);
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6) : "memory");
            
            total += t1 + t2 + t3 + t4 + t5 + t6;
            break;
        }
        case 1: {
            /* Nested loops with data-dependent exit */
            for (int j = 0; j < n; j++) {
                int k = j;
                while (k > 0 && (arr[k] % 7) != 0) {
                    float fval = helper3(farr[k], farr[k-1]);
                    int ival = (int)(fval * 100.0f);
                    
                    /* Memory barrier */
                    asm volatile("" : : : "memory");
                    
                    total += ival + helper2(arr, k);
                    k--;
                }
            }
            break;
        }
        case 2: {
            /* Mixed floating point and integer */
            for (int j = 0; j < n && j < 10; j++) {
                float f1 = farr[j];
                float f2 = farr[(j+1) % n];
                float f3 = f1 * f2 - f1 / (f2 + 1.0f);
                
                int i1 = (int)f3;
                int i2 = __builtin_bswap32(arr[j]);
                int i3 = i1 ^ i2;
                
                /* Architecture-specific builtin */
                i3 = __builtin_ia32_crc32si(i3, arr[j+1]);
                
                total += i3;
            }
            break;
        }
        case 3: {
            /* Computed goto to create complex control flow */
            void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            for (int j = 0; j < 5; j++) {
                goto *labels[arr[j] % 5];
                
                L0: total += arr[j] * 3; continue;
                L1: total += arr[j] << 2; continue;
                L2: total += arr[j] ^ 0xABCD; continue;
                L3: total += helper1(arr[j], j); continue;
                L4: total += __builtin_popcount(arr[j]); continue;
            }
            break;
        }
        case 4: {
            /* SIMD operations with horizontal reduction */
            __m128i vsum = _mm_setzero_si128();
            for (int j = 0; j + 4 <= n; j += 4) {
                __m128i v = _mm_loadu_si128((__m128i*)&arr[j]);
                vsum = _mm_add_epi32(vsum, v);
                
                /* Custom register constraints */
                register int r1 asm("eax") = arr[j];
                register int r2 asm("ebx") = arr[j+1];
                asm volatile (
                    "addl %%ebx, %%eax\n\t"
                    : "+r"(r1)
                    : "r"(r2)
                    : "cc"
                );
                total += r1;
            }
            
            /* Horizontal add */
            int vsum_arr[4];
            _mm_storeu_si128((__m128i*)vsum_arr, vsum);
            total += vsum_arr[0] + vsum_arr[1] + vsum_arr[2] + vsum_arr[3];
            break;
        }
    }
    
    return total;
}

/* Main function with multiple scheduling regions */
int main() {
    const int SIZE = 256;
    int int_arr[SIZE];
    float float_arr[SIZE];
    volatile int volatile_arr[SIZE];
    __m128i vec_arr[SIZE/4];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        float_arr[i] = (float)i * 0.5f;
        volatile_arr[i] = i;
        if (i % 4 == 0) {
            vec_arr[i/4] = _mm_set_epi32(i, i+1, i+2, i+3);
        }
    }
    
    int result = 0;
    
    /* Execute multiple complex regions to increase chance of state saving */
    
    /* Region 1: Standard complex operations */
    #pragma GCC optimize("O3")
    result += complex_region1(int_arr, float_arr, volatile_arr, SIZE);
    
    /* Region 2: Vector-heavy operations */
    #pragma GCC optimize("O2")
    result += complex_region2(int_arr, float_arr, vec_arr, SIZE);
    
    /* Region 3: Inline complex block */
    {
        int local_sum = 0;
        for (int i = 0; i < SIZE; i++) {
            /* Create instruction-level parallelism */
            int a = int_arr[i];
            int b = int_arr[(i + 7) % SIZE];
            int c = a + b;
            int d = a * b;
            int e = c ^ d;
            int f = __builtin_popcount(e);
            
            /* Memory barrier splitting scheduling region */
            asm volatile("" : : : "memory");
            
            /* More independent operations */
            float fa = float_arr[i];
            float fb = float_arr[(i + 3) % SIZE];
            float fc = fa * fb - fa / (fb + 1.0f);
            int fi = (int)(fc * 1000.0f);
            
            /* Volatile access */
            volatile int* vp = &volatile_arr[i];
            *vp = *vp + fi;
            
            local_sum += e + f + fi;
        }
        result += local_sum;
    }
    
    /* Region 4: Switch with many cases */
    for (int i = 0; i < 100; i++) {
        switch (i % 8) {
            case 0: result += int_arr[i] + 1; break;
            case 1: result += int_arr[i] * 2; break;
            case 2: result += int_arr[i] & 0xFF; break;
            case 3: result += int_arr[i] | 0x80; break;
            case 4: result += helper1(int_arr[i], i); break;
            case 5: result += helper2(int_arr, i % SIZE); break;
            case 6: result += (int)helper3(float_arr[i], float_arr[(i+1)%SIZE]); break;
            case 7: {
                /* Nested control flow */
                int j = i;
                do {
                    result += __builtin_ctz(int_arr[j % SIZE] | 1);
                    j--;
                } while (j > 0 && (result % 13) != 0);
                break;
            }
        }
    }
    
    /* Region 5: SIMD operations with mixed scheduling */
    {
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i + 4 <= SIZE; i += 4) {
            __m128i v = _mm_loadu_si128((__m128i*)&int_arr[i]);
            
            /* Multiple vector operations */
            __m128i vadd = _mm_add_epi32(v, _mm_set1_epi32(1));
            __m128i vmul = _mm_mullo_epi16(v, _mm_set1_epi16(2));
            __m128i vxor = _mm_xor_si128(vadd, vmul);
            
            /* Shuffle to create dependencies */
            vxor = _mm_shuffle_epi32(vxor, _MM_SHUFFLE(1, 0, 3, 2));
            
            vsum = _mm_add_epi32(vsum, vxor);
            
            /* Function call in loop */
            result += helper1(int_arr[i], i);
        }
        
        /* Extract vector result */
        int vres[4];
        _mm_storeu_si128((__m128i*)vres, vsum);
        result += vres[0] + vres[1] + vres[2] + vres[3];
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result % 1000;
}
