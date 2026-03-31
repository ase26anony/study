/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Global variables to create dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void cold_path_handler(int error_code) {
    /* Rarely taken cold path */
    asm volatile ("# Cold path barrier" ::: "memory", "cc");
    global_counter -= error_code;
}

__attribute__((noinline))
static float process_vector(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* Scalar prefix */
    for (i = start; i < start + 4 && i < end; i++) {
        sum += data[i] * 0.5f;
    }
    
    /* SIMD operations */
    if (end - start >= 8) {
        __m128 accum = _mm_setzero_ps();
        for (i = start + 4; i + 3 < end; i += 4) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            __m128 half = _mm_set1_ps(0.5f);
            vec = _mm_mul_ps(vec, half);
            accum = _mm_add_ps(accum, vec);
            
            /* Inline asm barrier between SIMD operations */
            asm volatile ("# SIMD barrier %0" : "+x"(vec) : : "memory", "cc");
        }
        
        /* Extract results from SIMD */
        float temp[4];
        _mm_storeu_ps(temp, accum);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* Scalar suffix */
    for (; i < end; i++) {
        sum += data[i] * 0.25f;
    }
    
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    /* Complex reduction with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        int block_sum = 0;
        for (j = 0; j < 8; j++) {
            int inner_acc = arr[i * 4];
            for (k = 1; k < 4; k++) {
                /* Loop-carried dependency */
                inner_acc = inner_acc * 3 + arr[i * 4 + k];
                
                /* Conditional branch inside innermost loop */
                if (inner_acc > 1000000) {
                    inner_acc /= 2;
                    /* Rare cold path call */
                    if (inner_acc < 0) cold_path_handler(inner_acc);
                }
                
                /* Inline asm with clobbers */
                asm volatile ("# Inner loop barrier %0" 
                             : "+r"(inner_acc) 
                             : : "memory", "cc", "%eax", "%ebx");
            }
            block_sum += inner_acc;
        }
        result ^= block_sum;  /* Non-linear combination */
    }
    
    return result;
}

/* Another noinline function with switch statement */
__attribute__((noinline))
static void process_switch(int value, float* farr, int* iarr) {
    /* Large switch with non-sequential cases */
    switch (value % SWITCH_CASES) {
        case 0:
            farr[0] = farr[0] * 1.1f + 0.5f;
            break;
        case 3:  /* Non-sequential */
            iarr[1] = iarr[0] << 2;
            break;
        case 7:
            farr[2] = process_vector(farr, 0, 8);
            break;
        case 1:
            iarr[3] = iarr[2] | 0xFF;
            break;
        case 15:
            /* Complex case with SIMD */
            __m128 v1 = _mm_loadu_ps(farr);
            __m128 v2 = _mm_set1_ps(2.0f);
            v1 = _mm_mul_ps(v1, v2);
            _mm_storeu_ps(farr, v1);
            break;
        case 22:
            iarr[4] = complex_reduction(iarr, 16);
            break;
        case 8:
            farr[5] = farr[4] / farr[3];
            break;
        case 19:
            iarr[6] = iarr[5] * iarr[4] - iarr[3];
            break;
        case 4:
            farr[7] = farr[6] + farr[5] * farr[4];
            break;
        case 11:
            iarr[8] = (iarr[7] << 3) | (iarr[6] >> 2);
            break;
        case 23:
            /* More SIMD */
            __m128i vi = _mm_set1_epi32(iarr[9]);
            asm volatile ("# Switch case barrier" ::: "memory", "cc");
            break;
        case 17:
            farr[10] = process_vector(farr, 8, 16);
            break;
        case 2:
            iarr[11] = iarr[10] ^ iarr[9];
            break;
        case 14:
            farr[12] = farr[11] - farr[10];
            break;
        case 21:
            iarr[13] = iarr[12] + iarr[11] * 3;
            break;
        case 6:
            farr[14] = farr[13] * 3.14f;
            break;
        case 18:
            iarr[15] = complex_reduction(iarr, 32);
            break;
        case 9:
            farr[16] = farr[15] / 2.0f + 1.0f;
            break;
        case 24:
            iarr[17] = iarr[16] & 0xFFFF;
            break;
        case 12:
            farr[18] = process_vector(farr, 16, 24);
            break;
        case 5:
            iarr[19] = iarr[18] | iarr[17];
            break;
        case 20:
            farr[20] = farr[19] * farr[18] - farr[17];
            break;
        case 13:
            iarr[21] = iarr[20] << 1;
            break;
        case 10:
            farr[22] = farr[21] + 100.0f;
            break;
        case 16:
            iarr[23] = complex_reduction(iarr, 48);
            break;
        default:  /* Case 25+ */
            /* Complex default case */
            for (int d = 0; d < 8; d++) {
                farr[d] = process_vector(farr, d * 4, d * 4 + 4);
                iarr[d] = complex_reduction(iarr, d * 8);
                asm volatile ("# Default case barrier %0" 
                             : "+r"(d) : : "memory", "cc", "%ecx", "%edx");
            }
            break;
    }
}

