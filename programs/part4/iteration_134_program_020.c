/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* __attribute__((noinline)) helper functions */
__attribute__((noinline))
static float compute_sum(float* arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        /* Artificial dependency chain */
        arr[i] = sum * 0.5f;
    }
    return sum;
}

__attribute__((noinline))
__attribute__((cold))
static void error_handler(const char* msg) {
    /* Cold path - rarely executed */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += i * 2;
    }
    (void)dummy;
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

__attribute__((noinline))
static void simd_operations(float* a, float* b, float* c, int n) {
    /* Mixed scalar and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        if (i + 4 <= n) {
            __m128 va = _mm_loadu_ps(&a[i]);
            __m128 vb = _mm_loadu_ps(&b[i]);
            __m128 vc = _mm_add_ps(va, vb);
            _mm_storeu_ps(&c[i], vc);
            
            /* Inline asm with clobbers - creates scheduling barrier */
            asm volatile(
                "movl $0, %%eax\n\t"
                "cpuid\n\t"
                : 
                : 
                : "%eax", "%ebx", "%ecx", "%edx", "memory"
            );
        } else {
            /* Scalar fallback */
            for (int j = i; j < n; j++) {
                c[j] = a[j] + b[j];
            }
        }
    }
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int sum = 0;
    int prod = 1;
    volatile int temp;  /* Volatile to prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Loop-carried dependencies */
        sum += arr[i];
        prod *= (arr[i] & 0xFF) + 1;
        
        /* Conditional with data dependency */
        if (arr[i] > 0) {
            sum -= (prod % 7);
        } else {
            sum += (prod % 5);
        }
        
        /* Another asm barrier */
        asm volatile("" : "=r"(temp) : "0"(sum) : "cc", "memory");
        
        /* Complex floating point operation */
        float fval = (float)sum / (i + 1);
        arr[i] = (int)(fval * 100.0f);
    }
    
    return sum ^ prod;
}

int main(void) {
    /* Initialize arrays */
    float fa[ARRAY_SIZE];
    float fb[ARRAY_SIZE];
    float fc[ARRAY_SIZE];
    int ia[ARRAY_SIZE];
    int ib[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)i * 0.1f;
        fb[i] = (float)(ARRAY_SIZE - i) * 0.2f;
        ia[i] = i * 3 - ARRAY_SIZE/2;
        ib[i] = (i << 2) | 1;
    }
    
    int result = 0;
    int restart_count = 0;
    
restart_point:  /* Label for goto (requirement 6) */
    if (restart_count++ > 3) {
        error_handler("Too many restarts");
        return 1;
    }
    
    /* Nested loops with complex control flow (requirement 1) */
    for (int outer = 0; outer < 5; outer++) {
        float outer_sum = 0.0f;
        
        for (int mid = 0; mid < 10; mid++) {
            int mid_acc = 0;
            
            for (int inner = 0; inner < ARRAY_SIZE/8; inner++) {
                /* Complex data dependencies */
                int idx = (outer * 200 + mid * 20 + inner) % ARRAY_SIZE;
                
                /* Mixed integer/float operations */
                float f1 = fa[idx];
                float f2 = fb[idx];
                float f3 = f1 * f2 - (float)inner;
                
                /* Conditional branch inside innermost loop */
                if (f3 > 100.0f) {
                    fa[idx] = f3 * 0.9f;
                    mid_acc += (int)f3;
                } else if (f3 < -50.0f) {
                    fb[idx] = f3 * 1.1f;
                    mid_acc -= (int)f3;
                } else {
                    /* Call noinline function */
                    fa[idx] = compute_sum(&fa[idx], 1);
                }
                
                /* Loop-carried dependency */
                ib[idx] = (ib[idx] + mid_acc) & 0xFFFF;
                
                /* Inline asm with clobbers (requirement 4) */
                asm volatile(
                    "mov %%eax, %%ecx\n\t"
                    "ror $7, %%ecx\n\t"
                    : 
                    : "a"(inner)
                    : "%ecx", "cc"
                );
                
                /* Occasionally trigger goto */
                if (inner % 77 == 0 && restart_count < 2) {
                    goto skip_rest;  /* Jump within nested loops */
                }
            skip_rest:
                continue;
            }
            
            /* Call SIMD function (requirement 3) */
            simd_operations(fa, fb, fc, ARRAY_SIZE);
            
            /* Another asm barrier */
            asm volatile("mfence" ::: "memory");
        }
        
        /* Compute reduction on integer array */
        result ^= complex_reduction(ia, ARRAY_SIZE);
    }
    
    /* Large switch statement with non-sequential cases (requirement 5) */
    int switch_val = (result & 0xFF) % SWITCH_CASES;
    
    switch (switch_val) {
        case 0:  result += ia[0] * 2; break;
        case 1:  result -= ib[1] / 3; break;
        case 3:  result |= 0x00FF00; break;  /* Skip case 2 */
        case 7:  result ^= 0x12345678; break;  /* Non-sequential */
        case 2:  result &= 0x55555555; break;
        case 15: result = result << 4; break;
        case 8:  result = result >> 2; break;
        case 4:  result += compute_sum(fa, 10); break;
        case 11: result -= complex_reduction(ib, 50); break;
        case 19: result *= 3; break;
        case 6:  result /= 2; break;
        case 23: result = ~result; break;
        case 14: result |= ib[14]; break;
        case 9:  result &= ia[9]; break;
        case 17: result ^= ib[17]; break;
        case 5:  result += 999; break;
        case 21: result -= 777; break;
        case 12: result = (result << 1) | (result >> 31); break;
        case 18: result = (result & 0xAA) | ((result & 0x55) << 1); break;
        case 22: result = result * result; break;
        case 10: result = result % 1000; break;
        case 13: result = abs(result); break;
        case 16: result = -result; break;
        case 20: result = result / 7; break;
        case 24: result = result * 11; break;
        default:  /* Complex default case */
            for (int i = 0; i < 100; i++) {
                result += (ia[i % ARRAY_SIZE] * ib[(i + 7) % ARRAY_SIZE]) / (i + 1);
                if (result < 0) {
                    result = -result;
                    /* Call cold function */
                    error_handler("Negative result in default case");
                }
            }
            break;
    }
    
    /* Conditional goto restart (requirement 6) */
    if ((result & 0x1) && restart_count == 1) {
        /* Modify arrays to change computation path */
        for (int i = 0; i < ARRAY_SIZE; i += 3) {
            ia[i] ^= 0xAAAAAAAA;
            fb[i] *= 1.5f;
        }
        goto restart_point;
    }
    
    /* Final validation */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += fa[i] + fb[i] + fc[i] + (float)ia[i] + (float)ib[i];
    }
    
    /* Use result to prevent dead code elimination */
    result += (int)final_sum;
    
    printf("Result: %d (checksum: %f)\n", result, final_sum);
    
    /* Trigger one more scheduling-intensive operation */
    volatile int final_check = 0;
    for (int i = 0; i < 1000; i++) {
        final_check += (result >> (i % 32)) & 1;
        /* Memory barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return (result ^ final_check) == 0 ? 0 : 1;
}
