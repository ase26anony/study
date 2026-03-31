/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -march=native -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Volatile globals to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 3.14f;
volatile int* g_volatile_ptr = (int*)0x1000;

/* Noinline functions to create scheduling barriers */
__attribute__((noinline)) int noinline_func1(int a, int b) {
    volatile int local = a * b;
    asm volatile ("" : "+r" (local) : : "memory");
    return local + (b >> 2);
}

__attribute__((noinline)) float noinline_func2(float x, float y) {
    volatile float result = x * y;
    asm volatile ("" : "+x" (result) : : "memory");
    return result - y;
}

__attribute__((noinline)) void noinline_func3(__m128i* vec) {
    volatile __m128i temp = *vec;
    asm volatile ("" : "+x" (temp) : : "memory");
    *vec = _mm_add_epi32(temp, _mm_set1_epi32(1));
}

/* Complex function with multiple scheduling regions */
int complex_scheduling_region(int seed) {
    int result = seed;
    float f_result = (float)seed;
    int* volatile mem_ptr = (int*)&g_volatile_counter;
    
    /* Region 1: Mixed integer operations with volatile accesses */
    {
        int a = result + 1;
        volatile int b = a * 2;
        int c = b >> 3;
        *mem_ptr = *mem_ptr + c;
        asm volatile ("nop; nop" : : : "memory", "eax", "ebx");
        result = noinline_func1(a, c);
        
        /* Create instruction queue pressure */
        int d = result ^ 0x55AA55AA;
        int e = d * 3;
        int f = e & 0x00FF00FF;
        volatile int g = f | 0xFF000000;
        result = g + (c << 2);
    }
    
    /* Region 2: SIMD/vector operations */
    {
        __m128i vec1 = _mm_set_epi32(result, result+1, result+2, result+3);
        __m128i vec2 = _mm_set_epi32(1, 2, 3, 4);
        
        /* Multiple SIMD operations */
        __m128i sum = _mm_add_epi32(vec1, vec2);
        __m128i mul = _mm_mullo_epi16(sum, _mm_set1_epi16(2));
        __m128i shift = _mm_slli_epi32(mul, 1);
        
        /* Force scheduling context with inline assembly */
        asm volatile (
            "movdqa %0, %%xmm0\n\t"
            "paddd %%xmm0, %%xmm0\n\t"
            "movdqa %%xmm0, %0"
            : "+x" (shift)
            :
            : "xmm0", "memory"
        );
        
        /* Extract results */
        int vals[4];
        _mm_storeu_si128((__m128i*)vals, shift);
        result += vals[0] + vals[1] + vals[2] + vals[3];
        
        /* Call noinline function with SIMD */
        noinline_func3(&shift);
    }
    
    /* Region 3: Floating point with volatile */
    {
        float a = f_result * 2.0f;
        volatile float b = a + g_volatile_float;
        float c = b / 1.5f;
        
        /* Mixed precision operations */
        double d = (double)c * 3.14159;
        volatile double e = d - 2.71828;
        float f = (float)e;
        
        /* Inline assembly with FP clobbers */
        asm volatile (
            "flds %0\n\t"
            "fadd %%st(0), %%st(0)\n\t"
            "fstps %0"
            : "+m" (f)
            :
            : "st", "memory"
        );
        
        f_result = noinline_func2(f, c);
        result += (int)f_result;
    }
    
    /* Region 4: Complex control flow with switch */
    switch (result & 0x7) {
        case 0: {
            /* Long chain of dependent operations */
            int x = result;
            for (int i = 0; i < 8; i++) {
                x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
                volatile int y = x >> 16;
                x = y ^ (x & 0xFFFF);
                asm volatile ("" : "+r" (x) : : "memory");
            }
            result = x;
            break;
        }
        case 1:
        case 2: {
            /* Nested loops with data-dependent exit */
            int sum = 0;
            for (int i = 0; i < 10; i++) {
                int inner = i * 2;
                while (inner > 0) {
                    sum += inner;
                    inner -= (result & 1) + 1;
                    volatile int barrier = inner;
                    asm volatile ("" : : "r" (barrier) : "memory");
                }
            }
            result += sum;
            break;
        }
        case 3:
        case 4:
        case 5: {
            /* Many independent instructions */
            int a = result * 2;
            int b = result + 3;
            int c = result ^ 0x12345678;
            int d = result >> 4;
            int e = result << 1;
            volatile int f = a + b;
            volatile int g = c - d;
            volatile int h = e | f;
            volatile int i = g & h;
            volatile int j = i * 3;
            result = a + b + c + d + e + f + g + h + i + j;
            break;
        }
        default: {
            /* Memory intensive region */
            int arr[16];
            for (int i = 0; i < 16; i++) {
                arr[i] = result + i;
                volatile int* p = &arr[i];
                *p = *p * 2;
                asm volatile ("" : : "r" (*p) : "memory");
            }
            
            /* Pointer chasing */
            int* ptr = arr;
            for (int i = 0; i < 8; i++) {
                ptr = arr + (*ptr & 0xF);
                volatile int val = *ptr;
                result += val;
            }
            break;
        }
    }
    
    /* Region 5: Mixed operations in tight loop */
    {
        __m128i simd_acc = _mm_setzero_si128();
        float fp_acc = 0.0f;
        
        for (int i = 0; i < 4; i++) {
            /* Integer SIMD */
            __m128i vec = _mm_set_epi32(i, i+1, i+2, i+3);
            simd_acc = _mm_add_epi32(simd_acc, vec);
            
            /* Floating point */
            fp_acc += (float)i * 0.5f;
            
            /* Volatile memory op */
            volatile float temp = fp_acc;
            g_volatile_float = temp;
            
            /* Inline assembly barrier */
            asm volatile ("mfence" : : : "memory");
            
            /* Function call */
            result += noinline_func1(result, i);
        }
        
        /* Extract SIMD result */
        int simd_vals[4];
        _mm_storeu_si128((__m128i*)simd_vals, simd_acc);
        result += simd_vals[0] + simd_vals[1] + simd_vals[2] + simd_vals[3];
        result += (int)fp_acc;
    }
    
    return result;
}

