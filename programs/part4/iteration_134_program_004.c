/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling decisions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SIMD processing */
    for (i = start; i + 3 < end; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrt_vec = _mm_sqrt_ps(squared);
        
        /* Inline assembly barrier creating scheduling boundary */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        
        float temp[4];
        _mm_storeu_ps(temp, sqrt_vec);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* Scalar tail processing */
    for (; i < end; i++) {
        sum += sqrtf(fabsf(data[i]));
    }
    
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with loop-carried dependencies */
    for (i = 0; i < size; i += 8) {
        int block_sum = 0;
        
        for (j = i; j < i + 8 && j < size; j++) {
            int temp = arr[j];
            
            /* Inner loop with data dependency */
            for (k = 0; k < 4; k++) {
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                
                /* Conditional branch inside innermost loop */
                if (temp % 7 == 0) {
                    temp >>= 1;
                } else if (temp % 13 == 0) {
                    temp <<= 1;
                }
                
                /* Another assembly barrier */
                asm volatile ("" ::: "cc", "memory", "eax", "ebx", "ecx", "edx");
            }
            
            block_sum += temp;
            
            /* Conditional with cold attribute hint */
            if (block_sum > 0x7fffffff) {
                error_handler("Overflow in reduction");
                block_sum = 0x7fffffff;
            }
        }
        
        result ^= block_sum;
        
        /* Complex dependency chain */
        arr[i % 16] = result;
    }
    
    return result;
}

__attribute__((noinline))
static void process_switch(int value, float* farr, int* iarr, int idx) {
    /* Large switch with non-sequential cases */
    switch (value) {
        case 100: farr[idx] *= 1.1f; break;
        case 101: iarr[idx] += 100; break;
        case 105: farr[idx] = sinf(farr[idx]); break;
        case 107: iarr[idx] ^= 0xAAAAAAAA; break;
        case 110: farr[idx] = expf(farr[idx]); break;
        case 115: iarr[idx] = (iarr[idx] << 3) | (iarr[idx] >> 29); break;
        case 120: farr[idx] += process_vector(farr, 0, 16); break;
        case 125: iarr[idx] = complex_reduction(iarr, 32, 1000); break;
        case 130: farr[idx] = farr[idx] * farr[idx] - 2.0f; break;
        case 135: iarr[idx] = ~iarr[idx]; break;
        case 140: farr[idx] = logf(fabsf(farr[idx]) + 1.0f); break;
        case 145: iarr[idx] = iarr[idx] * 3 + 7; break;
        case 150: farr[idx] = tanf(farr[idx]); break;
        case 155: iarr[idx] = iarr[idx] % 997; break;
        case 160: farr[idx] = 1.0f / (farr[idx] + 0.001f); break;
        case 165: iarr[idx] = iarr[idx] | 0x55555555; break;
        case 170: farr[idx] = cosf(farr[idx]); break;
        case 175: iarr[idx] = iarr[idx] & 0x0F0F0F0F; break;
        case 180: farr[idx] = powf(farr[idx], 1.5f); break;
        case 185: iarr[idx] = iarr[idx] - 42; break;
        case 190: farr[idx] = atanf(farr[idx]); break;
        case 195: iarr[idx] = iarr[idx] * iarr[idx]; break;
        case 200: farr[idx] = farr[idx] + 3.14159f; break;
        case 205: iarr[idx] = abs(iarr[idx]); break;
        default:  /* Complex default case */
            farr[idx] = process_vector(farr, idx, idx + 8);
            iarr[idx] = complex_reduction(iarr, idx % 32, 500);
            break;
    }
    
    /* Another scheduling barrier */
    asm volatile ("" ::: "memory", "xmm4", "xmm5", "xmm6", "xmm7");
}

int main(void) {
    /* Declare and initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (i % 100) * 0.1f - 5.0f;
        int_array[i] = i * 1103515245 + 12345;
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with complex control flow */
    for (i = 0; i < 16; i++) {
        float block_float_sum = 0.0f;
        
        for (j = 0; j < 32; j++) {
            int idx = (i * 32 + j) % ARRAY_SIZE;
            int temp_int = 0;
            
            for (k = 0; k < 8; k++) {
                /* Loop-carried dependency */
                temp_int = temp_int * 1664525 + 1013904223;
                
                /* Conditional inside innermost loop */
                if ((temp_int & 0xFF) < 64) {
                    float_array[idx] += sinf(temp_int * 0.001f);
                } else if ((temp_int & 0xFF) < 128) {
                    float_array[idx] -= cosf(temp_int * 0.001f);
                } else {
                    float_array[idx] *= 0.99f;
                }
                
                /* Mixed operations */
                int_array[idx] ^= temp_int;
                
                /* Assembly barrier with clobbers */
                asm volatile ("" ::: "cc", "memory", "esi", "edi");
            }
            
            block_float_sum += float_array[idx];
            
            /* Call noinline function with SIMD */
            if (j % 8 == 0) {
                float partial = process_vector(float_array, 
                                             idx, 
                                             (idx + 16) % ARRAY_SIZE);
                block_float_sum += partial;
            }
            
            /* Complex switch statement based on computed value */
            int switch_val = (int_array[idx] % 200) + 100;
            process_switch(switch_val, float_array, int_array, idx);
            
            /* Goto to create irreducible control flow */
            if (i == 8 && j == 16 && total_int > 1000000) {
                /* This creates a loop back to restart_point */
                total_int = 0;
                goto restart_point;
            }
        }
        
        total_float += block_float_sum;
        
        /* Call complex reduction function */
        total_int ^= complex_reduction(int_array + i * 16, 16, 10000);
        
        /* Another goto target */
        if (i == 12 && total_float > 10000.0f) {
            /* Jump forward */
            goto skip_section;
        }
    }
    
    /* Label for forward goto */
    skip_section:
    
    /* Additional processing with mixed control flow */
    for (i = 0; i < ARRAY_SIZE; i += 64) {
        /* Use continue with outer loop target */
        if (float_array[i] < -100.0f) {
            continue;
        }
        
        /* Nested loop with break to outer loop */
        for (j = 0; j < 16; j++) {
            if (int_array[i + j] < 0) {
                /* Break to outer loop */
                break;
            }
            
            /* More SIMD operations */
            __m128 vec1 = _mm_set_ps(float_array[i+j], 
                                    float_array[i+j+1], 
                                    float_array[i+j+2], 
                                    float_array[i+j+3]);
            __m128 vec2 = _mm_set1_ps(0.5f);
            __m128 result = _mm_add_ps(_mm_mul_ps(vec1, vec2), vec2);
            
            asm volatile ("" ::: "xmm0", "xmm1", "xmm2");
            
            float temp[4];
            _mm_storeu_ps(temp, result);
            
            for (k = 0; k < 4 && (i+j+k) < ARRAY_SIZE; k++) {
                float_array[i+j+k] = temp[k];
            }
        }
        
        /* Goto within loop */
        if (i == 512) {
            goto final_calculation;
        }
    }
    
    /* Another label */
    final_calculation:
    
    /* Final reduction and validation */
    float final_float_sum = 0.0f;
    int final_int_sum = 0;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_float_sum += float_array[i];
        final_int_sum ^= int_array[i];
    }
    
    /* Simple checksum validation */
    printf("Float checksum: %f\n", final_float_sum);
    printf("Int checksum: %d\n", final_int_sum);
    
    /* The actual values don't matter for coverage */
    if (!isnan(final_float_sum) && final_int_sum != 0) {
        printf("Test completed successfully\n");
    } else {
        printf("Test completed (validation skipped)\n");
    }
    
    return 0;
}
