/* Test program to exercise GCC scheduler state save/restore cleanup paths */
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
    /* Cold error path - unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SIMD processing */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline asm barrier to force scheduling boundaries */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract SIMD results */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Scalar tail processing */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier with different clobbers */
        asm volatile ("" ::: "cc", "memory", "rax", "rbx");
    }
    
    return sum;
}

__attribute__((noinline))
static int reduce_integer_array(int* arr, int size) {
    /* Complex reduction with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i += 16) {
        int block_sum = 0;
        int limit = (i + 16 < size) ? i + 16 : size;
        
        for (j = i; j < limit; j++) {
            int temp = arr[j];
            
            /* Innermost loop with conditional */
            for (k = 0; k < 8; k++) {
                if (temp & (1 << k)) {
                    block_sum += (j * k);
                } else {
                    block_sum -= (j + k);
                }
                
                /* Conditional goto creating irreducible flow */
                if (block_sum > 1000000) {
                    goto overflow_detected;
                }
            }
            
            /* Another asm barrier */
            asm volatile ("" ::: "memory", "rcx", "rdx");
        }
        
        result += block_sum;
        continue;
        
    overflow_detected:
        error_handler("Potential overflow detected");
        result = result / 2;  /* Try to recover */
    }
    
    return result;
}

__attribute__((noinline))
static void process_with_switch(int value, float* farr, int* iarr) {
    /* Large switch with non-sequential cases */
    switch (value % SWITCH_CASES) {
        case 0:
            farr[0] = sinf(farr[0]);
            break;
        case 1:
            iarr[1] = iarr[0] * 2;
            break;
        case 3:  /* Skip 2 */
            farr[2] = farr[1] + farr[3];
            break;
        case 7:  /* Non-linear jump */
            for (int i = 0; i < 10; i++) {
                iarr[i] += (int)(farr[i] * 100);
            }
            break;
        case 15:
            /* SIMD operation in switch case */
            __m128 a = _mm_set1_ps(1.5f);
            __m128 b = _mm_loadu_ps(farr);
            __m128 c = _mm_mul_ps(a, b);
            _mm_storeu_ps(farr, c);
            break;
        case 22:
            /* Nested switch inside switch */
            switch (iarr[0] % 3) {
                case 0: farr[5] *= 0.5f; break;
                case 1: farr[5] *= 1.5f; break;
                case 2: farr[5] *= 2.5f; break;
            }
            break;
        case 4:
        case 8:
        case 12:
            /* Multiple case fall-through */
            iarr[value % 10] += 1000;
            if (value % 3 == 0) break;
            /* Fall through */
        case 16:
            farr[4] = sqrtf(fabsf(farr[4]));
            break;
        case 19:
            /* Complex computation */
            {
                __m128 x = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
                __m128 y = _mm_loadu_ps(&farr[8]);
                __m128 z = _mm_add_ps(x, y);
                _mm_storeu_ps(&farr[8], z);
            }
            break;
        default:
            /* Default case with its own complexity */
            for (int i = 0; i < 5; i++) {
                farr[i] = process_chunk(farr, i * 4, (i + 1) * 4);
                asm volatile ("" ::: "memory", "xmm4", "xmm5", "xmm6", "xmm7");
            }
            break;
    }
}

int main(void) {
    /* Declare and initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (i % 100) * 0.01f;
        int_array[i] = i * 3;
    }
    
    int restart_count = 0;
    float total_float_sum = 0.0f;
    int total_int_sum = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with complex control flow */
    for (int outer = 0; outer < 5; outer++) {
        if (restart_count > 3) {
            error_handler("Too many restarts");
            break;
        }
        
        for (int mid = 0; mid < 10; mid++) {
            int chunk_start = (outer * 200 + mid * 20) % ARRAY_SIZE;
            int chunk_end = chunk_start + 16;
            if (chunk_end > ARRAY_SIZE) chunk_end = ARRAY_SIZE;
            
            /* Process chunk with mixed operations */
            float chunk_sum = process_chunk(float_array, chunk_start, chunk_end);
            total_float_sum += chunk_sum;
            
            /* Conditional with goto to outer scope */
            if (chunk_sum > 1000.0f && restart_count < 2) {
                restart_count++;
                printf("Restarting computation...\n");
                goto restart_point;  /* Irreducible flow */
            }
            
            for (int inner = 0; inner < 8; inner++) {
                /* Data-dependent branching */
                if ((outer + mid + inner) % 7 == 0) {
                    /* Call noinline function */
                    int_array[chunk_start + inner] = 
                        reduce_integer_array(int_array, chunk_end - chunk_start);
                    
                    /* Inline asm with clobbers */
                    asm volatile (""
                        : 
                        : "r"(int_array), "r"(chunk_start)
                        : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
                } else {
                    /* Different computation path */
                    float_array[chunk_start + inner] = 
                        sinf(float_array[chunk_start + inner]) * 
                        cosf(float_array[chunk_start + inner]);
                }
                
                /* Switch statement inside innermost loop */
                process_with_switch(outer * 100 + mid * 10 + inner, 
                                   float_array, int_array);
            }
            
            /* Continue/break with outer loop targets */
            if (mid == 5) {
                continue;  /* Skips to next mid iteration */
            }
            
            if (total_float_sum > 50000.0f) {
                break;  /* Breaks mid loop */
            }
        }
        
        /* Reduce integer array periodically */
        if (outer % 2 == 0) {
            total_int_sum += reduce_integer_array(int_array, ARRAY_SIZE);
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Final validation */
    float final_float_sum = 0;
    int final_int_sum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        /* SIMD final reduction */
        __m128 v = _mm_loadu_ps(&float_array[i]);
        __m128 sum = _mm_hadd_ps(v, v);
        sum = _mm_hadd_ps(sum, sum);
        final_float_sum += _mm_cvtss_f32(sum);
        
        final_int_sum += int_array[i];
    }
    
    printf("Results: float_sum = %f, int_sum = %d\n", 
           final_float_sum, final_int_sum);
    printf("Test completed %s\n", 
           (final_float_sum != 0.0f && final_int_sum != 0) ? 
           "successfully" : "with zeros");
    
    return 0;
}
