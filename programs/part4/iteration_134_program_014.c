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
    }
    
    /* Store SIMD result */
    float temp[4];
    _mm_storeu_ps(temp, vec_sum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < size; i++) {
        sum += data[i];
    }
    
    /* Requirement 4: Inline assembly barrier */
    asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j;
    
    /* Requirement 1: Nested loops with dependencies */
    for (i = 0; i < size; i++) {
        int temp = arr[i];
        
        for (j = 0; j < 8; j++) {
            /* Loop-carried dependency */
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            
            /* Conditional branch inside innermost loop */
            if (temp % 7 == 0) {
                temp >>= 1;
            } else if (temp > threshold) {
                temp -= threshold;
            }
            
            /* Mixed operations */
            float ftemp = (float)temp;
            ftemp = ftemp * 0.5f + sinf(ftemp * 0.001f);
            temp = (int)ftemp;
        }
        
        result ^= temp;
        
        /* Another assembly barrier */
        asm volatile ("# Barrier" ::: "cc", "memory");
    }
    
    return result;
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0:
            farr[0] = sinf(farr[0]);
            result = 1;
            break;
        case 1:
            iarr[1] = iarr[0] * 3 + 7;
            result = -1;
            break;
        case 3:  /* Non-sequential case */
            farr[2] = farr[1] * farr[3];
            result = 2;
            break;
        case 5:
            iarr[3] = (iarr[2] << 2) | (iarr[1] >> 3);
            result = 3;
            break;
        case 7:
            farr[4] = sqrtf(fabsf(farr[4]));
            result = 4;
            break;
        case 11:
            iarr[5] = iarr[4] ^ iarr[3] ^ iarr[2];
            result = 5;
            break;
        case 13:
            farr[6] = expf(farr[6] * 0.1f);
            result = 6;
            break;
        case 17:
            iarr[7] = iarr[6] % (iarr[5] + 1);
            result = 7;
            break;
        case 19:
            farr[8] = logf(fabsf(farr[8]) + 1.0f);
            result = 8;
            break;
        case 23:
            iarr[9] = ~iarr[8];
            result = 9;
            break;
        case 2:  /* Out of order */
            farr[10] = farr[9] / (farr[11] + 0.001f);
            result = 10;
            break;
        case 4:
            iarr[11] = iarr[10] - iarr[9] * 2;
            result = 11;
            break;
        case 6:
            farr[12] = cosf(farr[12] * 3.14159f / 180.0f);
            result = 12;
            break;
        case 8:
            iarr[13] = iarr[12] & 0x55555555;
            result = 13;
            break;
        case 9:
            farr[14] = tanf(farr[14]);
            result = 14;
            break;
        case 10:
            iarr[15] = iarr[14] | 0xAAAAAAAA;
            result = 15;
            break;
        case 12:
            farr[16] = farr[15] + farr[17] * 2.0f;
            result = 16;
            break;
        case 14:
            iarr[17] = iarr[16] + (iarr[15] << 3);
            result = 17;
            break;
        case 15:
            farr[18] = powf(farr[18], 1.5f);
            result = 18;
            break;
        case 16:
            iarr[19] = iarr[18] * iarr[17] / (iarr[16] + 1);
            result = 19;
            break;
        case 18:
            farr[20] = fmodf(farr[20], 3.14159f);
            result = 20;
            break;
        case 20:
            iarr[21] = (iarr[20] > iarr[19]) ? iarr[20] : iarr[19];
            result = 21;
            break;
        case 21:
            farr[22] = atanf(farr[22]);
            result = 22;
            break;
        case 22:
            iarr[23] = iarr[22] + iarr[21] * 3 - iarr[20];
            result = 23;
            break;
        case 24:
            farr[24] = farr[23] * 2.0f - farr[22];
            result = 24;
            break;
        default:  /* Complex default case */
            for (int k = 0; k < 10; k++) {
                farr[k] = farr[k] * 0.9f + 0.1f;
                iarr[k] = (iarr[k] + k) * 2;
            }
            result = -99;
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
        iarr[i] = i * 1103515245 + 12345;
    }
    
    int outer_result = 0;
    float float_result = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Triple nested loops */
    for (int outer = 0; outer < 3; outer++) {
        for (int mid = 0; mid < 5; mid++) {
            for (int inner = 0; inner < 10; inner++) {
                /* Complex computation with dependencies */
                int idx = (outer * 100 + mid * 20 + inner) % ARRAY_SIZE;
                
                /* Loop-carried dependency chain */
                static int chain = 0;
                chain = (chain * 13 + idx) & 0xFFF;
                
                /* Mixed integer/float operations */
                float fval = farr[idx] * 0.5f + sinf((float)chain * 0.01f);
                int ival = iarr[idx] ^ chain;
                
                /* Conditional with unlikely path */
                if (ival < 0) {
                    /* Cold path - rarely taken */
                    error_handler("Negative value detected");
                }
                
                /* Update arrays with dependency */
                farr[idx] = fval * 1.1f - cosf((float)ival * 0.001f);
                iarr[idx] = (ival * 3 + 7) & 0x7FFFFFFF;
                
                /* Another assembly barrier */
                asm volatile ("# Inner loop barrier" ::: "memory", "esi", "edi");
                
                /* Requirement 6: goto to create irreducible flow */
                if (restart_count < 2 && chain % 1000 == 999) {
                    restart_count++;
                    goto restart_point;
                }
            }
            
            /* Call noinline function with SIMD */
            if (mid % 2 == 0) {
                float_result += process_vector(farr, ARRAY_SIZE / 4);
            }
        }
        
        /* Call noinline function for integer reduction */
        outer_result ^= integer_reduction(iarr, ARRAY_SIZE / 8, 1000000);
        
        /* Requirement 5: Switch statement in outer loop */
        int switch_val = (outer_result + outer) & 0x7FFFFFFF;
        int switch_res = process_switch(switch_val, farr, iarr);
        
        /* Use switch result */
        outer_result += switch_res * 31;
        
        /* Requirement 6: continue with label target */
        if (outer == 1) {
            continue;
        }
    }
    
    /* Final validation */
    float final_sum = 0.0f;
    int final_xor = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += farr[i];
        final_xor ^= iarr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Results: float_sum=%.6f, int_xor=%d, outer_result=%d\n", 
           final_sum, final_xor, outer_result);
    
    /* Simple checksum validation */
    int checksum = ((int)final_sum) ^ final_xor ^ outer_result;
    if (checksum != 0) {
        printf("Validation passed (checksum: %d)\n", checksum);
    } else {
        printf("Validation failed\n");
    }
    
    return 0;
}
