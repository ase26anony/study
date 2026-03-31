/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* SIMD headers for different architectures */
#if defined(__SSE__)
#include <xmmintrin.h>
#elif defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Global arrays to create dependencies */
static int int_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path - rarely taken */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += i;
    }
    (void)msg;
}

__attribute__((noinline))
static void process_with_simd(float* arr, int size) {
#if defined(__SSE__)
    /* SSE SIMD operations */
    for (int i = 0; i < size; i += 4) {
        __m128 a = _mm_loadu_ps(&arr[i]);
        __m128 b = _mm_set1_ps(1.5f);
        __m128 result = _mm_mul_ps(a, b);
        
        /* Inline asm barrier to force scheduling boundaries */
        asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        
        _mm_storeu_ps(&arr[i], result);
    }
#elif defined(__ARM_NEON)
    /* ARM NEON SIMD operations */
    for (int i = 0; i < size; i += 4) {
        float32x4_t a = vld1q_f32(&arr[i]);
        float32x4_t b = vdupq_n_f32(1.5f);
        float32x4_t result = vmulq_f32(a, b);
        
        /* Inline asm barrier */
        asm volatile("" ::: "memory", "q0", "q1", "q2", "q3");
        
        vst1q_f32(&arr[i], result);
    }
#else
    /* Scalar fallback */
    for (int i = 0; i < size; i++) {
        arr[i] *= 1.5f;
    }
#endif
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int sum = 0;
    int prod = 1;
    
    /* Nested loops with dependencies */
    for (int i = 0; i < size; i++) {
        /* Outer loop with conditional */
        if (arr[i] > 0) {
            for (int j = 0; j < 8; j++) {
                /* Middle loop */
                int temp = arr[i] + j;
                for (int k = 0; k < 4; k++) {
                    /* Innermost loop with mixed operations */
                    sum += temp * k;
                    prod *= (temp > k) ? temp : 1;
                    
                    /* Conditional branch inside innermost loop */
                    if ((temp * k) % 7 == 0) {
                        sum -= k;
                    }
                }
            }
        }
    }
    
    /* Another asm barrier */
    asm volatile("" ::: "memory", "eax", "ebx", "ecx", "edx");
    
    return sum + prod;
}

