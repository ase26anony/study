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
    /* Cold error path - unlikely to be taken */
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
        
        /* Inline asm barrier to force scheduler partitioning */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract horizontal sum */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier with different clobbers */
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size) {
    int result = 0;
    volatile int* volatile_ptr = arr;  /* Prevent optimizations */
    
    for (int i = 0; i < size; i++) {
        /* Complex data dependency chain */
        result = (result * 1103515245 + 12345) ^ arr[i];
        
        /* Conditional with unpredictable branch */
        if (__builtin_expect((arr[i] & 0xFF) == 0, 0)) {
            /* Cold path inside hot loop */
            error_handler("Unexpected zero byte");
        }
        
        /* Memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    return result;
}

/* Global variables to create dependencies */
static float g_float_array[ARRAY_SIZE];
static int g_int_array[ARRAY_SIZE];
static int g_counter = 0;

int main(void) {
    /* Initialize arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        g_float_array[i] = sinf(i * 0.1f);
        g_int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    
    /* Nested loops with complex control flow (Requirement 1) */
    for (int outer = 0; outer < 10; outer++) {
        for (int middle = 0; middle < 20; middle++) {
            /* Loop-carried dependency */
            float chunk_sum = 0.0f;
            
            for (int inner = 0; inner < ARRAY_SIZE/10; inner++) {
                int idx = (outer * 100 + middle * 5 + inner) % ARRAY_SIZE;
                
                /* Mixed operations with dependencies */
                g_float_array[idx] = g_float_array[idx] * 1.01f + 0.5f;
                g_int_array[idx] = g_int_array[idx] + inner;
                
                /* Conditional inside innermost loop */
                if (g_float_array[idx] > 1000.0f) {
                    /* Reset if value too large */
                    g_float_array[idx] = 0.0f;
                    g_counter++;
                    
                    /* Use goto for irreducible flow (Requirement 6) */
                    if (g_counter > 100) {
                        goto restart_computation;
                    }
                }
                
                /* Accumulate with data dependency */
                chunk_sum += g_float_array[idx];
                
                /* Inline asm with clobbers (Requirement 4) */
                asm volatile ("# Dummy asm" ::: "eax", "ebx", "ecx", "edx", "memory");
            }
            
            total_sum += chunk_sum;
            
            /* Call noinline function (Requirement 2) */
            float processed = process_chunk(g_float_array, 
                                          middle * 50, 
                                          (middle + 1) * 50);
            total_sum += processed;
        }
        
        /* Complex switch statement (Requirement 5) */
        int switch_val = (int)(total_sum * 100) % SWITCH_CASES;
        
        switch (switch_val) {
            case 0:  g_int_array[0] += 1; break;
            case 1:  g_int_array[1] *= 2; break;
            case 3:  g_int_array[3] = ~g_int_array[3]; break;
            case 7:  g_int_array[7] ^= 0xAAAAAAAA; break;
            case 15: g_int_array[15] = g_int_array[15] >> 3; break;
            case 2:  g_int_array[2] -= g_int_array[1]; break;
            case 4:  g_int_array[4] = abs(g_int_array[4]); break;
            case 5:  g_int_array[5] = g_int_array[5] * 3 + 7; break;
            case 6:  g_int_array[6] = g_int_array[6] / 2; break;
            case 8:  g_int_array[8] |= 0xFF00FF00; break;
            case 9:  g_int_array[9] &= 0x00FF00FF; break;
            case 10: g_int_array[10] = g_int_array[10] << 4; break;
            case 11: g_int_array[11] = -g_int_array[11]; break;
            case 12: g_int_array[12] = g_int_array[12] % 17; break;
            case 13: g_int_array[13] = g_int_array[13] | g_int_array[14]; break;
            case 14: g_int_array[14] = g_int_array[14] & g_int_array[15]; break;
            case 16: g_int_array[16] = g_int_array[16] ^ g_int_array[17]; break;
            case 17: g_int_array[17] = g_int_array[17] + g_int_array[18]; break;
            case 18: g_int_array[18] = g_int_array[18] - g_int_array[19]; break;
            case 19: g_int_array[19] = g_int_array[19] * g_int_array[20]; break;
            case 20: 
                /* Default-like complex operation */
                for (int k = 0; k < 10; k++) {
                    g_int_array[k] = (g_int_array[k] * 13 + 7) % 256;
                }
                break;
            case 21: g_int_array[21] = g_int_array[21] >> 1; break;
            case 22: g_int_array[22] = g_int_array[22] << 2; break;
            case 23: g_int_array[23] = g_int_array[23] | 1; break;
            case 24: g_int_array[24] = g_int_array[24] & ~1; break;
            default: 
                /* Complex default case */
                for (int k = 20; k < 30; k++) {
                    g_int_array[k] = (g_int_array[k] + k) * 3;
                }
                break;
        }
        
        /* Vector intrinsics (Requirement 3) */
        __m128i vec_a = _mm_set_epi32(g_int_array[0], g_int_array[1], 
                                     g_int_array[2], g_int_array[3]);
        __m128i vec_b = _mm_set_epi32(g_int_array[4], g_int_array[5], 
                                     g_int_array[6], g_int_array[7]);
        __m128i vec_c = _mm_add_epi32(vec_a, vec_b);
        
        /* Store back */
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, vec_c);
        for (int j = 0; j < 4; j++) {
            g_int_array[j] = temp[j];
        }
    }
    
    /* Label for goto (Requirement 6) */
    restart_computation:
    
    /* Final reduction */
    int_result = integer_reduction(g_int_array, ARRAY_SIZE);
    
    /* Compute checksum */
    float final_checksum = total_sum + int_result;
    
    /* Validate (simplified) */
    printf("Computed checksum: %f\n", final_checksum);
    
    if (!isnan(final_checksum) && !isinf(final_checksum)) {
        printf("SUCCESS: Scheduler test completed\n");
        return 0;
    } else {
        printf("FAIL: Invalid result\n");
        return 1;
    }
}
