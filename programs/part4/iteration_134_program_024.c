#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics and SIMD operations */
    __m128 vsum = _mm_setzero_ps();
    int i;
    
    for (i = 0; i + 3 < size; i += 4) {
        __m128 v = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, v);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract results from SIMD register */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Scalar tail processing */
    for (; i < size; i++) {
        sum += data[i];
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, float* farr, int n) {
    int result = 0;
    float fresult = 0.0f;
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < n; i++) {
        int temp = arr[i];
        float ftemp = farr[i];
        
        for (int j = 0; j < i % 8; j++) {
            /* Loop-carried dependency */
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            
            for (int k = 0; k < 3; k++) {
                /* Mixed integer/float operations */
                ftemp = ftemp * 0.99f + (float)temp * 0.01f;
                
                /* Conditional branch inside innermost loop */
                if ((k & 1) && (temp % 7 == 0)) {
                    ftemp = -ftemp;
                    asm volatile ("" ::: "rax", "rbx", "rcx", "rdx");
                }
            }
            
            /* Another asm barrier */
            asm volatile ("" ::: "memory");
        }
        
        result ^= temp;
        fresult += ftemp;
    }
    
    /* Mix results */
    return result + (int)fresult;
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, int* arr, float* farr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = arr[0] + arr[1];
            asm volatile ("" ::: "r8", "r9", "r10", "r11");
            break;
        case 1:
            result = arr[1] * 2 - arr[2];
            break;
        case 3:  /* Non-sequential */
            result = (int)(farr[0] * 100.0f);
            break;
        case 7:
            result = arr[3] | arr[4];
            break;
        case 12:
            result = arr[5] ^ arr[6];
            asm volatile ("" ::: "r12", "r13", "r14", "r15");
            break;
        case 15:
            result = (arr[7] << 3) + (arr[8] >> 2);
            break;
        case 18:
            result = (int)(sinf(farr[1]) * 1000.0f);
            break;
        case 21:
            result = arr[9] % 17;
            break;
        case 24:
            result = -arr[10];
            break;
        case 2:
            result = arr[11] + arr[12] * 3;
            break;
        case 4:
            result = (int)(farr[2] + farr[3]);
            break;
        case 5:
            result = arr[13] & 0xFF;
            break;
        case 6:
            result = arr[14] * arr[15];
            break;
        case 8:
            result = arr[16] / (arr[17] + 1);
            break;
        case 9:
            result = (int)(cosf(farr[4]) * 500.0f);
            break;
        case 10:
            result = ~arr[18];
            break;
        case 11:
            result = arr[19] - arr[20];
            break;
        case 13:
            result = arr[21] << 1;
            break;
        case 14:
            result = arr[22] >> 2;
            break;
        case 16:
            result = (int)(farr[5] * farr[6]);
            break;
        case 17:
            result = arr[23] | 0x55;
            break;
        case 19:
            result = arr[24] ^ 0xAA;
            break;
        case 20:
            result = arr[25] + 100;
            break;
        case 22:
            result = (int)(sqrtf(fabsf(farr[7])) * 10.0f);
            break;
        case 23:
            result = arr[26] % 13;
            break;
        default:  /* Requirement 5: Complex default case */
            result = complex_reduction(arr, farr, 10);
            error_handler("Default case reached");
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 7;
        float_array[i] = (float)i * 0.1f + 0.5f;
    }
    
    int final_result = 0;
    int restart_count = 0;
    
    /* Requirement 6: goto and loop constructs with labels */
    restart_point:
    
    /* Requirement 1: Deep nested loops */
    for (int outer = 0; outer < 5; outer++) {
        int outer_temp = outer * 100;
        
        for (int mid = 0; mid < 8; mid++) {
            float mid_sum = 0.0f;
            
            for (int inner = 0; inner < 12; inner++) {
                /* Complex data dependencies */
                outer_temp = (outer_temp * 6364136223846793005ULL + 1) & 0x7fffffff;
                mid_sum += sinf((float)outer_temp * 0.0001f);
                
                /* Conditional with goto */
                if (restart_count < 2 && (outer_temp % 10007) == 0) {
                    restart_count++;
                    goto restart_point;  /* Requirement 6: goto to outer label */
                }
                
                /* Mixed operations */
                float_array[inner] = float_array[inner] * 0.9f + mid_sum * 0.1f;
                int_array[inner] += (int)(float_array[inner] * 10.0f);
                
                asm volatile ("" ::: "memory", "rax", "rbx");
            }
            
            /* Call noinline function with SIMD */
            float vector_result = process_vector(float_array, 64);
            int_array[mid] += (int)vector_result;
            
            /* Another asm barrier */
            asm volatile ("" ::: "xmm4", "xmm5", "xmm6", "xmm7");
        }
        
        /* Requirement 6: continue to outer loop */
        if (outer_temp % 2 == 0) {
            continue;
        }
        
        /* Switch statement in middle of loops */
        int switch_val = process_switch(outer_temp, int_array, float_array);
        final_result ^= switch_val;
        
        /* Requirement 6: break from outer loop under condition */
        if (final_result > 1000000) {
            break;
        }
    }
    
    /* More computations */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Data-dependent loop with complex indexing */
        int idx = (i * 16777619) % ARRAY_SIZE;
        
        /* Requirement 1: More nested loops */
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                float_array[idx] = float_array[idx] * 1.01f - 0.5f;
                int_array[idx] = (int_array[idx] + j * 11 - k * 7) & 0xFFFF;
                
                /* Conditional with goto to different label */
                if (float_array[idx] > 1000.0f) {
                    goto cleanup_section;
                }
            }
            
            /* Call complex reduction */
            int red = complex_reduction(int_array + i, float_array + i, 8);
            final_result += red;
        }
    }
    
    cleanup_section:
    
    /* Final validation */
    float final_float_sum = 0.0f;
    int final_int_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_float_sum += float_array[i];
        final_int_sum += int_array[i];
    }
    
    /* Use results to prevent optimization */
    final_result += (int)final_float_sum + final_int_sum;
    
    printf("Final result: %d (restart_count: %d)\n", final_result, restart_count);
    
    /* Simple validation */
    if (final_result != 0) {
        printf("Test completed successfully (non-zero result indicates computation occurred)\n");
        return 0;
    } else {
        error_handler("Unexpected zero result");
        return 1;
    }
}
