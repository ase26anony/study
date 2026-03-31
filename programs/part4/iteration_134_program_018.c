/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __SSE__
#include <xmmintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Non-inline helper functions to force call boundaries */
__attribute__((noinline)) 
static float compute_vector_sum(float* arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i += 4) {
        /* SIMD-like operation */
        float temp = arr[i] + arr[i+1] + arr[i+2] + arr[i+3];
        
        /* Artificial scheduling barrier */
        asm volatile ("" ::: "memory", "cc");
        
        sum += temp;
    }
    return sum;
}

__attribute__((noinline, cold))
static void error_handler(int code) {
    /* Cold path rarely taken */
    fprintf(stderr, "Error code: %d\n", code);
    asm volatile ("nop" ::: "cc");
}

__attribute__((noinline))
static void process_with_simd(int* dest, const int* src, int n) {
#ifdef __SSE__
    for (int i = 0; i < n; i += 4) {
        __m128i v1 = _mm_loadu_si128((__m128i*)&src[i]);
        __m128i v2 = _mm_set1_epi32(42);
        __m128i result = _mm_add_epi32(v1, v2);
        _mm_storeu_si128((__m128i*)&dest[i], result);
        
        /* Clobber multiple registers */
        asm volatile ("# SIMD barrier" ::: 
                     "xmm0", "xmm1", "xmm2", "xmm3", "cc", "memory");
    }
#elif __ARM_NEON
    for (int i = 0; i < n; i += 4) {
        int32x4_t v1 = vld1q_s32(&src[i]);
        int32x4_t v2 = vdupq_n_s32(42);
        int32x4_t result = vaddq_s32(v1, v2);
        vst1q_s32(&dest[i], result);
        
        asm volatile ("# NEON barrier" ::: 
                     "q0", "q1", "q2", "q3", "cc", "memory");
    }
#else
    /* Scalar fallback */
    for (int i = 0; i < n; ++i) {
        dest[i] = src[i] + 42;
        asm volatile ("# scalar barrier" ::: "eax", "ebx", "ecx", "edx", "cc", "memory");
    }
#endif
}

/* Complex nested loops with dependencies */
static long nested_loop_computation(int* A, float* B, int size) {
    long total = 0;
    volatile int counter = 0; /* Prevent optimization */
    
    /* Level 1 loop */
    for (int i = 0; i < size / 4; ++i) {
        /* Level 2 loop */
        for (int j = 0; j < 8; ++j) {
            /* Level 3 loop with conditional */
            for (int k = 0; k < 16; ++k) {
                /* Loop-carried dependency */
                total = total * 6364136223846793005ULL + 1442695040888963407ULL;
                
                /* Mixed operations */
                float fval = sinf(B[i * 4 + j % 4]) * cosf(B[k % size]);
                int ival = (int)(fval * 1000.0f);
                
                /* Conditional with data dependency */
                if (ival > 500 && (total & 1)) {
                    A[i] += ival;
                    counter++;
                    
                    /* Inline asm barrier */
                    asm volatile ("# inner conditional" ::: 
                                 "rax", "rbx", "rcx", "rdx", "cc", "memory");
                } else if (ival < -500) {
                    /* Cold path */
                    error_handler(ival);
                }
                
                /* More arithmetic with dependencies */
                B[k % size] = fmodf(B[k % size] + 0.1f, 1.0f);
                total += (long)(B[k % size] * 10000);
            }
            
            /* Cross-iteration dependency */
            if (j % 3 == 0) {
                A[i] ^= (A[i] << 13);
                A[i] ^= (A[i] >> 17);
                A[i] ^= (A[i] << 5);
            }
        }
        
        /* Call non-inline function */
        float vec_sum = compute_vector_sum(B, size);
        total += (long)vec_sum;
        
        /* Memory clobber */
        asm volatile ("# outer loop" ::: "memory");
    }
    
    return total;
}

