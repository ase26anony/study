/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
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
    
    /* SIMD processing */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 v = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, v);
        
        /* Inline asm barrier - forces scheduler to partition */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract SIMD results */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Scalar remainder */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier */
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    /* Complex integer operations with loop-carried dependencies */
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        int block_sum = 0;
        for (j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx >= size) break;
            
            /* Conditional inside innermost loop */
            if (arr[idx] > threshold) {
                for (k = 0; k < 3; k++) {
                    /* Artificial dependency chain */
                    block_sum += arr[idx] * (k + 1);
                    
                    /* Inline asm with clobbers */
                    asm volatile ("# Dependency barrier" 
                                 : "=r"(block_sum) 
                                 : "0"(block_sum) 
                                 : "rax", "rbx", "rcx");
                }
            } else {
                block_sum -= arr[idx];
            }
        }
        result += block_sum;
    }
    
    return result;
}

/* Global variables to create dependencies */
static float g_float_array[ARRAY_SIZE];
static int g_int_array[ARRAY_SIZE];
static volatile int g_switch_result = 0;

int main(void) {
    int i, j, k;
    float total_float = 0.0f;
    int total_int = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        g_float_array[i] = (i % 100) * 0.1f;
        g_int_array[i] = (i * 3) % 77;
    }
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with complex control flow (requirement 1) */
    for (i = 0; i < 10; i++) {
        int outer_acc = 0;
        
        for (j = 0; j < 20; j++) {
            float inner_sum = 0.0f;
            
            /* Innermost loop with mixed operations */
            for (k = 0; k < ARRAY_SIZE / 10; k++) {
                int idx = (i * 20 + j) * (ARRAY_SIZE / 200) + k;
                if (idx >= ARRAY_SIZE) {
                    /* Use goto to break to outer scope */
                    goto skip_inner;
                }
                
                /* Loop-carried dependency */
                inner_sum += g_float_array[idx] * (k % 5);
                
                /* Conditional branch inside innermost loop */
                if (g_int_array[idx] > 50) {
                    inner_sum *= 1.1f;
                    
                    /* Call noinline function */
                    total_float += process_chunk(g_float_array, 
                                                idx, 
                                                idx + 8);
                } else if (g_int_array[idx] < 10) {
                    /* Cold path hint */
                    if (g_int_array[idx] == 0) {
                        error_handler("Zero element detected");
                    }
                    inner_sum *= 0.9f;
                }
                
                /* Inline asm barrier */
                asm volatile ("# Loop barrier %0" : "+r"(inner_sum) :: "memory");
            }
            
            skip_inner:
            total_float += inner_sum;
            
            /* Complex integer reduction */
            if (j % 3 == 0) {
                total_int += integer_reduction(g_int_array, 
                                              ARRAY_SIZE / 4, 
                                              i * 10);
            }
            
            /* Non-linear control flow with goto */
            if (total_float > 1000000.0f) {
                /* This should rarely happen, but creates control flow edge */
                goto restart_point;
            }
        }
        
        /* Break to outer loop with label */
        if (i == 5 && total_int > 10000) {
            goto finish_loops;
        }
    }
    
    finish_loops:
    
    /* Large switch statement with non-sequential cases (requirement 5) */
    int switch_val = (total_int % 100) * 3;
    g_switch_result = 0;
    
    switch (switch_val) {
        case 0:  g_switch_result = total_int + 1; break;
        case 3:  g_switch_result = total_int * 2; break;
        case 7:  g_switch_result = total_int / 2; break;
        case 12: g_switch_result = total_int - 100; break;
        case 18: 
            /* Complex case with SIMD */
            {
                __m128i v1 = _mm_set1_epi32(total_int);
                __m128i v2 = _mm_set1_epi32(g_switch_result);
                __m128i v3 = _mm_add_epi32(v1, v2);
                int temp[4];
                _mm_storeu_si128((__m128i*)temp, v3);
                g_switch_result = temp[0] + temp[1] + temp[2] + temp[3];
            }
            break;
        case 23: g_switch_result = (int)(total_float * 0.5f); break;
        case 29: g_switch_result = total_int ^ 0xAAAA; break;
        case 35: g_switch_result = total_int << 2; break;
        case 42: g_switch_result = total_int >> 1; break;
        case 50: 
            {
                /* Nested switch in case */
                int inner_val = g_switch_result % 5;
                switch (inner_val) {
                    case 0: g_switch_result += 10; break;
                    case 1: g_switch_result += 20; break;
                    default: g_switch_result += 30; break;
                }
            }
            break;
        case 58: g_switch_result = -total_int; break;
        case 66: g_switch_result = total_int * total_int % 1000; break;
        case 75: g_switch_result = (int)(total_float) % 256; break;
        case 83: g_switch_result = ~total_int; break;
        case 91: g_switch_result = total_int | 0xFF; break;
        case 100: g_switch_result = total_int & 0xFFFF; break;
        case 110: 
            /* Call with inline asm */
            asm volatile (
                "mov %1, %%eax\n"
                "imul %%eax, %%eax\n"
                "mov %%eax, %0\n"
                : "=r"(g_switch_result)
                : "r"(total_int)
                : "%eax", "cc"
            );
            break;
        case 120: g_switch_result = abs(total_int); break;
        case 131: g_switch_result = total_int + switch_val; break;
        case 143: g_switch_result = total_int - switch_val; break;
        case 156: g_switch_result = total_int * switch_val % 100; break;
        case 170: g_switch_result = (total_int << 3) | (switch_val & 7); break;
        case 185: g_switch_result = (int)(total_float / 10.0f); break;
        case 201: g_switch_result = switch_val * 2 - total_int; break;
        case 218: g_switch_result = (switch_val > 100) ? total_int : -total_int; break;
        default:
            /* Default case with complex operation */
            {
                float f = total_float;
                for (int n = 0; n < 10; n++) {
                    f = f * 0.9f + total_int * 0.1f;
                    asm volatile ("" : "+r"(f) :: "memory");
                }
                g_switch_result = (int)f;
            }
            break;
    }
    
    /* Final validation */
    float checksum = total_float + total_int + g_switch_result;
    
    /* Use result to prevent optimization */
    if (checksum > 0.0f) {
        printf("Scheduler test completed. Checksum: %f\n", checksum);
        printf("Results: float=%f, int=%d, switch=%d\n", 
               total_float, total_int, g_switch_result);
    } else {
        error_handler("Negative checksum");
    }
    
    return 0;
}
