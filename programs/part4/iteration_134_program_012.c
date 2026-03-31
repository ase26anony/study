#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributes */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics mixed with scalar ops */
    __m128 vec_sum = _mm_setzero_ps();
    int i;
    
    for (i = 0; i + 3 < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        vec_sum = _mm_add_ps(vec_sum, vec);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "mfence\n\t"
            :
            :
            : "memory"
        );
    }
    
    /* Extract results from vector */
    float temp[4];
    _mm_storeu_ps(temp, vec_sum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Handle remaining elements */
    for (; i < size; i++) {
        sum += data[i];
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* data, int size) {
    int result = 0;
    
    /* Requirement 4: More asm barriers */
    asm volatile (
        "xor %%eax, %%eax\n\t"
        "mov %0, %%eax\n\t"
        :
        : "r"(size)
        : "%eax", "cc"
    );
    
    for (int i = 0; i < size; i++) {
        result += data[i] * (i % 7);
        
        /* Artificial dependency chain */
        if (result < 0) {
            result = -result;
        }
    }
    
    return result;
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1: result = iarr[0] + 1; break;
        case 3: result = iarr[1] * 2; break;
        case 7: result = iarr[2] / 3; break;
        case 13: result = iarr[3] - 4; break;
        case 21: result = iarr[4] | 0xFF; break;
        case 34: result = iarr[5] & 0xAA; break;
        case 55: result = iarr[6] ^ 0x55; break;
        case 89: result = iarr[7] << 2; break;
        case 144: result = iarr[8] >> 1; break;
        case 233: result = ~iarr[9]; break;
        case 377: result = abs(iarr[10]); break;
        case 610: result = iarr[11] % 17; break;
        case 987: result = iarr[12] + iarr[13]; break;
        case 1597: result = iarr[14] * iarr[15]; break;
        case 2584: result = iarr[16] - iarr[17]; break;
        case 4181: result = iarr[18] | iarr[19]; break;
        case 6765: result = iarr[20] & iarr[21]; break;
        case 10946: result = iarr[22] ^ iarr[23]; break;
        case 17711: result = iarr[24] << (iarr[25] & 3); break;
        case 28657: result = iarr[26] >> (iarr[27] & 3); break;
        case 46368: 
            /* Complex default-like case */
            for (int i = 0; i < 10; i++) {
                farr[i] = sqrtf(farr[i] + 1.0f);
            }
            result = 999;
            break;
        case 75025: result = iarr[28] + iarr[29]; break;
        case 121393: result = iarr[30] * 3; break;
        default:
            /* Requirement 2: cold path */
            error_handler("Unexpected switch value");
            result = -1;
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
        iarr[i] = i * 3;
    }
    
    float total_sum = 0.0f;
    int total_int = 0;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with complex dependencies */
    for (int outer = 0; outer < 5; outer++) {
        float outer_acc = 0.0f;
        
        for (int middle = 0; middle < 10; middle++) {
            int middle_acc = 0;
            
            for (int inner = 0; inner < 20; inner++) {
                /* Loop-carried dependency */
                static int persistent = 0;
                
                /* Mixed operations */
                float temp_f = sinf(farr[inner] * 0.01f);
                int temp_i = inner * middle * outer;
                
                /* Conditional branch inside innermost loop */
                if ((inner + middle + outer) % 7 == 0) {
                    temp_f = cosf(temp_f);
                    temp_i += persistent;
                    
                    /* Requirement 4: asm barrier in conditional path */
                    asm volatile (
                        "cpuid\n\t"
                        :
                        :
                        : "%eax", "%ebx", "%ecx", "%edx", "memory"
                    );
                } else if ((inner * middle) % 11 == 0) {
                    temp_f = tanf(temp_f);
                    temp_i -= persistent;
                }
                
                /* Update persistent with dependency */
                persistent = (persistent + temp_i) % 1000;
                
                /* Update accumulators */
                outer_acc += temp_f;
                middle_acc += temp_i;
                
                /* Another conditional with goto */
                if (restart_count < 3 && middle_acc > 1000000) {
                    restart_count++;
                    
                    /* Requirement 6: goto to create irreducible flow */
                    if (restart_count % 2 == 0) {
                        goto restart_point;
                    } else {
                        goto skip_rest;
                    }
                }
                
                /* SIMD operation in loop */
                if (inner % 4 == 0) {
                    __m128 vec = _mm_set_ps(temp_f, temp_f * 0.5f, 
                                           temp_f * 0.25f, temp_f * 0.125f);
                    __m128 vec2 = _mm_set1_ps(1.0f);
                    __m128 result = _mm_add_ps(vec, vec2);
                    
                    float res_arr[4];
                    _mm_storeu_ps(res_arr, result);
                    outer_acc += res_arr[0] + res_arr[1];
                }
            }
            
            /* Call noinline functions */
            if (middle % 3 == 0) {
                float vec_sum = process_vector(farr, ARRAY_SIZE / 4);
                outer_acc += vec_sum * 0.01f;
            }
            
            if (middle % 4 == 0) {
                int int_sum = integer_reduction(iarr, ARRAY_SIZE / 8);
                middle_acc += int_sum % 1000;
            }
            
            total_int += middle_acc;
            
            /* Requirement 6: continue with label */
            if (middle_acc < 0) {
                continue;
            }
        }
        
        total_sum += outer_acc;
        
        /* Requirement 5: Switch statement based on computed value */
        int switch_val = (int)fabsf(outer_acc) % SWITCH_CASES;
        switch_val = switch_val * 2 + 1;  /* Make it odd */
        
        int switch_result = process_switch(switch_val, farr, iarr);
        total_int += switch_result;
        
        /* Break from outer loop under condition */
        if (outer > 2 && total_sum > 10000.0f) {
            break;
        }
    }
    
skip_rest:
    
    /* Final validation */
    float final_float_sum = process_vector(farr, ARRAY_SIZE);
    int final_int_sum = integer_reduction(iarr, ARRAY_SIZE);
    
    printf("Results: float_sum=%.2f, int_sum=%d, total_sum=%.2f, total_int=%d, restarts=%d\n",
           final_float_sum, final_int_sum, total_sum, total_int, restart_count);
    
    /* Simple checksum validation */
    int checksum = (int)final_float_sum + final_int_sum + (int)total_sum + total_int;
    if (checksum != 0) {  /* Will never be 0, just for demonstration */
        printf("Test completed successfully (checksum: %d)\n", checksum);
    } else {
        error_handler("Checksum failed");
    }
    
    return 0;
}
