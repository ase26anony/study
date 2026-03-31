/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force no-inline to create scheduling barriers */
__attribute__((noinline)) static int helper1(int a, int b) {
    volatile int result = a + b;
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

__attribute__((noinline)) static float helper2(float a, float b) {
    volatile float result = a * b;
    asm volatile("" : "+x"(result) : : "memory");
    return result;
}

__attribute__((noinline)) static void helper3(int* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + 1;
    }
    asm volatile("" : : : "memory");
}

/* Always inline to create complex basic blocks */
__attribute__((always_inline)) static inline int complex_calculation(int a, int b, int c) {
    int t1 = a * b + c;
    int t2 = (t1 >> 3) & 0xFF;
    int t3 = __builtin_popcount(t2);
    int t4 = t3 * 7 - 13;
    
    /* Volatile access to prevent optimization */
    volatile int* p = &t4;
    *p = *p + 1;
    
    /* Inline assembly with clobbers */
    asm volatile (
        "addl $5, %0\n\t"
        "rorl $3, %0"
        : "+r"(t4)
        : 
        : "cc", "memory"
    );
    
    return t4;
}

int main(void) {
    int checksum = 0;
    volatile int v1 = 1, v2 = 2, v3 = 3;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    
    /* Array for various operations */
    int arr1[256];
    float arr2[256];
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = i * 0.5f;
    }
    
    /* REGION 1: Mixed operations with volatile and inline asm */
    {
        int a = v1 + v2;
        int b = a * v3;
        int c = b >> 2;
        int d = c & 0xFF;
        
        /* Force scheduling barrier */
        asm volatile("" : : : "memory");
        
        float e = f1 * f2;
        float f = e + f3;
        int g = (int)f;
        
        /* Vector operations (SIMD) */
        __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        
        int res[4];
        _mm_storeu_si128((__m128i*)res, vec3);
        
        checksum += a + b + c + d + g + res[0] + res[1] + res[2] + res[3];
    }
    
    /* REGION 2: Nested loops with data-dependent exits */
    for (int outer = 0; outer < 10; outer++) {
        int inner = 0;
        while (1) {
            int val = arr1[inner] * 3;
            arr1[inner] = val - 7;
            
            /* Complex calculation with function call */
            val = complex_calculation(val, outer, inner);
            
            /* Volatile write */
            volatile int* volatile_ptr = &arr1[inner];
            *volatile_ptr = *volatile_ptr ^ 0x55AA55AA;
            
            inner++;
            if (inner >= 50 || val > 1000) {
                checksum += val;
                break;
            }
        }
        
        /* Call noinline helper */
        checksum += helper1(outer, inner);
    }
    
    /* REGION 3: Switch statement with multiple cases */
    int switch_var = checksum & 0x7;
    switch (switch_var) {
        case 0: {
            /* Many independent instructions */
            int t1 = v1 * 2;
            int t2 = v2 + 5;
            int t3 = t1 | t2;
            int t4 = t3 << 1;
            int t5 = ~t4;
            float t6 = f1 / f2;
            int t7 = (int)(t6 * 100.0f);
            int t8 = __builtin_ctz(t7 | 1);
            
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull $77, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r"(t8)
                : "r"(t8)
                : "%eax", "memory"
            );
            
            checksum += t1 + t2 + t3 + t4 + t5 + t7 + t8;
            break;
        }
        
        case 1:
        case 2: {
            /* Vector operations with different types */
            __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 vec_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
            __m128 vec_c = _mm_add_ps(vec_a, vec_b);
            __m128 vec_d = _mm_mul_ps(vec_c, vec_a);
            
            float farr[4];
            _mm_storeu_ps(farr, vec_d);
            
            checksum += (int)farr[0] + (int)farr[1] + (int)farr[2] + (int)farr[3];
            
            /* Fall through */
            __attribute__((fallthrough));
            
        case 3:
            checksum += helper2(f1, f2);
            break;
            
        default:
            /* Complex pointer arithmetic */
            int* ptr = arr1;
            for (int i = 0; i < 32; i++) {
                *ptr = (*ptr * 3 + 1) & 0xFFFF;
                ptr++;
                
                /* Memory barrier every 8 iterations */
                if ((i & 7) == 7) {
                    asm volatile("" : : : "memory");
                }
            }
            helper3(arr1, 32);
            checksum += arr1[0];
    }
    
    /* REGION 4: Computed goto for complex control flow */
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    int label_idx = checksum % 5;
    
    goto *labels[label_idx];
    
    L0: {
        /* Many parallel independent operations */
        int p1 = v1 + 1;
        int p2 = v2 * 2;
        int p3 = v3 >> 1;
        int p4 = p1 ^ p2;
        int p5 = p3 & p4;
        int p6 = __builtin_popcount(p5);
        float p7 = f1 + f2;
        float p8 = p7 * f3;
        int p9 = (int)p8;
        
        checksum += p1 + p2 + p3 + p4 + p5 + p6 + p9;
        goto END;
    }
    
    L1: {
        /* Nested arithmetic with volatile */
        volatile int counter = 0;
        for (int i = 0; i < 20; i++) {
            counter = counter + arr1[i] * 3;
            if (counter > 1000) {
                asm volatile("" : : : "memory");
                counter = counter / 2;
            }
        }
        checksum += counter;
        goto END;
    }
    
    L2:
    L3: {
        /* Mixed integer/float with function calls */
        checksum += helper1(v1, v2);
        checksum += (int)helper2(f2, f3);
        goto END;
    }
    
    L4: {
        /* SIMD operations */
        __m128i simd1 = _mm_set_epi8(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
        __m128i simd2 = _mm_set1_epi8(5);
        __m128i simd3 = _mm_add_epi8(simd1, simd2);
        
        char simd_res[16];
        _mm_storeu_si128((__m128i*)simd_res, simd3);
        
        for (int i = 0; i < 16; i++) {
            checksum += simd_res[i];
        }
        goto END;
    }
    
    END:
    
    /* REGION 5: Final complex region with profile hints */
    for (int i = 0; i < 100; i++) {
        if (__builtin_expect(i < 90, 1)) {
            /* Hot path - many operations */
            arr1[i] = arr1[i] * 3 + 7;
            arr2[i] = arr2[i] * 1.1f - 0.5f;
            
            int tmp = arr1[i];
            tmp = (tmp << 3) | (tmp >> 29);
            tmp = tmp ^ 0xDEADBEEF;
            
            checksum += tmp + (int)arr2[i];
        } else {
            /* Cold path - still complex */
            volatile int cold_var = i * 7;
            for (int j = 0; j < 5; j++) {
                cold_var = (cold_var * 11 + 13) & 0x7FFF;
            }
            
            asm volatile (
                "movl %1, %%ecx\n\t"
                "roll $5, %%ecx\n\t"
                "movl %%ecx, %0\n\t"
                : "=r"(cold_var)
                : "r"(cold_var)
                : "%ecx"
            );
            
            checksum += cold_var;
        }
        
        /* Memory barrier every 25 iterations */
        if ((i % 25) == 24) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
