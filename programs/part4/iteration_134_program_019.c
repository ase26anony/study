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
__attribute__((noinline)) __attribute__((cold)) 
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float compute_reduction(float* data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += data[i] * data[i];
    }
    return sum;
}

__attribute__((noinline))
static void process_with_simd(float* dest, const float* src, int n) {
#ifdef __SSE__
    for (int i = 0; i < n; i += 4) {
        __m128 v1 = _mm_loadu_ps(&src[i]);
        __m128 v2 = _mm_set1_ps(2.5f);
        __m128 result = _mm_mul_ps(v1, v2);
        _mm_storeu_ps(&dest[i], result);
    }
#elif defined(__ARM_NEON)
    for (int i = 0; i < n; i += 4) {
        float32x4_t v1 = vld1q_f32(&src[i]);
        float32x4_t v2 = vdupq_n_f32(2.5f);
        float32x4_t result = vmulq_f32(v1, v2);
        vst1q_f32(&dest[i], result);
    }
#else
    for (int i = 0; i < n; i++) {
        dest[i] = src[i] * 2.5f;
    }
#endif
}

/* Requirement 4: Assembly barriers */
static inline void memory_barrier(void) {
    __asm__ volatile ("" ::: "memory");
}

static inline void clobber_registers(void) {
    __asm__ volatile (
        "mov $0, %%eax\n"
        "mov $0, %%ebx\n"
        "mov $0, %%ecx\n"
        "mov $0, %%edx\n"
        : : : "eax", "ebx", "ecx", "edx", "cc", "memory"
    );
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* arr, int* int_arr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            arr[0] = sinf(arr[0]);
            result = int_arr[0] * 2;
            break;
        case 1:
            arr[1] = cosf(arr[1]);
            result = int_arr[1] + 5;
            break;
        case 2:
            arr[2] = sqrtf(fabsf(arr[2]));
            result = int_arr[2] - 3;
            break;
        case 3:
            arr[3] = arr[3] * arr[3];
            result = int_arr[3] / 2;
            break;
        case 4:
            arr[4] = 1.0f / arr[4];
            result = int_arr[4] << 1;
            break;
        case 5:
            arr[5] = expf(arr[5]);
            result = int_arr[5] >> 1;
            break;
        case 6:
            arr[6] = logf(arr[6] + 1.0f);
            result = int_arr[6] | 0xFF;
            break;
        case 7:
            arr[7] = tanf(arr[7]);
            result = int_arr[7] & 0x7F;
            break;
        case 8:
            arr[8] = atanf(arr[8]);
            result = int_arr[8] ^ 0x55;
            break;
        case 9:
            arr[9] = asinf(arr[9]);
            result = int_arr[9] % 17;
            break;
        case 10:
            arr[10] = acosf(arr[10]);
            result = int_arr[10] + int_arr[11];
            break;
        case 11:
            arr[11] = sinhf(arr[11]);
            result = int_arr[11] - int_arr[12];
            break;
        case 12:
            arr[12] = coshf(arr[12]);
            result = int_arr[12] * int_arr[13];
            break;
        case 13:
            arr[13] = tanhf(arr[13]);
            result = int_arr[13] / (int_arr[14] + 1);
            break;
        case 14:
            arr[14] = powf(arr[14], 1.5f);
            result = int_arr[14] << (int_arr[15] & 3);
            break;
        case 15:
            arr[15] = fmodf(arr[15], 3.14f);
            result = int_arr[15] >> (int_arr[16] & 3);
            break;
        case 16:
            arr[16] = ceilf(arr[16]);
            result = int_arr[16] | int_arr[17];
            break;
        case 17:
            arr[17] = floorf(arr[17]);
            result = int_arr[17] & int_arr[18];
            break;
        case 18:
            arr[18] = roundf(arr[18]);
            result = int_arr[18] ^ int_arr[19];
            break;
        case 19:
            arr[19] = fabsf(arr[19]);
            result = ~int_arr[19];
            break;
        case 20:
            arr[20] = arr[20] + arr[21];
            result = int_arr[20] + int_arr[21] * 2;
            break;
        case 21:
            arr[21] = arr[21] - arr[22];
            result = int_arr[21] - int_arr[22] / 2;
            break;
        case 22:
            arr[22] = arr[22] * arr[23];
            result = int_arr[22] * (int_arr[23] + 1);
            break;
        case 23:
            arr[23] = arr[23] / (arr[24] + 0.001f);
            result = int_arr[23] / (int_arr[24] + 1);
            break;
        default:  /* Requirement 5: Complex default case */
            for (int i = 0; i < 10; i++) {
                arr[i] = compute_reduction(arr, 10);
                int_arr[i] = (int_arr[i] * 3 + 7) % 256;
            }
            result = -1;
            memory_barrier();
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float float_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    float temp_arr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_arr[i] = (i % 100) * 0.1f;
        int_arr[i] = i;
        temp_arr[i] = 0.0f;
    }
    
    float total_sum = 0.0f;
    int int_sum = 0;
    int restart_count = 0;
    
    /* Requirement 6: goto for restart mechanism */
