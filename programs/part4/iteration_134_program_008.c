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
    /* Cold path unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SIMD processing */
    for (i = start; i + 3 < end; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrt_vec = _mm_sqrt_ps(squared);
        
        /* Inline assembly barrier creating scheduling boundary */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        
        float temp[4];
        _mm_storeu_ps(temp, sqrt_vec);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* Scalar tail processing */
    for (; i < end; i++) {
        sum += sqrtf(fabsf(data[i]));
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with loop-carried dependencies */
    for (i = 0; i < size / 4; i++) {
        int outer_acc = arr[i];
        
        for (j = 0; j < 8; j++) {
            int middle_acc = outer_acc * j;
            
            for (k = 0; k < 4; k++) {
                /* Complex conditional with data dependency */
                if ((middle_acc + k) % 3 == 0) {
                    result += middle_acc << k;
                    
                    /* Another inline barrier */
                    asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
                } else {
                    result -= (middle_acc >> k) | 1;
                }
                
                /* Cross-iteration dependency */
                middle_acc = (middle_acc * 1103515245 + 12345) & 0x7fffffff;
            }
            
            /* Conditional branch affecting control flow */
            if (middle_acc > threshold && j % 2 == 0) {
                outer_acc += middle_acc % 17;
            } else {
                outer_acc -= middle_acc % 13;
            }
            
            /* Register clobbering asm */
            asm volatile ("# barrier" ::: "cc", "memory");
        }
        
        arr[i] = outer_acc;
    }
    
    return result;
}

/* Non-linear switch handler */
__attribute__((noinline))
static void process_switch(int value, float* farr, int* iarr, int idx) {
    /* Large switch with non-sequential cases */
    switch (value) {
        case 100: farr[idx] *= 1.1f; break;
        case 87:  iarr[idx] += 255; break;
        case 42:  farr[idx] = sinf(farr[idx]); break;
        case 999: iarr[idx] ^= 0xAAAAAAAA; break;
        case 13:  farr[idx] = expf(farr[idx]); break;
        case 77:  iarr[idx] = (iarr[idx] << 3) | (iarr[idx] >> 29); break;
        case 256: farr[idx] = 1.0f / (farr[idx] + 1.0f); break;
        case 512: iarr[idx] = ~iarr[idx]; break;
        case 333: farr[idx] = logf(fabsf(farr[idx]) + 1.0f); break;
        case 19:  iarr[idx] = iarr[idx] * 7 + 1; break;
        case 666: farr[idx] = -farr[idx]; break;
        case 21:  iarr[idx] = iarr[idx] % 1023; break;
        case 888: farr[idx] = farr[idx] * farr[idx]; break;
        case 55:  iarr[idx] = iarr[idx] & 0x55555555; break;
        case 777: farr[idx] = sqrtf(fabsf(farr[idx])); break;
        case 31:  iarr[idx] = iarr[idx] | 0x80000000; break;
        case 111: farr[idx] = farr[idx] + 3.14159f; break;
        case 63:  iarr[idx] = iarr[idx] ^ iarr[idx - 1]; break;
        case 222: farr[idx] = tanf(farr[idx]); break;
        case 95:  iarr[idx] = iarr[idx] + iarr[idx + 1]; break;
        case 444: farr[idx] = cosf(farr[idx]); break;
        case 127: iarr[idx] = iarr[idx] - iarr[idx - 1]; break;
        case 555: farr[idx] = farr[idx] / 2.0f; break;
        case 255: iarr[idx] = iarr[idx] * iarr[idx]; break;
        default:  /* Complex default case */
            farr[idx] = process_chunk(farr, idx % 16, idx % 16 + 8);
            iarr[idx] = integer_reduction(iarr, 32, 1000);
            break;
    }
}

int main(void) {
    /* Declare and initialize arrays */
    float float_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    int i, j, k;
    
    /* Initialize with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        float_arr[i] = (i % 100) * 0.1f;
        int_arr[i] = i * 1103515245 + 12345;
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    int restart_count = 0;
    
restart_point:  /* Label for goto jumps */
    
    /* Triple nested loops with mixed operations */
    for (i = 0; i < 16; i++) {
        float outer_sum = 0.0f;
        
        for (j = 0; j < 32; j++) {
            int inner_acc = 0;
            
            for (k = 0; k < 8; k++) {
                /* Complex data-dependent computation */
                int idx = (i * 2048 + j * 64 + k) % ARRAY_SIZE;
                
                /* Conditional with both paths taken */
                if ((i + j + k) % 7 == 0) {
                    float_arr[idx] = process_chunk(float_arr, 
                                                  idx % 32, 
                                                  (idx % 32) + 16);
                } else {
                    int_arr[idx] = integer_reduction(int_arr, 
                                                    64, 
                                                    (i * j * k) & 0xFF);
                }
                
                /* Use goto to create irreducible flow */
                if (restart_count < 3 && (i * j * k) % 1001 == 0) {
                    restart_count++;
                    goto restart_point;  /* Jump to outer label */
                }
                
                /* Call switch handler */
                process_switch((i * j + k * 17) % 1000, 
                              float_arr, 
                              int_arr, 
                              idx);
                
                /* Accumulate results with cross-iteration dependency */
                inner_acc += int_arr[idx] ^ (i * j * k);
                
                /* Memory barrier affecting scheduling */
                asm volatile ("# complex barrier" ::: 
                             "memory", "eax", "ebx", "ecx", "edx", 
                             "xmm0", "xmm1", "xmm2", "xmm3");
            }
            
            /* Conditional break to outer loop */
            if (inner_acc > 1000000 && j > 16) {
                break;  /* Breaks middle loop */
            }
            
            outer_sum += sqrtf(fabsf(inner_acc));
            
            /* Continue with label targeting outer loop */
            if (inner_acc < 0) {
                continue;  /* Continues middle loop */
            }
        }
        
        total_float += outer_sum;
        
        /* Another goto creating complex flow */
        if (i == 8 && restart_count == 0) {
            goto skip_section;
        }
        
        total_int += integer_reduction(int_arr, 128, 500);
        
skip_section:
        /* Empty section reached via goto */
        asm volatile ("" ::: "memory");
    }
    
    /* Final validation */
    float checksum_float = 0.0f;
    int checksum_int = 0;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum_float += float_arr[i];
        checksum_int += int_arr[i];
    }
    
    /* Use cold error handler */
    if (checksum_float != checksum_float || checksum_int != checksum_int) {
        error_handler("NaN detected in results");
    }
    
    printf("Final float checksum: %f\n", checksum_float);
    printf("Final int checksum: %d\n", checksum_int);
    printf("Restart count: %d\n", restart_count);
    
    return 0;
}