/* Main function with multiple complex regions */
int main() {
    int final_result = 0;
    
    /* Create multiple scheduling contexts */
    for (int region = 0; region < 5; region++) {
        /* Each iteration creates new scheduling context */
        int seed = final_result + region * 100;
        
        /* Profile-guided optimization hint */
        if (__builtin_expect(region < 3, 1)) {
            /* Likely path with complex operations */
            final_result ^= complex_scheduling_region(seed);
        } else {
            /* Unlikely path but still complex */
            final_result += complex_scheduling_region(seed * 2);
        }
        
        /* Memory barrier between regions */
        asm volatile ("" : : : "memory");
        
        /* Volatile access to prevent optimization */
        g_volatile_counter++;
    }
    
    /* Final mixed operations */
    {
        /* SIMD horizontal sum */
        __m128i final_vec = _mm_set_epi32(final_result, final_result>>1, 
                                         final_result>>2, final_result>>3);
        __m128i sum = _mm_hadd_epi32(final_vec, final_vec);
        sum = _mm_hadd_epi32(sum, sum);
        int simd_sum;
        _mm_store_ss((float*)&simd_sum, _mm_castsi128_ps(sum));
        
        /* Complex integer chain */
        int a = final_result;
        int b = a * 3 + 1;
        volatile int c = b ^ 0xDEADBEEF;
        int d = c >> 4;
        int e = d * 7;
        volatile int f = e & 0x0F0F0F0F;
        
        /* Inline assembly with multiple clobbers */
        asm volatile (
            "imull %%ecx, %%edx\n\t"
            "rorl $4, %%edx\n\t"
            "addl %%edx, %%eax"
            : "+a" (final_result)
            : "c" (f), "d" (simd_sum)
            : "cc", "memory"
        );
    }
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    /* Use builtins that may have special scheduling */
    final_result = __builtin_popcount(final_result) + 
                   __builtin_ctz(final_result | 1);
    
    return final_result;
}
