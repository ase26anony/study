#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: Noinline and cold attributed functions */
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
        
        /* Requirement 4: Assembly barrier */
        asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int result = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < size; i++) {
        /* Create artificial dependencies */
        result = (result * 1103515245 + 12345) ^ arr[i];
        
        /* Another assembly barrier with clobbers */
        asm volatile ("# barrier" ::: "cc", "memory", "esi", "edi");
        
        if (i % 7 == 0) {
            barrier = result;
        }
    }
    return result;
}

/* Requirement 5: Large switch with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1:   result = iarr[0] * 2; break;
        case 3:   result = iarr[1] + iarr[2]; break;
        case 7:   result = iarr[3] | iarr[4]; break;
        case 15:  result = iarr[5] ^ iarr[6]; break;
        case 31:  result = iarr[7] << 2; break;
        case 63:  result = iarr[8] >> 1; break;
        case 127: result = (int)(farr[0] * 100.0f); break;
        case 255: result = (int)(farr[1] + farr[2]); break;
        case 511: result = iarr[9] * iarr[10]; break;
        case 1023: result = complex_reduction(iarr, 50); break;
        case 2047: 
            for (int i = 0; i < 10; i++) {
                result += (int)farr[i];
            }
            break;
        case 4095: result = iarr[11] % 17; break;
        case 8191: result = iarr[12] & iarr[13]; break;
        case 16383: result = ~iarr[14]; break;
        case 32767: result = abs(iarr[15]); break;
        case 65535: result = iarr[16] + iarr[17] * 3; break;
        case 131071: result = iarr[18] - iarr[19]; break;
        case 262143: result = iarr[20] / (iarr[21] + 1); break;
        case 524287: result = (iarr[22] << 3) | (iarr[23] >> 2); break;
        case 1048575: result = iarr[24] ^ 0xAAAAAAAA; break;
        case 2097151: result = -iarr[25]; break;
        case 4194303: result = iarr[26] + iarr[27] - iarr[28]; break;
        case 8388607: result = (iarr[29] * iarr[30]) & 0xFF; break;
        case 16777215: result = iarr[31] | 0x12345678; break;
        default:
            /* Complex default case */
            result = 0;
            for (int i = 0; i < 32 && i < ARRAY_SIZE; i++) {
                result += iarr[i] * (i + 1);
            }
            error_handler("Default case reached");
            break;
    }
    
    /* Assembly barrier in switch */
    asm volatile ("# switch_end" ::: "memory", "eax", "ebx");
    return result;
}

int main() {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i % 100) * 0.1f;
        iarr[i] = i * 3 + 7;
    }
    
    int total = 0;
    float fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with dependencies */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependencies */
                total = total * 1664525 + 1013904223;
                total ^= (i << 16) | (j << 8) | k;
                
                /* Mixed integer/float operations */
                fsum += sinf(farr[(i + j + k) % ARRAY_SIZE]) * 
                       cosf(farr[(i * j + k) % ARRAY_SIZE]);
                
                /* Conditional branch inside innermost loop */
                if ((i * j * k) % 17 == 0) {
                    /* Call noinline function */
                    fsum += process_chunk(farr, k % ARRAY_SIZE, 
                                         (k + 32) % ARRAY_SIZE);
                    
                    /* Another assembly barrier */
                    asm volatile ("# inner_loop_barrier" ::: 
                                 "memory", "ecx", "edx");
                }
                
                /* More complex condition */
                if ((i ^ j ^ k) % 13 == 5) {
                    total += complex_reduction(&iarr[k % ARRAY_SIZE], 16);
                }
            }
            
            /* Requirement 6: goto to create irreducible flow */
            if (j == 15 && restart_count < 2) {
                restart_count++;
                goto restart_point;
            }
        }
        
        /* Call switch statement */
        int switch_val = total % (1 << 24);
        int switch_result = process_switch(switch_val, farr, iarr);
        total += switch_result;
        
        /* Conditional continue to outer loop */
        if (i % 3 == 1) {
            continue;
        }
        
        /* Break to outer loop with goto */
        if (i == 8 && total > 1000000) {
            goto outer_break;
        }
    }
    
outer_break:
    
    /* More irreducible flow */
    if (restart_count == 1) {
        goto final_calc;
    } else if (restart_count == 2) {
        goto cleanup;
    }
    
final_calc:
    /* Final computation with mixed operations */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        __m128 fvec1 = _mm_loadu_ps(&farr[i]);
        __m128 fvec2 = _mm_loadu_ps(&farr[i + 4]);
        __m128 fsum_vec = _mm_add_ps(fvec1, fvec2);
        
        /* Interleave with integer operations */
        total += iarr[i] + iarr[i + 1];
        
        /* Memory barrier */
        asm volatile ("# final_barrier" ::: "memory");
        
        float temp[4];
        _mm_storeu_ps(temp, fsum_vec);
        fsum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
cleanup:
    /* Validate results */
    int checksum = total + (int)fsum;
    
    /* Use checksum to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Scheduler stress test completed. Checksum: %d\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    return 0;
}
