#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>

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
    
    /* Requirement 3: SIMD operations */
    __m128 vec_sum = _mm_setzero_ps();
    int i;
    
    for (i = 0; i + 3 < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        vec_sum = _mm_add_ps(vec_sum, vec);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
    }
    
    /* Extract results from SIMD register */
    float temp[4];
    _mm_storeu_ps(temp, vec_sum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Handle remaining elements */
    for (; i < size; i++) {
        sum += data[i];
    }
    
    return sum;
}

__attribute__((noinline))
static int compute_checksum(int* arr, int size) {
    int sum = 0;
    
    /* Requirement 4: Another asm barrier */
    asm volatile ("mfence" ::: "memory");
    
    for (int i = 0; i < size; i++) {
        sum ^= arr[i];
        sum = (sum << 3) | (sum >> 29); /* Rotate left */
    }
    
    return sum;
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1:   result = iarr[0] * 2; break;
        case 3:   result = iarr[1] + iarr[2]; break;
        case 7:   result = iarr[3] ^ iarr[4]; break;
        case 13:  result = (int)(farr[0] * 100.0f); break;
        case 21:  result = iarr[5] % 17; break;
        case 34:  result = iarr[6] << 2; break;
        case 55:  result = iarr[7] >> 1; break;
        case 89:  result = iarr[8] | iarr[9]; break;
        case 144: result = iarr[10] & iarr[11]; break;
        case 233: result = iarr[12] + iarr[13] * 2; break;
        case 377: result = iarr[14] - iarr[15]; break;
        case 610: result = (int)(farr[1] + farr[2]); break;
        case 987: result = iarr[16] * iarr[17]; break;
        case 1597: result = iarr[18] / (iarr[19] + 1); break;
        case 2584: result = ~iarr[20]; break;
        case 4181: result = iarr[21] + iarr[22] + iarr[23]; break;
        case 6765: result = (int)(sqrtf(farr[3]) * 10.0f); break;
        case 10946: result = iarr[24] ^ 0xABCDEF; break;
        case 17711: result = iarr[25] * 3 + 7; break;
        case 28657: result = iarr[26] << 3; break;
        case 46368: result = iarr[27] >> 2; break;
        case 75025: result = iarr[28] + iarr[29] * 4; break;
        case 121393: result = (int)(fabsf(farr[4]) * 1000.0f); break;
        case 196418: result = iarr[30] % 31; break;
        case 317811: result = iarr[31] | 0xFF00; break;
        default:  /* Complex default case */
            result = 0;
            for (int i = 0; i < 32 && i < ARRAY_SIZE; i++) {
                result += iarr[i];
                if (i % 3 == 0) {
                    result *= 2;
                }
            }
            result = (result < 0) ? -result : result;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i * 0.1);
        iarr[i] = i * 3 + 7;
    }
    
    int outer_sum = 0;
    float outer_fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with complex dependencies */
    for (int i = 0; i < 50; i++) {
        int middle_sum = i * 2;
        
        for (int j = 0; j < 40; j++) {
            float inner_fsum = 0.0f;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependencies */
                middle_sum += (i * j + k) % 17;
                inner_fsum += sinf((float)(i + j + k) * 0.01f);
                
                /* Conditional branch inside innermost loop */
                if ((i * j * k) % 13 == 0) {
                    middle_sum -= 5;
                    
                    /* Requirement 4: asm barrier inside conditional */
                    asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11");
                } else if ((i + j + k) % 7 == 3) {
                    middle_sum *= 2;
                }
                
                /* Mixed integer/float operations */
                farr[(i + j + k) % ARRAY_SIZE] += inner_fsum * 0.01f;
                iarr[(i * j + k) % ARRAY_SIZE] ^= middle_sum;
                
                /* Another conditional with goto (Requirement 6) */
                if (restart_count < 2 && middle_sum > 1000000) {
                    restart_count++;
                    goto restart_point;  /* Jump to outer label */
                }
            }
            
            outer_fsum += inner_fsum;
            
            /* Call noinline function with SIMD */
            if (j % 5 == 0) {
                float vec_result = process_vector(farr, ARRAY_SIZE / 2);
                outer_fsum += vec_result * 0.001f;
            }
        }
        
        outer_sum += middle_sum;
        
        /* Call noinline function */
        int checksum = compute_checksum(iarr, ARRAY_SIZE / 4);
        outer_sum ^= checksum;
        
        /* Requirement 5: Execute switch statement */
        int switch_val = (i * 37) % 500000;
        int switch_result = process_switch(switch_val, farr, iarr);
        outer_sum += switch_result;
        
        /* Use goto to implement continue-like behavior for outer loop */
        if (i % 10 == 0) {
            continue;  /* Normal continue */
        }
        
        /* Another goto pattern (Requirement 6) */
        if (i == 25) {
            goto skip_increment;
        }
        
        outer_sum += 1000;
        
skip_increment:
        /* Empty label for goto target */
        asm volatile ("" ::: "memory");
    }
    
    /* Final validation */
    float final_fsum = process_vector(farr, ARRAY_SIZE);
    int final_isum = compute_checksum(iarr, ARRAY_SIZE);
    
    printf("Results: outer_sum=%d, outer_fsum=%f, final_fsum=%f, final_isum=%d\n",
           outer_sum, outer_fsum, final_fsum, final_isum);
    
    /* Call cold function (rarely taken path) */
    if (outer_sum < 0) {
        error_handler("Negative sum detected");
    }
    
    return 0;
}
