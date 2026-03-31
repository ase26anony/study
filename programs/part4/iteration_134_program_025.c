/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __SSE__
#include <xmmintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    float sum = 0.0f;
    for (int i = start; i < end; i++) {
        sum += data[i] * 0.5f;
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result ^= arr[i] * (i % 7);
    }
    return result;
}

/* Requirement 3: SIMD operations */
__attribute__((noinline))
static void simd_processing(float* dest, const float* src, int n) {
#ifdef __SSE__
    for (int i = 0; i < n; i += 4) {
        __m128 vec = _mm_loadu_ps(&src[i]);
        __m128 half = _mm_set1_ps(0.5f);
        vec = _mm_mul_ps(vec, half);
        _mm_storeu_ps(&dest[i], vec);
    }
#elif defined(__ARM_NEON)
    for (int i = 0; i < n; i += 4) {
        float32x4_t vec = vld1q_f32(&src[i]);
        float32x4_t half = vdupq_n_f32(0.5f);
        vec = vmulq_f32(vec, half);
        vst1q_f32(&dest[i], vec);
    }
#else
    /* Fallback scalar version */
    for (int i = 0; i < n; i++) {
        dest[i] = src[i] * 0.5f;
    }
#endif
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1: result = iarr[0] + 1; break;
        case 3: result = iarr[1] * 2; break;
        case 7: result = iarr[2] / 3; break;
        case 11: result = iarr[3] | 0xFF; break;
        case 13: result = iarr[4] & 0x0F; break;
        case 17: result = iarr[5] ^ iarr[6]; break;
        case 19: result = (int)(farr[0] * 10.0f); break;
        case 23: result = (int)(farr[1] + farr[2]); break;
        case 29: result = iarr[7] << 2; break;
        case 31: result = iarr[8] >> 1; break;
        case 37: result = iarr[9] % 17; break;
        case 41: result = -iarr[10]; break;
        case 43: result = ~iarr[11]; break;
        case 47: result = iarr[12] + iarr[13]; break;
        case 53: result = iarr[14] - iarr[15]; break;
        case 59: result = iarr[16] * iarr[17]; break;
        case 61: result = abs(iarr[18]); break;
        case 67: result = iarr[19] * 3 + 7; break;
        case 71: result = (iarr[20] << 3) | 0x0F; break;
        case 73: result = iarr[21] & iarr[22]; break;
        case 79: result = iarr[23] | iarr[24]; break;
        case 83: result = iarr[25] ^ 0xAA; break;
        case 89: result = iarr[26] + 100; break;
        case 97: result = iarr[27] - 50; break;
        default: /* Requirement 5: complex default case */
            result = complex_reduction(iarr, 28);
            for (int i = 0; i < 10; i++) {
                result += (int)(farr[i] * 2.0f);
            }
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i % 100) * 0.1f;
        iarr[i] = i * 3 + 7;
    }
    
    float fsum = 0.0f;
    int isum = 0;
    int restart_count = 0;
    
restart_point: /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with complex dependencies */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 20; k++) {
                /* Loop-carried dependencies */
                fsum += farr[(i + j + k) % ARRAY_SIZE];
                isum ^= iarr[(i * j + k) % ARRAY_SIZE];
                
                /* Conditional branch inside innermost loop */
                if ((i * j * k) % 7 == 0) {
                    fsum *= 0.99f;
                    isum += 1;
                } else if ((i + j + k) % 11 == 0) {
                    fsum /= 1.01f;
                    isum -= 1;
                }
                
                /* Mixed integer and floating-point operations */
                float temp = sinf(fsum * 0.01f);
                fsum += temp * 0.1f;
                isum += (int)(temp * 100.0f);
                
                /* Requirement 4: Inline assembly with clobbers */
                asm volatile (
                    "nop\n\t"
                    "nop\n\t"
                    : 
                    : 
                    : "memory", "cc"
                );
            }
            
            /* Call noinline functions */
            if (j % 5 == 0) {
                float chunk_sum = process_chunk(farr, j * 10, (j + 1) * 10);
                fsum += chunk_sum;
                
                /* Requirement 4: More inline assembly */
                asm volatile (
                    "mov $0, %%eax\n\t"
                    "add $1, %%eax\n\t"
                    : 
                    : 
                    : "eax", "cc"
                );
            }
        }
        
        /* SIMD processing */
        if (i % 3 == 0) {
            float farr2[ARRAY_SIZE];
            simd_processing(farr2, farr, ARRAY_SIZE);
            
            for (int m = 0; m < 100; m++) {
                fsum += farr2[m % ARRAY_SIZE];
            }
        }
        
        /* Requirement 5: Switch statement */
        int switch_val = (i * 17 + 23) % 100;
        int switch_result = process_switch(switch_val, farr, iarr);
        isum += switch_result;
        
        /* Requirement 6: goto for restart mechanism */
        if (restart_count < 2 && i == 25 && (isum % 1000) < 10) {
            restart_count++;
            
            /* Requirement 4: Assembly barrier */
            asm volatile (
                "mfence\n\t"
                : 
                : 
                : "memory"
            );
            
            goto restart_point;
        }
    }
    
    /* More complex control flow with goto */
    if (isum < 0) {
        goto negative_path;
    }
    
    /* Final processing */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        farr[i] = sqrtf(fabsf(farr[i]));
        iarr[i] = (iarr[i] * 1103515245 + 12345) & 0x7fffffff;
    }
    
    goto finish;
    
negative_path:
    error_handler("Negative sum detected");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = -farr[i];
        iarr[i] = -iarr[i];
    }
    
finish:
    /* Validate results */
    float final_fsum = 0.0f;
    int final_isum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_fsum += farr[i];
        final_isum ^= iarr[i];
    }
    
    printf("Final fsum: %f\n", final_fsum);
    printf("Final isum: %d\n", final_isum);
    printf("Restart count: %d\n", restart_count);
    
    /* Simple validation */
    if (!isnan(final_fsum) && final_isum != 0) {
        printf("Test completed successfully\n");
    } else {
        printf("Test completed with unusual results\n");
    }
    
    return 0;
}
