#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xmmintrin.h>  // SSE intrinsics

#define ARRAY_SIZE 1024
#define SWITCH_CASES 25

/* Requirement 2: noinline and cold attributed functions */
__attribute__((noinline, cold))
static void error_handler(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

__attribute__((noinline))
static float process_vector(float* data, int size) {
    float sum = 0.0f;
    
    /* Requirement 3: Vector intrinsics mixed with scalar ops */
    for (int i = 0; i < size; i += 4) {
        if (i + 4 <= size) {
            __m128 vec = _mm_loadu_ps(&data[i]);
            __m128 squared = _mm_mul_ps(vec, vec);
            
            /* Requirement 4: asm volatile with clobbers */
            asm volatile (
                "movaps %%xmm0, %%xmm1\n\t"
                "shufps $0x1B, %%xmm0, %%xmm0\n\t"
                :
                :
                : "xmm0", "xmm1", "cc"
            );
            
            float temp[4];
            _mm_storeu_ps(temp, squared);
            sum += temp[0] + temp[1] + temp[2] + temp[3];
        } else {
            /* Scalar fallback */
            for (int j = i; j < size; j++) {
                sum += data[j] * data[j];
            }
        }
    }
    return sum;
}

__attribute__((noinline))
static int complex_reduction(int* arr, int n) {
    int result = 0;
    volatile int barrier = 0;  /* Prevent optimization */
    
    /* Requirement 1: Nested loops with dependencies */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                /* Loop-carried dependency */
                result += arr[(i * j + k) % n];
                
                /* Conditional branch inside innermost loop */
                if ((i ^ j ^ k) & 1) {
                    result -= arr[(j * k + i) % n];
                } else {
                    result += arr[(k * i + j) % n] * 2;
                }
                
                /* Mixed FP operations */
                float ftemp = (float)result * 0.5f;
                result = (int)ftemp;
                
                /* Another asm barrier */
                asm volatile ("" ::: "memory", "eax", "ebx", "ecx", "edx");
            }
        }
    }
    return result;
}

/* Requirement 5: Large switch with non-linear cases */
static int process_switch(int value, int* data, float* fdata) {
    int result = 0;
    
    switch (value) {
        case 100: result = data[0] * 2; break;
        case 23:  result = data[1] + data[2]; break;
        case 47:  result = data[3] - data[4]; break;
        case 81:  result = data[5] | data[6]; break;
        case 12:  result = data[7] & data[8]; break;
        case 56:  result = data[9] ^ data[10]; break;
        case 33:  result = data[11] << 2; break;
        case 67:  result = data[12] >> 1; break;
        case 92:  result = ~data[13]; break;
        case 18:  result = data[14] + data[15] * 3; break;
        case 41:  result = data[16] / (data[17] + 1); break;
        case 75:  result = (int)(fdata[0] * 100.0f); break;
        case 29:  result = (int)(fdata[1] + fdata[2]); break;
        case 63:  result = (int)(fdata[3] - fdata[4]); break;
        case 88:  result = (int)(fdata[5] * fdata[6]); break;
        case 14:  result = (int)(fdata[7] / (fdata[8] + 1.0f)); break;
        case 37:  result = complex_reduction(data, 8); break;
        case 71:  result = (int)process_vector(fdata, 16); break;
        case 95:  result = data[18] * data[19] - data[20]; break;
        case 22:  result = data[21] + complex_reduction(&data[22], 4); break;
        case 51:  result = data[23] | (data[24] << 8); break;
        case 84:  result = data[25] ^ ~data[26]; break;
        case 19:  result = (data[27] * data[28]) / (data[29] + 1); break;
        case 44:  result = (int)sqrtf(fdata[9] * fdata[10]); break;
        default:  /* Requirement 5: Complex default case */
            result = complex_reduction(data, 12);
            result += (int)process_vector(fdata, 24);
            result = (result * 1103515245 + 12345) & 0x7fffffff;
            if (result < 0) {
                error_handler("Negative result in default case");
            }
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize arrays */
    int int_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = (i * 1103515245 + 12345) & 0x7fff;
        float_data[i] = (float)int_data[i] * 0.001f;
    }
    
    int total = 0;
    int restart_count = 0;
    
restart_point:  /* Requirement 6: goto label */
    
    /* Requirement 1: Deep nested loops with complex flow */
    for (int outer = 0; outer < 3; outer++) {
        for (int middle = 0; middle < 5; middle++) {
            for (int inner = 0; inner < 7; inner++) {
                /* Complex data dependencies */
                int idx = (outer * 100 + middle * 10 + inner) % ARRAY_SIZE;
                
                /* Mixed operations */
                int temp = int_data[idx];
                float ftemp = float_data[idx];
                
                /* Conditional with unlikely path */
                if (temp < 0) {
                    /* Requirement 2: cold path */
                    error_handler("Unexpected negative value");
                    __attribute__((cold)) int cold_var = 1;
                    (void)cold_var;
                }
                
                /* Loop-carried dependency */
                total += temp;
                total -= (int)(ftemp * 100.0f);
                
                /* SIMD operation in the middle */
                if ((inner & 3) == 0) {
                    __m128 v1 = _mm_set_ps(float_data[idx], 
                                          float_data[(idx+1)%ARRAY_SIZE],
                                          float_data[(idx+2)%ARRAY_SIZE],
                                          float_data[(idx+3)%ARRAY_SIZE]);
                    __m128 v2 = _mm_set1_ps(0.5f);
                    __m128 v3 = _mm_add_ps(v1, v2);
                    
                    float fvals[4];
                    _mm_storeu_ps(fvals, v3);
                    total += (int)(fvals[0] + fvals[1] + fvals[2] + fvals[3]);
                }
                
                /* Requirement 6: goto to create irreducible flow */
                if (restart_count < 2 && total > 1000000) {
                    restart_count++;
                    goto restart_point;  /* Jump back to restart */
                }
                
                /* Another asm barrier with multiple clobbers */
                asm volatile (
                    "cpuid\n\t"
                    : 
                    : "a" (0)
                    : "ebx", "ecx", "edx", "memory", "cc"
                );
            }
            
            /* Break to outer loop sometimes */
            if (middle == 3 && (total & 1)) {
                break;
            }
        }
        
        /* Continue to next iteration or skip */
        if (outer == 1 && restart_count > 0) {
            continue;
        }
        
        /* Requirement 5: Switch in outer loop */
        int switch_val = (total ^ outer) % 100;
        int switch_result = process_switch(switch_val, int_data, float_data);
        total += switch_result;
        
        /* More vector operations */
        float vector_sum = process_vector(float_data, 64);
        total += (int)vector_sum;
    }
    
    /* Final validation */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i += 32) {
        checksum ^= int_data[i];
        checksum += (int)(float_data[i] * 1000.0f);
    }
    
    printf("Result: total=%d, checksum=%d, restarts=%d\n", 
           total, checksum, restart_count);
    
    /* Simple validation */
    if (total != 0 || checksum != 0) {
        printf("Test completed (non-zero results expected for complex computation)\n");
    }
    
    return 0;
}
