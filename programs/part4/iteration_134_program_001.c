/* Test program to trigger free_sched_state cleanup in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Non-inline functions to force call boundaries */
__attribute__((noinline, cold))
void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
float complex_calculation(float a, float b, int iter) {
    float result = a;
    for (int i = 0; i < iter % 8; i++) {
        result = result * b - a / (b + 1.0f);
        /* Inline asm barrier to force scheduler partitioning */
        asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
    }
    return result;
}

__attribute__((noinline))
void process_simd(float* arr, int size) {
    /* SIMD operations mixed with scalar */
    for (int i = 0; i < size - 3; i += 4) {
        __m128 vec_a = _mm_loadu_ps(&arr[i]);
        __m128 vec_b = _mm_set1_ps(1.618034f); /* Golden ratio */
        __m128 vec_c = _mm_mul_ps(vec_a, vec_b);
        
        /* Scalar operation in SIMD loop */
        float temp = arr[i] * 0.5f;
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2");
        
        __m128 vec_d = _mm_add_ps(vec_c, _mm_set1_ps(temp));
        _mm_storeu_ps(&arr[i], vec_d);
    }
}

__attribute__((noinline))
int conditional_reduction(int* data, int size, int threshold) {
    int sum = 0;
    int product = 1;
    
    /* Complex loop with carried dependencies */
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            sum += data[i];
            /* Artificial scheduling barrier */
            asm volatile ("" ::: "cc", "memory");
            product *= (data[i] % 7) + 1;
        } else {
            sum -= data[i] / 2;
        }
        
        /* Loop-carried dependency */
        data[i] = sum ^ product;
    }
    return sum ^ product;
}

/* Global variables for cross-function dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

int main(void) {
    /* Initialize arrays with pattern */
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 13) % 97;
        float_data[i] = sinf(i * 0.1f) * 100.0f;
    }
    
    /* Triple nested loops with complex dependencies */
    int outer_sum = 0;
    float outer_product = 1.0f;
    
    for (int i = 0; i < 32; i++) {
        int middle_sum = 0;
        
        for (int j = 0; j < 16; j++) {
            int inner_sum = 0;
            
            for (int k = 0; k < 8; k++) {
                /* Loop-carried dependency chain */
                inner_sum = inner_sum * 3 + int_data[(i + j + k) % ARRAY_SIZE];
                
                /* Conditional inside innermost loop */
                if ((inner_sum & 1) == 0) {
                    float_data[(i * j + k) % ARRAY_SIZE] += 
                        complex_calculation(float_data[k], inner_sum * 0.01f, k);
                } else {
                    /* Cold path */
                    if (inner_sum < 0) {
                        error_handler("Negative inner sum");
                    }
                }
                
                /* Mixed floating-point operations */
                outer_product *= 0.999f + float_data[k] * 0.0001f;
                
                /* Scheduling barrier */
                asm volatile ("" ::: "memory", "esi", "edi");
            }
            
            middle_sum ^= inner_sum;
            
            /* Goto for irreducible control flow */
            if (middle_sum > 1000 && j < 8) {
                goto restart_middle;
            }
            continue;
            
        restart_middle:
            middle_sum /= 2;
            j += 2; /* Skip ahead */
        }
        
        outer_sum += middle_sum;
        
        /* Call SIMD function */
        process_simd(float_data + i * 16, 16);
    }
    
    /* Large switch with non-linear cases */
    int switch_key = (outer_sum ^ (int)outer_product) % SWITCH_CASES;
    float switch_result = 0.0f;
    
    switch (switch_key) {
        case 0:  switch_result = float_data[0] * 2.0f; break;
        case 1:  switch_result = float_data[1] / 3.0f; break;
        case 3:  switch_result = float_data[3] + 100.0f; break; /* Skip 2 */
        case 7:  switch_result = float_data[7] - 50.0f; break; /* Skip 4-6 */
        case 8:  switch_result = sqrtf(float_data[8]); break;
        case 12: switch_result = float_data[12] * float_data[13]; break;
        case 15: switch_result = 1.0f / float_data[15]; break;
        case 18: switch_result = powf(float_data[18], 1.5f); break;
        case 21: switch_result = fmodf(float_data[21], 7.0f); break;
        case 24: switch_result = float_data[24] + float_data[25]; break;
        default:
            /* Complex default case */
            for (int i = 0; i < 10; i++) {
                switch_result += complex_calculation(
                    float_data[i], 
                    switch_key * 0.1f, 
                    i
                );
            }
            switch_result *= 0.5f;
            break;
    }
    
    /* Final reduction with function call */
    int final_result = conditional_reduction(int_data, ARRAY_SIZE, 50);
    
    /* Update globals with dependencies */
    global_counter = final_result ^ (int)switch_result;
    global_accumulator += switch_result;
    
    /* Validation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= int_data[i];
        checksum += (int)(float_data[i] * 1000);
    }
    
    checksum ^= global_counter;
    checksum ^= (int)(global_accumulator * 1000);
    
    printf("Result: checksum = 0x%08x\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
