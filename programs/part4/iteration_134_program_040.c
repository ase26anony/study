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
    
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < size; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrt_vec = _mm_sqrt_ps(squared);
        
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "movaps %%xmm0, %%xmm1\n\t"
            "addps %%xmm1, %%xmm0\n\t"
            : 
            : 
            : "xmm0", "xmm1", "cc", "memory"
        );
        
        float temp[4];
        _mm_storeu_ps(temp, sqrt_vec);
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int result = 0;
    
    /* Requirement 4: More asm barriers */
    asm volatile (
        "mfence\n\t"
        : 
        : 
        : "memory"
    );
    
    for (int i = 0; i < size; i++) {
        result ^= (arr[i] * 0x5A827999) >> 3;
    }
    
    asm volatile (
        "lfence\n\t"
        : 
        : 
        : "memory"
    );
    
    return result;
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    switch (value) {
        case 100: result = iarr[0] * 2; break;
        case 87:  result = (int)(farr[1] * 100.0f); break;
        case 42:  result = complex_reduction(iarr, 16); break;
        case 13:  result = iarr[2] ^ iarr[3]; break;
        case 255: result = (int)process_vector(farr, 8); break;
        case 64:  result = iarr[4] << 2; break;
        case 128: result = iarr[5] >> 3; break;
        case 7:   result = iarr[6] % 17; break;
        case 99:  result = iarr[7] + iarr[8]; break;
        case 33:  result = iarr[9] - iarr[10]; break;
        case 150: result = iarr[11] | iarr[12]; break;
        case 200: result = iarr[13] & iarr[14]; break;
        case 11:  result = ~iarr[15]; break;
        case 90:  result = iarr[16] * iarr[17]; break;
        case 180: result = iarr[18] / (iarr[19] + 1); break;
        case 210: result = abs(iarr[20]); break;
        case 5:   result = iarr[21] << iarr[22]; break;
        case 72:  result = iarr[23] >> iarr[24]; break;
        case 140: result = iarr[25] + complex_reduction(iarr, 4); break;
        case 30:  result = (int)(sinf(farr[26]) * 1000.0f); break;
        case 85:  result = (int)(cosf(farr[27]) * 1000.0f); break;
        case 120: result = (int)(tanf(farr[28]) * 1000.0f); break;
        case 240: result = (int)(logf(farr[29] + 1.0f) * 1000.0f); break;
        case 15:  result = (int)(expf(farr[30]) * 1000.0f); break;
        default:  /* Requirement 5: Complex default case */
            result = complex_reduction(iarr, 32);
            result ^= (int)process_vector(farr, 16);
            if (result < 0) {
                error_handler("Negative result in default case");
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
        farr[i] = (i % 100) * 0.1f;
        iarr[i] = i * 3 + 7;
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label for restart */
    
    /* Requirement 1: Nested loops with complex control flow and data dependencies */
    for (int i = 0; i < 10; i++) {
        int loop_sum = 0;
        float loop_float = 0.0f;
        
        for (int j = 0; j < 20; j++) {
            int inner_sum = 0;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependencies */
                inner_sum += iarr[(i * 400 + j * 20 + k) % ARRAY_SIZE];
                
                /* Mixed integer and floating-point operations */
                loop_float += farr[(i * 200 + j * 15 + k) % ARRAY_SIZE] * 0.5f;
                
                /* Conditional branches inside innermost loop */
                if ((inner_sum & 1) == 0) {
                    inner_sum ^= 0xAAAAAAAA;
                    loop_float = loop_float * 0.9f + 0.1f;
                } else {
                    inner_sum ^= 0x55555555;
                    loop_float = loop_float * 1.1f - 0.1f;
                    
                    /* Requirement 6: goto to outer scope */
                    if (restart_count < 2 && inner_sum > 1000000) {
                        restart_count++;
                        goto restart_point;
                    }
                }
                
                /* More complex dependencies */
                if (k % 7 == 0) {
                    inner_sum = (inner_sum << 3) | (inner_sum >> 29);
                    loop_float = sqrtf(fabsf(loop_float));
                }
            }
            
            loop_sum += inner_sum;
            
            /* Requirement 4: asm barrier between dependent operations */
            asm volatile (
                "xor %%eax, %%eax\n\t"
                "cpuid\n\t"
                : 
                : 
                : "eax", "ebx", "ecx", "edx", "cc", "memory"
            );
        }
        
        total_sum += loop_sum;
        float_sum += loop_float;
        
        /* Requirement 2: Call noinline functions */
        float_sum += process_vector(farr + i * 10, 40);
        
        /* Requirement 5: Switch statement with dependencies across cases */
        int switch_val = (loop_sum % 256);
        int switch_result = process_switch(switch_val, farr, iarr);
        total_sum ^= switch_result;
        
        /* Requirement 6: continue with label target */
        if (i == 5) {
            continue;
        }
        
        /* More asm barriers */
        asm volatile (
            "rdtsc\n\t"
            "mov %%eax, %%ebx\n\t"
            : 
            : 
            : "eax", "ebx", "edx", "cc"
        );
    }
    
    /* Additional complex computation */
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Data-dependent array access pattern */
        int idx1 = (i * 17) % ARRAY_SIZE;
        int idx2 = (i * 23) % ARRAY_SIZE;
        
        /* SIMD-like operations using scalar code */
        __m128 v1 = _mm_set_ps(farr[idx1], farr[idx1+1], farr[idx1+2], farr[idx1+3]);
        __m128 v2 = _mm_set_ps(farr[idx2], farr[idx2+1], farr[idx2+2], farr[idx2+3]);
        __m128 v3 = _mm_add_ps(v1, v2);
        
        float temp[4];
        _mm_storeu_ps(temp, v3);
        
        for (int j = 0; j < 4; j++) {
            float_sum += temp[j];
        }
        
        /* Requirement 6: break to outer loop simulation */
        if (float_sum > 1e6f) {
            break;
        }
    }
    
    /* Final validation */
    int checksum = total_sum + (int)float_sum;
    printf("Computation complete. Checksum: %d\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    if (checksum != 0) {
        printf("SUCCESS: Scheduler workload executed.\n");
    } else {
        error_handler("Unexpected zero checksum");
    }
    
    return 0;
}
