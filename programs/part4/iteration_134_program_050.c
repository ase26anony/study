#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics mixed with scalar ops */
    for (int i = 0; i < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "movaps %%xmm0, %%xmm1\n\t"
            "shufps $0x1B, %%xmm1, %%xmm1\n\t"
            :
            :
            : "xmm0", "xmm1", "cc"
        );
        
        float temp[4];
        _mm_storeu_ps(temp, squared);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    volatile int barrier = 0;  /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Create data dependencies */
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        result ^= arr[i];
        
        /* Requirement 4: Another asm barrier */
        asm volatile (
            "addl $1, %0\n\t"
            : "+r" (barrier)
            :
            : "cc", "memory"
        );
    }
    return result;
}

/* Requirement 5: Large switch with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 100: result = iarr[0] * 2; break;
        case 87:  result = (int)(farr[1] * 100.0f); break;
        case 42:  result = iarr[2] ^ iarr[3]; break;
        case 13:  result = iarr[4] % 17; break;
        case 255: result = ~iarr[5]; break;
        case 64:  result = iarr[6] << 2; break;
        case 128: result = iarr[7] >> 1; break;
        case 33:  result = iarr[8] + iarr[9]; break;
        case 99:  result = iarr[10] - iarr[11]; break;
        case 7:   result = iarr[12] | iarr[13]; break;
        case 19:  result = iarr[14] & iarr[15]; break;
        case 200: result = iarr[16] / 2; break;
        case 150: result = iarr[17] * 3; break;
        case 88:  result = iarr[18] ^ 0xFF; break;
        case 44:  result = iarr[19] + 100; break;
        case 22:  result = iarr[20] - 50; break;
        case 11:  result = iarr[21] << 1; break;
        case 5:   result = iarr[22] >> 2; break;
        case 250: result = iarr[23] * iarr[24]; break;
        case 125: result = iarr[25] % 10; break;
        case 63:  result = iarr[26] | 0x0F; break;
        case 31:  result = iarr[27] & 0xF0; break;
        case 16:  result = iarr[28] ^ iarr[29]; break;
        default:  /* Complex default case */
            result = complex_reduction(iarr, 30);
            if (result < 0) {
                error_handler("Negative result in switch default");
            }
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i * 0.1f);
        iarr[i] = i * 1103515245 + 12345;
    }
    
    int outer_sum = 0;
    float fp_sum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Triple nested loops with dependencies */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            int inner_acc = i * j;
            
            for (int k = 0; k < 64; k++) {
                /* Loop-carried dependency */
                inner_acc = (inner_acc * 1664525 + 1013904223) & 0x7FFFFFFF;
                
                /* Mixed integer/float operations */
                fp_sum += sinf(farr[(i + j + k) % ARRAY_SIZE]) * 0.1f;
                
                /* Requirement 1: Conditional branch inside innermost loop */
                if ((inner_acc & 0xFF) > 128) {
                    outer_sum += inner_acc;
                    farr[(i + j) % ARRAY_SIZE] += 0.5f;
                } else if ((inner_acc & 0x3F) < 16) {
                    outer_sum -= inner_acc / 2;
                    farr[(j + k) % ARRAY_SIZE] -= 0.25f;
                } else {
                    outer_sum ^= inner_acc;
                }
                
                /* More data dependencies */
                iarr[(i * 2048 + j * 64 + k) % ARRAY_SIZE] = 
                    (iarr[(i * 2048 + j * 64 + k) % ARRAY_SIZE] + inner_acc) & 0xFFFF;
            }
            
            /* Call noinline function with SIMD */
            if (j % 8 == 0) {
                fp_sum += process_vector(farr, 64);
            }
        }
        
        /* Requirement 6: goto to create irreducible flow */
        if (i == 8 && restart_count == 0) {
            restart_count++;
            goto restart_point;  /* Jump back to restart */
        }
        
        /* Another asm barrier between loop iterations */
        asm volatile (
            "mfence\n\t"
            :
            :
            : "memory"
        );
    }
    
    /* Requirement 5: Large switch statement */
    int switch_val = (outer_sum & 0xFF) % 300;
    int switch_result = process_switch(switch_val, farr, iarr);
    
    /* More complex control flow with goto */
    if (switch_result < 0) {
        goto cleanup;
    }
    
    /* Final reduction */
    int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* Unrolled with dependencies */
        final_sum += iarr[i] + iarr[i+1];
        final_sum ^= iarr[i+2] * iarr[i+3];
        final_sum = (final_sum + iarr[i+4]) | iarr[i+5];
        final_sum = (final_sum - iarr[i+6]) & iarr[i+7];
        
        /* Another asm barrier */
        asm volatile (
            "clflush (%0)\n\t"
            :
            : "r" (&iarr[i])
            : "memory"
        );
    }
    
    /* Mix in floating point results */
    final_sum += (int)(fp_sum * 1000.0f);
    final_sum += switch_result;
    
cleanup:
    /* Validate results (simplified) */
    printf("Result: %d (checksum: %08x)\n", 
           final_sum, 
           (unsigned int)(final_sum ^ 0xDEADBEEF));
    
    if (final_sum != 0) {
        printf("Test completed successfully (non-zero result indicates computation occurred)\n");
    }
    
    return 0;
}
