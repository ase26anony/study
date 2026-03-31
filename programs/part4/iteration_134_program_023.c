/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
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
    
    /* SSE operations */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline asm barrier between dependent operations */
        asm volatile ("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "cc", "memory");
    }
    
    /* Extract SSE result */
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
    /* Complex integer operations with data dependencies */
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        int val = arr[i];
        
        /* Artificial dependency chain */
        for (j = 0; j < 3; j++) {
            val = (val * 1103515245 + 12345) & 0x7fffffff;
            
            /* Another asm barrier */
            asm volatile ("" ::: "eax", "ebx", "ecx", "edx", "cc", "memory");
        }
        
        if (val > threshold) {
            result += val;
        } else {
            result -= val / 2;
        }
    }
    
    return result;
}

/* Main computational function with nested loops */
__attribute__((noinline))
static double complex_computation(float* farr, int* iarr, int size) {
    double total = 0.0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        float outer_sum = 0.0f;
        
        for (j = 0; j < 8; j++) {
            int inner_acc = 0;
            
            for (k = 0; k < 16; k++) {
                /* Loop-carried dependency */
                inner_acc = inner_acc * 1664525 + 1013904223;
                
                /* Conditional branch inside innermost loop */
                if ((inner_acc & 255) > 128) {
                    outer_sum += sinf(farr[(i + j + k) % size]);
                } else {
                    outer_sum -= cosf(farr[(i * j + k) % size]);
                }
                
                /* Mix with integer array */
                iarr[(i * 16 + j * 2 + k) % size] = inner_acc % 1000;
            }
            
            /* Inline asm with clobbers */
            asm volatile ("# Complex dependency here" 
                         ::: "rax", "rbx", "rcx", "rdx", 
                             "xmm0", "xmm1", "xmm2", "xmm3",
                             "cc", "memory");
            
            total += outer_sum * (j + 1);
        }
        
        /* Call to SIMD function */
        float chunk_sum = process_chunk(farr, i * 4, (i + 1) * 4);
        total += chunk_sum;
    }
    
    return total;
}

/* Large switch statement with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: result = iarr[0] + iarr[1]; break;
        case 1: result = iarr[1] * iarr[2]; break;
        case 3: result = iarr[3] - iarr[4]; break;  /* Skip case 2 */
        case 7: result = iarr[5] / (iarr[6] + 1); break;
        case 15: result = (int)(farr[0] * 100.0f); break;
        case 4: result = iarr[4] << 2; break;
        case 8: result = iarr[8] >> 1; break;
        case 12: result = iarr[12] | iarr[13]; break;
        case 19: result = iarr[19] & iarr[20]; break;
        case 5: result = iarr[5] ^ iarr[6]; break;
        case 9: result = -iarr[9]; break;
        case 11: result = abs(iarr[11]); break;
        case 17: result = iarr[17] % 17; break;
        case 21: result = iarr[21] * 3 + 7; break;
        case 23: result = iarr[23] / 2; break;
        case 2: result = iarr[2] + 42; break;  /* Out of order */
        case 6: result = iarr[6] * iarr[7]; break;
        case 10: result = iarr[10] - iarr[11]; break;
        case 13: result = (int)(farr[13] * farr[14]); break;
        case 14: result = iarr[14] << 3; break;
        case 16: result = iarr[16] >> 2; break;
        case 18: result = iarr[18] | 0xFF; break;
        case 20: result = iarr[20] & 0x0F; break;
        case 22: result = iarr[22] ^ 0xAA; break;
        case 24: result = ~iarr[24]; break;
        default:  /* Complex default case */
            result = integer_reduction(iarr, 100, 500);
            error_handler("Default case reached");
            break;
    }
    
    return result;
}

int main(void) {
    /* Declare and initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    int i;
    
    /* Initialize with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = sinf(i * 0.1f) * 100.0f;
        int_array[i] = (i * 1103515245 + 12345) & 0x7fff;
    }
    
    double total = 0.0;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Nested loops with complex computation */
    for (i = 0; i < 3; i++) {
        int j;
        
        for (j = 0; j < 5; j++) {
            /* Call complex computation */
            double partial = complex_computation(float_array, int_array, ARRAY_SIZE);
            total += partial;
            
            /* Use goto to create irreducible flow */
            if (restart_count < 2 && (i == 1 && j == 2)) {
                restart_count++;
                printf("Restarting computation...\n");
                goto restart_point;  /* Jump back */
            }
            
            /* Large switch statement */
            int switch_val = int_array[(i * 5 + j) % ARRAY_SIZE];
            int switch_result = process_switch(switch_val, float_array, int_array);
            
            /* Modify arrays based on switch result */
            int idx = (i * 17 + j * 23) % ARRAY_SIZE;
            if (switch_result > 0) {
                float_array[idx] += switch_result * 0.01f;
                int_array[idx] += switch_result;
            } else {
                float_array[idx] -= (-switch_result) * 0.01f;
                int_array[idx] -= switch_result;
            }
            
            /* Another asm barrier */
            asm volatile ("# Loop iteration complete" 
                         ::: "rax", "rbx", "rcx", "rdx", 
                             "xmm0", "xmm1", "xmm2", "xmm3",
                             "cc", "memory");
        }
        
        /* Continue to outer loop or break based on condition */
        if (total > 1e6) {
            printf("Total too large, breaking outer loop\n");
            break;  /* Break outer loop */
        }
        
        /* Call integer reduction */
        int reduction = integer_reduction(int_array, ARRAY_SIZE / 2, 1000);
        total += reduction;
    }
    
    /* Final validation */
    float checksum = 0.0f;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += float_array[i] + int_array[i];
    }
    
    printf("Computation complete.\n");
    printf("Total: %f\n", total);
    printf("Checksum: %f\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    /* Simple validation */
    if (!isnan(total) && !isnan(checksum)) {
        printf("SUCCESS: Results are valid numbers\n");
        return 0;
    } else {
        printf("FAILURE: Invalid results\n");
        return 1;
    }
}
