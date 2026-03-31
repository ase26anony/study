#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  // SSE intrinsics
#include <emmintrin.h>  // SSE2 intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: Noinline and cold attributed functions */
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
static void simd_operation(float* a, float* b, float* c, int n) {
    /* Requirement 3: Vector intrinsics and SIMD operations */
    for (int i = 0; i < n; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_storeu_ps(&c[i], vc);
    }
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += arr[i];
        /* Requirement 4: asm volatile with clobbers */
        asm volatile (
            "movl %%eax, %%ebx\n\t"
            "addl $1, %%ebx\n\t"
            : /* no outputs */
            : /* no inputs */
            : "%eax", "%ebx", "cc", "memory"
        );
    }
    return result;
}

static int __attribute__((noinline)) validate_results(float fsum, int isum) {
    return (fsum > 0.0f && isum > 0);
}

int main(void) {
    /* Initialize arrays */
    float farr1[ARRAY_SIZE];
    float farr2[ARRAY_SIZE];
    float farr3[ARRAY_SIZE];
    int iarr1[ARRAY_SIZE];
    int iarr2[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr1[i] = (float)(i % 100) * 0.1f;
        farr2[i] = (float)((i + 1) % 100) * 0.2f;
        farr3[i] = 0.0f;
        iarr1[i] = i;
        iarr2[i] = ARRAY_SIZE - i;
    }
    
    float total_fsum = 0.0f;
    int total_isum = 0;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label for restart mechanism */
    
    /* Requirement 1: Nested loops with complex control flow and data dependencies */
    for (int i = 0; i < 10; i++) {
        float outer_sum = 0.0f;
        
        for (int j = 0; j < 20; j++) {
            int inner_sum = 0;
            
            for (int k = 0; k < 30; k++) {
                /* Loop-carried dependency */
                inner_sum += iarr1[(i * j + k) % ARRAY_SIZE];
                
                /* Conditional branch inside innermost loop */
                if ((i + j + k) % 7 == 0) {
                    outer_sum += farr1[(i * j + k) % ARRAY_SIZE];
                    
                    /* Another asm barrier */
                    asm volatile (
                        "movq %%rcx, %%rdx\n\t"
                        "incq %%rdx\n\t"
                        : /* no outputs */
                        : /* no inputs */
                        : "%rcx", "%rdx", "cc"
                    );
                } else if ((i + j + k) % 13 == 0) {
                    /* Cold path - rarely taken */
                    if (restart_count > 3) {
                        error_handler("Too many restarts");
                    }
                }
                
                /* Mixed integer and floating-point operations */
                farr2[(i * j + k) % ARRAY_SIZE] = 
                    farr1[(i * j + k) % ARRAY_SIZE] * 2.0f + (float)inner_sum;
            }
            
            total_isum += inner_sum;
            
            /* Call noinline function with SIMD operations */
            if (j % 3 == 0) {
                simd_operation(farr1, farr2, farr3, 256);
            }
        }
        
        total_fsum += outer_sum;
        
        /* Call noinline function for chunk processing */
        float chunk_sum = process_chunk(farr3, i * 100, (i + 1) * 100);
        total_fsum += chunk_sum;
    }
    
    /* Compute a value for the switch statement */
    int switch_val = (total_isum % SWITCH_CASES) * 3 + 1;
    
    /* Requirement 5: Large switch statement with dense, non-linear cases */
    switch (switch_val) {
        case 1:
            total_isum += iarr2[0] * 2;
            break;
        case 4:
            total_isum += complex_reduction(iarr1, 128);
            break;
        case 7:
            total_fsum *= 1.1f;
            break;
        case 10:
            for (int i = 0; i < 50; i++) {
                iarr1[i] = iarr2[ARRAY_SIZE - i - 1];
            }
            break;
        case 13:
            total_isum -= 1000;
            break;
        case 16:
            simd_operation(farr2, farr3, farr1, 512);
            break;
        case 19:
            total_fsum = process_chunk(farr1, 0, ARRAY_SIZE);
            break;
        case 22:
            total_isum = complex_reduction(iarr2, ARRAY_SIZE);
            break;
        case 25:
            total_fsum += total_fsum * 0.5f;
            break;
        case 28:
            for (int i = 0; i < ARRAY_SIZE; i += 8) {
                iarr1[i] = iarr1[i] ^ iarr2[i];
            }
            break;
        case 31:
            total_isum = total_isum >> 2;
            break;
        case 34:
            total_fsum = total_fsum / 3.14159f;
            break;
        case 37:
            /* More asm with clobbers */
            asm volatile (
                "movl $0, %%eax\n\t"
                "cpuid\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%eax", "%ebx", "%ecx", "%edx", "cc"
            );
            break;
        case 40:
            for (int i = 0; i < 100; i++) {
                farr3[i] = farr1[i] + farr2[ARRAY_SIZE - i - 1];
            }
            break;
        case 43:
            total_isum += switch_val * 100;
            break;
        case 46:
            total_fsum -= 500.0f;
            break;
        case 49:
            /* Call error handler (cold function) */
            if (total_fsum < 0) {
                error_handler("Negative sum");
            }
            break;
        case 52:
            for (int i = 0; i < ARRAY_SIZE; i++) {
                iarr1[i] = (iarr1[i] + iarr2[i]) % 1000;
            }
            break;
        case 55:
            total_isum = total_isum | 0xFF;
            break;
        case 58:
            total_fsum = total_fsum * total_fsum;
            break;
        case 61:
            /* Another SIMD operation */
            __m128 vsum = _mm_setzero_ps();
            for (int i = 0; i < ARRAY_SIZE; i += 4) {
                __m128 v = _mm_loadu_ps(&farr1[i]);
                vsum = _mm_add_ps(vsum, v);
            }
            float temp[4];
            _mm_storeu_ps(temp, vsum);
            total_fsum += temp[0] + temp[1] + temp[2] + temp[3];
            break;
        case 64:
            total_isum = ~total_isum;
            break;
        case 67:
            for (int i = 0; i < ARRAY_SIZE; i++) {
                farr2[i] = farr1[i] * farr3[i];
            }
            break;
        case 70:
            total_isum = abs(total_isum);
            break;
        case 73:
            total_fsum = sqrtf(fabsf(total_fsum));
            break;
        default:  /* Complex default case */
            for (int i = 0; i < ARRAY_SIZE; i++) {
                iarr1[i] = (iarr1[i] * 3 + iarr2[i]) / 2;
                farr1[i] = (farr1[i] + farr2[i] + farr3[i]) / 3.0f;
                if (i % 11 == 0) {
                    asm volatile (
                        "movq %%r8, %%r9\n\t"
                        "decq %%r9\n\t"
                        : /* no outputs */
                        : /* no inputs */
                        : "%r8", "%r9", "cc"
                    );
                }
            }
            total_isum += complex_reduction(iarr1, ARRAY_SIZE);
            total_fsum += process_chunk(farr1, 0, ARRAY_SIZE);
            break;
    }
    
    /* Requirement 6: goto to implement restart mechanism */
    if (restart_count < 2 && (total_isum % 1000) < 100) {
        restart_count++;
        total_fsum *= 0.9f;
        total_isum /= 2;
        goto restart_point;
    }
    
    /* Final validation */
    if (validate_results(total_fsum, total_isum)) {
        printf("Test completed successfully. fsum=%f, isum=%d, restarts=%d\n",
               total_fsum, total_isum, restart_count);
        return 0;
    } else {
        printf("Validation failed. fsum=%f, isum=%d\n", total_fsum, total_isum);
        return 1;
    }
}
