/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling decisions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold error path - unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* data, int start, int end) {
    /* Mixed scalar and SIMD operations */
    float sum = 0.0f;
    int i;
    
    /* SSE operations */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline asm barrier to force scheduler partitioning */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Store SSE result */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Handle remainder */
    for (; i < end; i++) {
        sum += data[i];
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    /* Complex integer operations with dependencies */
    int result = 0;
    int i, j;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i++) {
        int temp = arr[i];
        for (j = 0; j < 8; j++) {
            temp = (temp * 1103515245 + 12345) & 0x7fffffff;
            for (int k = 0; k < 4; k++) {
                /* Artificial dependency chain */
                temp ^= (temp >> 13);
                temp ^= (temp << 17);
                temp ^= (temp >> 5);
                
                /* Conditional inside innermost loop */
                if (temp % 7 == 0) {
                    temp += k;
                } else {
                    temp -= k;
                }
            }
            
            /* Another asm barrier */
            asm volatile ("" ::: "eax", "ebx", "ecx", "edx", "memory");
        }
        
        result ^= temp;
        
        /* Complex condition */
        if (result > threshold && (i & 3) == 0) {
            result >>= 1;
        }
    }
    
    return result;
}

/* Global variables to create dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

int main(void) {
    /* Declare and initialize arrays */
    float fdata[ARRAY_SIZE];
    int idata[ARRAY_SIZE];
    int switch_data[SWITCH_CASES];
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fdata[i] = sinf(i * 0.01f);
        idata[i] = (i * 1103515245 + 12345) & 0x7fff;
    }
    
    for (int i = 0; i < SWITCH_CASES; i++) {
        switch_data[i] = (i * 31) % 100;
    }
    
    /* Main computation with nested loops */
    float total_sum = 0.0f;
    int loop_result = 0;
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with mixed operations */
    for (int outer = 0; outer < 16; outer++) {
        float chunk_sum = 0.0f;
        
        for (int middle = 0; middle < 8; middle++) {
            /* Call noinline function with SIMD */
            float partial = process_chunk(fdata, 
                                         (outer * 64 + middle * 8) % ARRAY_SIZE,
                                         (outer * 64 + middle * 8 + 64) % ARRAY_SIZE);
            chunk_sum += partial;
            
            /* Inline asm with clobbers */
            asm volatile ("# Barrier\n\t" 
                         ::: "memory", "cc", "eax", "ebx", "ecx", "edx");
            
            for (int inner = 0; inner < 4; inner++) {
                /* Loop-carried dependency */
                global_accumulator = global_accumulator * 0.99f + chunk_sum * 0.01f;
                
                /* Conditional with floating point */
                if (global_accumulator > 100.0f) {
                    global_accumulator = 0.0f;
                    global_counter++;
                }
                
                /* Mixed integer operations */
                int temp = (inner + middle * 4 + outer * 32) & 0xff;
                temp = (temp * 13 + 7) % 256;
                
                /* Another asm barrier */
                asm volatile ("" ::: "memory");
                
                /* Complex condition */
                if ((temp & 0x0f) == 0 && inner > 1) {
                    /* Call cold function (unlikely) */
                    if (temp == 0) {
                        error_handler("Zero temp value");
                    }
                    
                    /* Modify data with dependency */
                    idata[temp % ARRAY_SIZE] ^= temp;
                }
            }
            
            /* goto to create irreducible flow */
            if (middle == 3 && outer > 8) {
                goto skip_section;
            }
        }
        
        skip_section:
        total_sum += chunk_sum;
        
        /* Another asm barrier between loop iterations */
        asm volatile ("# Loop barrier\n\t" 
                     ::: "memory", "xmm4", "xmm5", "xmm6", "xmm7");
    }
    
    /* Integer reduction with noinline call */
    loop_result = integer_reduction(idata, ARRAY_SIZE / 4, 1000000);
    
    /* Large switch statement with non-sequential cases (requirement 5) */
    int switch_var = (loop_result & 0xff) % SWITCH_CASES;
    float switch_modifier = 0.0f;
    
    switch (switch_var) {
        case 0: switch_modifier = 1.0f; break;
        case 3: switch_modifier = 2.5f; break;
        case 7: switch_modifier = 0.5f; break;
        case 12: switch_modifier = 3.0f; break;
        case 1: switch_modifier = 1.2f; break;
        case 19: switch_modifier = 0.8f; break;
        case 24: switch_modifier = 2.0f; break;
        case 5: switch_modifier = 1.5f; break;
        case 8: switch_modifier = 0.3f; break;
        case 15: switch_modifier = 1.8f; break;
        case 2: switch_modifier = 0.7f; break;
        case 11: switch_modifier = 2.2f; break;
        case 17: switch_modifier = 1.1f; break;
        case 20: switch_modifier = 0.9f; break;
        case 6: switch_modifier = 1.3f; break;
        case 9: switch_modifier = 0.6f; break;
        case 13: switch_modifier = 2.8f; break;
        case 18: switch_modifier = 1.4f; break;
        case 21: switch_modifier = 0.4f; break;
        case 4: switch_modifier = 1.9f; break;
        case 10: switch_modifier = 2.1f; break;
        case 14: switch_modifier = 1.7f; break;
        case 16: switch_modifier = 0.2f; break;
        case 22: switch_modifier = 2.3f; break;
        case 23: switch_modifier = 1.6f; break;
        default:
            /* Complex default case */
            switch_modifier = sqrtf(fabsf(total_sum));
            for (int i = 0; i < 10; i++) {
                switch_modifier = sinf(switch_modifier) * 2.0f;
            }
            break;
    }
    
    /* Modify data based on switch result */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        fdata[i] *= switch_modifier;
        
        /* Dependency across iterations */
        if (i > 0) {
            fdata[i] += fdata[i-1] * 0.1f;
        }
    }
    
    /* goto-based restart mechanism (requirement 6) */
    if (global_counter < 2 && loop_result > 500000) {
        /* Jump back to create complex control flow */
        goto restart_point;
    }
    
    /* Final validation */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += fdata[i] + idata[i];
    }
    
    int final_int = (int)fabsf(final_sum) % 1000;
    
    /* Simple checksum */
    if (final_int != 0) {
        printf("Test completed successfully. Checksum: %d\n", final_int);
        printf("Global counter: %d, Accumulator: %f\n", 
               global_counter, global_accumulator);
    } else {
        printf("Test completed (zero checksum)\n");
    }
    
    return 0;
}
