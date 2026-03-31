/* mcf_stress_test.c
 * Stress test for GCC's min-cost flow solver in register allocation
 * Compile with: gcc -O3 -fdump-ira-all -fdump-ira-details -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -fno-move-loop-invariants mcf_stress_test.c -o mcf_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex expression with many intermediate values */
double complex_expression(double a, double b, double c, double d, double e, double f) {
    /* Many intermediate calculations requiring registers */
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = t1 / (t2 + 1.0);
    double t4 = sin(t1) * cos(t2);
    double t5 = exp(t3) * log(fabs(t4) + 1.0);
    double t6 = t5 * t4 / t3;
    double t7 = sqrt(t6 * t6 + t5 * t5);
    double t8 = t7 * tan(t3) + atan(t4);
    double t9 = t8 * t7 / t6 * t5;
    double t10 = t9 + t8 + t7 + t6 + t5 + t4 + t3;
    
    /* Force register pressure with many live values */
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4), 
                       "+r"(t5), "+r"(t6), "+r"(t7), "+r"(t8) : : "memory");
    
    return t10;
}

/* Function with deeply nested loops and complex control flow */
void nested_loop_stress(int *data, int size) {
    int i, j, k, l;
    volatile int temp[10];  /* Prevent optimization */
    
    for (i = 0; i < size / 4; i++) {
        for (j = 0; j < size / 8; j++) {
            /* Early return creates complex CFG */
            if (data[i * j % size] < 0) {
                return;
            }
            
            for (k = 0; k < size / 16; k++) {
                /* Complex expression with many temporaries */
                double val = complex_expression(
                    i * 0.1, j * 0.2, k * 0.3,
                    data[i] * 0.01, data[j] * 0.02, data[k] * 0.03
                );
                
                /* Multiple conditions with different paths */
                if (val > 100.0) {
                    for (l = 0; l < 5; l++) {
                        temp[l] = (int)(val * l);
                        if (temp[l] % 2 == 0) {
                            continue;  /* Creates back edges */
                        }
                        data[(i + j + k + l) % size] += temp[l];
                    }
                } else if (val > 50.0) {
                    break;  /* Early loop exit */
                } else {
                    continue;
                }
                
                /* Inline assembly with fixed register constraints */
                asm volatile (
                    "movl %0, %%eax\n\t"
                    "addl %1, %%eax\n\t"
                    "movl %%eax, %0\n\t"
                    : "+m"(data[i])
                    : "r"(j)
                    : "%eax", "memory"
                );
            }
        }
    }
}

/* Function with large switch statement creating complex CFG */
int switch_complex_cfg(int value) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: {
            double a = sin(value * 0.1);
            double b = cos(value * 0.2);
            result = (int)(a * b * 1000);
            /* Fall through */
        }
        case 1:
            result += value * 2;
            break;
        case 2: {
            volatile int x = value;
            asm volatile("imull %%ecx, %%eax" 
                        : "+a"(result) 
                        : "c"(x) 
                        : "memory");
            /* Fall through */
        }
        case 3:
        case 4:
            result -= value / 3;
            break;
        case 5: {
            float f1 = value * 0.5f;
            float f2 = value * 0.25f;
            float f3 = f1 * f2;
            result += (int)f3;
            /* Fall through */
        }
        case 6:
            result *= 7;
            break;
        case 7:
        case 8:
        case 9:
            result = result >> (value % 4);
            break;
        case 10: {
            long long ll = (long long)value * 1000000LL;
            result = (int)(ll % 1000);
            break;
        }
        case 11:
            result = ~result;
            break;
        case 12:
            result = result ^ value;
            break;
        case 13:
            result = result | 0xFF;
            break;
        case 14:
            result = result & 0x7F;
            break;
        default:
            result = -1;
    }
    
    return result;
}

