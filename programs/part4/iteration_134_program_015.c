#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    float sum = 0.0f;
    for (int i = start; i < end; i += 4) {
        /* Requirement 3: SIMD operations */
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        float temp[4];
        _mm_storeu_ps(temp, squared);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1");
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    volatile int* varr = arr; /* Prevent optimizations */
    
    for (int i = 0; i < n; i++) {
        result ^= varr[i];
        /* Artificial scheduling barrier */
        asm volatile ("# Barrier" : : : "cc", "memory");
    }
    return result;
}

/* Requirement 5: Large switch with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1: result = iarr[0] + 1; break;
        case 3: result = iarr[1] * 2; break;
        case 7: result = iarr[2] ^ iarr[3]; break;
        case 13: result = (int)farr[0] + iarr[4]; break;
        case 21: result = iarr[5] << 2; break;
        case 34: result = iarr[6] >> 1; break;
        case 55: result = iarr[7] | 0xFF; break;
        case 89: result = iarr[8] & 0x0F; break;
        case 144: result = iarr[9] - iarr[10]; break;
        case 233: result = iarr[11] % 17; break;
        case 377: result = iarr[12] * iarr[13]; break;
        case 610: result = iarr[14] + iarr[15]; break;
        case 987: result = iarr[16] ^ iarr[17]; break;
        case 1597: result = iarr[18] | iarr[19]; break;
        case 2584: result = iarr[20] & iarr[21]; break;
        case 4181: result = iarr[22] << iarr[23]; break;
        case 6765: result = iarr[24] >> 3; break;
        case 10946: result = ~iarr[25]; break;
        case 17711: result = iarr[26] + iarr[27]; break;
        case 28657: result = iarr[28] * 3; break;
        case 46368: result = iarr[29] / 2; break;
        case 75025: result = iarr[30] ^ 0xAAAA; break;
        case 121393: result = iarr[31] | 0x5555; break;
        case 196418: result = iarr[32] & 0x3333; break;
        default:
            /* Complex default case */
            result = 0;
            for (int i = 0; i < 10; i++) {
                result += iarr[i % 32];
                asm volatile ("# Default barrier" : : : "memory", "rsi", "rdi");
            }
            if (result < 0) {
                error_handler("Negative result in default case");
            }
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[SIZE];
    int iarr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        farr[i] = (i % 100) * 0.1f;
        iarr[i] = i * 3;
    }
    
    int outer_sum = 0;
    float fp_sum = 0.0f;
    int restart_count = 0;
    
restart_point: /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with complex dependencies */
    for (int i = 0; i < 32; i++) {
        int mid_sum = 0;
        
        for (int j = 0; j < 64; j++) {
            float inner_fp = 0.0f;
            
            for (int k = 0; k < 16; k++) {
                /* Loop-carried dependencies */
                mid_sum += iarr[(i + j + k) % SIZE];
                inner_fp += farr[(i * j + k) % SIZE];
                
                /* Conditional branch inside innermost loop */
                if ((i ^ j ^ k) % 7 == 0) {
                    mid_sum -= iarr[k % SIZE];
                    inner_fp *= 0.99f;
                    
                    /* Requirement 4: More asm barriers */
                    asm volatile ("# Inner barrier" : : : "memory", "r8", "r9", "r10", "r11");
                } else if ((i * j * k) % 13 == 0) {
                    mid_sum ^= 0x55AA;
                    inner_fp = inner_fp / 1.5f;
                }
                
                /* Mixed integer/float operations */
                if (inner_fp > 100.0f) {
                    iarr[k % SIZE] = (int)(inner_fp * 0.5f);
                }
            }
            
            fp_sum += inner_fp;
            
            /* Call noinline function with SIMD */
            if (j % 8 == 0) {
                float chunk_sum = process_chunk(farr, j * 4, (j + 8) * 4);
                fp_sum += chunk_sum;
            }
        }
        
        outer_sum += mid_sum;
        
        /* Requirement 6: goto to create irreducible flow */
        if (mid_sum > 1000000 && restart_count < 2) {
            restart_count++;
            iarr[i % SIZE] = mid_sum % 1000;
            goto restart_point;
        }
        
        /* Break to outer loop with label */
        if (outer_sum > 5000000) {
            goto finish_loops;
        }
    }
    
finish_loops:
    
    /* Process switch statement */
    int switch_val = (outer_sum % 46368) + 1;
    int switch_result = process_switch(switch_val, farr, iarr);
    
    /* Call complex reduction */
    int reduction_result = complex_reduction(iarr, SIZE / 4);
    
    /* Final validation */
    int final_result = outer_sum + (int)fp_sum + switch_result + reduction_result;
    
    /* Use result to prevent dead code elimination */
    volatile int sink = final_result;
    
    printf("Test completed. Result: %d (sink: %d)\n", final_result, sink);
    
    /* Trigger cold error path occasionally */
    if (final_result < 0) {
        error_handler("Unexpected negative result");
    }
    
    return 0;
}
