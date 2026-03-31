/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a * b + (a ^ b);
    return result;
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float result = a * b - a / b;
    return result;
}

/* Function with mixed operations to create complex scheduling regions */
__attribute__((noinline)) int complex_region(int *arr, float *farr, volatile int *varr, 
                                            __m128i *vec_arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    /* Region 1: Mixed integer operations with volatile accesses */
    {
        int a = arr[0];
        volatile int b = varr[0];
        int c = a + b;
        int d = c * 2;
        int e = d >> 3;
        int f = e ^ 0xABCD;
        int g = f & 0xFF;
        int h = g | 0x100;
        int i = h << 2;
        int j = i - c;
        int k = j % 17;
        int l = k * d;
        int m = l / (e + 1);
        sum += m;
        
        /* Memory barrier to split scheduling region */
        asm volatile("" : : : "memory");
    }
    
    /* Region 2: SIMD operations with inline assembly */
    {
        __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
        __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
        __m128i v3 = _mm_add_epi32(v1, v2);
        __m128i v4 = _mm_mullo_epi32(v3, v1);
        
        /* Inline assembly with explicit clobbers */
        asm volatile(
            "movdqa %0, %%xmm0\n\t"
            "pslld $2, %%xmm0\n\t"
            "movdqa %%xmm0, %1"
            : "=m"(vec_arr[0])
            : "m"(v4)
            : "xmm0", "memory"
        );
        
        /* More integer operations to fill instruction queue */
        int x = arr[1];
        int y = x * x;
        int z = y + (x << 3);
        sum += z;
        
        /* Function call creates scheduling barrier */
        sum += helper1(x, y);
    }
    
    /* Region 3: Nested control flow with switch statement */
    {
        int selector = arr[2] & 0x7;
        
        switch(selector) {
            case 0: {
                int t1 = arr[3];
                int t2 = t1 * 3;
                int t3 = t2 >> 1;
                sum += t3;
                /* Fall through */
            }
            case 1: {
                float f1 = farr[0];
                float f2 = f1 * 2.5f;
                fsum += f2;
                int t4 = (int)f2;
                sum += t4;
                break;
            }
            case 2: {
                int t5 = arr[4];
                for (int i = 0; i < 3; i++) {
                    t5 = t5 * 2 + i;
                    if (t5 > 1000) break;
                }
                sum += t5;
                break;
            }
            default: {
                int t6 = arr[5];
                int t7 = t6 ^ 0xDEADBEEF;
                sum += t7;
                break;
            }
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Region 4: Loop with data-dependent exit condition */
    {
        int counter = arr[6];
        int acc = 0;
        
        while (counter > 0) {
            int val = arr[counter % n];
            acc += val;
            counter = (counter * 13 + 7) & 0xFF;
            if (acc > 10000) {
                /* Early exit creates scheduling complexity */
                break;
            }
        }
        sum += acc;
    }
    
    /* Region 5: Parallel independent operations */
    {
        int p1 = arr[7];
        int p2 = arr[8];
        int p3 = arr[9];
        int p4 = arr[10];
        
        /* Independent operations that can be scheduled in parallel */
        int r1 = p1 * p1;
        int r2 = p2 + p2;
        int r3 = p3 & 0xFF;
        int r4 = p4 | 0x80;
        int r5 = p1 ^ p2;
        int r6 = p3 << 2;
        int r7 = p4 >> 1;
        int r8 = p1 + p4;
        
        sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
        
        /* Architecture-specific builtins */
        sum += __builtin_popcount(p1);
        sum += __builtin_ctz(p2 | 1);
    }
    
    /* Region 6: Floating point and integer mix */
    {
        float fa = farr[1];
        float fb = farr[2];
        float fc = fa * fb;
        float fd = fc / (fa + 1.0f);
        
        /* Function call with floating point */
        fsum += helper2(fa, fb);
        
        /* Convert to integer operations */
        int ia = (int)fa;
        int ib = (int)fb;
        int ic = ia * ib;
        int id = ic + (int)fd;
        
        sum += id;
    }
    
    return sum + (int)fsum;
}

/* Another complex function to create interprocedural scheduling */
__attribute__((always_inline)) 
static inline int inline_helper(int x, int y) {
    /* Inline function with complex operations */
    int a = x + y;
    int b = x * y;
    int c = a ^ b;
    int d = c << 3;
    int e = d >> 1;
    int f = e & 0x7F;
    int g = f | 0x80;
    int h = g * 2;
    int i = h - a;
    int j = i % 17;
    return j;
}

/* Main function with multiple scheduling regions */
int main() {
    /* Initialize arrays with different data types */
    int int_arr[256];
    float float_arr[256];
    volatile int volatile_arr[256];
    __m128i vec_arr[64];
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = i * 3 + 7;
        float_arr[i] = i * 0.5f;
        volatile_arr[i] = i * 2;
    }
    
    for (int i = 0; i < 64; i++) {
        vec_arr[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    int total_sum = 0;
    
    /* Execute multiple complex regions to increase scheduling state usage */
    
    /* Region A */
    total_sum += complex_region(int_arr, float_arr, volatile_arr, vec_arr, 256);
    
    /* Region B with different patterns */
    {
        int local_sum = 0;
        
        /* Use profile-guided optimization hints */
        if (__builtin_expect(int_arr[0] > 100, 0)) {
            /* Unlikely path */
            for (int i = 0; i < 50; i++) {
                local_sum += inline_helper(int_arr[i], int_arr[i+1]);
            }
        } else {
            /* Likely path with more operations */
            for (int i = 0; i < 100; i++) {
                local_sum += inline_helper(int_arr[i], int_arr[255-i]);
            }
        }
        
        /* More operations with volatile */
        volatile int v = volatile_arr[10];
        for (int i = 0; i < 20; i++) {
            v = v * 3 + 1;
            volatile_arr[i] = v;
        }
        
        total_sum += local_sum + v;
    }
    
    /* Region C: Switch with computed goto-like behavior */
    {
        int selector = total_sum & 0x3;
        int case_result = 0;
        
        switch(selector) {
            case 0:
                case_result = int_arr[selector] + volatile_arr[selector];
                /* Fall through */
            case 1:
                case_result *= 2;
                /* Fall through */
            case 2:
                case_result ^= 0x1234;
                break;
            case 3:
                case_result = int_arr[selector] - volatile_arr[selector];
                break;
        }
        
        total_sum += case_result;
    }
    
    /* Region D: Nested loops with complex exit conditions */
    {
        int outer_acc = 0;
        for (int i = 0; i < 10; i++) {
            int inner_acc = 0;
            int j = 0;
            while (j < 20) {
                inner_acc += int_arr[i * 20 + j];
                j++;
                if (inner_acc > 1000) {
                    /* Complex exit condition */
                    break;
                }
            }
            outer_acc += inner_acc;
            
            /* Memory barrier every few iterations */
            if (i % 3 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        total_sum += outer_acc;
    }
    
    /* Final computation with SIMD */
    {
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 16; i++) {
            vsum = _mm_add_epi32(vsum, vec_arr[i]);
        }
        
        /* Extract results */
        int vresult[4];
        _mm_storeu_si128((__m128i*)vresult, vsum);
        total_sum += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = total_sum;
    
    return final_result % 255;
}
