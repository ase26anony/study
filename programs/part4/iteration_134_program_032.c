/* Test program to trigger free_sched_state cleanup in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path - unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SSE operations */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 v = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, v);
        
        /* Inline asm barrier between dependent operations */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Store SSE result */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Handle remainder */
    for (; i < end; i++) {
        sum += data[i];
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i += 16) {
        int block_sum = 0;
        for (j = i; j < i + 16 && j < size; j++) {
            int val = arr[j];
            
            /* Inner loop with conditional */
            for (k = 0; k < 8; k++) {
                if (val & (1 << k)) {
                    block_sum += (val * k);
                } else {
                    block_sum -= (val / (k + 1));
                }
                
                /* Another asm barrier */
                asm volatile ("" ::: "cc", "memory");
            }
            
            /* Conditional branch inside innermost loop */
            if (block_sum > threshold) {
                block_sum >>= 1;
                asm volatile ("" ::: "rax", "rbx", "rcx");
            }
        }
        result += block_sum;
    }
    
    return result;
}

/* Complex switch handler */
__attribute__((noinline))
static void process_switch(int case_id, float* farr, int* iarr, int idx) {
    /* Large switch with non-sequential cases */
    switch (case_id) {
        case 100: farr[idx] *= 1.1f; break;
        case 23:  iarr[idx] += 100; break;
        case 47:  farr[idx] = sqrtf(farr[idx]); break;
        case 12:  iarr[idx] ^= 0x55AA55AA; break;
        case 89:  farr[idx] = farr[idx] * 2.0f - 1.0f; break;
        case 34:  iarr[idx] = iarr[idx] << 3; break;
        case 56:  farr[idx] = 1.0f / farr[idx]; break;
        case 78:  iarr[idx] = ~iarr[idx]; break;
        case 3:   farr[idx] = sinf(farr[idx]); break;
        case 67:  iarr[idx] = iarr[idx] * 3 + 7; break;
        case 45:  farr[idx] = farr[idx] * farr[idx]; break;
        case 90:  iarr[idx] = iarr[idx] | 0xFF00FF00; break;
        case 18:  farr[idx] = logf(farr[idx] + 1.0f); break;
        case 72:  iarr[idx] = iarr[idx] % 256; break;
        case 29:  farr[idx] = cosf(farr[idx]); break;
        case 81:  iarr[idx] = iarr[idx] & 0x00FF00FF; break;
        case 5:   farr[idx] = expf(farr[idx]); break;
        case 63:  iarr[idx] = iarr[idx] >> 2; break;
        case 14:  farr[idx] = tanf(farr[idx]); break;
        case 95:  iarr[idx] = iarr[idx] * iarr[idx]; break;
        case 37:  farr[idx] = farr[idx] + farr[idx]; break;
        case 52:  iarr[idx] = -iarr[idx]; break;
        case 8:   farr[idx] = fabsf(farr[idx]); break;
        case 41:  iarr[idx] = abs(iarr[idx]); break;
        default:  /* Complex default case */
            farr[idx] = process_chunk(farr, idx, idx + 4);
            iarr[idx] = integer_reduction(iarr, 8, 1000);
            error_handler("Default case executed");
            break;
    }
}

int main(void) {
    /* Declare and initialize arrays */
    float float_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < ARRAY_SIZE; i++) {
        float_arr[i] = (i % 100) * 0.1f + 0.5f;
        int_arr[i] = i * 3 + 7;
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with complex control flow */
    for (i = 0; i < ARRAY_SIZE; i += 32) {
        float block_float = 0.0f;
        
        for (j = i; j < i + 32 && j < ARRAY_SIZE; j++) {
            float val = float_arr[j];
            int ival = int_arr[j];
            
            /* Innermost loop with data dependencies */
            for (k = 0; k < 4; k++) {
                /* Loop-carried dependency */
                block_float += val * k;
                ival += (int)(val * 100);
                
                /* Conditional with mixed operations */
                if ((j + k) % 7 == 0) {
                    val = sinf(val) + 1.0f;
                    ival ^= 0x12345678;
                    
                    /* Inline asm with clobbers */
                    asm volatile ("# Test barrier" ::: 
                                 "rax", "rbx", "rcx", "rdx", 
                                 "xmm0", "xmm1", "xmm2", "xmm3",
                                 "memory", "cc");
                } else if ((j + k) % 13 == 0) {
                    val = cosf(val) * 2.0f;
                    ival &= 0xF0F0F0F0;
                }
                
                /* Call noinline function periodically */
                if (k == 2) {
                    float temp = process_chunk(float_arr, j, j + 8);
                    block_float += temp;
                }
            }
            
            /* Update arrays with dependencies */
            float_arr[j] = val;
            int_arr[j] = ival;
            
            /* Complex switch based on computed values */
            int case_id = (ival % 100) + 100 * ((int)val % 3);
            process_switch(case_id, float_arr, int_arr, j);
            
            /* Goto to create irreducible flow */
            if (restart_count < 3 && ival > 1000000) {
                restart_count++;
                goto restart_point;  /* Jump to outer label */
            }
        }
        
        total_float += block_float;
        
        /* Break to outer loop with label */
        if (total_float > 1e6f) {
            break;
        }
    }
    
    /* Second pass with different pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* More mixed operations */
        total_int += integer_reduction(int_arr, 16, 5000);
        
        /* Conditional goto */
        if (i == 512 && restart_count == 0) {
            restart_count++;
            goto restart_point;
        }
        
        /* Continue to next iteration */
        if (int_arr[i] < 0) {
            int_arr[i] = -int_arr[i];
            continue;
        }
        
        /* Call cold function on rare condition */
        if (int_arr[i] == 0x7FFFFFFF) {
            error_handler("Max int detected");
        }
    }
    
    /* Final reduction */
    float final_float = process_chunk(float_arr, 0, ARRAY_SIZE);
    int final_int = integer_reduction(int_arr, ARRAY_SIZE, 10000);
    
    /* Simple validation */
    printf("Results: float=%f, int=%d, restart_count=%d\n", 
           final_float, final_int, restart_count);
    
    /* Check for reasonable values */
    if (!isnan(final_float) && !isinf(final_float) && final_int != 0) {
        printf("Test completed successfully\n");
        return 0;
    } else {
        printf("Test produced invalid results\n");
        return 1;
    }
}
