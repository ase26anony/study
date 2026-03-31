/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fsched-spec-load -fno-schedule-insns -fselective-scheduling2 -march=native -mtune=native */

#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining to force function call scheduling */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int barrier = 0;
    asm volatile ("" : "+r"(a), "+r"(b) : : "memory");
    return a * b + (barrier ? 0 : 1);
}

__attribute__((noinline)) float helper2(float a, float b) {
    volatile float barrier = 0.0f;
    asm volatile ("" : "+x"(a), "+x"(b) : : "memory");
    return a * b + (barrier ? 0.0f : 1.0f);
}

__attribute__((noinline)) void helper3(__m128i* dst, __m128i src) {
    volatile int barrier = 0;
    *dst = _mm_add_epi32(src, _mm_set1_epi32(barrier ? 0 : 1));
}

/* Force register constraints */
__attribute__((noinline)) int helper4(int a) {
    int result;
    /* Tie to specific registers */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl $1, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(result)
                  : "r"(a)
                  : "eax", "memory");
    return result;
}

int main() {
    volatile int v1 = 1, v2 = 2, v3 = 3;
    volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    int arr_int[256];
    float arr_float[256];
    __m128i vec_data[16];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        arr_int[i] = i;
        arr_float[i] = i * 0.5f;
    }
    
    int checksum = 0;
    
    /* Region 1: Complex integer operations with volatile barriers */
    {
        int a = v1;
        int b = v2;
        int c = v3;
        
        /* Create long dependency chain */
        a = a + b;
        asm volatile ("" : : : "memory");  /* Scheduling barrier */
        b = b ^ c;
        c = a * b;
        a = c >> 2;
        b = a & 0xFF;
        c = b | 0x80;
        a = c << 1;
        b = a + helper1(a, b);  /* Function call breaks scheduling */
        c = b - helper4(c);     /* Another function call */
        
        /* More operations to fill instruction queue */
        a = a * 3;
        b = b / 2;
        c = c % 7;
        a = a ^ b ^ c;
        b = (b << 3) | (c >> 5);
        c = helper1(a, b) + helper4(c);
        
        checksum += a + b + c;
    }
    
    /* Region 2: Mixed integer/float with SIMD */
    {
        float f = f1;
        int i = v1;
        
        for (int j = 0; j < 8; j++) {
            /* Data-dependent loop with mixed operations */
            f = f * f2 + helper2(f, f3);
            i = i + arr_int[j * 3] - helper1(i, j);
            
            /* SIMD operations */
            __m128i v = _mm_set_epi32(i, j, (int)f, checksum);
            v = _mm_add_epi32(v, _mm_set1_epi32(1));
            v = _mm_slli_epi32(v, 2);
            vec_data[j] = v;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            /* Conditional break to create complex control flow */
            if (j > 3 && (i & 0x1)) {
                i = helper4(i);
                break;
            }
        }
        
        checksum += (int)f + i;
    }
    
    /* Region 3: Switch with multiple cases creating different scheduling contexts */
    {
        int selector = checksum & 0x3;
        
        switch (selector) {
            case 0: {
                /* Case with many independent instructions */
                int t1 = arr_int[0] + 1;
                int t2 = arr_int[1] * 2;
                int t3 = arr_int[2] & 0xF;
                int t4 = arr_int[3] | 0x10;
                int t5 = arr_int[4] ^ 0x20;
                int t6 = arr_int[5] << 1;
                int t7 = arr_int[6] >> 2;
                int t8 = helper1(t1, t2);
                int t9 = helper4(t3);
                
                /* All can execute in parallel */
                checksum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
                break;
            }
            
            case 1: {
                /* Case with vector operations */
                __m128i vsum = _mm_setzero_si128();
                for (int i = 0; i < 4; i++) {
                    __m128i v = vec_data[i];
                    v = _mm_add_epi32(v, _mm_set1_epi32(i));
                    v = _mm_mullo_epi16(v, v);  /* Simulate more complex op */
                    vsum = _mm_add_epi32(vsum, v);
                    helper3(&vec_data[i + 4], v);
                }
                
                /* Extract results */
                int results[4];
                _mm_storeu_si128((__m128i*)results, vsum);
                checksum += results[0] + results[1] + results[2] + results[3];
                break;
            }
            
            case 2: {
                /* Case with pointer arithmetic and memory ops */
                volatile int* p = &arr_int[16];
                for (int i = 0; i < 8; i++) {
                    *p = *p + i;
                    p++;
                    asm volatile ("" : : : "memory");
                }
                checksum += arr_int[16];
                break;
            }
            
            case 3: {
                /* Case with floating point and conversions */
                float sum = 0.0f;
                for (int i = 0; i < 16; i++) {
                    sum += arr_float[i] * helper2(arr_float[i], f1);
                    if (i > 8) {
                        sum -= helper2(sum, f2);
                    }
                }
                checksum += (int)sum;
                break;
            }
        }
    }
    
    /* Region 4: Nested loops with computed goto (simulated) */
    {
        int outer = 4;
        int inner = 8;
        
        while (outer-- > 0) {
            int* ptr = arr_int;
            float* fptr = arr_float;
            
            for (int i = 0; i < inner; i++) {
                /* Mix of operations */
                *ptr = helper1(*ptr, i);
                *fptr = helper2(*fptr, (float)i);
                
                /* SIMD every other iteration */
                if (i & 0x1) {
                    __m128i v = _mm_loadu_si128((__m128i*)ptr);
                    v = _mm_srai_epi32(v, 1);
                    _mm_storeu_si128((__m128i*)ptr, v);
                }
                
                ptr += 2;
                fptr += 2;
                
                /* Early exit condition */
                if (i > 3 && (*ptr & 0x1)) {
                    inner--;
                    break;
                }
            }
            
            checksum += arr_int[0] + (int)arr_float[0];
        }
    }
    
    /* Region 5: Profile-guided style with __builtin_expect */
    {
        int likely_value = __builtin_expect(checksum > 0, 1);
        int unlikely_value = __builtin_expect(checksum < 0, 0);
        
        if (likely_value) {
            /* Hot path with many instructions */
            for (int i = 0; i < 32; i += 2) {
                arr_int[i] = helper4(arr_int[i]);
                arr_int[i + 1] = helper1(arr_int[i], arr_int[i + 1]);
                
                /* Create instruction-level parallelism */
                int t1 = arr_int[i] * 3;
                int t2 = arr_int[i + 1] / 2;
                int t3 = t1 ^ t2;
                int t4 = t1 & t2;
                int t5 = t3 | t4;
                
                arr_float[i / 2] = helper2(arr_float[i / 2], (float)t5);
                
                /* Memory barrier every 4 iterations */
                if ((i & 0x7) == 0) {
                    asm volatile ("" : : : "memory");
                }
            }
        }
        
        if (unlikely_value) {
            /* Cold path - still executed but unlikely */
            checksum = helper4(checksum);
        }
        
        /* Final SIMD reduction */
        __m128i vsum = _mm_setzero_si128();
        for (int i = 0; i < 16; i += 4) {
            __m128i v = _mm_loadu_si128((__m128i*)&arr_int[i]);
            vsum = _mm_add_epi32(vsum, v);
        }
        
        int final[4];
        _mm_storeu_si128((__m128i*)final, vsum);
        checksum += final[0] + final[1] + final[2] + final[3];
    }
    
    return checksum & 0xFF;  /* Prevent overflow */
}