/* Function with large switch statement */
__attribute__((noinline))
static int process_switch(int value, int* arr, float* farr) {
    int result = 0;
    
    /* Dense, non-linear switch cases */
    switch (value % SWITCH_CASES) {
        case 0:
            result = arr[0] + (int)farr[0];
            asm volatile("" ::: "memory");
            break;
        case 1:
            result = arr[1] * 2;
            farr[1] += 1.0f;
            break;
        case 3:  /* Skip 2 to make it non-linear */
            result = arr[3] - arr[2];
            break;
        case 7:
            result = (int)(sinf(farr[7]) * 100);
            break;
        case 11:
            result = arr[11] ^ arr[12];
            break;
        case 13:
            result = arr[13] % 17;
            break;
        case 17:
            result = (int)(farr[17] * farr[18]);
            break;
        case 19:
            result = arr[19] << 2;
            break;
        case 23:
            result = arr[23] >> 1;
            break;
        case 29 % SWITCH_CASES:  /* 4 */
            result = arr[4] + arr[5] + arr[6];
            break;
        case 31 % SWITCH_CASES:  /* 6 */
            result = (int)(cosf(farr[6]) * 50);
            break;
        case 37 % SWITCH_CASES:  /* 12 */
            result = arr[12] | arr[13];
            break;
        case 41 % SWITCH_CASES:  /* 16 */
            result = arr[16] & 0xFF;
            break;
        case 43 % SWITCH_CASES:  /* 18 */
            result = ~arr[18];
            break;
        case 47 % SWITCH_CASES:  /* 22 */
            result = arr[22] * arr[23];
            break;
        case 53 % SWITCH_CASES:  /* 3 (already used) */
            result = arr[3] * 3;  /* Different from case 3 */
            break;
        case 59 % SWITCH_CASES:  /* 9 */
            result = arr[9] / 2;
            break;
        case 61 % SWITCH_CASES:  /* 11 (already used) */
            result = arr[11] + 100;  /* Different from case 11 */
            break;
        case 67 % SWITCH_CASES:  /* 17 (already used) */
            result = (int)(sqrtf(farr[17]) * 10);
            break;
        case 71 % SWITCH_CASES:  /* 21 */
            result = arr[21] * arr[21];
            break;
        case 73 % SWITCH_CASES:  /* 23 (already used) */
            result = arr[23] * 5;  /* Different from case 23 */
            break;
        case 79 % SWITCH_CASES:  /* 4 (already used) */
            result = arr[4] - arr[5];  /* Different from case 4 */
            break;
        case 83 % SWITCH_CASES:  /* 8 */
            result = arr[8] << 3;
            break;
        case 89 % SWITCH_CASES:  /* 14 */
            result = arr[14] ^ 0xAAAA;
            break;
        case 97 % SWITCH_CASES:  /* 22 (already used) */
            result = arr[22] + arr[24];
            break;
        default:
            /* Complex default case */
            result = complex_reduction(arr, 32);
            for (int i = 0; i < 16; i++) {
                farr[i] = farr[i] * 0.5f + farr[i + 16];
            }
            asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 37) % 101;
        float_array[i] = (float)(i * 0.12345);
        double_array[i] = (double)(i * 0.6789);
    }
    
    int total_sum = 0;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with complex control flow */
    for (int outer = 0; outer < 5; outer++) {
        if (restart_count > 2) {
            /* Rare cold path */
            error_handler("Too many restarts");
            break;
        }
        
        for (int middle = 0; middle < 10; middle++) {
            int local_sum = 0;
            
            for (int inner = 0; inner < ARRAY_SIZE / 8; inner++) {
                int idx = (outer * 200 + middle * 20 + inner) % ARRAY_SIZE;
                
                /* Loop-carried dependencies */
                static int prev_value = 0;
                int current = int_array[idx];
                int_array[idx] = current + prev_value;
                prev_value = current;
                
                /* Mixed integer/float operations */
                float_array[idx] = float_array[idx] * 1.1f + (float)current;
                double_array[idx] = double_array[idx] * 0.9 + (double)current;
                
                /* Conditional with goto to create irreducible flow */
                if ((outer > 2) && (middle % 7 == 0) && (inner == 5)) {
                    restart_count++;
                    if (restart_count < 3) {
                        /* Jump to outer scope */
                        goto restart_point;
                    }
                }
                
                /* Another conditional with continue to outer loop */
                if ((current % 13 == 0) && (inner > 10)) {
                    continue;  /* Continues inner loop */
                }
                
                /* Break to middle loop under specific condition */
                if ((current % 29 == 0) && (inner > 20)) {
                    break;  /* Breaks inner loop */
                }
                
                local_sum += current;
            }
            
            /* Call SIMD function */
            process_with_simd(float_array, ARRAY_SIZE);
            
            /* Inline asm with clobbers */
            asm volatile(
                "movl %%eax, %%ebx\n\t"
                "addl $1, %%ebx"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ebx", "cc"
            );
            
            total_sum += local_sum;
        }
        
        /* Process switch statement */
        int switch_val = total_sum % 1000;
        int switch_result = process_switch(switch_val, int_array, float_array);
        total_sum += switch_result;
        
        /* Call reduction function */
        total_sum += complex_reduction(int_array, ARRAY_SIZE / 4);
    }
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i] % 256;
        checksum += (int)float_array[i] % 256;
    }
    
    printf("Result: total_sum = %d, checksum = %d\n", total_sum, checksum);
    
    /* Simple validation */
    if (checksum != 0) {
        printf("Test completed successfully (non-zero checksum)\n");
        return 0;
    } else {
        printf("Warning: checksum is zero\n");
        return 1;
    }
}
