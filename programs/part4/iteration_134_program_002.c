#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with specific attributes */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    __m128 sum_vec = _mm_setzero_ps();
    float result = 0.0f;
    
    for (int i = 0; i < size; i += 4) {
        if (i + 4 <= size) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            sum_vec = _mm_add_ps(sum_vec, vec);
            
            // Artificial scheduling barrier
            asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        }
    }
    
    // Horizontal sum
    __m128 shuf = _mm_shuffle_ps(sum_vec, sum_vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(sum_vec, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    _mm_store_ss(&result, sums);
    
    return result;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int sum = 0;
    int temp = 0;
    
    for (int i = 0; i < size; i++) {
        // Complex data dependency chain
        temp = arr[i] * 3 + (temp >> 2);
        
        // Conditional with loop-carried dependency
        if (temp > threshold) {
            sum += temp;
            // Another scheduling barrier
            asm volatile ("" ::: "eax", "ebx", "ecx", "edx", "memory");
        } else {
            sum -= temp / 2;
        }
        
        // Cross-iteration dependency
        arr[i] = (sum + temp) % 1000;
    }
    
    return sum;
}

__attribute__((noinline))
static void mixed_operations(float* farr, int* iarr, int size) {
    for (int i = 0; i < size; i++) {
        // Mixed float/int operations
        float fval = farr[i];
        int ival = iarr[i];
        
        // Data-dependent branching
        if (fval > 0.5f) {
            farr[i] = sinf(fval) * ival;
            // Register clobber
            asm volatile ("" ::: "xmm4", "xmm5", "xmm6", "xmm7");
        } else {
            farr[i] = cosf(fval) / (ival + 1);
        }
        
        // Update integer array with dependency
        iarr[i] = (int)(farr[i] * 100) ^ ival;
    }
}

int main(void) {
    // Initialize arrays
    float float_array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    int int_array2[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_array[i] = (i % 100) * 0.01f;
        int_array[i] = i * 3;
        int_array2[i] = i * 7;
    }
    
    int outer_sum = 0;
    float outer_float_sum = 0.0f;
    
    // Nested loops with complex control flow (Requirement 1)
    for (int i = 0; i < 10; i++) {
        int middle_sum = 0;
        
        for (int j = 0; j < 20; j++) {
            int inner_sum = 0;
            
            for (int k = 0; k < 30; k++) {
                // Loop-carried dependencies
                static int persistent = 0;
                persistent = (persistent + i * j * k) % 1000;
                
                // Conditional inside innermost loop
                if ((i + j + k) % 7 == 0) {
                    inner_sum += persistent * 2;
                    // Scheduling barrier
                    asm volatile ("" ::: "memory", "esi", "edi");
                } else if ((i * j * k) % 11 == 0) {
                    inner_sum -= persistent / 3;
                } else {
                    inner_sum ^= persistent;
                }
                
                // Floating-point operation with dependency
                float_array[(i * j + k) % ARRAY_SIZE] += 
                    sinf(inner_sum * 0.001f) * 0.5f;
            }
            
            middle_sum += inner_sum;
            
            // Call noinline function with SIMD (Requirements 2 & 3)
            if (j % 5 == 0) {
                outer_float_sum += process_vector(float_array, ARRAY_SIZE);
            }
        }
        
        outer_sum += middle_sum;
        
        // Call noinline function with inline asm (Requirement 4)
        int reduction_result = integer_reduction(int_array, ARRAY_SIZE, 500);
        outer_sum ^= reduction_result;
        
        // Mixed operations
        mixed_operations(float_array, int_array2, ARRAY_SIZE);
    }
    
    // Large switch statement with non-linear cases (Requirement 5)
    int switch_var = outer_sum % 100;
    
    // Use goto for complex control flow (Requirement 6)
    restart_point:
    
    switch (switch_var) {
        case 0:  outer_sum += 1000; break;
        case 3:  outer_sum *= 2; break;
        case 7:  outer_sum /= 3; break;
        case 12: outer_sum ^= 0xABCD; break;
        case 15: outer_sum = abs(outer_sum); break;
        case 18: outer_sum += int_array[0]; break;
        case 22: outer_sum -= float_array[0]; break;
        case 25: outer_sum |= 0xFF00; break;
        case 28: outer_sum &= 0x00FF; break;
        case 31: outer_sum = ~outer_sum; break;
        case 35: outer_sum += process_vector(float_array, 128); break;
        case 38: outer_sum += integer_reduction(int_array2, 256, 200); break;
        case 42: outer_sum = outer_sum << 3; break;
        case 45: outer_sum = outer_sum >> 2; break;
        case 48: outer_sum += sinf(outer_sum) * 100; break;
        case 51: outer_sum = cosf(outer_sum) * 50; break;
        case 55: outer_sum += rand() % 100; break;
        case 58: outer_sum -= time(NULL) % 50; break;
        case 62: outer_sum = outer_sum % 777; break;
        case 66: outer_sum = outer_sum * outer_sum % 1000; break;
        case 70: outer_sum = sqrtf(abs(outer_sum)); break;
        case 75: outer_sum = logf(abs(outer_sum) + 1); break;
        case 80: outer_sum = expf(outer_sum * 0.01f); break;
        case 88: 
            // Complex default-like case
            for (int i = 0; i < 10; i++) {
                outer_sum += int_array[i] * float_array[i];
            }
            break;
        default:
            // Cold error path
            if (outer_sum < 0) {
                error_handler("Negative sum detected");
            }
            outer_sum = abs(outer_sum) + 100;
            break;
    }
    
    // Use goto to create irreducible flow (Requirement 6)
    if (outer_sum > 1000000) {
        outer_sum /= 1000;
        goto restart_point;
    }
    
    // Another level of nested loops with goto
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            if (i * j > 30) {
                goto skip_inner;
            }
            outer_sum += i * j;
            
            // Early continue to outer loop
            if ((i + j) % 3 == 0) {
                continue;
            }
            
            skip_inner:
            // Label for goto target
            asm volatile ("" ::: "memory");
        }
        
        // Break to specific label
        if (i == 3) {
            goto final_calculation;
        }
    }
    
    // This should be skipped by the goto
    outer_sum += 9999;
    
    final_calculation:
    
    // Final mixed operations
    mixed_operations(float_array, int_array, ARRAY_SIZE / 2);
    
    // Compute checksum
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i] + (int)float_array[i] + int_array2[i];
        checksum = (checksum * 13 + 7) % 1000000;
    }
    
    checksum += outer_sum + (int)outer_float_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
