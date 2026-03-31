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
static float process_chunk(float* data, int start, int end) {
    float sum = 0.0f;
    for (int i = start; i < end; i++) {
        sum += data[i] * 0.5f;
    }
    return sum;
}

__attribute__((noinline))
static void simd_operation(float* a, float* b, int n) {
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        __m128 vec_a = _mm_loadu_ps(&a[i]);
        __m128 vec_b = _mm_loadu_ps(&b[i]);
        __m128 result = _mm_add_ps(_mm_mul_ps(vec_a, _mm_set1_ps(1.5f)), vec_b);
        _mm_storeu_ps(&a[i], result);
    }
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %%eax, %1\n\t"
            : "+r" (arr[i]), "+r" (result)
            : 
            : "eax", "cc", "memory"
        );
    }
    return result;
}

static void process_with_barriers(int* data, int n) {
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
        
        /* Another asm barrier */
        asm volatile ("" : : : "memory");
        
        if (data[i] > 1000) {
            data[i] = data[i] % 100;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.1f;
        double_array[i] = i * 0.01;
    }
    
    int outer_sum = 0;
    float middle_sum = 0.0f;
    double inner_sum = 0.0;
    
    /* Requirement 1: Nested loops with complex control flow */
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    for (int i = 0; i < 10; i++) {
        if (restart_count > 3) {
            error_handler("Too many restarts");
            break;
        }
        
        for (int j = 0; j < 20; j++) {
            /* Loop-carried dependency */
            outer_sum += i * j;
            
            for (int k = 0; k < 30; k++) {
                /* Mixed integer and floating-point operations */
                inner_sum += sin(double_array[k % ARRAY_SIZE]) * cos(double_array[j % ARRAY_SIZE]);
                
                /* Conditional branch inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    middle_sum += float_array[(i + j + k) % ARRAY_SIZE];
                    
                    /* Requirement 4: asm volatile barrier */
                    asm volatile (
                        "mfence\n\t"
                        : 
                        : 
                        : "memory"
                    );
                } else if ((i + j + k) % 13 == 0) {
                    /* Call noinline function */
                    middle_sum += process_chunk(float_array, k, k + 8);
                }
                
                /* Data dependency chain */
                int_array[(i * j + k) % ARRAY_SIZE] = 
                    int_array[(i * j + k) % ARRAY_SIZE] * 3 + 1;
            }
            
            /* SIMD operations */
            if (j % 5 == 0) {
                simd_operation(float_array, float_array + ARRAY_SIZE/2, ARRAY_SIZE/2);
            }
        }
        
        /* Requirement 6: goto to create irreducible flow */
        if (i == 5 && outer_sum % 10000 == 0) {
            restart_count++;
            goto restart_point;
        }
    }
    
    /* Process with barriers */
    process_with_barriers(int_array, ARRAY_SIZE);
    
    /* Complex reduction */
    int reduction_result = complex_reduction(int_array, ARRAY_SIZE);
    
    /* Requirement 5: Large switch with non-linear cases */
    int switch_var = (reduction_result + outer_sum) % SWITCH_CASES;
    float switch_result = 0.0f;
    
    switch (switch_var) {
        case 0:  switch_result = float_array[0] * 2.0f; break;
        case 1:  switch_result = sinf(float_array[1]); break;
        case 3:  switch_result = float_array[3] + float_array[4]; break;
        case 7:  switch_result = float_array[7] / 3.0f; break;
        case 15: switch_result = sqrtf(fabsf(float_array[15])); break;
        case 2:  switch_result = float_array[2] - float_array[1]; break;
        case 4:  switch_result = float_array[4] * float_array[5]; break;
        case 8:  switch_result = logf(fabsf(float_array[8]) + 1.0f); break;
        case 16: switch_result = float_array[16] * 3.14159f; break;
        case 5:  switch_result = float_array[5] + 100.0f; break;
        case 9:  switch_result = float_array[9] / 7.0f; break;
        case 17: switch_result = expf(float_array[17] * 0.01f); break;
        case 6:  switch_result = float_array[6] - 50.0f; break;
        case 10: switch_result = float_array[10] * float_array[11]; break;
        case 18: switch_result = powf(float_array[18], 1.5f); break;
        case 11: switch_result = float_array[11] + 200.0f; break;
        case 19: switch_result = float_array[19] / 11.0f; break;
        case 12: switch_result = float_array[12] * 0.333f; break;
        case 20: switch_result = tanf(float_array[20]); break;
        case 13: switch_result = float_array[13] + 300.0f; break;
        case 21: switch_result = float_array[21] * 2.718f; break;
        case 14: switch_result = float_array[14] / 13.0f; break;
        case 22: switch_result = asinf(float_array[22] * 0.01f); break;
        case 23: switch_result = float_array[23] + 400.0f; break;
        case 24: switch_result = float_array[24] * 0.25f; break;
        default: /* Complex default case */
            for (int i = 0; i < 10; i++) {
                switch_result += float_array[i] * i;
                asm volatile ("" : : : "memory");
            }
            switch_result = sqrtf(fabsf(switch_result));
            break;
    }
    
    /* More complex control flow with goto */
    if (switch_result > 1000.0f) {
        goto cleanup;
    }
    
    /* Final computation with dependencies */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += int_array[i] * 0.01 + float_array[i] + double_array[i];
        
        /* Another asm barrier to force scheduling boundaries */
        if (i % 50 == 0) {
            asm volatile (
                "movq %%rax, %%rbx\n\t"
                : 
                : 
                : "rax", "rbx", "cc"
            );
        }
    }
    
cleanup:
    /* Validate results */
    double checksum = outer_sum + middle_sum + inner_sum + final_result + switch_result;
    
    printf("Computation complete. Checksum: %f\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    if (!isnan(checksum) && !isinf(checksum)) {
        printf("SUCCESS: Scheduler stress test completed.\n");
    } else {
        printf("FAILURE: Invalid checksum.\n");
    }
    
    return 0;
}
