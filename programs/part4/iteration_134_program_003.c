#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#define SIZE 1024
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
static void simd_operation(float* a, float* b, float* result, int n) {
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        __m128 vres = _mm_add_ps(_mm_mul_ps(va, vb), _mm_set1_ps(1.0f));
        _mm_storeu_ps(&result[i], vres);
    }
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "mov %0, %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %0"
            : "+r" (arr[i])
            : 
            : "eax", "cc", "memory"
        );
        sum += arr[i];
    }
    return sum;
}

static void process_with_barriers(int* data, int n) {
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
        
        /* Another asm barrier */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
        
        if (data[i] > 1000) {
            data[i] = data[i] % 100;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    int int_array[SIZE];
    float float_array[SIZE];
    float float_array2[SIZE];
    float result_array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 1.5f;
        float_array2[i] = i * 2.5f;
    }
    
    int restart_count = 0;
    float total_sum = 0.0f;
    
restart_point:  /* Requirement 6: goto label for restart */
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 10; i++) {
        int outer_acc = 0;
        
        for (int j = 0; j < 20; j++) {
            float inner_sum = 0.0f;
            
            for (int k = 0; k < SIZE/10; k++) {
                /* Loop-carried dependency */
                int idx = (i * 200 + j * 10 + k) % SIZE;
                
                /* Mixed integer and floating-point operations */
                float_array[idx] = float_array[idx] * 0.99f + 0.01f;
                int_array[idx] = int_array[idx] + (int)(float_array[idx] * 10);
                
                /* Conditional branch inside innermost loop */
                if (int_array[idx] % 7 == 0) {
                    inner_sum += float_array[idx];
                    /* Call noinline function */
                    total_sum += process_chunk(float_array, idx, idx + 4);
                } else if (int_array[idx] % 13 == 0) {
                    /* Cold path */
                    if (int_array[idx] > 10000) {
                        error_handler("Value too large");
                    }
                }
                
                /* Another asm barrier */
                asm volatile ("" : : : "memory");
            }
            
            outer_acc += (int)inner_sum;
            
            /* SIMD operations */
            if (j % 3 == 0) {
                simd_operation(float_array, float_array2, result_array, SIZE);
            }
        }
        
        /* Process with barriers */
        process_with_barriers(int_array, SIZE);
        
        /* Complex reduction */
        int reduction_result = complex_reduction(int_array, SIZE);
        
        /* Requirement 5: Large switch statement with non-linear cases */
        int switch_val = (reduction_result + outer_acc) % SWITCH_CASES;
        
        switch (switch_val) {
            case 0: total_sum += 1.0f; break;
            case 3: total_sum *= 1.1f; break;
            case 7: total_sum -= float_array[0]; break;
            case 12: 
                for (int x = 0; x < 10; x++) {
                    total_sum += int_array[x] * 0.5f;
                }
                break;
            case 15:
                /* Nested loop in switch case */
                for (int x = 0; x < 5; x++) {
                    for (int y = 0; y < 5; y++) {
                        total_sum += x * y * 0.01f;
                    }
                }
                break;
            case 18:
                /* Use goto within switch */
                if (restart_count < 2) {
                    restart_count++;
                    goto restart_point;
                }
                break;
            case 21:
                /* Another SIMD operation */
                simd_operation(result_array, float_array, float_array2, SIZE);
                break;
            case 24:
                /* Complex default-like case at high value */
                total_sum = total_sum * 0.5f + 100.0f;
                for (int x = 0; x < SIZE; x += 8) {
                    float_array[x] = total_sum * 0.01f;
                }
                break;
            default:
                /* Default case with complex operation */
                for (int x = 0; x < SIZE; x++) {
                    if (x % 2 == 0) {
                        float_array[x] = float_array[x] * 2.0f - 1.0f;
                    } else {
                        float_array[x] = float_array[x] * 0.5f + 0.5f;
                    }
                }
                total_sum += 50.0f;
                break;
        }
        
        /* Requirement 6: goto to jump within loops */
        if (i == 5 && total_sum > 10000.0f) {
            goto skip_processing;
        }
        
        /* Continue normal processing */
        for (int x = 0; x < 100; x++) {
            int_array[x] = int_array[x] * 2 - x;
        }
        
skip_processing:
        /* Another asm barrier with many clobbers */
        asm volatile (
            "mov $0, %%eax\n\t"
            "mov $0, %%ebx\n\t"
            "mov $0, %%ecx"
            : 
            : 
            : "eax", "ebx", "ecx", "cc", "memory"
        );
    }
    
    /* Final validation */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum += float_array[i] + int_array[i] * 0.01f;
    }
    
    checksum += total_sum;
    
    printf("Final checksum: %f\n", checksum);
    printf("Restart count: %d\n", restart_count);
    
    if (checksum > 0) {
        printf("SUCCESS: Scheduler test completed\n");
    } else {
        printf("FAILURE: Invalid checksum\n");
    }
    
    return 0;
}
