#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

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
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_storeu_ps(&a[i], vc);
    }
}

__attribute__((noinline))
static void barrier_operation(int* counter) {
    /* Requirement 4: asm volatile statements with clobbers */
    asm volatile (
        "addl $1, %0\n\t"
        : "+r" (*counter)
        : 
        : "cc", "memory"
    );
    
    /* Another barrier with more clobbers */
    asm volatile (
        "mfence\n\t"
        : 
        : 
        : "memory", "%eax", "%ebx", "%ecx", "%edx"
    );
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, float* arr, int idx) {
    int result = 0;
    
    switch (value) {
        case 1: arr[idx] += 1.0f; result = 1; break;
        case 3: arr[idx] *= 2.0f; result = 3; break;
        case 7: arr[idx] -= 0.5f; result = 7; break;
        case 13: arr[idx] /= 3.0f; result = 13; break;
        case 21: arr[idx] = arr[idx] * arr[idx]; result = 21; break;
        case 34: arr[idx] = -arr[idx]; result = 34; break;
        case 55: arr[idx] = 1.0f / arr[idx]; result = 55; break;
        case 89: arr[idx] = arr[idx] + arr[idx-1]; result = 89; break;
        case 144: arr[idx] = arr[idx] - arr[idx+1]; result = 144; break;
        case 233: arr[idx] = arr[idx] * 0.618f; result = 233; break;
        case 377: arr[idx] = arr[idx] / 1.618f; result = 377; break;
        case 610: arr[idx] = arr[idx] + 3.14159f; result = 610; break;
        case 987: arr[idx] = arr[idx] - 2.71828f; result = 987; break;
        case 1597: arr[idx] = arr[idx] * 1.4142f; result = 1597; break;
        case 2584: arr[idx] = arr[idx] / 1.7321f; result = 2584; break;
        case 4181: arr[idx] = arr[idx] + arr[idx] * 0.1f; result = 4181; break;
        case 6765: arr[idx] = arr[idx] - arr[idx] * 0.05f; result = 6765; break;
        case 10946: arr[idx] = sqrtf(arr[idx]); result = 10946; break;
        case 17711: arr[idx] = sinf(arr[idx]); result = 17711; break;
        case 28657: arr[idx] = cosf(arr[idx]); result = 28657; break;
        case 46368: arr[idx] = expf(arr[idx]); result = 46368; break;
        case 75025: arr[idx] = logf(arr[idx] + 1.0f); result = 75025; break;
        case 121393: arr[idx] = tanf(arr[idx]); result = 121393; break;
        case 196418: arr[idx] = atanf(arr[idx]); result = 196418; break;
        default: 
            /* Complex default case */
            for (int i = 0; i < 5; i++) {
                arr[idx] = arr[idx] * 0.9f + 0.1f;
                barrier_operation(&result);
            }
            result = -1;
            break;
    }
    
    return result;
}

int main() {
    /* Initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    float temp_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (float)(i % 100) * 0.1f;
        int_array[i] = i;
        temp_array[i] = 0.0f;
    }
    
    float total_sum = 0.0f;
    int outer_counter = 0;
    int restart_flag = 0;
    
    /* Requirement 6: goto and loop constructs */
    computation_start:
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 10; i++) {
        if (restart_flag && i > 5) {
            /* Use goto to jump out of nested loops */
            goto restart_point;
        }
        
        for (int j = 0; j < 20; j++) {
            float local_sum = 0.0f;
            
            for (int k = 0; k < ARRAY_SIZE / 10; k++) {
                /* Loop-carried dependency */
                int idx = (i * 200 + j * 10 + k) % ARRAY_SIZE;
                
                /* Mixed integer and floating-point operations */
                float_array[idx] = float_array[idx] + (float)int_array[idx] * 0.01f;
                
                /* Conditional branch inside innermost loop */
                if (float_array[idx] > 50.0f) {
                    float_array[idx] = 50.0f;
                    if (j % 7 == 0) {
                        error_handler("Value capped");
                    }
                } else if (float_array[idx] < -50.0f) {
                    float_array[idx] = -50.0f;
                }
                
                /* Data dependency chain */
                local_sum += float_array[idx];
                
                /* Call noinline function periodically */
                if (k % 100 == 0) {
                    float chunk_sum = process_chunk(float_array, idx, idx + 10);
                    local_sum += chunk_sum;
                }
                
                /* SIMD operations on temp array */
                if (k % 50 == 0) {
                    simd_operation(temp_array, float_array, 16);
                }
                
                /* Barrier operations */
                if (k % 25 == 0) {
                    barrier_operation(&outer_counter);
                }
                
                /* Switch statement with dependencies */
                int switch_val = (int)(float_array[idx] * 10) % SWITCH_CASES;
                switch_val = process_switch(switch_val, float_array, idx);
                
                /* Update int array based on switch result */
                int_array[idx] = (int_array[idx] + switch_val) % 1000;
            }
            
            total_sum += local_sum;
            
            /* Occasionally trigger restart */
            if (i == 3 && j == 7 && !restart_flag) {
                restart_flag = 1;
                goto computation_start;
            }
        }
        
        restart_point:
        /* Continue after goto */
        if (restart_flag) {
            restart_flag = 0;
            break;
        }
    }
    
    /* More complex control flow with goto */
    int retry_count = 0;
    retry_loop:
    for (int i = 0; i < 5; i++) {
        if (retry_count > 2) {
            goto final_check;
        }
        
        /* Another nested loop */
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                int idx = (i * 6 + j * 2 + k) % ARRAY_SIZE;
                float_array[idx] = float_array[idx] * 1.1f - 0.1f;
                
                /* More barriers */
                asm volatile (
                    "pushf\n\t"
                    "popf\n\t"
                    : 
                    : 
                    : "cc"
                );
                
                if (float_array[idx] < 0 && retry_count < 3) {
                    retry_count++;
                    goto retry_loop;
                }
            }
        }
    }
    
    final_check:
    
    /* Final validation */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += float_array[i] + int_array[i];
    }
    
    printf("Computation completed. Checksum: %f\n", checksum);
    printf("Total sum: %f, Outer counter: %d\n", total_sum, outer_counter);
    
    if (checksum != checksum) { /* NaN check */
        error_handler("Invalid checksum");
        return 1;
    }
    
    return 0;
}
