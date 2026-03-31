/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with specific attributes */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path unlikely to be taken */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) dummy += i;
    (void)msg;
}

__attribute__((noinline))
static float process_chunk(float* arr, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    
    /* SIMD operations */
    __m128 vsum = _mm_setzero_ps();
    for (int i = start; i + 3 < end; i += 4) {
        __m128 v = _mm_loadu_ps(&arr[i]);
        vsum = _mm_add_ps(vsum, v);
        
        /* Inline asm barrier between dependent operations */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract SIMD results */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Scalar tail processing */
    for (int i = (end - start) & ~3; i < (end - start); i++) {
        sum += arr[start + i];
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size) {
    int result = 0;
    volatile int counter = 0; /* Prevent optimizations */
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < size; i++) {
        result += arr[i];
        
        /* Conditional with side effect */
        if (arr[i] > 1000) {
            result -= 500;
            counter++;
            
            /* Another asm barrier */
            asm volatile ("# barrier" ::: "cc", "memory");
        }
        
        /* Modulo operation creates dependency chain */
        arr[i] = result % 997;
    }
    
    return result;
}

/* Main computational function */
__attribute__((noinline))
static double complex_computation(int* int_arr, float* float_arr, int size) {
    double total = 0.0;
    int restart_count = 0;
    
restart_point:  /* Label for goto (requirement 6) */
    
    /* Nested loops (3 levels) with dependencies */
    for (int i = 0; i < size / 4; i++) {
        int outer_sum = 0;
        
        for (int j = 0; j < 16; j++) {
            float inner_acc = 0.0f;
            
            for (int k = 0; k < 8; k++) {
                /* Loop-carried dependency */
                outer_sum += int_arr[(i * 64 + j * 4 + k) % size];
                
                /* Conditional branch inside innermost loop */
                if (outer_sum & 1) {
                    inner_acc += float_arr[(j * 8 + k) % size];
                    
                    /* SIMD operation mixed with scalar */
                    __m128 a = _mm_set1_ps(inner_acc);
                    __m128 b = _mm_loadu_ps(&float_arr[(k * 4) % size]);
                    __m128 c = _mm_add_ps(a, b);
                    
                    float temp[4];
                    _mm_storeu_ps(temp, c);
                    inner_acc = temp[0];
                    
                    /* Another asm barrier */
                    asm volatile ("" ::: "xmm4", "xmm5", "xmm6", "xmm7");
                } else {
                    inner_acc -= float_arr[(j * 8 + k) % size] * 0.5f;
                }
                
                /* Cross-iteration dependency */
                int_arr[(i * 64 + j * 4 + k) % size] = 
                    (int)(inner_acc * 1000) ^ outer_sum;
            }
            
            total += inner_acc;
            
            /* Occasionally call cold function */
            if ((j & 0x7) == 0 && inner_acc > 10000.0f) {
                error_handler("High value detected");
            }
        }
        
        /* Goto to create irreducible flow */
        if (restart_count < 2 && (i % 50) == 25) {
            restart_count++;
            goto restart_point;  /* Jump back to restart */
        }
        
        /* Break to outer loop with label */
        if (outer_sum > 1000000) {
            break;
        }
    }
    
    return total;
}

/* Large switch statement */
__attribute__((noinline))
static void process_switch(int value, int* arr, float* farr, int idx) {
    /* Dense, non-linear cases */
    switch (value % SWITCH_CASES) {
        case 0:
            arr[idx] += 100;
            farr[idx] *= 1.1f;
            break;
        case 1:
            arr[idx] -= 50;
            /* Fall through */
        case 2:
            farr[idx] /= 2.0f;
            break;
        case 5:  /* Non-sequential */
            arr[idx] ^= 0xAAAA;
            break;
        case 7:
            farr[idx] = farr[idx] * farr[idx] + 1.0f;
            break;
        case 10:
            arr[idx] = arr[idx] * 3 + 1;
            break;
        case 13:
            farr[idx] = __builtin_sqrtf(farr[idx]);
            break;
        case 15:
            arr[idx] = (arr[idx] << 3) | (arr[idx] >> 29);
            break;
        case 18:
            farr[idx] = __builtin_sinf(farr[idx]);
            break;
        case 21:
            arr[idx] = __builtin_popcount(arr[idx]);
            break;
        case 23:
            {
                __m128 v = _mm_set1_ps(farr[idx]);
                __m128 r = _mm_mul_ps(v, v);
                float temp[4];
                _mm_storeu_ps(temp, r);
                farr[idx] = temp[0];
            }
            break;
        /* More cases... */
        case 3:
        case 4:
        case 6:
        case 8:
        case 9:
        case 11:
        case 12:
        case 14:
        case 16:
        case 17:
        case 19:
        case 20:
        case 22:
        case 24:
            arr[idx] += value;
            farr[idx] -= (float)value;
            break;
        default:  /* Complex default case */
            {
                int x = arr[idx];
                for (int i = 0; i < 8; i++) {
                    x = (x * 1103515245 + 12345) & 0x7fffffff;
                }
                arr[idx] = x;
                farr[idx] = (float)x / 1000.0f;
                
                /* Inline asm in default case */
                asm volatile ("# complex default" 
                            ::: "eax", "ebx", "ecx", "edx", "memory");
            }
            break;
    }
}

int main(void) {
    /* Initialize arrays */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    
    if (!int_array || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fff;
        float_array[i] = (float)int_array[i] / 100.0f;
    }
    
    /* Perform complex computation with nested loops */
    double result = complex_computation(int_array, float_array, SIZE);
    
    /* Process integer reduction */
    int int_result = integer_reduction(int_array, SIZE);
    
    /* Process chunks with SIMD */
    float chunk_results[4];
    for (int i = 0; i < 4; i++) {
        chunk_results[i] = process_chunk(float_array, 
                                        i * SIZE/4, 
                                        (i + 1) * SIZE/4);
    }
    
    /* Apply switch statement to multiple elements */
    for (int i = 0; i < SIZE; i += 7) {
        process_switch(int_array[i] + i, int_array, float_array, i);
    }
    
    /* Final reduction with goto for control flow */
    double final_sum = result;
    int use_goto = 1;
    
    if (use_goto) {
        goto compute_final;
    }
    
    /* This code is jumped over */
    final_sum += 1000.0;
    
compute_final:
    for (int i = 0; i < SIZE; i++) {
        final_sum += int_array[i] + float_array[i];
        
        /* Another asm barrier in final loop */
        if ((i % 32) == 0) {
            asm volatile ("# final barrier" ::: "memory");
        }
    }
    
    /* Validate results (simplified) */
    volatile double checksum = final_sum + int_result + 
                              chunk_results[0] + chunk_results[1] +
                              chunk_results[2] + chunk_results[3];
    
    printf("Computation completed. Checksum: %f\n", (double)checksum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    
    return 0;
}
