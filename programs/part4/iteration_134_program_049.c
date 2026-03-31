/* Test program to exercise GCC scheduler state save/restore cleanup paths */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling decisions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path unlikely to be taken */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) dummy += i;
    (void)msg;
}

__attribute__((noinline))
static float process_vector(float* data, int index) {
    /* Mix scalar and SIMD operations */
    __m128 vec1 = _mm_loadu_ps(&data[index]);
    __m128 vec2 = _mm_set_ps(1.1f, 2.2f, 3.3f, 4.4f);
    __m128 result = _mm_add_ps(vec1, vec2);
    
    /* Inline asm barrier creating scheduling boundary */
    asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    
    float temp[4];
    _mm_storeu_ps(temp, result);
    return temp[0] + temp[1] + temp[2] + temp[3];
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int sum = 0;
    volatile int barrier = 0;  /* Prevent optimization */
    
    /* Nested loops with dependencies */
    for (int i = 0; i < size; i++) {
        if (arr[i] < 0) {
            error_handler("Negative value");
        }
        
        for (int j = 0; j < 8; j++) {
            /* Loop-carried dependency */
            sum = sum * 1103515245 + 12345;
            
            /* Conditional inside inner loop */
            if ((sum & 255) > 128) {
                sum -= arr[i] * j;
            } else {
                sum += arr[i] / (j + 1);
            }
            
            /* Another asm barrier */
            asm volatile ("" ::: "cc", "rax", "rbx");
        }
        
        /* Floating point operation mixing types */
        float fval = (float)arr[i];
        barrier = (int)fval;  /* Use barrier to prevent dead code elimination */
    }
    
    return sum ^ barrier;
}

/* Another noinline function with SIMD */
__attribute__((noinline))
static void simd_transform(float* src, float* dst, int n) {
    for (int i = 0; i < n; i += 4) {
        __m128 v = _mm_loadu_ps(&src[i]);
        __m128 scale = _mm_set1_ps(2.5f);
        __m128 result = _mm_mul_ps(v, scale);
        
        /* Memory clobber forcing scheduler to handle side effects */
        asm volatile ("" ::: "memory");
        
        _mm_storeu_ps(&dst[i], result);
    }
}

