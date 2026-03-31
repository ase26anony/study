#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  // SSE intrinsics
#include <emmintrin.h>  // SSE2 intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: Noinline functions with cold attributes */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics mixed with scalar ops */
    for (int i = 0; i < size; i += 4) {
        if (i + 4 <= size) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            __m128 squared = _mm_mul_ps(vec, vec);
            
            /* Requirement 4: Inline assembly with clobbers */
            asm volatile (
                "movaps %%xmm0, %%xmm1\n\t"
                "haddps %%xmm1, %%xmm0\n\t"
                "haddps %%xmm0, %%xmm0"
                : "+x"(squared)
                :
                : "xmm1", "cc"
            );
            
            float temp;
            _mm_store_ss(&temp, squared);
            sum += temp;
        } else {
            /* Scalar fallback */
            for (int j = i; j < size; j++) {
                sum += data[j] * data[j];
            }
        }
    }
    
    /* Another inline asm barrier */
    asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int result = 0;
    volatile int* volatile_ptr = arr;  /* Prevent optimizations */
    
    for (int i = 0; i < size; i++) {
        /* Complex data dependency chain */
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        result ^= volatile_ptr[i];
        
        /* Requirement 4: More inline asm */
        asm volatile (
            "rorl $7, %0\n\t"
            "roll $13, %0"
            : "+r"(result)
            :
            : "cc"
        );
    }
    
    return result & threshold;
}

/* Requirement 5: Large switch statement */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 100: result = iarr[0] + 1; break;
        case 87:  result = iarr[1] * 2; break;
        case 42:  result = iarr[2] / 3; break;
        case 156: result = iarr[3] - 4; break;
        case 23:  result = iarr[4] | 0xFF; break;
        case 189: result = iarr[5] & 0xAA; break;
        case 64:  result = iarr[6] ^ 0x55; break;
        case 31:  result = iarr[7] << 2; break;
        case 142: result = iarr[8] >> 1; break;
        case 78:  result = iarr[9] % 17; break;
        case 205: result = (int)(farr[0] * 10.0f); break;
        case 91:  result = (int)(farr[1] / 2.0f); break;
        case 33:  result = (int)(farr[2] + 100.0f); break;
        case 167: result = (int)(farr[3] - 50.0f); break;
        case 54:  result = iarr[10] * iarr[11]; break;
        case 118: result = iarr[12] + iarr[13]; break;
        case 72:  result = iarr[14] - iarr[15]; break;
        case 199: result = iarr[16] / (iarr[17] + 1); break;
        case 11:  result = iarr[18] | iarr[19]; break;
        case 88:  result = iarr[20] & iarr[21]; break;
        case 135: result = iarr[22] ^ iarr[23]; break;
        case 47:  result = iarr[24] << (iarr[25] & 3); break;
        case 176: result = iarr[26] >> (iarr[27] & 3); break;
        case 62:  result = (int)(farr[4] * farr[5]); break;
        default:  /* Requirement 5: Complex default case */
            result = integer_reduction(iarr, 50, 0xFF);
            if (result < 0) {
                error_handler("Negative result in default case");
            }
            result = process_vector(farr, 50) + result;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (i % 100) * 0.1f;
        iarr[i] = i * 1103515245 + 12345;
    }
    
    int outer_sum = 0;
    float fp_sum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Triple nested loops with dependencies */
    for (int i = 0; i < 10; i++) {
        if (restart_count > 3) {
            /* Cold path */
            error_handler("Too many restarts");
            break;
        }
        
        for (int j = 0; j < 20; j++) {
            int inner_acc = i * j;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependency */
                inner_acc = (inner_acc * 1664525 + 1013904223) & 0x7fffffff;
                
                /* Conditional branch inside innermost loop */
                if ((inner_acc & 0xFF) > 128) {
                    fp_sum += farr[(i + j + k) % ARRAY_SIZE];
                    
                    /* Mixed integer/float operations */
                    int idx = (inner_acc ^ (i << 16) ^ (j << 8) ^ k) % ARRAY_SIZE;
                    float temp = farr[idx];
                    farr[idx] = temp * 0.99f + 0.01f * (float)inner_acc;
                    
                    /* Requirement 4: Inline asm barrier */
                    asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11");
                } else {
                    iarr[(i * 400 + j * 20 + k) % ARRAY_SIZE] += inner_acc;
                }
                
                /* Complex floating-point dependency chain */
                if (k % 7 == 0) {
                    float x = farr[(i * j + k) % ARRAY_SIZE];
                    float y = farr[(j * k + i) % ARRAY_SIZE];
                    farr[(k * i + j) % ARRAY_SIZE] = x * x - y * y + 2.0f * x * y;
                }
            }
            
            /* Requirement 6: goto to create irreducible flow */
            if ((inner_acc & 0xFFFF) == 0 && restart_count < 2) {
                restart_count++;
                goto restart_point;
            }
            
            outer_sum += inner_acc;
        }
        
        /* Call noinline functions */
        fp_sum += process_vector(&farr[i * 50], 50);
        
        /* Inline asm with multiple clobbers */
        asm volatile (
            "cpuid\n\t"
            "rdtsc\n\t"
            "mov %%eax, %%ebx"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
    }
    
    /* Requirement 5: Large switch statement */
    int switch_val = outer_sum % 256;
    int switch_result = process_switch(switch_val, farr, iarr);
    
    /* More complex control flow with goto */
    if (switch_result < 0) {
        goto cleanup;
    }
    
    /* Additional loop with mixed operations */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* SIMD operations */
        __m128i ivec1 = _mm_loadu_si128((__m128i*)&iarr[i]);
        __m128i ivec2 = _mm_loadu_si128((__m128i*)&iarr[i + 4]);
        __m128i iresult = _mm_add_epi32(ivec1, ivec2);
        _mm_storeu_si128((__m128i*)&iarr[i], iresult);
        
        /* Scalar operations in parallel */
        fp_sum += farr[i] + farr[i + 1] + farr[i + 2] + farr[i + 3];
        
        /* Conditional with continue */
        if (i % 64 == 0) {
            continue;
        }
        
        /* Break to outer scope */
        if (fp_sum > 1e6f) {
            break;
        }
    }
    
cleanup:
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= iarr[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Result: checksum = 0x%08X, fp_sum = %f, switch_result = %d\n",
           checksum, fp_sum, switch_result);
    
    return 0;
}
