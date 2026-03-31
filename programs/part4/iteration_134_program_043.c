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
    /* Cold path unlikely to be taken */
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
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline asm barrier between dependent operations */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract horizontal sum */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier creating scheduling partition */
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    /* Complex integer operations with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        int block_sum = 0;
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx >= size) break;
            
            /* Conditional branch inside innermost loop */
            if (arr[idx] > threshold) {
                for (k = 0; k < 3; k++) {
                    /* Artificial dependency chain */
                    block_sum += arr[idx] * (k + 1);
                    
                    /* Inline asm with clobbers */
                    asm volatile ("# Dependency barrier" 
                                 ::: "eax", "ebx", "ecx", "edx", "memory");
                }
            } else {
                block_sum -= arr[idx];
            }
            
            /* Another conditional with goto target */
            if (block_sum > 1000000) {
                goto reduce_overflow;
            }
        }
        result += block_sum;
        
        /* SSE operation in the middle of integer loop */
        if (i % 8 == 0) {
            __m128i v = _mm_set1_epi32(result);
            int temp[4];
            _mm_storeu_si128((__m128i*)temp, v);
            result = temp[0];  /* Use result */
        }
    }
    
    return result;

reduce_overflow:
    error_handler("Integer reduction overflow");
    return result / 2;  /* Try to recover */
}

/* Function with large switch statement */
__attribute__((noinline))
static void process_switch(int value, float* farr, int* iarr, int size) {
    /* Dense, non-linear switch cases */
    switch (value) {
        case 0: farr[0] *= 1.1f; break;
        case 100: iarr[0] += 100; break;
        case 1: farr[1] = sinf(farr[0]); break;
        case 50: iarr[1] ^= 0x55; break;
        case 2: farr[2] = farr[0] + farr[1]; break;
        case 75: iarr[2] = iarr[0] | iarr[1]; break;
        case 3: farr[3] = farr[0] * farr[1] * farr[2]; break;
        case 25: iarr[3] = iarr[0] & iarr[1]; break;
        case 4: farr[4] = 1.0f / farr[0]; break;
        case 125: iarr[4] = iarr[0] << 2; break;
        case 5: farr[5] = sqrtf(farr[0]); break;
        case 150: iarr[5] = iarr[0] >> 1; break;
        case 6: farr[6] = logf(farr[0] + 1.0f); break;
        case 175: iarr[6] = ~iarr[0]; break;
        case 7: farr[7] = expf(farr[0]); break;
        case 200: iarr[7] = iarr[0] * 3; break;
        case 8: farr[8] = farr[0] - farr[1]; break;
        case 225: iarr[8] = iarr[0] % 17; break;
        case 9: farr[9] = farr[0] / farr[1]; break;
        case 250: iarr[9] = -iarr[0]; break;
        case 10: farr[10] = fabsf(farr[0]); break;
        case 275: iarr[10] = abs(iarr[0]); break;
        case 11: farr[11] = ceilf(farr[0]); break;
        case 300: iarr[11] = iarr[0] * iarr[1]; break;
        default:
            /* Complex default case */
            for (int i = 0; i < size && i < 12; i++) {
                farr[i] = farr[i] * 0.5f + iarr[i % 12];
                iarr[i] = (int)farr[i] ^ iarr[i];
                
                /* Inline asm in default case */
                asm volatile ("# Switch default processing" 
                             ::: "memory", "xmm4", "xmm5", "xmm6", "xmm7");
            }
            break;
    }
}

int main(void) {
    /* Declare and initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i % 100) * 0.1f;
        iarr[i] = i * 2;
    }
    
    int outer_loop_counter = 0;
    float total_float_sum = 0.0f;
    int total_int_sum = 0;
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with complex control flow */
    for (int i = 0; i < 10; i++) {
        if (outer_loop_counter++ > 100) {
            /* Rare condition that might trigger goto */
            goto finish_up;
        }
        
        for (int j = 0; j < 20; j++) {
            int chunk_start = (i * 20 + j) * 2;
            int chunk_end = chunk_start + 2;
            
            if (chunk_end > ARRAY_SIZE) {
                chunk_end = ARRAY_SIZE;
            }
            
            /* Call noinline function with SIMD */
            float chunk_sum = process_chunk(farr, chunk_start, chunk_end);
            total_float_sum += chunk_sum;
            
            /* Conditional with goto to outer scope */
            if (chunk_sum > 1000.0f && j % 7 == 0) {
                goto skip_inner;
            }
            
            for (int k = 0; k < 5; k++) {
                /* Mixed operations in innermost loop */
                int idx = (i * 100 + j * 5 + k) % ARRAY_SIZE;
                
                /* Data-dependent branching */
                if (iarr[idx] > 500) {
                    farr[idx] = farr[idx] * 2.0f - 1.0f;
                    
                    /* Inline asm creating scheduling barrier */
                    asm volatile ("# Inner loop barrier" 
                                 ::: "rax", "rbx", "rcx", "rdx", 
                                     "r8", "r9", "r10", "r11", "memory");
                } else {
                    farr[idx] = farr[idx] * 0.5f + 0.1f;
                }
                
                /* Loop-carried dependency */
                total_int_sum += iarr[idx] * (k + 1);
                
                /* Another conditional that might break to outer loop */
                if (total_int_sum > 1000000 && k == 3) {
                    goto skip_inner;
                }
            }
            
            skip_inner:
            
            /* Call integer reduction function */
            int int_result = integer_reduction(iarr + chunk_start, 
                                              chunk_end - chunk_start, 
                                              250);
            total_int_sum += int_result;
            
            /* Large switch statement based on computed value */
            int switch_val = (int)farr[chunk_start] + int_result;
            switch_val = switch_val % SWITCH_CASES * 10;
            
            process_switch(switch_val, farr + chunk_start, 
                          iarr + chunk_start, chunk_end - chunk_start);
            
            /* Check for restart condition */
            if (total_float_sum > 50000.0f && i == 5 && j == 10) {
                /* This goto creates irreducible control flow */
                total_float_sum *= 0.9f;
                goto restart_point;
            }
        }
    }
    
    finish_up:
    
    /* Final validation */
    float final_float_sum = 0.0f;
    int final_int_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_float_sum += farr[i];
        final_int_sum += iarr[i];
    }
    
    /* Simple checksum validation */
    float checksum = final_float_sum + (float)final_int_sum;
    printf("Final checksum: %f\n", checksum);
    
    if (checksum > 0.0f) {
        printf("Test completed successfully (scheduler stress test)\n");
    } else {
        error_handler("Unexpected checksum value");
    }
    
    return 0;
}