int main(void) {
    /* Declare and initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (float)(i % 100) * 0.1f;
        int_array[i] = i * 3 + 7;
    }
    
    int outer_loop_counter = 0;
    float checksum = 0.0f;
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with complex control flow */
    for (int outer = 0; outer < 4; outer++) {
        if (outer_loop_counter > 1000) {
            /* Use goto to create irreducible flow */
            goto skip_inner;
        }
        
        for (int middle = 0; middle < 8; middle++) {
            int block_start = (outer * 8 + middle) * 16;
            
            /* Mixed operations with dependencies */
            float block_sum = 0.0f;
            for (int inner = 0; inner < 16; inner++) {
                int idx = block_start + inner;
                
                /* Loop-carried dependency */
                float val = float_array[idx];
                val = val * 1.5f + (float)inner * 0.1f;
                
                /* Conditional with likely/unlikely pattern */
                if (__builtin_expect(val > 50.0f, 0)) {
                    val = val / 2.0f;
                    /* Call cold function on rare condition */
                    if (val < 0) cold_path_handler((int)val);
                }
                
                float_array[idx] = val;
                block_sum += val;
                
                /* Inline asm barrier with clobbers */
                asm volatile ("# Inner loop computation %0" 
                             : "+r"(idx), "+t"(val) 
                             : : "memory", "cc", "%xmm0", "%xmm1");
                
                /* Another conditional that might trigger goto */
                if (inner == 7 && block_sum > 1000.0f) {
                    /* Jump to outer scope */
                    goto skip_inner;
                }
            }
            
            checksum += block_sum;
            
            /* Process int array with SIMD */
            __m128i accum_i = _mm_setzero_si128();
            for (int simd_idx = 0; simd_idx < 16; simd_idx += 4) {
                __m128i vec = _mm_loadu_si128(
                    (__m128i*)&int_array[block_start + simd_idx]);
                accum_i = _mm_add_epi32(accum_i, vec);
            }
            
            /* Store back */
            int temp[4];
            _mm_storeu_si128((__m128i*)temp, accum_i);
            for (int t = 0; t < 4; t++) {
                int_array[block_start + t] = temp[t];
            }
            
            skip_inner:
            /* Continue after goto target */
            outer_loop_counter++;
            
            /* Call noinline function periodically */
            if (middle % 3 == 0) {
                float partial = process_vector(float_array, 
                                             block_start, 
                                             block_start + 8);
                checksum += partial;
            }
        }
        
        /* Complex switch statement based on loop state */
        process_switch(outer_loop_counter, float_array, int_array);
        
        /* Another goto to create non-structured flow */
        if (outer == 2 && checksum > 50000.0f) {
            /* Jump backward to restart */
            checksum = 0.0f;
            goto restart_point;
        }
    }
    
    /* Final reduction with complex dependency chain */
    int final_result = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        int chunk_result = complex_reduction(int_array + i, 8);
        
        /* Interleave with float operations */
        float chunk_float = process_vector(float_array + i, 0, 8);
        checksum += chunk_float;
        
        /* Non-linear combination */
        final_result ^= chunk_result;
        final_result = (final_result << 3) | (final_result >> 29);  /* rotate */
        
        /* Memory barrier */
        asm volatile ("# Final reduction barrier" ::: "memory", "cc");
    }
    
    /* Validation */
    printf("Final result: %d\n", final_result);
    printf("Checksum: %f\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Simple validation */
    if (final_result != 0 || checksum != 0.0f) {
        printf("Test completed with computed values\n");
    } else {
        printf("Test completed - zero results (unexpected)\n");
    }
    
    return 0;
}