/* Large switch with non-linear cases */
static int complex_switch(int value, int* arr, float* farr) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: result = arr[0] * 2; break;
        case 1: result = arr[1] + arr[2]; break;
        case 3: /* Non-sequential */
            result = (int)(farr[0] * farr[1]);
            asm volatile ("# case 3" ::: "cc");
            break;
        case 7:
            for (int i = 0; i < 5; ++i) {
                result += arr[i] << i;
            }
            break;
        case 11:
            result = arr[value % ARRAY_SIZE] ^ 0xDEADBEEF;
            break;
        case 13:
            /* SIMD operation in switch case */
            process_with_simd(&arr[100], &arr[200], 50);
            result = arr[100];
            break;
        case 17:
            result = (int)sqrtf(fabsf(farr[17]));
            break;
        case 19:
            result = arr[19] * arr[23] - arr[29];
            break;
        case 23:
            /* Nested loop in switch */
            for (int i = 0; i < 10; ++i) {
                result += (arr[i] & 0xF) << (i * 4);
            }
            break;
        case 29:
            result = (int)(sinf(farr[29]) * 1000.0f);
            break;
        case 31:
            result = arr[31] | arr[37];
            break;
        case 37:
            /* Complex computation */
            result = 0;
            for (int i = 0; i < 8; ++i) {
                result = (result << 4) | (arr[i] & 0xF);
            }
            break;
        case 41:
            result = arr[41] % 137;
            break;
        case 43:
            result = (arr[43] > 0) ? arr[43] : -arr[43];
            break;
        case 47:
            result = (int)(logf(fabsf(farr[47]) + 1.0f) * 100.0f);
            break;
        case 53:
            result = arr[53] * 3 + arr[59] * 7;
            break;
        case 59:
            result = ~arr[59];
            break;
        case 61:
            result = (arr[61] << 1) | (arr[61] >> 31);
            break;
        case 67:
            result = arr[67] & arr[71] & arr[73];
            break;
        case 71:
            result = (int)(expf(farr[71] / 100.0f) * 100.0f);
            break;
        case 73:
            result = arr[73] + arr[79] + arr[83];
            break;
        case 79:
            result = arr[79] ^ arr[83] ^ arr[89];
            break;
        case 83:
            result = arr[83] * arr[89] / (arr[97] + 1);
            break;
        case 89:
            result = (arr[89] << 16) | (arr[97] & 0xFFFF);
            break;
        default:
            /* Default case with complex operation */
            result = 0;
            for (int i = 0; i < 20; ++i) {
                result += arr[(value + i) % ARRAY_SIZE] * i;
                farr[i] = sinf(farr[i] + result * 0.01f);
            }
            asm volatile ("# default case" ::: 
                         "rax", "rbx", "rcx", "rdx", "cc", "memory");
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)(int_array[i] % 1000) / 1000.0f;
    }
    
    long total = 0;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Nested loops with complex control flow */
    total = nested_loop_computation(int_array, float_array, ARRAY_SIZE);
    
    /* Process with SIMD */
    process_with_simd(int_array, int_array, ARRAY_SIZE);
    
    /* Large switch statement */
    int switch_result = 0;
    for (int i = 0; i < 100; ++i) {
        switch_result += complex_switch(int_array[i] + i, int_array, float_array);
        
        /* Conditional goto to create irreducible flow */
        if (i == 50 && restart_count < 2) {
            restart_count++;
            goto restart_point;  /* Jump back to restart */
        }
        
        /* Another goto forward */
        if (i == 75 && switch_result > 1000000) {
            goto skip_section;
        }
        
        /* Continue with more computations */
        float_array[i] = cosf(float_array[i] * 3.14159f);
    }
    
skip_section:
    
    /* More computations after label */
    for (int i = 0; i < ARRAY_SIZE / 2; ++i) {
        int_array[i] = (int_array[i] * 3 + int_array[ARRAY_SIZE - i - 1]) / 4;
        
        /* Mix with float operations */
        float_array[i] = float_array[i] * 0.9f + float_array[ARRAY_SIZE - i - 1] * 0.1f;
        
        /* Another asm barrier */
        asm volatile ("# final loop" ::: "memory", "cc");
    }
    
    /* Final validation checksum */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        checksum += int_array[i];
        checksum += (long)(float_array[i] * 1000.0f);
    }
    
    checksum += total + switch_result;
    
    printf("Computation completed. Checksum: %ld\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    
    return 0;
}
