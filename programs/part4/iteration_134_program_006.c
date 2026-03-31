/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Non-inline helper functions to force call boundaries */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold error path that scheduler might speculate around */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SSE vector operations */
    __m128 vec_sum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        vec_sum = _mm_add_ps(vec_sum, vec);
        
        /* Inline asm barrier to force scheduling boundaries */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Store vector sum */
    float temp[4];
    _mm_storeu_ps(temp, vec_sum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Handle remainder */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier with different clobbers */
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int reduce_integer(int* arr, int size, int threshold) {
    /* Complex reduction with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i += 16) {
        int block_sum = 0;
        for (j = i; j < i + 16 && j < size; j++) {
            int val = arr[j];
            
            /* Inner loop with data-dependent computation */
            for (k = 0; k < 8; k++) {
                /* Loop-carried dependency */
                val = (val * 1103515245 + 12345) & 0x7fffffff;
                
                /* Conditional branch inside innermost loop */
                if (val % 7 == 0) {
                    block_sum += val % 31;
                } else {
                    block_sum -= val % 17;
                }
            }
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
        }
        
        result += block_sum;
        
        /* Conditional that might be rarely taken */
        if (result > threshold) {
            error_handler("Threshold exceeded");
            result = threshold;
        }
    }
    
    return result;
}

__attribute__((noinline))
static void process_with_simd(float* farr, int* iarr, int n) {
    /* Mixed integer/float SIMD operations */
    int i;
    
    for (i = 0; i + 7 < n; i += 8) {
        /* Load two SSE float vectors */
        __m128 v1 = _mm_loadu_ps(&farr[i]);
        __m128 v2 = _mm_loadu_ps(&farr[i + 4]);
        
        /* Perform arithmetic */
        __m128 mul1 = _mm_mul_ps(v1, _mm_set1_ps(1.5f));
        __m128 mul2 = _mm_mul_ps(v2, _mm_set1_ps(0.75f));
        
        /* Store results */
        _mm_storeu_ps(&farr[i], mul1);
        _mm_storeu_ps(&farr[i + 4], mul2);
        
        /* Integer SIMD emulation with scalar ops */
        int j;
        for (j = i; j < i + 8 && j < n; j++) {
            iarr[j] = (int)(farr[j] * 100.0f);
        }
        
        /* Barrier with extensive clobber list */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3",
                                     "xmm4", "xmm5", "xmm6", "xmm7");
    }
}

/* Global state to create dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

int main(void) {
    /* Declare and initialize arrays */
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (i % 37) * 0.1f;
        int_array[i] = i * 3;
    }
    
    /* Triple nested loops with complex control flow */
    float total_sum = 0.0f;
    int outer_loop_count = 0;
    
restart_point:  /* Label for goto (requirement 6) */
    
    for (i = 0; i < 8; i++) {
        if (global_counter > 1000) {
            /* Rare path using goto to create irreducible flow */
            goto skip_inner;
        }
        
        for (j = 0; j < 16; j++) {
            int chunk_start = (i * 16 + j) * 4;
            if (chunk_start >= ARRAY_SIZE) {
                continue;  /* Continue to next j iteration */
            }
            
            int chunk_end = chunk_start + 4;
            if (chunk_end > ARRAY_SIZE) {
                chunk_end = ARRAY_SIZE;
            }
            
            /* Call non-inline function with SIMD */
            float chunk_sum = process_chunk(float_array, chunk_start, chunk_end);
            
            /* Loop-carried dependency */
            global_accumulator += chunk_sum;
            
            for (k = 0; k < 4; k++) {
                /* Innermost loop with conditional */
                int idx = chunk_start + k;
                if (idx < ARRAY_SIZE) {
                    if (global_accumulator > 100.0f) {
                        /* Complex dependency chain */
                        int_array[idx] += (int)(global_accumulator);
                        global_accumulator *= 0.99f;
                    } else {
                        int_array[idx] -= (int)(global_accumulator * 2.0f);
                    }
                    
                    /* Inline asm with clobbers */
                    asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11");
                }
                
                /* Early break from innermost loop */
                if (int_array[chunk_start] > 10000) {
                    break;
                }
            }
            
            /* Another goto to create non-structured flow */
            if (j == 8 && i == 4) {
                goto special_case;
            }
        }
        
        /* Call integer reduction function */
        int reduction = reduce_integer(int_array, ARRAY_SIZE, 500000);
        global_counter += reduction % 100;
        
        /* Check for restart condition */
        if (global_counter < 0) {
            /* This should rarely happen, but creates control flow */
            goto restart_point;
        }
        
        continue;
        
    special_case:
        /* Special handling with SIMD */
        process_with_simd(float_array, int_array, ARRAY_SIZE);
        global_counter += 50;
    }
    
