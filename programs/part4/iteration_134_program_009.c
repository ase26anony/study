/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* arr, int start, int end) {
    float sum = 0.0f;
    /* Mix scalar and SIMD operations */
    for (int i = start; i + 3 < end; i += 4) {
        __m128 vec = _mm_loadu_ps(&arr[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        float tmp[4];
        _mm_storeu_ps(tmp, squared);
        sum += tmp[0] + tmp[1] + tmp[2] + tmp[3];
        
        /* Inline asm barrier to force scheduler partitioning */
        asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
    }
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size) {
    int result = 0;
    /* Complex loop with carried dependency */
    for (int i = 0; i < size; i++) {
        result = (result * 1103515245 + 12345) ^ arr[i];
        /* Conditional branch inside loop */
        if (result < 0) {
            result = -result;
            /* Another asm barrier */
            asm volatile ("" ::: "cc", "memory");
        }
    }
    return result;
}

/* Main computation with nested loops and complex control flow */
int main(void) {
    float farr[SIZE];
    int iarr[SIZE];
    double darr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        farr[i] = (i % 100) * 0.1f;
        iarr[i] = i * 3;
        darr[i] = (i % 50) * 0.2;
    }
    
    float fsum = 0.0f;
    int isum = 0;
    double dsum = 0.0;
    
    /* Nested loops (3 levels) with dependencies */
    for (int outer = 0; outer < 10; outer++) {
        for (int mid = 0; mid < 20; mid++) {
            for (int inner = 0; inner < 15; inner++) {
                /* Loop-carried dependencies */
                fsum += farr[(outer + mid + inner) % SIZE] * 0.5f;
                isum += iarr[(inner * 7) % SIZE];
                
                /* Conditional with mixed operations */
                if ((outer * mid + inner) % 7 == 0) {
                    dsum += darr[inner % SIZE];
                    /* Call noinline function */
                    fsum += process_chunk(farr, inner, inner + 16);
                } else if ((outer + mid) % 3 == 0) {
                    /* Another asm barrier */
                    asm volatile ("" ::: "memory", "esi", "edi");
                    isum -= iarr[mid % SIZE];
                }
                
                /* Additional floating point operation chain */
                float temp = farr[inner % SIZE];
                for (int k = 0; k < 3; k++) {
                    temp = temp * 1.1f - 0.3f;
                }
                farr[(inner + 1) % SIZE] = temp;
            }
            
            /* Call integer reduction periodically */
            if (mid % 5 == 0) {
                isum ^= integer_reduction(iarr, 64);
            }
        }
        
        /* Goto-based control flow (irreducible) */
        if (outer == 5) {
            goto restart_point;
        }
        
        continue;
        
        restart_point:
        /* Modify array to create different data pattern */
        for (int i = 0; i < SIZE; i += 2) {
            iarr[i] = (iarr[i] << 1) | 1;
        }
    }
    
    /* Large switch statement with non-linear cases */
    int switch_val = (isum ^ (int)fsum) % SWITCH_CASES;
    switch (switch_val) {
        case 0:  fsum *= 1.1f; break;
        case 1:  isum += 1000; break;
        case 3:  dsum -= 50.0; break;  /* Skip case 2 */
        case 5:  fsum = -fsum; break;
        case 7:  isum = integer_reduction(iarr, 128); break;
        case 11: dsum *= 2.0; break;   /* Skip several cases */
        case 13: 
            /* Complex case with SIMD */
            __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
            __m128 v3 = _mm_add_ps(v1, v2);
            float ftmp[4];
            _mm_storeu_ps(ftmp, v3);
            fsum += ftmp[0] + ftmp[1] + ftmp[2] + ftmp[3];
            break;
        case 17: 
            /* Another asm barrier with many clobbers */
            asm volatile ("nop" ::: "memory", "eax", "ebx", "ecx", "edx", 
                         "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3");
            break;
        case 19: isum = (isum << 3) | (isum >> 29); break;
        case 23: 
            /* Call cold function (unlikely) */
            if (isum < 0) error_handler("Negative sum");
            break;
        case 4:  /* Fall through */
        case 8:  /* Fall through */
        case 12: /* Fall through */
        case 16: isum *= 2; break;
        default:
            /* Complex default case */
            for (int i = 0; i < 8; i++) {
                fsum += process_chunk(farr, i * 32, (i + 1) * 32);
                asm volatile ("" ::: "memory");
            }
            isum = integer_reduction(iarr, 256);
            break;
    }
    
    /* More goto-based control flow */
    int restart_count = 0;
    computation_restart:
    for (int i = 0; i < SIZE / 2; i++) {
        /* Data-dependent branching */
        if (iarr[i] % 2 == 0) {
            farr[i] = fsum * 0.01f;
            if (i % 3 == 0) {
                goto skip_point;
            }
        } else {
            iarr[i] = isum % 1000;
        }
        
        if (i == SIZE / 4 && restart_count == 0) {
            restart_count++;
            goto computation_restart;  /* Create irreducible flow */
        }
        
        skip_point:
        darr[i] += dsum * 0.001;
    }
    
    /* Final validation (simple checksum) */
    int final_check = (int)fsum + isum + (int)dsum;
    printf("Result: %d\n", final_check);
    
    /* Use results to prevent optimization */
    volatile float sink_f = fsum;
    volatile int sink_i = isum;
    volatile double sink_d = dsum;
    
    return (final_check != 0) ? 0 : 1;
}
