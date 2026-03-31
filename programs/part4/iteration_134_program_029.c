#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: Noinline and cold attributed functions */
__attribute__((noinline, cold))
void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
void process_with_simd(float* restrict a, float* restrict b, int n) {
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        __m128 vec_a = _mm_loadu_ps(&a[i]);
        __m128 vec_b = _mm_loadu_ps(&b[i]);
        __m128 result = _mm_add_ps(_mm_mul_ps(vec_a, vec_b),
                                   _mm_set1_ps(1.0f));
        _mm_storeu_ps(&a[i], result);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
    }
}

__attribute__((noinline))
int complex_reduction(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Create artificial dependency chain */
        arr[i] = (sum * 3) / 2;
    }
    return sum;
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 1:  result = iarr[0] + 1; break;
        case 3:  result = iarr[1] * 2; break;
        case 7:  result = iarr[2] / 3; break;
        case 13: result = iarr[3] - 4; break;
        case 21: result = iarr[4] | 0xFF; break;
        case 34: result = iarr[5] & 0xAA; break;
        case 55: result = iarr[6] ^ 0x55; break;
        case 89: result = iarr[7] << 2; break;
        case 144: result = iarr[8] >> 1; break;
        case 233: 
            farr[0] = sinf(farr[0]) * cosf(farr[1]);
            result = (int)farr[0];
            break;
        case 377: 
            farr[1] = sqrtf(farr[1] + farr[2]);
            result = (int)farr[1];
            break;
        case 610: 
            farr[2] = farr[2] * farr[3] / farr[4];
            result = (int)farr[2];
            break;
        case 987: 
            for (int i = 0; i < 8; i++) {
                result += iarr[i] * i;
            }
            break;
        case 1597: 
            result = iarr[0] * iarr[1] - iarr[2] * iarr[3];
            break;
        case 2584: 
            result = (iarr[4] + iarr[5]) * (iarr[6] - iarr[7]);
            break;
        case 4181: 
            result = iarr[8] % 17 + iarr[9] % 19;
            break;
        case 6765: 
            result = ~iarr[10] & iarr[11];
            break;
        case 10946: 
            result = iarr[12] | iarr[13] | iarr[14];
            break;
        case 17711: 
            result = (iarr[15] << 3) | (iarr[16] >> 2);
            break;
        case 28657: 
            result = iarr[17] ^ iarr[18] ^ iarr[19];
            break;
        case 46368: 
            result = abs(iarr[20]) * abs(iarr[21]);
            break;
        case 75025: 
            result = iarr[22] + iarr[23] * 2 - iarr[24];
            break;
        case 121393: 
            result = complex_reduction(iarr, 10);
            break;
        default:
            /* Complex default case */
            float sum = 0.0f;
            for (int i = 0; i < 16; i++) {
                sum += farr[i % 8] * i;
            }
            result = (int)sum;
            if (result < 0) {
                error_handler("Negative result in switch default");
            }
            break;
    }
    
    /* Another asm barrier */
    asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11");
    
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
    float outer_fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 32; i++) {
        int mid_sum = 0;
        
        for (int j = 0; j < 64; j++) {
            float inner_fsum = 0.0f;
            
            for (int k = 0; k < 16; k++) {
                /* Loop-carried dependencies */
                mid_sum += iarr[k] * (i + j + k);
                inner_fsum += farr[k] * (i - j + k);
                
                /* Conditional branch inside innermost loop */
                if ((i * j * k) % 7 == 0) {
                    iarr[k] += 2;
                    farr[k] *= 1.1f;
                } else if ((i + j + k) % 11 == 0) {
                    iarr[k] -= 1;
                    farr[k] /= 1.05f;
                    
                    /* Requirement 4: asm barrier with clobbers */
                    asm volatile ("" ::: "memory", "cc", "r12", "r13", "r14", "r15");
                }
                
                /* Mixed integer and floating-point operations */
                if (k % 3 == 0) {
                    farr[k] = sinf(farr[k]) + cosf(inner_fsum);
                    iarr[k] = (int)(farr[k] * 100) ^ iarr[k];
                }
            }
            
            /* Call noinline function with SIMD */
            process_with_simd(&farr[j * 4], &farr[j * 4 + 16], 16);
            
            outer_fsum += inner_fsum;
        }
        
        outer_sum += mid_sum;
        
        /* Requirement 5: Switch statement called from loop */
        int switch_val = (i * 17) % 30000;
        int switch_result = process_switch(switch_val, farr, iarr);
        outer_sum += switch_result;
        
        /* Requirement 6: goto to create irreducible flow */
        if (i == 15 && restart_count < 2) {
            restart_count++;
            goto restart_point;
        }
        
        /* Another asm barrier */
        asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
    }
    
    /* Additional complex loop with goto */
    int idx = 0;
    while (idx < 100) {
        if (idx % 17 == 0) {
            goto skip_computation;
        }
        
        iarr[idx] = complex_reduction(&iarr[idx], 8);
        idx += 3;
        continue;
        
    skip_computation:
        farr[idx] = sqrtf(fabsf(farr[idx]));
        idx += 2;
        
        if (idx > 50 && restart_count < 3) {
            restart_count++;
            goto restart_point;
        }
    }
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += iarr[i] + (int)farr[i];
    }
    
    printf("Result: outer_sum=%d, outer_fsum=%f, checksum=%d\n", 
           outer_sum, outer_fsum, checksum);
    
    if (checksum != 0) {  /* Will never be 0, just for demonstration */
        printf("Test completed successfully (scheduler stress test)\n");
    }
    
    return 0;
}
