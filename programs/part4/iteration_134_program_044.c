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
void process_with_simd(float* restrict a, float* restrict b, int n) {
    /* SIMD operations mixed with scalar */
    for (int i = 0; i < n; i += 4) {
        __m128 vec_a = _mm_loadu_ps(&a[i]);
        __m128 vec_b = _mm_loadu_ps(&b[i]);
        __m128 result = _mm_add_ps(vec_a, vec_b);
        
        /* Inline asm barrier creating scheduling boundary */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
        
        _mm_storeu_ps(&a[i], result);
        
        /* Another barrier with different clobbers */
        asm volatile ("# barrier" ::: "cc", "xmm4", "xmm5");
    }
}

__attribute__((noinline))
int complex_reduction(int* arr, int n) {
    int sum = 0;
    int prod = 1;
    
    /* Nested loops with dependencies */
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0) {
            for (int j = 0; j < i; j++) {
                sum += arr[j];
                /* Loop-carried dependency */
                arr[j] = (arr[j] * 13 + 7) % 97;
                
                /* Conditional inside inner loop */
                if ((i * j) % 11 == 0) {
                    prod *= (arr[j] + 1);
                    asm volatile ("" ::: "rax", "rbx", "rcx");
                }
            }
        } else {
            /* Cold path */
            error_handler("Negative element encountered");
        }
    }
    
    return sum ^ prod;
}

/* Function with irreducible control flow using goto */
__attribute__((noinline))
void irreducible_flow(int* data, int n) {
    int i = 0;
    
loop_start:
    if (i >= n) goto loop_end;
    
    /* Complex computation with goto */
    data[i] = data[i] * 3 + 1;
    
    if (data[i] % 7 == 0) {
        i += 2;
        goto skip_increment;
    }
    
    if (data[i] > 1000) {
        /* Jump back to start */
        i = 0;
        goto loop_start;
    }
    
    i++;
    
skip_increment:
    if (i < n / 2) {
        goto loop_start;
    }
    
    /* Nested goto structure */
    for (int j = 0; j < 3; j++) {
        if (data[i] % (j + 2) == 0) {
            goto outer_continue;
        }
        data[i] -= j;
    }
    
outer_continue:
    goto loop_start;

loop_end:
    return;
}

int main(void) {
    /* Declare and initialize arrays */
    int int_arr[SIZE];
    float float_arr_a[SIZE];
    float float_arr_b[SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = (i * 17 + 23) % 123;
        float_arr_a[i] = (float)(i % 97) * 0.1f;
        float_arr_b[i] = (float)((i + 13) % 97) * 0.2f;
    }
    
    int restart_count = 0;
    int checksum = 0;
    
restart_point:
    if (restart_count++ > 3) {
        printf("Too many restarts\n");
        return 1;
    }
    
    /* Triple nested loops with dependencies */
    for (int i = 0; i < 16; i++) {
        int outer_sum = 0;
        
        for (int j = 0; j < 32; j++) {
            float inner_prod = 1.0f;
            
            for (int k = 0; k < 64; k++) {
                /* Loop-carried dependencies */
                int idx = (i * 2048 + j * 64 + k) % SIZE;
                
                /* Mixed integer/float operations */
                float_arr_a[idx] = float_arr_a[idx] * 1.1f + float_arr_b[idx];
                
                /* Conditional branch inside innermost loop */
                if (int_arr[idx] % 13 == 0) {
                    inner_prod *= float_arr_a[idx];
                    int_arr[idx] = (int_arr[idx] + k) % 255;
                    
                    /* Inline asm with clobbers */
                    asm volatile ("# inner loop barrier" 
                                 ::: "r8", "r9", "r10", "r11", "xmm6", "xmm7");
                } else {
                    int_arr[idx] = (int_arr[idx] * 2 - j) & 0xFF;
                }
                
                /* Another asm barrier */
                asm volatile ("" ::: "memory");
                
                outer_sum += int_arr[idx] * (k + 1);
            }
            
            /* Call to noinline function with SIMD */
            process_with_simd(float_arr_a, float_arr_b, SIZE);
            
            checksum += (int)(inner_prod * 1000) ^ outer_sum;
        }
        
        /* Complex reduction function call */
        checksum ^= complex_reduction(int_arr, SIZE);
    }
    
    /* Large switch statement with non-linear cases */
    int switch_val = checksum % 100;
    
    switch (switch_val) {
        case 0:  int_arr[0] += 100; break;
        case 5:  int_arr[1] *= 2; break;
        case 12: int_arr[2] = int_arr[1] + int_arr[0]; break;
        case 19: int_arr[3] ^= 0x55; break;
        case 23: float_arr_a[0] *= 1.5f; break;
        case 31: int_arr[4] = int_arr[3] << 2; break;
        case 37: int_arr[5] = ~int_arr[4]; break;
        case 42: float_arr_b[1] = float_arr_a[1] * 0.8f; break;
        case 48: int_arr[6] = int_arr[5] | 0xF0; break;
        case 53: int_arr[7] = int_arr[6] & 0x0F; break;
        case 59: float_arr_a[2] = float_arr_b[2] + 1.0f; break;
        case 64: int_arr[8] = int_arr[7] * 3; break;
        case 67: int_arr[9] = int_arr[8] / 2; break;
        case 71: float_arr_b[3] = float_arr_a[3] - 0.5f; break;
        case 76: int_arr[10] = int_arr[9] % 17; break;
        case 82: int_arr[11] = int_arr[10] ^ int_arr[9]; break;
        case 88: float_arr_a[4] = float_arr_b[4] / 2.0f; break;
        case 91: int_arr[12] = int_arr[11] << 1; break;
        case 94: int_arr[13] = int_arr[12] >> 2; break;
        case 97: float_arr_b[5] = float_arr_a[5] * 3.14f; break;
        case 3:  int_arr[14] = -int_arr[13]; break;
        case 8:  int_arr[15] = abs(int_arr[14]); break;
        case 15: float_arr_a[6] = sqrtf(float_arr_b[6]); break;
        case 21: int_arr[16] = int_arr[15] + 999; break;
        default:
            /* Complex default case */
            for (int i = 0; i < 50; i++) {
                int_arr[i % 16] += switch_val * i;
                if (i % 7 == 0) {
                    float_arr_a[i % 8] += switch_val * 0.01f;
                }
            }
            asm volatile ("# switch default barrier" ::: "r12", "r13", "r14", "r15");
            break;
    }
    
    /* Irreducible control flow */
    irreducible_flow(int_arr, 128);
    
    /* Conditional restart using goto */
    if (checksum % 777 == 0) {
        printf("Restarting computation...\n");
        goto restart_point;
    }
    
    /* Final validation */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += int_arr[i];
        final_sum += (int)(float_arr_a[i] * 100);
        final_sum += (int)(float_arr_b[i] * 100);
    }
    
    printf("Computation completed. Checksum: %d, Final sum: %d\n", 
           checksum, final_sum);
    
    return 0;
}
