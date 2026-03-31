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
    asm volatile("" ::: "memory");  /* Memory barrier */
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
        asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "cc", "memory");
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
    /* Complex integer operations with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        int block_sum = 0;
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx >= size) break;
            
            /* Conditional inside innermost loop */
            if (arr[idx] > threshold) {
                for (k = 0; k < 3; k++) {
                    /* Artificial dependency chain */
                    block_sum += arr[idx] * (k + 1);
                    
                    /* Another asm barrier */
                    asm volatile("" ::: "eax", "ebx", "ecx", "edx", "cc", "memory");
                }
            } else {
                block_sum -= arr[idx];
            }
        }
        result += block_sum;
        
        /* Conditional goto to create irreducible flow */
        if (result < -1000000) {
            error_handler("Underflow detected");
            goto restart_point;  /* Requirement 6 */
        }
    }
    
    return result;

restart_point:
    /* Recovery code - rarely executed */
    return 0;
}

/* Another noinline function with switch statement */
__attribute__((noinline))
static void process_switch(int value, float* farr, int* iarr, int idx) {
    /* Large switch with non-sequential cases */
    switch (value) {
        case 1:  farr[idx] *= 1.1f; break;
        case 3:  farr[idx] += iarr[idx] * 0.5f; break;
        case 7:  farr[idx] = sqrtf(farr[idx]); break;
        case 12: iarr[idx] ^= 0xFF; break;
        case 15: farr[idx] = farr[idx] * farr[idx] - 1.0f; break;
        case 18: iarr[idx] = (iarr[idx] << 3) | (iarr[idx] >> 29); break;
        case 22: farr[idx] = 1.0f / (farr[idx] + 0.001f); break;
        case 25: iarr[idx] += (int)(farr[idx] * 100.0f); break;
        case 30: farr[idx] = sinf(farr[idx]); break;
        case 35: iarr[idx] = iarr[idx] * 17 + 1; break;
        case 40: farr[idx] = farr[idx] > 0 ? farr[idx] : -farr[idx]; break;
        case 45: iarr[idx] = ~iarr[idx]; break;
        case 50: farr[idx] = logf(farr[idx] + 1.0f); break;
        case 55: iarr[idx] = iarr[idx] % 97; break;
        case 60: farr[idx] = farr[idx] * 2.0f - 0.5f; break;
        case 65: iarr[idx] = iarr[idx] & 0x55555555; break;
        case 70: farr[idx] = cosf(farr[idx]); break;
        case 75: iarr[idx] = iarr[idx] | 0xAAAAAAAA; break;
        case 80: farr[idx] = expf(farr[idx] * 0.01f); break;
        case 85: iarr[idx] = iarr[idx] << 1; break;
        case 90: farr[idx] = tanf(farr[idx]); break;
        case 95: iarr[idx] = iarr[idx] >> 2; break;
        case 100: farr[idx] = fmodf(farr[idx], 3.14159f); break;
        default:  /* Complex default case */
            farr[idx] = process_chunk(farr, idx, idx + 4);
            iarr[idx] = integer_reduction(iarr, 8, 100);
            asm volatile("" ::: "memory", "cc", "eax", "ebx", "ecx", "edx");
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
        float_arr[i] = (float)(i % 100) * 0.1f + 0.5f;
        int_arr[i] = (i * 17) % 233;
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    
    /* Triple nested loops with complex control flow */
    for (i = 0; i < 10; i++) {
        float outer_sum = 0.0f;
        
        for (j = 0; j < 20; j++) {
            int inner_sum = 0;
            
            for (k = 0; k < ARRAY_SIZE / 20; k++) {
                int idx = (i * 20 + j) * (ARRAY_SIZE / 200) + k;
                if (idx >= ARRAY_SIZE) continue;
                
                /* Loop-carried dependencies */
                inner_sum += int_arr[idx];
                
                /* Conditional branch inside innermost loop */
                if (int_arr[idx] % 7 == 0) {
                    float_arr[idx] = process_chunk(float_arr, 
                                                  idx > 4 ? idx - 4 : 0, 
                                                  idx + 4 < ARRAY_SIZE ? idx + 4 : ARRAY_SIZE);
                } else if (int_arr[idx] % 13 == 0) {
                    /* Call to function with switch */
                    process_switch(int_arr[idx] % SWITCH_CASES, 
                                  float_arr, int_arr, idx);
                }
                
                /* More dependencies */
                if (k > 0) {
                    float_arr[idx] += float_arr[idx - 1] * 0.1f;
                }
                
                /* Inline asm with clobbers */
                asm volatile("" ::: "xmm0", "xmm1", "xmm2", "xmm3", 
                                       "eax", "ebx", "ecx", "edx", 
                                       "cc", "memory");
            }
            
            /* Goto to create irreducible control flow */
            if (inner_sum > 1000000) {
                goto reduce_scale;
            }
            
            outer_sum += (float)inner_sum * 0.01f;
            continue;
            
        reduce_scale:
            /* This label creates a loop with multiple entry points */
            outer_sum += (float)inner_sum * 0.001f;
            
            /* Another asm barrier */
            asm volatile("" ::: "memory", "cc");
        }
        
        total_float += outer_sum;
        
        /* Call integer reduction function */
        total_int += integer_reduction(int_arr, ARRAY_SIZE, 100 + i * 10);
        
        /* Another switch in main loop */
        switch (i % 8) {
            case 0: total_int += 1; break;
            case 1: total_float += 1.0f; break;
            case 2: total_int *= 2; break;
            case 3: total_float *= 1.5f; break;
            case 4: total_int -= 3; break;
            case 5: total_float -= 0.5f; break;
            case 6: total_int ^= 0x1234; break;
            case 7: total_float = sqrtf(total_float); break;
        }
    }
    
    /* Final validation */
    float checksum = total_float + (float)total_int;
    
    /* Use the results to prevent optimization */
    if (checksum != 0.0f) {
        printf("Computation completed. Checksum: %f\n", checksum);
    } else {
        error_handler("Zero checksum");
    }
    
    return 0;
}
