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
    asm volatile ("" : : : "memory");
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
        
        /* Artificial scheduling barrier */
        asm volatile ("# SSE Barrier" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "memory");
    }
    
    /* Extract horizontal sum */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < end; i++) {
        sum += data[i];
        /* Another barrier */
        asm volatile ("# Scalar Barrier" : : : "rax", "rbx", "rcx", "rdx", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i += 16) {
        int block_sum = 0;
        for (j = i; j < i + 16 && j < size; j++) {
            int val = arr[j];
            
            /* Innermost loop with conditional */
            for (k = 0; k < 8; k++) {
                if (val & (1 << k)) {
                    block_sum += (j * k);
                } else {
                    block_sum -= (j / (k + 1));
                }
                
                /* Loop-carried dependency */
                val = (val * 1103515245 + 12345) & 0x7fffffff;
            }
            
            /* Conditional branch inside innermost loop */
            if (block_sum > 1000000) {
                block_sum >>= 1;
                asm volatile ("# Reduction Barrier" : : : "r8", "r9", "r10", "r11", "memory");
            }
        }
        result ^= block_sum;
    }
    
    return result;
}

__attribute__((noinline, cold))
static void unlikely_path_operation(float* farr, int* iarr, int idx) {
    /* Cold function with complex control flow */
    volatile float temp = 0.0f;
    
    switch (idx % 7) {
        case 0: temp = sinf(farr[idx]); break;
        case 1: temp = cosf(farr[idx]); break;
        case 2: temp = sqrtf(fabsf(farr[idx])); break;
        case 3: temp = farr[idx] * farr[idx]; break;
        case 4: temp = 1.0f / (fabsf(farr[idx]) + 0.001f); break;
        case 5: temp = logf(fabsf(farr[idx]) + 1.0f); break;
        default: temp = expf(farr[idx] * 0.01f); break;
    }
    
    /* Memory clobber */
    asm volatile ("# Cold Path Barrier" : : : "memory");
    
    iarr[idx] += (int)(temp * 1000);
}

/* Main computational function */
static int complex_computation(float* farr, int* iarr, int size) {
    int i, j, k;
    float total = 0.0f;
    int checksum = 0;
    
    /* Label for goto jumps */
    restart_point:
    
    /* Triple nested loops with mixed operations */
    for (i = 0; i < size; i += 32) {
        float block_acc = 0.0f;
        
        for (j = i; j < i + 32 && j < size; j++) {
            float fval = farr[j];
            int ival = iarr[j];
            
            /* Innermost loop with data dependencies */
            for (k = 0; k < 4; k++) {
                /* Mixed FP and integer operations */
                fval = fval * 1.01f + (float)k * 0.5f;
                ival = (ival * 13 + 17) % 1024;
                
                /* Conditional with goto potential */
                if ((ival & 0xFF) == 0x7F) {
                    /* Rare condition - jump to outer label */
                    if (j > size / 2) {
                        iarr[j] = ival;
                        goto restart_point;  /* Irreducible control flow */
                    }
                }
                
                /* SIMD-like operations using scalar */
                __m128 v1 = _mm_set_ps(fval, fval * 2.0f, fval * 3.0f, fval * 4.0f);
                __m128 v2 = _mm_set_ps((float)k, (float)(k+1), (float)(k+2), (float)(k+3));
                __m128 v3 = _mm_add_ps(v1, v2);
                
                float temp[4];
                _mm_storeu_ps(temp, v3);
                block_acc += temp[0] + temp[1] + temp[2] + temp[3];
                
                /* Register clobber */
                asm volatile ("# Inner Loop Barrier" 
                            : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                                  "xmm4", "xmm5", "xmm6", "xmm7", "memory");
            }
            
            farr[j] = fval;
            iarr[j] = ival;
            
            /* Conditional call to cold function */
            if ((j & 0x3F) == 0x3F) {  /* Every 64th element */
                unlikely_path_operation(farr, iarr, j);
            }
        }
        
        total += block_acc;
        
        /* Call to noinline function with SIMD */
        if (i % 128 == 0) {
            float chunk_sum = process_chunk(farr, i, i + 32);
            checksum += (int)(chunk_sum * 100.0f);
        }
    }
    
    /* Large switch statement with non-linear cases */
    int switch_val = ((int)total ^ checksum) % SWITCH_CASES;
    
    switch (switch_val) {
        case 0:  checksum += iarr[0] * 2; break;
        case 1:  checksum += iarr[1] / 3; break;
        case 3:  checksum += iarr[2] << 1; break;  /* Gap in sequence */
        case 7:  checksum -= iarr[3]; break;
        case 15: checksum ^= iarr[4]; break;
        case 2:  checksum |= iarr[5]; break;  /* Out of order */
        case 4:  checksum &= iarr[6]; break;
        case 8:  checksum = ~checksum; break;
        case 16: checksum = abs(checksum); break;
        case 5:  checksum += (int)farr[0]; break;
        case 6:  checksum *= 2; break;
        case 9:  checksum /= 2; break;
        case 10: checksum = checksum % 1000; break;
        case 11: checksum = -checksum; break;
        case 12: checksum = checksum >> 2; break;
        case 13: checksum = checksum << 2; break;
        case 14: checksum = checksum | 0xFF; break;
        case 17: checksum = checksum & 0xFF; break;
        case 18: checksum ^= 0xAAAA; break;
        case 19: checksum += 0x5555; break;
        case 20: checksum = (checksum * 3) / 2; break;
        case 21: checksum = checksum + size; break;
        case 22: checksum = checksum - total; break;
        case 23: checksum = checksum * checksum; break;
        case 24: checksum = sqrtf(fabsf(checksum)); break;
        default: /* Complex default case */
            for (i = 0; i < 16; i++) {
                checksum += iarr[i] * i;
                asm volatile ("# Switch Default Barrier" : : : "r12", "r13", "r14", "r15", "memory");
            }
            break;
    }
    
    return checksum;
}

int main(void) {
    /* Initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (float)(i % 100) * 0.1f;
        int_array[i] = i * 1103515245 + 12345;
    }
    
    /* Perform integer reduction first */
    int int_result = integer_reduction(int_array, ARRAY_SIZE);
    printf("Integer reduction result: %d\n", int_result);
    
    /* Main complex computation */
    int checksum = complex_computation(float_array, int_array, ARRAY_SIZE);
    printf("Computation checksum: %d\n", checksum);
    
    /* Validate with simple check */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += float_array[i];
    }
    
    printf("Final float sum: %f\n", final_sum);
    
    if (checksum != 0 || final_sum != final_sum) {  /* Check for NaN */
        error_handler("Computation validation failed");
        return 1;
    }
    
    printf("Test completed successfully\n");
    return 0;
}