/* Function with many function calls in loops */
void function_call_stress(double *array, int size) {
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Multiple function calls creating call-clobbered conflicts */
        double x = sin(array[i]);
        double y = cos(array[i * 2 % size]);
        double z = tan(array[i * 3 % size]);
        
        /* Complex expression using all results */
        array[i] = x * y + z * exp(x) - log(fabs(y) + 1.0);
        
        /* Nested loop with more calls */
        for (j = 0; j < 10; j++) {
            double temp = pow(array[i], j + 1);
            array[(i + j) % size] += temp * 0.01;
            
            /* Inline assembly with multiple constraints */
            asm volatile (
                "movsd %1, %%xmm0\n\t"
                "mulsd %2, %%xmm0\n\t"
                "movsd %%xmm0, %0\n\t"
                : "=m"(array[i])
                : "m"(temp), "m"(global_accumulator)
                : "%xmm0", "memory"
            );
        }
        
        /* Switch statement inside loop */
        int switch_result = switch_complex_cfg(i);
        array[i] += switch_result * 0.001;
    }
}

/* Function with mixed data types stressing register classes */
void mixed_type_stress(char *cdata, short *sdata, int *idata, 
                       long *ldata, float *fdata, double *ddata, int size) {
    int i;
    
    for (i = 0; i < size; i++) {
        /* Operations on different types requiring different registers */
        char c = cdata[i];
        short s = sdata[i];
        int i_val = idata[i];
        long l = ldata[i];
        float f = fdata[i];
        double d = ddata[i];
        
        /* Complex conversions and calculations */
        i_val += (int)c * (int)s;
        l += (long)i_val * 1000L;
        f = (float)l * 0.001f + sinf(f);
        d = (double)f * 2.0 + cos(d);
        
        /* Store back with type conversions */
        cdata[i] = (char)(d * 10);
        sdata[i] = (short)(f * 100);
        idata[i] = i_val + (int)d;
        ldata[i] = l + (long)(d * 1000);
        fdata[i] = f + (float)i_val * 0.01f;
        ddata[i] = d * 0.5 + (double)l * 0.0001;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Function with many arguments stressing calling convention */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   double f1, double f2, double f3, double f4) {
    /* Use all arguments in complex ways */
    int sum_i = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double sum_f = f1 + f2 + f3 + f4;
    
    /* Complex calculations with all values live */
    double temp1 = sin(f1) * cos(f2);
    double temp2 = exp(f3) * log(fabs(f4) + 1.0);
    int temp3 = (a1 * a2 + a3 * a4 - a5 * a6) / (a7 + a8 - a9 + a10);
    
    /* Inline assembly using multiple registers */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "addl %%esi, %%eax\n\t"
        "addl %%edi, %%eax\n\t"
        : "+a"(sum_i)
        : "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5)
        : "memory"
    );
    
    return sum_i + (int)(sum_f * 100) + temp3;
}

/* Main test driver */
int main() {
    int i, j;
    unsigned long long checksum = 0;
    
    /* Allocate and initialize test arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_data || !double_data || !char_data || 
        !short_data || !long_data || !float_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random-ish data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (rand() % 1000) * 0.01;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        long_data[i] = rand() % 10000;
        float_data[i] = (rand() % 1000) * 0.01f;
    }
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < 5; j++) {
        for (i = 0; i < ARRAY_SIZE / 100; i++) {
            int_data[i] = switch_complex_cfg(int_data[i]);
        }
    }
    
    /* Memory barrier between test functions */
    asm volatile("" ::: "memory");
    
    /* Run stress tests in sequence */
    printf("Starting register pressure stress tests...\n");
    
    /* Test 1: Nested loops with complex control flow */
    nested_loop_stress(int_data, ARRAY_SIZE);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Test 2: Function call stress */
    function_call_stress(double_data, ARRAY_SIZE);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Test 3: Mixed data types */
    mixed_type_stress(char_data, short_data, int_data, 
                      long_data, float_data, double_data, ARRAY_SIZE);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* Test 4: Many arguments function called in loop */
    for (i = 0; i < ARRAY_SIZE / 10; i++) {
        int result = many_arguments(
            int_data[i], int_data[i+1], int_data[i+2], int_data[i+3], int_data[i+4],
            int_data[i+5], int_data[i+6], int_data[i+7], int_data[i+8], int_data[i+9],
            double_data[i], double_data[i+1], double_data[i+2], double_data[i+3]
        );
        checksum += result;
    }
    
    /* Compute final checksum */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_data[i] + (int)(double_data[i] * 100) + 
                   char_data[i] + short_data[i] + 
                   (int)(float_data[i] * 100);
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    
    return 0;
}
