/* Test program to exercise GCC HAIFA scheduler state save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  /* SSE intrinsics */

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Helper functions with attributes to affect scheduling decisions */
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
    
    /* SIMD operations */
    __m128 vsum = _mm_setzero_ps();
    for (i = start; i + 3 < end; i += 4) {
        __m128 chunk = _mm_loadu_ps(&data[i]);
        vsum = _mm_add_ps(vsum, chunk);
        
        /* Inline assembly barrier - forces scheduler to partition */
        asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Extract SIMD results */
    float temp[4];
    _mm_storeu_ps(temp, vsum);
    sum = temp[0] + temp[1] + temp[2] + temp[3];
    
    /* Process remaining elements */
    for (; i < end; i++) {
        sum += data[i];
        
        /* Another barrier */
        asm volatile ("" ::: "cc", "memory");
    }
    
    return sum;
}

__attribute__((noinline))
static int integer_reduction(int* arr, int size, int threshold) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loop with dependencies */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (j % 2 == 0) {
                /* Conditional branch inside innermost loop */
                for (k = 0; k < size; k++) {
                    /* Loop-carried dependency */
                    result += arr[(i + j + k) % size];
                    
                    /* Complex condition with floating point */
                    if (k > threshold && (result & 1)) {
                        result *= 2;
                        
                        /* Inline assembly with clobbers */
                        asm volatile (
                            "movl %%eax, %%ecx\n\t"
                            "addl $1, %%ecx"
                            : 
                            : "a"(result)
                            : "ecx", "cc"
                        );
                    }
                    
                    /* Another conditional */
                    if (result > 1000000) {
                        result /= 3;
                    }
                }
            } else {
                /* Alternative path */
                for (k = size - 1; k >= 0; k--) {
                    result -= arr[(i * j + k) % size];
                    
                    /* Memory barrier */
                    asm volatile ("" ::: "memory");
                }
            }
        }
    }
    
    return result;
}

/* Complex switch handler */
__attribute__((noinline))
static int switch_handler(int value, float* farr, int* iarr) {
    int result = 0;
    
    /* Large switch with non-sequential cases */
    switch (value % SWITCH_CASES) {
        case 0:
            result = iarr[0] + iarr[1];
            /* SIMD operation in switch case */
            __m128 v1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
            __m128 v2 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 v3 = _mm_add_ps(v1, v2);
            _mm_storeu_ps(farr, v3);
            break;
            
        case 1:
            result = iarr[1] * 2;
            break;
            
        case 3:  /* Skip case 2 */
            result = iarr[2] - iarr[3];
            break;
            
        case 7:
            result = iarr[4] | iarr[5];
            break;
            
        case 15:
            result = iarr[6] & iarr[7];
            break;
            
        case 4:
            result = iarr[8] ^ iarr[9];
            break;
            
        case 9:
            result = iarr[10] << 2;
            break;
            
        case 18:
            result = iarr[11] >> 1;
            break;
            
        case 22:
            result = abs(iarr[12]);
            break;
            
        case 5:
            result = iarr[13] % 17;
            break;
            
        case 11:
            result = iarr[14] + iarr[15];
            break;
            
        case 19:
            result = iarr[16] * iarr[17];
            break;
            
        case 6:
            result = iarr[18] - iarr[19];
            break;
            
        case 13:
            result = iarr[20] | 0xFF;
            break;
            
        case 21:
            result = iarr[21] & 0xAA;
            break;
            
        case 8:
            result = iarr[22] ^ 0x55;
            break;
            
        case 17:
            result = iarr[23] << 3;
            break;
            
        case 24:
            result = iarr[24] >> 2;
            break;
            
        case 2:
            result = abs(iarr[25]);
            break;
            
        case 12:
            result = iarr[26] % 23;
            break;
            
        case 20:
            result = iarr[27] + 100;
            break;
            
        case 10:
            result = iarr[28] * 3;
            break;
            
        case 16:
            result = iarr[29] - 50;
            break;
            
        case 14:
            result = iarr[30] | iarr[31];
            break;
            
        case 23:
            result = iarr[32] & iarr[33];
            break;
            
        default:  /* Complex default case */
            result = 0;
            for (int i = 0; i < 10; i++) {
                result += iarr[i] * i;
                asm volatile ("" ::: "memory", "eax", "ebx", "ecx");
            }
            
            /* Call cold function in default case */
            if (result < 0) {
                error_handler("Negative result in default case");
            }
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    float farr[ARRAY_SIZE];
    int iarr[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)(i % 100) * 0.1f;
        iarr[i] = i * 3 - 1500;
    }
    
    int total = 0;
    float fsum = 0.0f;
    int restart_count = 0;
    
restart_point:  /* Label for goto */
    
    /* Triple nested loops with complex control flow */
    for (int i = 0; i < 8; i++) {
        if (restart_count > 3) {
            /* Use goto to break out to outer label */
            goto skip_loops;
        }
        
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                /* Loop-carried dependencies */
                total += iarr[(i * 64 + j * 4 + k) % ARRAY_SIZE];
                
                /* Conditional with floating point */
                if ((i + j + k) % 7 == 0) {
                    fsum += farr[(j * 32 + k) % ARRAY_SIZE];
                    
                    /* Inline assembly barrier */
                    asm volatile ("" ::: "memory", "cc");
                }
                
                /* Nested condition */
                if (k % 11 == 0 && total > 10000) {
                    total /= 2;
                    
                    /* Call noinline function */
                    fsum += process_chunk(farr, k, k + 16);
                }
                
                /* Another conditional that might trigger goto */
                if (total < -1000000 && restart_count < 5) {
                    restart_count++;
                    goto restart_point;  /* Irreducible control flow */
                }
            }
            
            /* Break to outer loop */
            if (j % 5 == 0 && total > 50000) {
                break;
            }
        }
        
        /* Continue to next iteration */
        if (i % 3 == 0) {
            continue;
        }
        
        /* Call integer reduction */
        total += integer_reduction(iarr, 16, 8);
    }
    
skip_loops:
    
    /* Execute large switch statement */
    int switch_result = 0;
    for (int i = 0; i < 100; i++) {
        switch_result += switch_handler(total + i, farr, iarr);
        
        /* Memory barrier between iterations */
        asm volatile ("" ::: "memory");
    }
    
    total += switch_result;
    
    /* More complex control flow with goto */
    if (total % 2 == 0) {
        goto even_path;
    } else {
        goto odd_path;
    }
    
even_path:
    total *= 2;
    goto merge_point;
    
odd_path:
    total = (total * 3) / 2;
    /* Fall through */
    
merge_point:
    
    /* Final validation */
    int checksum = total + (int)fsum;
    
    /* Use result to prevent dead code elimination */
    if (checksum != 0) {
        printf("Computed checksum: %d (fsum: %f)\n", checksum, fsum);
        printf("Test completed - scheduler state cleanup should be triggered\n");
    } else {
        error_handler("Unexpected zero checksum");
    }
    
    return 0;
}
