/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    /* Cold path - unlikely to be taken */
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
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline asm barrier between dependent operations */
        asm volatile ("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "memory");
    }
    
    /* Extract SIMD results */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier */
        asm volatile ("" ::: "rax", "rbx", "rcx", "rdx", "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int size) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size / 4; i++) {
        int outer_acc = arr[i];
        
        for (j = i + 1; j < size / 2; j++) {
            int middle_acc = outer_acc * 2;
            
            for (k = j + 1; k < size; k += 3) {
                /* Loop-carried dependency */
                middle_acc += arr[k] - arr[i];
                
                /* Conditional inside innermost loop */
                if ((k & 7) == 0) {
                    middle_acc >>= 1;
                    asm volatile ("" ::: "r8", "r9", "r10", "r11", "memory");
                } else if ((k & 3) == 0) {
                    middle_acc *= 3;
                }
                
                /* Floating point in integer loop */
                float ftemp = (float)middle_acc;
                ftemp = sqrtf(fabsf(ftemp));
                middle_acc = (int)ftemp;
            }
            
            outer_acc ^= middle_acc;
            
            /* Volatile asm with clobbers */
            asm volatile ("# Complex barrier" ::: 
                         "rax", "rbx", "rcx", "rdx", 
                         "rsi", "rdi", "r8", "r9",
                         "xmm0", "xmm1", "xmm2", "xmm3",
                         "xmm4", "xmm5", "xmm6", "xmm7",
                         "cc", "memory");
        }
        
        result += outer_acc;
    }
    
    return result;
}

/* Large switch handler */
__attribute__((noinline))
static int handle_switch(int value, float* farr, int* iarr) {
    int result = 0;
    
    /* Non-linear case labels */
    switch (value % SWITCH_CASES) {
        case 0:
            result = iarr[0] + iarr[1];
            farr[0] = result * 0.5f;
            break;
        case 1:
            result = iarr[1] * iarr[2];
            /* Fall through */
        case 2:
            result += iarr[3];
            farr[1] = sinf(result);
            break;
        case 5:
            result = iarr[5] | iarr[6];
            break;
        case 7:
            result = iarr[7] & iarr[8];
            /* SIMD in switch case */
            {
                __m128 a = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
                __m128 b = _mm_set1_ps(2.0f);
                __m128 c = _mm_mul_ps(a, b);
                _mm_storeu_ps(farr, c);
            }
            break;
        case 10:
            result = iarr[10] << 2;
            break;
        case 13:
            result = iarr[13] >> 1;
            break;
        case 17:
            result = iarr[17] ^ iarr[18];
            break;
        case 21:
            result = iarr[21] + iarr[22] * iarr[23];
            break;
        case 3:
        case 4:
            result = iarr[3] + iarr[4];
            farr[2] = cosf(result);
            break;
        case 6:
        case 8:
        case 9:
            result = iarr[6] + iarr[8] + iarr[9];
            /* Another asm barrier */
            asm volatile ("" ::: "xmm8", "xmm9", "xmm10", "xmm11", "memory");
            break;
        case 11:
        case 12:
            result = iarr[11] - iarr[12];
            break;
        case 14:
        case 15:
        case 16:
            result = iarr[14] * iarr[15] / (iarr[16] + 1);
            break;
        case 18:
        case 19:
        case 20:
            result = (iarr[18] + iarr[19]) * iarr[20];
            break;
        case 22:
        case 23:
        case 24:
            result = iarr[22] | iarr[23] | iarr[24];
            break;
        default:
            /* Complex default case */
            for (int i = 0; i < 10; i++) {
                result += iarr[i] * (i + 1);
                farr[i] = result * 0.1f;
            }
            error_handler("Default case taken");
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize data arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i * 0.1);
        iarr[i] = i * 3;
    }
    
    int total = 0;
    float fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with complex control flow */
    for (int i = 0; i < 10; i++) {
        if (restart_count > 3) {
            /* Use goto to break structure */
            goto skip_outer;
        }
        
        int outer_val = iarr[i];
        
        for (int j = 0; j < 15; j++) {
            if (j == 7 && i == 5) {
                /* Unlikely condition - trigger cold path */
                error_handler("Mid-loop condition");
            }
            
            int middle_val = outer_val * j;
            
            for (int k = 0; k < 20; k++) {
                /* Data dependencies across loops */
                middle_val += iarr[k] - iarr[j];
                
                /* Conditional with goto */
                if (middle_val > 1000000 && restart_count < 2) {
                    restart_count++;
                    goto restart_point;  /* Irreducible control flow */
                }
                
                /* Mixed operations */
                float ftemp = process_chunk(farr, k * 10, (k + 1) * 10);
                fsum += ftemp;
                
                /* Inline asm between dependent ops */
                asm volatile ("# Inner loop barrier" ::: 
                             "rax", "rbx", "rcx", "rdx",
                             "rsi", "rdi", "memory", "cc");
                
                /* Another level of nesting */
                for (int m = 0; m < 5; m++) {
                    middle_val += (k * m) & 0xF;
                    
                    /* Early continue to outer loop */
                    if ((m & 1) == 0) {
                        continue;
                    }
                    
                    /* Break to middle loop */
                    if (middle_val > 5000) {
                        break;
                    }
                }
            }
            
            /* Call to noinline function with SIMD */
            fsum += process_chunk(farr, j * 20, (j + 1) * 20);
            
            /* Large switch statement */
            int switch_val = middle_val % SWITCH_CASES;
            total += handle_switch(switch_val, &farr[j * 10], &iarr[j * 10]);
        }
        
        /* Complex reduction on entire array */
        total += complex_reduction(iarr, ARRAY_SIZE);
        
        skip_outer:
        /* Empty statement after label */
        asm volatile ("" ::: "memory");
    }
    
    /* Final validation */
    printf("Results: total = %d, fsum = %f, restarts = %d\n", 
           total, fsum, restart_count);
    
    /* Simple checksum */
    int checksum = total + (int)fsum + restart_count;
    if (checksum != 0) {  /* Will never be 0 with this data */
        printf("Test completed successfully (checksum = %d)\n", checksum);
    }
    
    return 0;
}
