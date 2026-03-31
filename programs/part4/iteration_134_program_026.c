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
static int validate_result(long total) {
    return (total % 1000) == 42;
}

/* Requirement 3: SIMD operations */
#ifdef __SSE__
__attribute__((noinline))
static __m128 simd_reduce(__m128* data, int count) {
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < count; i++) {
        sum = _mm_add_ps(sum, data[i]);
        /* Requirement 4: inline assembly barrier */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    return sum;
}
#endif

/* Requirement 5: Large switch statement */
static int process_switch(int value, int* arr, float* farr) {
    int result = 0;
    
    switch (value) {
        case 1: result = arr[0] + 1; break;
        case 3: result = arr[1] * 2; break;
        case 7: result = arr[2] / 3; break;
        case 13: result = arr[3] - 4; break;
        case 21: result = arr[4] | 0xFF; break;
        case 34: result = arr[5] & 0x7F; break;
        case 55: result = arr[6] ^ 0x55; break;
        case 89: result = arr[7] << 2; break;
        case 144: result = arr[8] >> 1; break;
        case 233: result = (int)(farr[0] * 10.0f); break;
        case 377: result = (int)(farr[1] + farr[2]); break;
        case 610: result = (int)(farr[3] - farr[4]); break;
        case 987: result = (int)(farr[5] * farr[6]); break;
        case 1597: result = (int)(farr[7] / farr[8]); break;
        case 2584: result = arr[9] % 17; break;
        case 4181: result = ~arr[10]; break;
        case 6765: result = arr[11] + arr[12]; break;
        case 10946: result = arr[13] - arr[14]; break;
        case 17711: result = arr[15] * arr[16]; break;
        case 28657: result = arr[17] / (arr[18] + 1); break;
        case 46368: result = arr[19] | arr[20]; break;
        case 75025: result = arr[21] & arr[22]; break;
        case 121393: result = arr[23] ^ arr[24]; break;
        case 196418: result = arr[0] << arr[1]; break;
        default: /* Complex default case */
            for (int i = 0; i < 10; i++) {
                result += arr[i] * (i + 1);
                farr[i] = sqrtf(farr[i] + 1.0f);
            }
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    long long_array[ARRAY_SIZE / 2];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 3) % 97;
        float_array[i] = (float)(i * 0.1);
    }
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        long_array[i] = i * 5;
    }
    
    long total_sum = 0;
    float fp_sum = 0.0f;
    
    /* Requirement 1: Nested loops with complex dependencies */
    int outer_restart = 0;
    
outer_loop:
    for (int i = 0; i < 10; i++) {
        if (outer_restart > 3) {
            error_handler("Too many restarts");
            break;
        }
        
        for (int j = 0; j < 20; j++) {
            /* Loop-carried dependency */
            long running_sum = 0;
            
            for (int k = 0; k < 50; k++) {
                /* Mixed integer and FP operations */
                int idx = (i * 20 + j * 50 + k) % ARRAY_SIZE;
                
                /* Conditional branch inside innermost loop */
                if (int_array[idx] > 50) {
                    float_array[idx] = float_array[idx] * 1.5f + 0.1f;
                    running_sum += int_array[idx] * 2;
                    
                    /* Requirement 4: Assembly barrier */
                    asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11");
                } else {
                    float_array[idx] = float_array[idx] * 0.5f - 0.1f;
                    running_sum += int_array[idx] / 2;
                }
                
                /* More complex data dependency */
                if (k > 0) {
                    float_array[idx] += float_array[idx - 1] * 0.1f;
                }
                
                /* SIMD operations */
                #ifdef __SSE__
                if (k % 4 == 0 && idx + 3 < ARRAY_SIZE) {
                    __m128 v1 = _mm_loadu_ps(&float_array[idx]);
                    __m128 v2 = _mm_set1_ps(0.25f);
                    __m128 result = _mm_mul_ps(v1, v2);
                    _mm_storeu_ps(&float_array[idx], result);
                }
                #endif
                
                /* Requirement 6: goto for irreducible flow */
                if (running_sum > 1000000 && k < 10) {
                    outer_restart++;
                    goto outer_loop;  /* Restart computation */
                }
            }
            
            total_sum += running_sum;
            
            /* Call noinline function */
            fp_sum += process_chunk(float_array, j * 50, (j + 1) * 50);
        }
        
        /* Requirement 5: Switch statement with dense cases */
        int switch_val = (int_array[i] * 13) % 200000;
        int switch_result = process_switch(switch_val, int_array, float_array);
        total_sum += switch_result;
        
        /* Another assembly barrier */
        asm volatile ("" ::: "cc", "memory");
    }
    
    /* Additional complex loop with goto */
    int restart_count = 0;
    int loop_counter = 0;
    
complex_loop:
    for (int a = 0; a < 5; a++) {
        for (int b = 0; b < 10; b++) {
            loop_counter++;
            
            if (loop_counter % 7 == 0) {
                continue;  /* Skip some iterations */
            }
            
            if (loop_counter > 100 && restart_count < 2) {
                restart_count++;
                goto complex_loop;  /* Requirement 6: goto to outer loop */
            }
            
            int idx = (a * 10 + b) % (ARRAY_SIZE / 2);
            long_array[idx] = long_array[idx] * 3 + a - b;
            
            /* Break to outer loop */
            if (long_array[idx] > 1000000) {
                break;
            }
        }
    }
    
    /* Final validation */
    if (!validate_result(total_sum)) {
        error_handler("Validation failed");
        return 1;
    }
    
    printf("Test completed successfully. Total sum: %ld\n", total_sum);
    return 0;
}
