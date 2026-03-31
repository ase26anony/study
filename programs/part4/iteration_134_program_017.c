/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    float sum = 0.0f;
    for (int i = start; i < end; i++) {
        sum += data[i] * 0.5f;
    }
    return sum;
}

__attribute__((noinline))
static int validate_result(long total) {
    return (total % 1000) == 0;
}

/* SIMD processing function */
__attribute__((noinline))
#ifdef __SSE__
static __m128 simd_process(__m128 a, __m128 b) {
    __m128 result = _mm_add_ps(a, b);
    result = _mm_mul_ps(result, _mm_set1_ps(1.5f));
    return result;
}
#else
static float simd_process(float a, float b) {
    return (a + b) * 1.5f;
}
#endif

/* Main computational function with complex control flow */
__attribute__((noinline))
static long compute_kernel(int* int_data, float* float_data, int size) {
    long total = 0;
    float fsum = 0.0f;
    int restart_count = 0;
    
    /* Nested loops with dependencies (Requirement 1) */
    for (int i = 0; i < size / 4; i++) {
        int outer_acc = 0;
        for (int j = 0; j < 16; j++) {
            float inner_sum = 0.0f;
            for (int k = 0; k < 8; k++) {
                /* Loop-carried dependency */
                outer_acc += int_data[(i * 16 + j + k) % size];
                
                /* Conditional branch inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    inner_sum += sqrtf(fabsf(float_data[(j * 8 + k) % size]));
                    
                    /* Inline assembly barrier (Requirement 4) */
                    asm volatile("" ::: "memory", "cc");
                } else {
                    inner_sum += float_data[(j * 8 + k) % size] * 2.0f;
                }
                
                /* Mixed integer/float operations */
                fsum += inner_sum * (k + 1);
                
                /* Another assembly barrier with clobbers */
                asm volatile("" ::: "r8", "r9", "r10");
            }
            
            /* SIMD operations (Requirement 3) */
#ifdef __SSE__
            if (j % 3 == 0) {
                __m128 vec1 = _mm_set_ps(float_data[j], float_data[j+1], 
                                        float_data[j+2], float_data[j+3]);
                __m128 vec2 = _mm_set1_ps(0.25f);
                __m128 result = simd_process(vec1, vec2);
                float temp[4];
                _mm_store_ps(temp, result);
                fsum += temp[0] + temp[1] + temp[2] + temp[3];
            }
#endif
            
            /* Call to noinline function (Requirement 2) */
            if (j % 5 == 0) {
                fsum += process_chunk(float_data, j, j + 4);
            }
        }
        
        total += outer_acc;
        
        /* Complex switch statement (Requirement 5) */
        switch (i % 23) {
            case 0: total += 100; break;
            case 1: total *= 2; break;
            case 2: total -= fsum; break;
            case 3: total += int_data[i] * 3; break;
            case 4: total ^= 0xABCD; break;
            case 5: total += (long)(fsum * 10); break;
            case 6: total = total >> 2; break;
            case 7: total += int_data[i % size] << 3; break;
            case 8: total *= 1.5; break;
            case 9: total += 999; break;
            case 10: total -= int_data[(i + 1) % size]; break;
            case 11: total += (i * 777); break;
            case 12: total = ~total; break;
            case 13: total += (long)(sinf(fsum) * 1000); break;
            case 14: total ^= total >> 4; break;
            case 15: total += int_data[(i + 2) % size] * 7; break;
            case 16: total *= 0.75; break;
            case 17: total += 1234; break;
            case 18: total -= (long)(cosf(fsum) * 500); break;
            case 19: total += int_data[(i + 3) % size]; break;
            case 20: total = total & 0x7FFFFFFF; break;
            case 21: total += (i * 4321); break;
            default: 
                /* Complex default case */
                for (int d = 0; d < 4; d++) {
                    total += int_data[(i + d) % size] * (d + 1);
                    fsum += float_data[(i + d) % size] / (d + 2);
                }
                asm volatile("" ::: "memory");
                break;
        }
        
        /* goto for irreducible control flow (Requirement 6) */
        if (restart_count < 2 && total < 0) {
            restart_count++;
            goto restart_point;
        }
        
        continue;
        
    restart_point:
        /* Reset some state and continue */
        fsum = fsum * 0.9f;
        asm volatile("" ::: "r11", "r12", "r13", "r14", "r15");
    }
    
    return total + (long)fsum;
}

int main(void) {
    const int SIZE = 1024;
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    float* float_data = (float*)malloc(SIZE * sizeof(float));
    
    if (!int_data || !float_data) {
        error_handler("Memory allocation failed");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = (i * 37) % 101;
        float_data[i] = (i * 0.123f) - 15.0f;
    }
    
    /* Perform computation with complex scheduling patterns */
    long result = compute_kernel(int_data, float_data, SIZE);
    
    /* Additional computations to increase scheduling complexity */
    for (int iter = 0; iter < 3; iter++) {
        float temp_sum = 0.0f;
        for (int i = 0; i < SIZE; i += 8) {
            for (int j = 0; j < 8; j++) {
                if ((i + j) % 3 == 0) {
                    temp_sum += int_data[i + j] * 1.5f;
                    asm volatile("" ::: "rax", "rbx", "rcx");
                } else {
                    temp_sum -= float_data[i + j] * 0.75f;
                }
            }
            
            /* Nested switch inside loop */
            switch (iter) {
                case 0: result += temp_sum; break;
                case 1: result -= temp_sum * 2; break;
                case 2: result ^= (long)temp_sum; break;
            }
        }
    }
    
    /* Validate and output result */
    if (validate_result(result)) {
        printf("Computation successful: result = %ld\n", result);
    } else {
        error_handler("Result validation failed");
    }
    
    free(int_data);
    free(float_data);
    
    return 0;
}
