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
        __m128 result = _mm_add_ps(_mm_mul_ps(vec_a, vec_b), 
                                  _mm_set1_ps(1.0f));
        _mm_storeu_ps(&a[i], result);
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
        "movl %%eax, %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        : 
        : "a" (*counter)
        : "ebx", "cc", "memory"
    );
}

/* Requirement 5: Large switch statement with non-linear cases */
static int process_switch(int value, float* arr, int* int_arr) {
    int result = 0;
    
    switch (value) {
        case 1: result = arr[0] * 2; break;
        case 3: result = int_arr[1] + 5; break;
        case 7: result = arr[2] / 3.0f; break;
        case 13: result = int_arr[3] * 2; break;
        case 21: result = arr[4] + arr[5]; break;
        case 34: result = int_arr[6] - 10; break;
        case 55: result = arr[7] * arr[8]; break;
        case 89: result = int_arr[9] << 2; break;
        case 144: result = arr[10] / 2.0f; break;
        case 233: result = int_arr[11] | 0xFF; break;
        case 377: result = arr[12] + 100.0f; break;
        case 610: result = int_arr[13] & 0x0F; break;
        case 987: result = arr[14] * 3.14f; break;
        case 1597: result = int_arr[15] ^ 0xAA; break;
        case 2584: result = arr[16] - 50.0f; break;
        case 4181: result = int_arr[17] % 17; break;
        case 6765: result = arr[18] * arr[19]; break;
        case 10946: result = int_arr[20] + 999; break;
        case 17711: result = arr[21] / 1.618f; break;
        case 28657: result = int_arr[22] * 3; break;
        case 46368: result = arr[23] + 777.0f; break;
        case 75025: result = int_arr[24] >> 1; break;
        case 121393: result = arr[25] * 2.718f; break;
        case 196418: result = int_arr[26] - 1234; break;
        default:
            /* Complex default case */
            for (int i = 0; i < 10; i++) {
                result += int_arr[i] * arr[i];
            }
            result = (result * 31) & 0xFFFF;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float float_arr[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    double double_arr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_arr[i] = (i % 100) * 0.1f;
        int_arr[i] = i * 3;
        double_arr[i] = i * 0.01;
    }
    
    float total_sum = 0.0f;
    int int_sum = 0;
    double double_sum = 0.0;
    
    int restart_counter = 0;
    int asm_counter = 0;
    
restart_point:  /* Requirement 6: goto label for restart mechanism */
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 50; i++) {
        float loop_sum = 0.0f;
        
        for (int j = 0; j < 40; j++) {
            int inner_sum = 0;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependencies */
                inner_sum += int_arr[(i + j + k) % ARRAY_SIZE];
                
                /* Conditional branch inside innermost loop */
                if ((i * j * k) % 7 == 0) {
                    loop_sum += float_arr[(i + j) % ARRAY_SIZE] * 0.3f;
                    
                    /* Mixed integer and floating-point operations */
                    double_sum += (double)inner_sum * 0.01;
                } else if ((i + j + k) % 11 == 0) {
                    loop_sum -= float_arr[(j + k) % ARRAY_SIZE] * 0.2f;
                    int_sum -= k * 2;
                }
                
                /* SIMD operations in some iterations */
                if (k % 8 == 0) {
                    simd_operation(float_arr, float_arr + 128, 64);
                }
                
                /* Inline assembly barriers */
                if (k % 5 == 0) {
                    barrier_operation(&asm_counter);
                }
            }
            
            /* Data dependencies across loop iterations */
            int_arr[i % ARRAY_SIZE] = inner_sum % 1000;
            
            /* Call noinline function */
            if (j % 10 == 0) {
                float chunk_sum = process_chunk(float_arr, j * 10, (j + 1) * 10);
                total_sum += chunk_sum;
            }
        }
        
        total_sum += loop_sum;
        
        /* Large switch statement based on computed values */
        int switch_val = (int)(fabs(loop_sum) * 100) % (SWITCH_CASES * 1000);
        int switch_result = process_switch(switch_val, float_arr, int_arr);
        int_sum += switch_result;
        
        /* Requirement 6: goto to create irreducible control flow */
        if (i == 25 && restart_counter < 2) {
            restart_counter++;
            printf("Restarting computation...\n");
            goto restart_point;
        }
        
        /* Another goto jumping to different point */
        if (i == 35 && int_sum > 1000000) {
            printf("Jumping to cleanup section\n");
            goto cleanup_section;
        }
    }
    
    /* Another complex loop with goto and continue to outer loop */
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < 20; inner++) {
            if ((outer * inner) % 13 == 0) {
                /* Jump to outer loop continue */
                if (inner % 3 == 0) {
                    continue;
                }
                
                /* Jump to different label */
                if (inner == 15) {
                    goto special_processing;
                }
            }
            
            /* More computations */
            double_arr[outer * 20 + inner] = 
                sin(float_arr[inner]) * cos(double_arr[outer]);
        }
        
        special_processing:
        /* Complex processing block */
        for (int i = 0; i < 10; i++) {
            float_arr[outer * 10 + i] = 
                sqrt(fabs(float_arr[outer * 10 + i])) + 1.0f;
        }
    }
    
cleanup_section:
    
    /* Final validation */
    float checksum = total_sum + int_sum + (float)double_sum;
    printf("Computation complete. Checksum: %f\n", checksum);
    
    /* Cold error path (rarely taken) */
    if (checksum != checksum) {  /* NaN check */
        error_handler("Invalid checksum detected");
        return 1;
    }
    
    return 0;
}
