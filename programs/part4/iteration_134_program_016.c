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
static void simd_operation(float* a, float* b, float* c, int n) {
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_storeu_ps(&c[i], vc);
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
        /* More asm barriers to force scheduler partitioning */
        asm volatile ("" ::: "memory");
        data[i] = data[i] * 2 + 1;
        asm volatile ("" ::: "memory");
    }
}

int main(void) {
    /* Initialize arrays */
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    float float_array2[ARRAY_SIZE];
    float float_array3[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i % 100;
        float_array[i] = (float)i * 0.1f;
        float_array2[i] = (float)i * 0.2f;
        float_array3[i] = 0.0f;
    }
    
    int outer_sum = 0;
    float outer_float_sum = 0.0f;
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 10; i++) {
        int middle_sum = 0;
        
        for (int j = 0; j < 20; j++) {
            int inner_sum = 0;
            float inner_float = 0.0f;
            
            for (int k = 0; k < ARRAY_SIZE / 10; k++) {
                /* Loop-carried dependencies */
                inner_sum += int_array[k] * (i + j + k);
                inner_float += float_array[k] * (i * j * 0.01f);
                
                /* Conditional branches inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    inner_sum -= int_array[k] / 2;
                    inner_float *= 0.9f;
                } else if ((i + j + k) % 13 == 0) {
                    inner_sum += int_array[k] * 3;
                    inner_float /= 1.1f;
                }
                
                /* Mixed integer and floating-point operations */
                if (k % 5 == 0) {
                    float temp = sinf(inner_float * 0.01f);
                    inner_sum += (int)(temp * 100);
                }
            }
            
            middle_sum += inner_sum;
            outer_float_sum += inner_float;
            
            /* Call noinline functions */
            if (j % 3 == 0) {
                float chunk_sum = process_chunk(float_array, 0, ARRAY_SIZE/4);
                outer_float_sum += chunk_sum;
            }
        }
        
        outer_sum += middle_sum;
        
        /* SIMD operations */
        simd_operation(float_array, float_array2, float_array3, ARRAY_SIZE);
        
        /* Process with assembly barriers */
        process_with_barriers(int_array, ARRAY_SIZE/8);
    }
    
    /* Complex reduction with asm */
    int complex_result = complex_reduction(int_array, ARRAY_SIZE/2);
    outer_sum += complex_result;
    
    /* Requirement 5: Large switch statement with non-linear cases */
    int switch_var = (outer_sum % 1000) * 7;
    
    /* Requirement 6: goto with labels */
    int restart_count = 0;
    
restart_point:
    
    switch (switch_var) {
        case 1:
            outer_sum += int_array[0] * 2;
            break;
        case 7:
            outer_sum -= int_array[1] / 3;
            break;
        case 13:
            outer_float_sum += float_array[2] * 1.5f;
            break;
        case 42:
            outer_sum = (outer_sum << 3) | (outer_sum >> 29);
            break;
        case 100:
            outer_float_sum = sqrtf(fabsf(outer_float_sum));
            break;
        case 256:
            outer_sum ^= int_array[3];
            break;
        case 512:
            outer_float_sum = logf(fabsf(outer_float_sum) + 1.0f);
            break;
        case 777:
            outer_sum = ~outer_sum;
            break;
        case 1024:
            outer_float_sum = outer_float_sum * outer_float_sum;
            break;
        case 2048:
            outer_sum = outer_sum * 3 + 7;
            break;
        case 3333:
            outer_float_sum = cosf(outer_float_sum);
            break;
        case 4096:
            outer_sum = outer_sum & 0xFFFF;
            break;
        case 5000:
            outer_float_sum = 1.0f / (outer_float_sum + 0.001f);
            break;
        case 6000:
            outer_sum = outer_sum | 0xFF00;
            break;
        case 7000:
            outer_float_sum = tanf(outer_float_sum);
            break;
        case 8192:
            outer_sum = outer_sum % 12345;
            break;
        case 9000:
            outer_float_sum = expf(outer_float_sum * 0.01f);
            break;
        case 10000:
            outer_sum = (outer_sum << 1) + (outer_sum >> 31);
            break;
        case 11000:
            outer_float_sum = powf(fabsf(outer_float_sum), 1.5f);
            break;
        case 12000:
            outer_sum = outer_sum + 0xABCD;
            break;
        case 13000:
            outer_float_sum = asinf(fabsf(outer_float_sum) > 1.0f ? 1.0f : fabsf(outer_float_sum));
            break;
        case 14000:
            outer_sum = outer_sum * 11 / 7;
            break;
        case 15000:
            outer_float_sum = atanf(outer_float_sum);
            break;
        case 16384:
            outer_sum = outer_sum ^ 0xDEADBEEF;
            break;
        default:
            /* Complex default case */
            for (int i = 0; i < 50; i++) {
                outer_sum += i * int_array[i % ARRAY_SIZE];
                outer_float_sum += sinf(float_array[i % ARRAY_SIZE]);
            }
            if (restart_count < 2) {
                restart_count++;
                switch_var = (switch_var * 13 + 7) % 15000;
                goto restart_point;  /* Requirement 6: goto to create irreducible flow */
            }
            break;
    }
    
    /* More complex control flow with goto */
    if (outer_sum < 0) {
        goto negative_path;
    } else {
        goto positive_path;
    }
    
negative_path:
    outer_sum = -outer_sum;
    for (int i = 0; i < ARRAY_SIZE/16; i++) {
        if (i % 2 == 0) {
            continue;  /* Targeting outer loop */
        }
        int_array[i] = outer_sum + i;
    }
    goto finish;
    
positive_path:
    for (int i = 0; i < ARRAY_SIZE/16; i++) {
        if (i == 10) {
            break;  /* Break from this loop */
        }
        int_array[i] = outer_sum - i;
    }
    
finish:
    /* Final validation */
    int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE/32; i++) {
        final_sum += int_array[i];
    }
    
    float final_float_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE/32; i++) {
        final_float_sum += float_array[i] + float_array3[i];
    }
    
    /* Use cold function on error path */
    if (final_sum == 0 && final_float_sum == 0.0f) {
        error_handler("Unexpected zero result");
    }
    
    printf("Result: sum=%d, float_sum=%f\n", final_sum, final_float_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