skip_inner:
    
    /* Large switch statement with non-sequential cases */
    int switch_value = global_counter % 37;
    float switch_result = 0.0f;
    
    switch (switch_value) {
        case 0:
            switch_result = float_array[0] * 2.0f;
            int_array[0] = (int)switch_result;
            break;
        case 1:
            for (i = 0; i < 10; i++) {
                switch_result += float_array[i];
            }
            break;
        case 3:  /* Note: case 2 is skipped */
            switch_result = process_chunk(float_array, 0, 64);
            break;
        case 7:
            switch_result = global_accumulator;
            /* Fall through */
        case 8:
            switch_result *= 1.1f;
            break;
        case 12:
            {
                __m128 v = _mm_set1_ps(switch_result);
                float temp[4];
                _mm_storeu_ps(temp, _mm_sqrt_ps(v));
                switch_result = temp[0];
            }
            break;
        case 15:
            for (i = 0; i < ARRAY_SIZE; i += 2) {
                switch_result += float_array[i] - float_array[i + 1];
            }
            break;
        case 18:
            switch_result = reduce_integer(int_array, 100, 1000) / 1000.0f;
            break;
        case 21:
            /* Complex case with nested loops */
            for (i = 0; i < 5; i++) {
                for (j = 0; j < 5; j++) {
                    switch_result += i * j * 0.1f;
                }
            }
            break;
        case 24:
            /* Use inline asm in switch case */
            asm volatile (
                "mov $0x1, %%eax\n"
                "cpuid\n"
                ::: "eax", "ebx", "ecx", "edx", "memory"
            );
            switch_result = 42.0f;
            break;
        case 30:
            /* Another SIMD operation */
            {
                __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
                __m128 b = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
                __m128 c = _mm_add_ps(a, b);
                float temp[4];
                _mm_storeu_ps(temp, c);
                switch_result = temp[0] + temp[1] + temp[2] + temp[3];
            }
            break;
        case 33:
            /* Call error handler (cold function) */
            error_handler("Switch case 33 reached");
            break;
        case 36:
            /* Dense computation */
            for (i = 0; i < ARRAY_SIZE; i++) {
                switch_result += float_array[i] * int_array[i];
            }
            break;
        default:
            /* Default case with complex operation */
            if (switch_value % 2 == 0) {
                for (i = 0; i < ARRAY_SIZE; i++) {
                    float_array[i] = float_array[i] * 0.5f + switch_result;
                    asm volatile ("" ::: "memory");
                }
            } else {
                switch_result = process_chunk(float_array, 
                                             ARRAY_SIZE / 2, 
                                             ARRAY_SIZE);
            }
            break;
    }
    
    /* Final validation */
    float final_sum = 0.0f;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += float_array[i] + int_array[i];
    }
    
    final_sum += switch_result + global_accumulator + global_counter;
    
    /* Print result for verification */
    printf("Final computed value: %f\n", final_sum);
    printf("Test completed successfully\n");
    
    return 0;
}
