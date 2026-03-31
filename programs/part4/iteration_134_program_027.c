/* Test program to trigger free_sched_state cleanup in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
float process_chunk(float* data, int start, int end) {
    float sum = 0.0f;
    for (int i = start; i < end; i += 4) {
        /* Use SSE intrinsics */
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        float temp[4];
        _mm_storeu_ps(temp, squared);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
        
        /* Insert assembly barrier */
        asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
    }
    return sum;
}

__attribute__((noinline))
int integer_reduction(int* arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        result += arr[i];
        /* Complex dependency chain */
        arr[i] = (arr[i] * 3 + 7) % 256;
        
        /* Another barrier */
        asm volatile ("# barrier" ::: "cc", "memory");
    }
    return result;
}

/* Complex control flow function */
__attribute__((noinline))
int nested_switch_processor(int value, float* farr, int* iarr) {
    int result = 0;
    
    /* Large switch with non-sequential cases */
    switch (value) {
        case 100: result = iarr[0] * 2; break;
        case 87:  result = (int)farr[1] + iarr[1]; break;
        case 42:  result = iarr[2] / 3; break;
        case 15:  result = iarr[3] << 2; break;
        case 99:  result = iarr[4] | 0xFF; break;
        case 33:  result = iarr[5] & 0x0F; break;
        case 71:  result = iarr[6] ^ iarr[7]; break;
        case 88:  result = iarr[8] + iarr[9]; break;
        case 12:  result = iarr[10] - iarr[11]; break;
        case 55:  result = iarr[12] * iarr[13]; break;
        case 23:  result = iarr[14] % 17; break;
        case 64:  result = ~iarr[15]; break;
        case 19:  result = iarr[16] >> 3; break;
        case 91:  result = iarr[17] << 1; break;
        case 7:   result = iarr[18] + 42; break;
        case 28:  result = iarr[19] - 100; break;
        case 50:  result = iarr[20] | iarr[21]; break;
        case 37:  result = iarr[22] & iarr[23]; break;
        case 82:  result = iarr[24] ^ 0xAA; break;
        case 3:   result = iarr[25] * 7; break;
        case 66:  result = iarr[26] / 2; break;
        case 44:  result = iarr[27] % 9; break;
        case 77:  result = iarr[28] << 4; break;
        case 11:  result = iarr[29] >> 2; break;
        default:  /* Complex default case */
            for (int i = 0; i < 10; i++) {
                result += iarr[i] * farr[i];
                asm volatile ("# default barrier" ::: "memory");
            }
            if (result < 0) error_handler("Negative result in default case");
            break;
    }
    
    return result;
}

int main() {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (i % 100) * 0.1f;
        iarr[i] = i * 3;
    }
    
    int outer_sum = 0;
    float fp_sum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with dependencies */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            int inner_product = 0;
            float fp_product = 1.0f;
            
            for (int k = 0; k < 16; k++) {
                /* Loop-carried dependencies */
                inner_product += iarr[i * 16 + k] * iarr[j * 8 + k % 8];
                fp_product *= farr[i * 16 + k] + farr[j * 8 + k % 8];
                
                /* Conditional inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    inner_product -= 5;
                    fp_product /= 2.0f;
                    
                    /* Call noinline function with SIMD */
                    if (k % 3 == 0) {
                        float chunk_sum = process_chunk(farr, k * 4, (k + 4) * 4);
                        fp_product += chunk_sum;
                    }
                } else if ((i * j * k) % 13 == 0) {
                    /* Cold path */
                    if (inner_product > 1000000) {
                        error_handler("Potential overflow");
                    }
                }
                
                /* Inline assembly with clobbers */
                asm volatile ("# inner loop barrier %0" : "+r" (inner_product) :: "cc", "memory", "eax", "ebx");
            }
            
            outer_sum += inner_product;
            fp_sum += fp_product;
            
            /* Complex switch based on computed values */
            int switch_val = (inner_product % 100) + 1;
            int switch_result = nested_switch_processor(switch_val, farr, iarr);
            
            /* Modify arrays based on switch result */
            iarr[i * 2] ^= switch_result;
            farr[j * 2] += switch_result * 0.01f;
            
            /* Unstructured control flow with goto */
            if (restart_count < 2 && outer_sum > 1000000 && j % 13 == 0) {
                restart_count++;
                goto restart_point;  /* Irreducible control flow */
            }
            
            /* Break to outer loop with label */
            if (outer_sum > 5000000) {
                goto outer_loop_break;
            }
        }
        
        /* Continue statement with outer loop target */
        if (i % 4 == 0) {
            continue;
        }
        
        /* Another function call with mixed operations */
        int reduction = integer_reduction(&iarr[i * 16], 16);
        outer_sum += reduction;
        
        asm volatile ("# outer loop sync" ::: "memory", "ecx", "edx");
    }
    
outer_loop_break:
    
    /* Final validation */
    float final_fp_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        __m128 vec1 = _mm_loadu_ps(&farr[i]);
        __m128 vec2 = _mm_loadu_ps(&farr[i + 4]);
        __m128 sum = _mm_add_ps(vec1, vec2);
        float temp[4];
        _mm_storeu_ps(temp, sum);
        final_fp_sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    printf("Results: outer_sum = %d, fp_sum = %f, final_fp_sum = %f\n", 
           outer_sum, fp_sum, final_fp_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
