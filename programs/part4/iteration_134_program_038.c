#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline)) __attribute__((cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics and SIMD operations */
    __m128 accum = _mm_setzero_ps();
    int i;
    
    for (i = 0; i + 3 < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        accum = _mm_add_ps(accum, vec);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "mfence\n\t"
            : 
            : 
            : "memory", "cc"
        );
    }
    
    /* Extract results from SIMD accumulator */
    float temp[4];
    _mm_storeu_ps(temp, accum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < size; i++) {
        sum += data[i];
    }
    
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j, k;
    
    /* Requirement 1: Nested loops with complex control flow */
    for (i = 0; i < size; i++) {
        int temp = arr[i];
        
        for (j = 0; j < 3; j++) {
            float fp_temp = (float)temp;
            
            for (k = 0; k < 2; k++) {
                /* Loop-carried dependency */
                fp_temp = fp_temp * 1.1f + (float)k;
                
                /* Conditional branch inside innermost loop */
                if (fp_temp > threshold * 100.0f) {
                    /* Requirement 4: Another asm barrier */
                    asm volatile (
                        "pause\n\t"
                        : 
                        : 
                        : "memory"
                    );
                    fp_temp *= 0.5f;
                }
                
                /* Mixed integer/float operations */
                temp = (int)fp_temp + (k << 2);
            }
            
            /* Data dependency across loop levels */
            arr[i] = temp ^ (j * 0x5A5A5A5A);
        }
        
        result += temp;
        
        /* Another asm barrier with register clobber */
        asm volatile (
            "xor %%eax, %%eax\n\t"
            "cpuid\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "memory", "cc"
        );
    }
    
    return result;
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = iarr[0] + (int)farr[0];
            farr[0] *= 1.5f;
            break;
        case 1:
            result = iarr[1] * 2;
            farr[1] = process_vector(farr, 10);
            break;
        case 3:  /* Non-sequential case */
            result = iarr[3] | 0xFF;
            break;
        case 5:
            result = iarr[5] << 2;
            farr[5] = result * 0.01f;
            break;
        case 7:
            result = complex_reduction(iarr, 50, 100);
            break;
        case 9:
            for (int i = 0; i < 10; i++) {
                farr[i] = farr[i] * farr[i] + 1.0f;
            }
            result = (int)farr[9];
            break;
        case 11:
            result = iarr[11] ^ iarr[12];
            break;
        case 13:
            result = (int)(farr[13] * 100.0f);
            break;
        case 15:
            result = iarr[15] % 17;
            break;
        case 17:
            result = iarr[17] + iarr[18] + iarr[19];
            break;
        case 19:
            result = (iarr[19] > 0) ? iarr[19] : -iarr[19];
            break;
        case 21:
            result = iarr[21] * iarr[22];
            break;
        case 23:
            result = iarr[23] / (iarr[24] + 1);
            break;
        case 2:  /* Out of order */
            result = iarr[2] - 100;
            break;
        case 4:
            result = iarr[4] + 0xABCD;
            break;
        case 6:
            result = (int)(farr[6] / farr[7]);
            break;
        case 8:
            result = iarr[8] & 0xFFFF;
            break;
        case 10:
            result = iarr[10] | iarr[11];
            break;
        case 12:
            result = iarr[12] ^ 0x12345678;
            break;
        case 14:
            result = iarr[14] + iarr[15] * 2;
            break;
        case 16:
            result = (int)(farr[16] * farr[17]);
            break;
        case 18:
            result = iarr[18] << 4;
            break;
        case 20:
            result = iarr[20] >> 2;
            break;
        case 22:
            result = iarr[22] + iarr[23] - iarr[24];
            break;
        case 24:
            result = iarr[24] * 3;
            break;
        default:  /* Complex default case */
            result = complex_reduction(iarr, 20, 50);
            for (int i = 0; i < 5; i++) {
                farr[i] = process_vector(farr + i * 5, 5);
            }
            result += (int)farr[0];
            break;
    }
    
    return result;
}

int main() {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i * 1.1);
        iarr[i] = i * 3 + 7;
    }
    
    int total_result = 0;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label for irreducible flow */
    
    /* Requirement 1: Deep nested loops with dependencies */
    for (int outer = 0; outer < 5; outer++) {
        int outer_acc = outer * 100;
        
        for (int mid = 0; mid < 10; mid++) {
            float mid_fp = (float)mid * 0.5f;
            
            for (int inner = 0; inner < 20; inner++) {
                /* Complex data dependencies */
                mid_fp = mid_fp * 1.05f + (float)inner;
                outer_acc += (int)mid_fp;
                
                /* Conditional with unlikely path */
                if (mid_fp > 1000.0f && inner % 17 == 0) {
                    /* Cold path - rarely taken */
                    error_handler("Threshold exceeded");
                    mid_fp *= 0.1f;
                }
                
                /* SIMD operation inside innermost loop */
                if (inner % 5 == 0) {
                    __m128 a = _mm_set_ps(farr[inner], farr[inner+1], 
                                         farr[inner+2], farr[inner+3]);
                    __m128 b = _mm_set_ps(1.1f, 1.2f, 1.3f, 1.4f);
                    __m128 c = _mm_mul_ps(a, b);
                    
                    float temp[4];
                    _mm_storeu_ps(temp, c);
                    farr[inner] = temp[0];
                }
                
                /* Requirement 6: goto to create irreducible flow */
                if (restart_count < 2 && outer_acc > 5000 && inner == 15) {
                    restart_count++;
                    goto restart_point;  /* Jump back to restart */
                }
            }
            
            /* Mix integer and float operations */
            iarr[mid] = outer_acc ^ (int)(mid_fp * 10.0f);
            
            /* Another asm barrier */
            asm volatile (
                "lfence\n\t"
                : 
                : 
                : "memory"
            );
        }
        
        /* Requirement 5: Switch statement inside outer loop */
        int switch_val = process_switch(outer_acc, farr + outer * 50, iarr + outer * 50);
        total_result += switch_val;
        
        /* Break to outer label */
        if (outer_acc > 10000) {
            goto finish_early;
        }
    }
    
    /* Continue normal flow */
    for (int i = 0; i < ARRAY_SIZE / 4; i++) {
        total_result += complex_reduction(iarr + i * 4, 4, 100);
    }
    
    goto finalize;
    
finish_early:
    total_result = total_result % 0x7FFF;
    
    /* Requirement 6: Jump forward */
    if (total_result < 0) {
        goto finalize;
    }
    
    /* More computation */
    for (int i = 0; i < 100; i++) {
        farr[i] = process_vector(farr, i + 1);
    }
    
finalize:
    /* Final validation */
    float final_sum = process_vector(farr, ARRAY_SIZE);
    int int_sum = complex_reduction(iarr, ARRAY_SIZE, 1000);
    
    printf("Results: total_result=%d, float_sum=%.2f, int_sum=%d\n",
           total_result, final_sum, int_sum);
    
    /* Simple checksum validation */
    int checksum = total_result + (int)final_sum + int_sum;
    if (checksum != 0) {
        printf("Test completed successfully (checksum: %d)\n", checksum);
    } else {
        error_handler("Zero checksum detected");
    }
    
    return 0;
}
