/* Test program to exercise GCC scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path unlikely to be taken */
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_chunk(float* arr, int start, int end) {
    float sum = 0.0f;
    /* Mixed scalar and SIMD operations */
    for (int i = start; i + 3 < end; i += 4) {
        __m128 vec = _mm_loadu_ps(&arr[i]);
        __m128 squared = _mm_mul_ps(vec, vec);
        
        /* Inline asm barrier creating scheduling boundary */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        
        float tmp[4];
        _mm_storeu_ps(tmp, squared);
        sum += tmp[0] + tmp[1] + tmp[2] + tmp[3];
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Artificial dependency chain */
        result = (result * 1103515245 + 12345) ^ arr[i];
        
        /* Another asm barrier */
        asm volatile ("# barrier" ::: "cc", "memory");
        
        if (barrier & 0x1) {
            result >>= 1;
        }
    }
    return result;
}

/* Main computational function with nested loops */
static void compute_patterns(float* farr, int* iarr, int size) {
    int i, j, k;
    float acc = 0.0f;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        float local_sum = 0.0f;
        
        for (j = 0; j < 8; j++) {
            int base = (i * 8 + j) % size;
            
            for (k = 0; k < 4; k++) {
                /* Loop-carried dependency */
                acc = acc * 0.99f + farr[(base + k) % size];
                
                /* Conditional inside innermost loop */
                if (acc > 1000.0f) {
                    acc *= 0.5f;
                    /* Call cold function on rare condition */
                    if (acc > 2000.0f) {
                        error_handler("Acc overflow");
                    }
                }
                
                /* Mixed integer operation */
                iarr[(base + k) % size] += (int)acc;
                
                /* Another asm barrier */
                asm volatile ("# inner loop barrier" ::: "memory");
            }
            
            /* Data-dependent branch */
            if (j % 3 == 0) {
                local_sum += process_chunk(farr, base, base + 16);
            }
        }
        
        farr[i] = local_sum;
    }
}

/* Function with large switch statement */
__attribute__((noinline))
static int dispatch_operation(int opcode, float* farr, int* iarr, int idx) {
    int result = 0;
    
    switch (opcode) {
        case 0:  result = iarr[idx] * 2; break;
        case 1:  result = iarr[idx] + farr[idx]; break;
        case 3:  result = iarr[idx] << 1; break;  /* Gap in cases */
        case 7:  result = iarr[idx] >> 2; break;
        case 12: result = iarr[idx] ^ 0x55AA; break;
        case 15: result = iarr[idx] | 0xFF00; break;
        case 18: result = iarr[idx] & 0x00FF; break;
        case 22: result = iarr[idx] * 3; break;
        case 25: result = iarr[idx] / 2; break;
        case 30: result = iarr[idx] + 100; break;
        case 35: result = iarr[idx] - 50; break;
        case 40: result = iarr[idx] % 17; break;
        case 45: result = ~iarr[idx]; break;
        case 50: result = iarr[idx] * iarr[idx+1]; break;
        case 55: result = iarr[idx] + iarr[idx-1]; break;
        case 60: result = abs(iarr[idx]); break;
        case 65: result = iarr[idx] * 5; break;
        case 70: result = iarr[idx] << 2; break;
        case 75: result = iarr[idx] >> 3; break;
        case 80: result = iarr[idx] ^ iarr[idx+2]; break;
        case 85: result = iarr[idx] | iarr[idx-2]; break;
        case 90: result = iarr[idx] & 0xF0F0; break;
        case 95: result = iarr[idx] * 7; break;
        default: /* Complex default case */
            result = complex_reduction(iarr, 64);
            farr[idx % SIZE] = result * 0.01f;
            break;
    }
    
    return result;
}

int main(void) {
    /* Declare and initialize arrays */
    float float_arr[SIZE];
    int int_arr[SIZE * 2];  /* Larger for safety */
    
    for (int i = 0; i < SIZE; i++) {
        float_arr[i] = (i % 100) * 0.1f;
        int_arr[i] = i * 3;
    }
    
    int restart_count = 0;
    float total = 0.0f;
    
restart_point:  /* Label for goto */
    
    /* Nested loops with complex control flow */
    compute_patterns(float_arr, int_arr, SIZE);
    
    /* Process with switch statement */
    for (int i = 0; i < SIZE; i++) {
        int opcode = (int_arr[i] * 1103515245 + 12345) % SWITCH_CASES;
        
        /* Use goto to create irreducible flow */
        if (i == SIZE/2 && restart_count == 0) {
            restart_count++;
            goto restart_point;  /* Jump back to restart */
        }
        
        int result = dispatch_operation(opcode, float_arr, int_arr, i % SIZE);
        
        /* Mix with SIMD operations */
        if (i % 8 == 0) {
            __m128 v1 = _mm_set_ps(float_arr[i], float_arr[i+1], 
                                  float_arr[i+2], float_arr[i+3]);
            __m128 v2 = _mm_set1_ps(result * 0.001f);
            __m128 v3 = _mm_add_ps(v1, v2);
            
            asm volatile ("# SIMD barrier" ::: "xmm0", "xmm1", "xmm2", "xmm3");
            
            float tmp[4];
            _mm_storeu_ps(tmp, v3);
            float_arr[i] = tmp[0];
        }
        
        total += result * 0.0001f;
        
        /* Conditional continue to outer loop */
        if (total > 100.0f) {
            total *= 0.9f;
            continue;
        }
        
        /* Break from middle of loop */
        if (total < -50.0f) {
            break;
        }
    }
    
    /* Another loop with goto to different label */
    int counter = 0;
    while (counter < 10) {
        counter++;
        
        if (counter == 5) {
            goto skip_section;
        }
        
        total += counter * 0.1f;
    }
    
skip_section:
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += int_arr[i] + (int)float_arr[i];
    }
    
    printf("Result: checksum=%d, total=%f\n", checksum, total);
    
    return 0;
}