int main(void) {
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    float float_data2[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7FFF;
        float_data[i] = (float)(i * 0.1f);
        float_data2[i] = 0.0f;
    }
    
    int total_sum = 0;
    float float_sum = 0.0f;
    
    /* Label for goto jumps (requirement 6) */
    restart_point:
    
    /* Triple nested loops with complex control flow (requirement 1) */
    for (int outer = 0; outer < 4; outer++) {
        if (outer == 2) {
            /* Call noinline function with SIMD */
            simd_transform(float_data, float_data2, ARRAY_SIZE);
        }
        
        for (int mid = 0; mid < 8; mid++) {
            int local_sum = 0;
            
            for (int inner = 0; inner < ARRAY_SIZE/8; inner++) {
                int idx = (outer * 256 + mid * 32 + inner) % ARRAY_SIZE;
                
                /* Data-dependent branching */
                if (int_data[idx] % 3 == 0) {
                    local_sum += int_data[idx] * 2;
                    
                    /* Inline asm with clobbers (requirement 4) */
                    asm volatile ("# Dummy asm" ::: "r12", "r13", "r14", "r15");
                } else if (int_data[idx] % 3 == 1) {
                    local_sum -= int_data[idx];
                    
                    /* Call noinline function */
                    float_sum += process_vector(float_data, idx % (ARRAY_SIZE - 4));
                } else {
                    /* Use goto to create irreducible flow (requirement 6) */
                    if (local_sum > 1000000) {
                        local_sum = 0;
                        goto reset_inner;  /* Jump to label inside loop */
                    }
                    local_sum ^= int_data[idx];
                }
                
                /* Complex floating-point operation mixing with integer */
                float temp = float_data[idx] * 0.5f + (float)local_sum * 0.01f;
                float_data[idx] = temp;
                
                reset_inner:
                /* Empty label target for goto */
                if (inner % 100 == 99) {
                    /* Another asm barrier */
                    asm volatile ("" ::: "memory");
                }
            }
            
            total_sum += complex_reduction(&int_data[mid * 32], 32);
            
            /* Mix with SIMD operation */
            if (mid % 2 == 0) {
                __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
                __m128 v2 = _mm_set_ps(float_sum, float_sum * 0.5f, 
                                      float_sum * 0.25f, float_sum * 0.125f);
                __m128 v3 = _mm_add_ps(v1, v2);
                float temp[4];
                _mm_storeu_ps(temp, v3);
                float_sum += temp[0];
            }
        }
        
        /* Conditional goto to outer scope */
        if (outer == 1 && total_sum < 0) {
            total_sum = -total_sum;
            goto restart_point;  /* Jump back to restart */
        }
    }
    
    /* Large switch statement with non-sequential cases (requirement 5) */
    int switch_val = (total_sum ^ (int)float_sum) % 50;
    
    switch (switch_val) {
        case 0: total_sum += 100; break;
        case 1: total_sum -= 200; break;
        case 3: total_sum *= 2; break;  /* Skip case 2 */
        case 7: total_sum /= 3; break;  /* Non-linear */
        case 4: total_sum ^= 0xABCD; break;
        case 10: total_sum |= 0xFF; break;
        case 15: total_sum &= 0x7F; break;
        case 20: total_sum <<= 2; break;
        case 25: total_sum >>= 1; break;
        case 30: total_sum = ~total_sum; break;
        case 35: total_sum += int_data[0]; break;
        case 40: total_sum -= int_data[1]; break;
        case 45: total_sum *= int_data[2]; break;
        case 5:  /* Complex case with function call */
            total_sum += complex_reduction(int_data, 64);
            break;
        case 12: /* Case with SIMD */
            {
                __m128 v = _mm_set1_ps((float)total_sum);
                float f[4];
                _mm_storeu_ps(f, v);
                total_sum += (int)f[0];
            }
            break;
        case 18: /* Case with asm */
            asm volatile ("# Switch case asm" ::: "rax", "rbx", "rcx", "rdx");
            total_sum ^= 0x12345678;
            break;
        case 22: /* Nested loop in case */
            for (int i = 0; i < 10; i++) {
                total_sum += i * i;
            }
            break;
        case 28: /* Conditional in case */
            if (total_sum > 0) {
                total_sum = -total_sum;
            } else {
                total_sum *= 2;
            }
            break;
        case 33: /* Another function call */
            float_sum += process_vector(float_data, 0);
            break;
        case 38: /* Memory operation */
            memset(&int_data[100], 0, 10 * sizeof(int));
            break;
        case 42: /* Use goto inside switch */
            if (total_sum == 0) goto default_case;
            total_sum += 42;
            break;
        case 48: /* SIMD and scalar mix */
            {
                float f = process_vector(float_data, 100);
                total_sum += (int)f;
            }
            break;
        default_case:
        default: /* Complex default case */
            {
                /* Nested loops in default */
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        total_sum += i * j;
                        if (total_sum > 1000) {
                            total_sum %= 1000;
                        }
                    }
                }
                
                /* Call cold function */
                if (total_sum < 0) {
                    error_handler("Switch default negative");
                }
                
                /* SIMD operation */
                __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
                __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
                __m128 c = _mm_add_ps(a, b);
                float ftmp[4];
                _mm_storeu_ps(ftmp, c);
                float_sum += ftmp[0] + ftmp[1];
            }
            break;
    }
    
    /* Final validation */
    int checksum = total_sum + (int)float_sum;
    
    /* Use checksum to prevent dead code elimination */
    volatile int result = checksum;
    
    printf("Test completed. Checksum: %d\n", checksum);
    
    return 0;
}
