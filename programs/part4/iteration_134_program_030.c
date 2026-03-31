/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
float complex_float_op(float a, float b, float c) {
    /* Mixed operations with inline asm barrier */
    float t1 = a * b + c;
    asm volatile ("" : : : "memory", "xmm0", "xmm1");
    float t2 = t1 / (b + 1.0f);
    asm volatile ("" : : : "cc", "xmm2", "xmm3");
    return t2 - a;
}

__attribute__((noinline))
int complex_int_op(int a, int b, int c) {
    /* Integer operations with dependencies */
    int t1 = (a * b) ^ c;
    asm volatile ("" : : : "memory", "eax", "ebx", "ecx");
    int t2 = t1 + (b << 3);
    int t3 = t2 | (c & 0xFF);
    return t3;
}

__attribute__((noinline))
void simd_operations(float* arr, int n) {
    /* SSE SIMD operations */
    for (int i = 0; i < n; i += 4) {
        if (i + 3 < n) {
            __m128 vec1 = _mm_loadu_ps(&arr[i]);
            __m128 vec2 = _mm_set1_ps(2.5f);
            __m128 result = _mm_add_ps(_mm_mul_ps(vec1, vec2), 
                                      _mm_set1_ps(1.0f));
            _mm_storeu_ps(&arr[i], result);
            asm volatile ("" : : : "xmm4", "xmm5", "xmm6", "xmm7");
        }
    }
}

/* Global variables for cross-function dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

int main(void) {
    /* Declare and initialize arrays */
    int int_array[SIZE];
    float float_array[SIZE];
    double double_array[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 100;
        float_array[i] = (i % 100) * 0.1f;
        double_array[i] = (i % 100) * 0.01;
    }
    
    int result = 0;
    float fresult = 0.0f;
    int outer_loop_counter = 0;
    
    /* Requirement 1: Nested loops with complex control flow */
    for (int i = 0; i < 50; i++) {
        if (i % 10 == 0) {
            /* Call noinline function */
            fresult += complex_float_op(float_array[i], fresult, i * 0.5f);
        }
        
        for (int j = 0; j < 40; j++) {
            int loop_dep = i * j;
            
            /* Loop-carried dependency */
            result += complex_int_op(int_array[j], result, loop_dep);
            
            for (int k = 0; k < 30; k++) {
                /* Innermost loop with conditional branches */
                if ((i + j + k) % 7 == 0) {
                    float_array[k] = float_array[k] * 1.1f + k;
                    asm volatile ("" : : : "memory");
                } else if ((i + j + k) % 13 == 0) {
                    int_array[k] = (int_array[k] ^ k) + j;
                    asm volatile ("" : : : "eax", "ebx");
                } else {
                    double temp = double_array[k];
                    double_array[k] = temp * 0.99 + (i * j * k) * 0.001;
                }
                
                /* Mixed integer/float operations */
                if (k % 3 == 0) {
                    fresult += float_array[j] * 0.3f - int_array[k] * 0.01f;
                }
                
                /* SIMD operations periodically */
                if ((i + j + k) % 100 == 0) {
                    simd_operations(float_array, SIZE);
                }
            }
            
            /* Requirement 6: goto with loop constructs */
            if (j == 20 && result > 1000000) {
                goto restart_point;
            }
        }
        
        outer_loop_counter++;
    }
    
    /* Requirement 5: Large switch statement with non-linear cases */
    int switch_val = (result % 97) * 3;
    
    switch (switch_val) {
        case 0:  result += int_array[0] * 2; break;
        case 3:  fresult = float_array[1] / 2.0f; break;
        case 7:  result ^= 0xABCD; break;
        case 12: fresult = complex_float_op(fresult, 2.0f, 3.0f); break;
        case 18: int_array[5] = result | 0xFF; break;
        case 23: float_array[10] *= 1.5f; break;
        case 29: result = complex_int_op(result, 17, 42); break;
        case 35: fresult -= float_array[15]; break;
        case 41: int_array[20] += switch_val; break;
        case 47: fresult = fresult * fresult - 1.0f; break;
        case 53: result = (result << 3) ^ 0xDEAD; break;
        case 59: float_array[25] = 0.0f; break;
        case 64: int_array[30] = ~int_array[30]; break;
        case 70: fresult = 1.0f / fresult; break;
        case 76: result += int_array[35] * 3; break;
        case 82: float_array[40] = complex_float_op(float_array[40], 0.5f, 2.0f); break;
        case 88: result = result % 1000; break;
        case 94: fresult += 10.0f; break;
        case 101: int_array[45] = switch_val; break;
        case 107: float_array[50] = -float_array[50]; break;
        case 113: result = result & 0xFFFF; break;
        case 119: fresult = fresult * 0.25f; break;
        case 125: int_array[55] = result + switch_val; break;
        case 131: float_array[60] = 3.14159f; break;
        case 137: result = result ^ int_array[65]; break;
        default:
            /* Complex default case */
            for (int i = 0; i < 20; i++) {
                result += complex_int_op(int_array[i], switch_val, i);
                fresult += complex_float_op(float_array[i], fresult, i * 0.1f);
                asm volatile ("" : : : "memory", "edx", "esi", "edi");
            }
            if (result > 1000000) {
                error_handler("Result too large in default case");
            }
            break;
    }
    
    /* Another level of nested loops with goto */
    for (int a = 0; a < 10; a++) {
        for (int b = 0; b < 10; b++) {
            if ((a * b) % 11 == 0) {
                goto skip_inner;
            }
            result += a * b;
            
            for (int c = 0; c < 5; c++) {
                if (c == 3) {
                    continue;  /* Skip iteration */
                }
                fresult += c * 0.1f;
            }
            
            skip_inner:
            /* Continue after goto */
            if (b == 8) {
                break;  /* Break out of inner loop */
            }
        }
        
        if (a == 5) {
            /* Requirement 6: goto to outer label */
            goto outer_skip;
        }
        
        result += a * 100;
        
        outer_skip:
        /* Empty statement after label */
        asm volatile ("" : : : "memory");
    }
    
    restart_point:
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < SIZE; i += 16) {
        checksum += int_array[i];
        checksum ^= (int)(float_array[i] * 100);
    }
    
    printf("Result: %d, Float result: %f, Checksum: %d\n", 
           result, fresult, checksum);
    
    if (checksum != 0) {
        printf("Test completed successfully (non-zero checksum)\n");
    }
    
    return 0;
}