restart_point:
    if (restart_count++ > 3) {
        error_handler("Too many restarts");
        return 1;
    }
    
    /* Requirement 1: Triple nested loops with dependencies */
    for (int i = 0; i < 10; i++) {
        float outer_sum = 0.0f;
        
        for (int j = 0; j < 20; j++) {
            float middle_sum = 0.0f;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependency */
                middle_sum += float_arr[(i * 100 + j * 5 + k) % ARRAY_SIZE];
                
                /* Conditional branch inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    float_arr[(i * 50 + j * 3 + k) % ARRAY_SIZE] *= 1.1f;
                    clobber_registers();  /* Requirement 4 */
                } else if ((i * j * k) % 11 == 0) {
                    float_arr[(i * 30 + j * 2 + k) % ARRAY_SIZE] /= 1.05f;
                }
                
                /* Mixed integer operations */
                int_arr[(i * 80 + j * 4 + k) % ARRAY_SIZE] += k;
                
                /* SIMD operations in helper */
                if (k % 8 == 0) {
                    process_with_simd(temp_arr, float_arr, 64);
                    memory_barrier();
                }
            }
            
            outer_sum += middle_sum * j;
            
            /* Requirement 5: Switch statement with dependencies */
            int switch_result = process_switch(i * j, float_arr, int_arr);
            int_sum += switch_result;
            
            /* Requirement 6: goto to create irreducible flow */
            if (j == 15 && outer_sum > 1000.0f) {
                goto skip_inner;
            }
            
            /* More computations */
            float_arr[j % ARRAY_SIZE] = sinf(outer_sum);
            continue;
            
        skip_inner:
            float_arr[j % ARRAY_SIZE] = cosf(outer_sum);
            
            /* Requirement 6: Break to outer loop */
            if (int_sum > 1000000) {
                break;
            }
        }
        
        total_sum += outer_sum / (i + 1);
        
        /* Requirement 6: Continue to next iteration */
        if (i == 5 && total_sum < 500.0f) {
            continue;
        }
        
        /* Trigger restart with goto */
        if (i == 8 && int_sum < 0) {
            goto restart_point;
        }
    }
    
    /* Additional complex processing */
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        /* Data dependencies across iterations */
        float_arr[i + 1] = float_arr[i] * 0.99f + float_arr[i + 1] * 0.01f;
        int_arr[i + 1] = (int_arr[i] + int_arr[i + 1]) / 2;
        
        /* More assembly barriers */
        if (i % 16 == 0) {
            __asm__ volatile (
                "mov %0, %%eax\n"
                "add $1, %%eax\n"
                : : "r"(int_arr[i]) : "eax", "cc"
            );
        }
    }
    
    /* Final validation */
    float final_checksum = compute_reduction(float_arr, ARRAY_SIZE);
    int int_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_checksum += int_arr[i];
    }
    
    printf("Final checksum - float: %f, int: %d\n", final_checksum, int_checksum);
    printf("Total iterations - restart_count: %d\n", restart_count);
    
    if (final_checksum > 0 && int_checksum > 0) {
        printf("Test completed successfully\n");
        return 0;
    } else {
        error_handler("Invalid checksum results");
        return 1;
    }
}
