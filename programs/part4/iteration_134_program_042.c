/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Non-inline helper functions to force call boundaries */
__attribute__((noinline)) 
static float vector_sum(float* arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&arr[i]);
        __m128 sum_vec = _mm_hadd_ps(vec, vec);
        sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
        sum += _mm_cvtss_f32(sum_vec);
        
        /* Artificial scheduling barrier */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    return sum;
}

__attribute__((noinline, cold))
static void error_handler(int code) {
    /* Cold path unlikely to be taken */
    volatile int* dummy = NULL;
    if (code > 1000) {
        /* This should never happen but creates control flow */
        *dummy = code; /* Potential crash - but cold attribute makes it unlikely */
    }
}

__attribute__((noinline))
static int process_chunk(int* data, float* fdata, int start, int end) {
    int result = 0;
    __m128i accum = _mm_setzero_si128();
    
    for (int i = start; i < end; i += 4) {
        __m128i chunk = _mm_loadu_si128((__m128i*)&data[i]);
        accum = _mm_add_epi32(accum, chunk);
        
        /* Mix with floating point operations */
        __m128 fchunk = _mm_loadu_ps(&fdata[i]);
        __m128 squared = _mm_mul_ps(fchunk, fchunk);
        _mm_storeu_ps(&fdata[i], squared);
        
        /* Another scheduling barrier */
        asm volatile ("# Scheduling barrier" ::: 
                     "xmm0", "xmm1", "xmm2", "xmm3", 
                     "xmm4", "xmm5", "xmm6", "xmm7");
    }
    
    /* Horizontal sum of vector */
    int temp[4];
    _mm_storeu_si128((__m128i*)temp, accum);
    result = temp[0] + temp[1] + temp[2] + temp[3];
    
    return result;
}

/* Complex switch handler */
static int handle_switch_case(int case_id, int* arr, float* farr) {
    int result = 0;
    
    /* Non-sequential case labels to create complex jump table */
    switch (case_id) {
        case 1:  result = arr[0] * 2; break;
        case 3:  result = arr[1] + arr[2]; break;
        case 7:  result = (int)(farr[0] * 100.0f); break;
        case 12: result = arr[3] ^ arr[4]; break;
        case 15: result = arr[5] << 2; break;
        case 18: result = (int)sqrtf(fabsf(farr[1])); break;
        case 21: result = arr[6] | arr[7]; break;
        case 24: result = arr[8] & arr[9]; break;
        case 27: result = arr[10] % 17; break;
        case 30: result = -arr[11]; break;
        case 33: result = arr[12] / (arr[13] + 1); break;
        case 36: result = (int)(sinf(farr[2]) * 1000.0f); break;
        case 39: result = arr[14] * arr[15]; break;
        case 42: result = arr[16] + arr[17] * 2; break;
        case 45: result = (int)(logf(fabsf(farr[3]) + 1.0f) * 100.0f); break;
        case 48: result = arr[18] ^ 0xAAAAAAAA; break;
        case 51: result = arr[19] >> 3; break;
        case 54: result = (int)(cosf(farr[4]) * 1000.0f); break;
        case 57: result = arr[20] + arr[21] - arr[22]; break;
        case 60: result = arr[23] * arr[24] / 2; break;
        case 63: result = (int)(expf(farr[5] * 0.01f)); break;
        case 66: result = ~arr[25]; break;
        case 69: result = arr[26] + arr[27] * 3; break;
        case 72: result = (int)(tanf(farr[6]) * 100.0f); break;
        default: /* Complex default case */
            result = process_chunk(arr, farr, 0, 64);
            error_handler(result);
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 37) % 101;
        float_data[i] = (float)(i % 50) * 0.1f - 2.5f;
    }
    
    int outer_sum = 0;
    float outer_fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Label for goto jumps */
    
    /* Triple nested loops with loop-carried dependencies */
    for (int i = 0; i < 32; i++) {
        int middle_sum = 0;
        
        for (int j = 0; j < 64; j++) {
            int inner_sum = int_data[(i * 32 + j) % ARRAY_SIZE];
            
            for (int k = 0; k < 16; k++) {
                /* Complex data dependencies */
                inner_sum += int_data[(j * 16 + k) % ARRAY_SIZE];
                inner_sum *= (k % 7) + 1;
                
                /* Conditional branch inside innermost loop */
                if ((inner_sum & 0xFF) > 128) {
                    float_data[(i * 64 + j + k) % ARRAY_SIZE] *= 1.1f;
                    middle_sum += inner_sum % 97;
                } else {
                    float_data[(i * 64 + j + k) % ARRAY_SIZE] *= 0.9f;
                    middle_sum -= inner_sum % 53;
                }
                
                /* Mix integer and floating point */
                if (k % 3 == 0) {
                    float temp = sinf(float_data[(i * 64 + j) % ARRAY_SIZE]);
                    float_data[(i * 64 + j + k) % ARRAY_SIZE] += temp;
                }
            }
            
            /* Call to non-inline function with SIMD */
            if (j % 8 == 0) {
                middle_sum += process_chunk(int_data, float_data, 
                                          j * 4, (j + 8) * 4);
            }
            
            /* Inline assembly barrier */
            asm volatile ("# Loop barrier %0" : "+r"(middle_sum) :: 
                         "rax", "rbx", "rcx", "rdx", "memory");
            
            /* Unlikely cold path */
            if (middle_sum > 1000000) {
                error_handler(middle_sum);
            }
        }
        
        outer_sum += middle_sum;
        
        /* Use goto to create irreducible control flow */
        if ((i % 7 == 0) && (restart_count < 2)) {
            restart_count++;
            goto restart_point;  /* Jump back to restart */
        }
        
        if (i % 5 == 0) {
            goto skip_section;  /* Forward jump */
        }
        
        /* This section sometimes skipped by goto */
        outer_fsum += vector_sum(float_data, 128);
        
    skip_section:
        
        /* Large switch statement */
        int case_id = (outer_sum % 73) + 1;
        int switch_result = handle_switch_case(case_id, int_data, float_data);
        outer_sum ^= switch_result;
        
        /* Another goto to continue outer loop */
        if (i == 10) {
            goto continue_outer;
        }
        
        /* Additional computation */
        for (int m = 0; m < 8; m++) {
            __m128 vec1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 vec2 = _mm_loadu_ps(&float_data[(i * 8 + m) * 4 % ARRAY_SIZE]);
            __m128 res = _mm_add_ps(vec1, vec2);
            _mm_storeu_ps(&float_data[(i * 8 + m) * 4 % ARRAY_SIZE], res);
        }
        
    continue_outer:
        /* Empty statement for label */
        ;
    }
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_data[i] & 0xFF;
        checksum ^= (int)(float_data[i] * 100.0f);
    }
    
    checksum += outer_sum;
    checksum += (int)outer_fsum;
    
    printf("Scheduler test completed. Checksum: %d\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    /* Use result to prevent dead code elimination */
    if (checksum == 0x12345678) {  /* Extremely unlikely */
        printf("Impossible condition!\n");
    }
    
    return 0;
}
