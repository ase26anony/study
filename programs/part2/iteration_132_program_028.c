/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_accumulator = 0.0;

/* Complex data structure to stress register allocation */
typedef struct {
    int data[8];
    double weights[4];
    char metadata[16];
    short indices[12];
    float partials[6];
} ComplexData;

/* Function 1: Deeply nested loops with many live ranges */
__attribute__((noinline))
double stress_loop_nesting(double* array, int size) {
    double sum = 0.0;
    double temp1, temp2, temp3, temp4, temp5;
    int i, j, k, l;
    
    /* Declare variables at function scope but use in nested blocks */
    volatile double live_var1 = 0.0;
    volatile double live_var2 = 0.0;
    volatile double live_var3 = 0.0;
    
    for (i = 0; i < size; i += 8) {
        temp1 = array[i];
        temp2 = array[i + 1];
        
        for (j = 0; j < 16; j++) {
            temp3 = temp1 * j + temp2;
            
            for (k = 0; k < 8; k++) {
                temp4 = temp3 * k;
                
                for (l = 0; l < 4; l++) {
                    temp5 = temp4 / (l + 1);
                    sum += temp5;
                    
                    /* Keep variables alive across loop iterations */
                    live_var1 = temp1;
                    live_var2 = temp2;
                    live_var3 = temp3;
                }
                
                /* Complex expression with many intermediates */
                sum += (temp1 * temp2 - temp3 * temp4 + temp5) / 
                       (1.0 + temp1 * temp1 + temp2 * temp2);
            }
            
            /* Early continue creates complex CFG */
            if (j % 3 == 0) continue;
            if (j % 5 == 0) break;
            
            /* Nested if-else chain */
            if (temp3 > 100.0) {
                sum *= 1.1;
            } else if (temp3 > 50.0) {
                sum *= 1.05;
            } else if (temp3 > 25.0) {
                sum *= 1.02;
            } else {
                sum *= 0.99;
            }
        }
        
        /* Use the live variables to prevent dead store elimination */
        sum += live_var1 + live_var2 + live_var3;
    }
    
    return sum;
}

/* Function 2: Switch statement with many cases and fall-through */
__attribute__((noinline))
int stress_switch(int value, double* array, int size) {
    int result = 0;
    double acc = 0.0;
    int i;
    
    switch (value % 20) {
        case 0:
            acc = array[0];
            /* Fall through */
        case 1:
            acc += array[1];
            result += (int)acc;
            break;
        case 2:
            acc = array[2] * 2;
            /* Fall through */
        case 3:
            acc += array[3] * 3;
            /* Fall through */
        case 4:
            acc += array[4] * 4;
            result += (int)acc;
            break;
        case 5:
            for (i = 0; i < size; i += 2) {
                acc += array[i] * array[i + 1];
            }
            result = (int)acc;
            break;
        case 6:
        case 7:
        case 8:
            acc = array[value % size] * 10;
            result = (int)(acc * 1.5);
            break;
        case 9:
            result = stress_loop_nesting(array, size / 2);
            break;
        case 10:
            /* Complex expression with many temporaries */
            acc = (array[0] * array[1] + array[2] * array[3] - 
                   array[4] * array[5] / array[6]) * 1.618;
            result = (int)acc;
            break;
        case 11:
        case 12:
        case 13:
        case 14:
            /* Multiple cases sharing code */
            acc = 0.0;
            for (i = 0; i < 8; i++) {
                acc += array[(value + i) % size] * i;
            }
            result = (int)acc;
            break;
        case 15:
            result = size * 2;
            break;
        case 16:
            result = size / 2;
            break;
        case 17:
            result = size % 17;
            break;
        case 18:
            result = -size;
            break;
        case 19:
            result = 0;
            break;
        default:
            result = -1;
    }
    
    /* Multiple returns create complex CFG */
    if (result < 0) return -result;
    if (result > 1000) return result / 2;
    return result * 3;
}

/* Function 3: Inline assembly with register constraints */
__attribute__((noinline))
double stress_asm_constraints(double a, double b, double c, double d,
                              double e, double f, double g, double h) {
    double result1, result2, result3, result4;
    
    /* Force specific register allocation with inline asm */
    asm volatile (
        /* Use fixed registers for input/output */
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (result1)
        : "m" (a), "m" (b)
        : "xmm0", "xmm1", "memory"
    );
    
    asm volatile (
        "movsd %1, %%xmm2\n\t"
        "movsd %2, %%xmm3\n\t"
        "mulsd %%xmm3, %%xmm2\n\t"
        "movsd %%xmm2, %0\n\t"
        : "=m" (result2)
        : "m" (c), "m" (d)
        : "xmm2", "xmm3", "memory"
    );
    
    /* Compete for the same registers */
    asm volatile (
        "movsd %1, %%xmm0\n\t"  /* Reuse xmm0 */
        "movsd %2, %%xmm1\n\t"  /* Reuse xmm1 */
        "subsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (result3)
        : "m" (e), "m" (f)
        : "xmm0", "xmm1", "memory"
    );
    
    asm volatile (
        "movsd %1, %%xmm4\n\t"
        "movsd %2, %%xmm5\n\t"
        "divsd %%xmm5, %%xmm4\n\t"
        "movsd %%xmm4, %0\n\t"
        : "=m" (result4)
        : "m" (g), "m" (h)
        : "xmm4", "xmm5", "memory"
    );
    
    /* Force register spilling with many live values */
    volatile double temp1 = result1;
    volatile double temp2 = result2;
    volatile double temp3 = result3;
    volatile double temp4 = result4;
    
    return temp1 + temp2 - temp3 * temp4;
}

/* Function 4: Mixed data types and many function arguments */
__attribute__((noinline))
ComplexData stress_mixed_types(char c, short s, int i, long l,
                               float f, double d, int* ptr, ComplexData* data) {
    ComplexData result;
    int j;
    
    /* Initialize with mixed operations */
    result.data[0] = (int)c + s;
    result.data[1] = i * 2;
    result.data[2] = (int)(l % 1000);
    result.data[3] = *ptr + i;
    
    /* Force different register classes */
    result.weights[0] = (double)f * 1.5;
    result.weights[1] = d / 2.0;
    result.weights[2] = result.weights[0] + result.weights[1];
    result.weights[3] = result.weights[0] * result.weights[1];
    
    /* Character operations */
    for (j = 0; j < 16; j++) {
        result.metadata[j] = c + (char)j;
    }
    
    /* Short operations */
    for (j = 0; j < 12; j++) {
        result.indices[j] = s + (short)j;
    }
    
    /* Float operations */
    for (j = 0; j < 6; j++) {
        result.partials[j] = f * (float)j;
    }
    
    /* Complex address calculations */
    for (j = 0; j < 4; j++) {
        result.data[4 + j] = data->data[j] + 
                            (int)(data->weights[j % 2] * 100.0) +
                            data->indices[j * 2] +
                            (int)data->partials[j % 3];
    }
    
    return result;
}

/* Function 5: Vector operations using multiple SIMD registers */
__attribute__((noinline))
void stress_simd_operations(double* input, double* output, int size) {
    int i;
    double temp[8];
    
    for (i = 0; i < size - 7; i += 8) {
        /* Multiple parallel computations to use many SIMD registers */
        temp[0] = input[i] * 1.1;
        temp[1] = input[i + 1] * 1.2;
        temp[2] = input[i + 2] * 1.3;
        temp[3] = input[i + 3] * 1.4;
        temp[4] = input[i + 4] * 1.5;
        temp[5] = input[i + 5] * 1.6;
        temp[6] = input[i + 6] * 1.7;
        temp[7] = input[i + 7] * 1.8;
        
        /* Cross-element dependencies */
        output[i] = temp[0] + temp[1];
        output[i + 1] = temp[1] + temp[2];
        output[i + 2] = temp[2] + temp[3];
        output[i + 3] = temp[3] + temp[4];
        output[i + 4] = temp[4] + temp[5];
        output[i + 5] = temp[5] + temp[6];
        output[i + 6] = temp[6] + temp[7];
        output[i + 7] = temp[7] + temp[0];
        
        /* Additional computations to increase register pressure */
        output[i] += sin(temp[0]) * cos(temp[1]);
        output[i + 1] += sin(temp[1]) * cos(temp[2]);
        output[i + 2] += sin(temp[2]) * cos(temp[3]);
        output[i + 3] += sin(temp[3]) * cos(temp[4]);
        output[i + 4] += sin(temp[4]) * cos(temp[5]);
        output[i + 5] += sin(temp[5]) * cos(temp[6]);
        output[i + 6] += sin(temp[6]) * cos(temp[7]);
        output[i + 7] += sin(temp[7]) * cos(temp[0]);
    }
}

/* Function 6: Irreducible control flow with computed goto */
__attribute__((noinline))
double stress_computed_goto(int iterations) {
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    double result = 0.0;
    int i = 0;
    
    /* Create irreducible control flow */
    if (iterations % 2 == 0) goto *labels[0];
    else goto *labels[5];
    
label0:
    result += 1.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 3) % 10];
    
label1:
    result += 2.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 5) % 10];
    
label2:
    result += 3.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 7) % 10];
    
label3:
    result += 4.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 11) % 10];
    
label4:
    result += 5.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 13) % 10];
    
label5:
    result -= 1.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 17) % 10];
    
label6:
    result -= 2.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 19) % 10];
    
label7:
    result -= 3.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 23) % 10];
    
label8:
    result -= 4.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 29) % 10];
    
label9:
    result -= 5.0;
    if (++i >= iterations) goto end;
    goto *labels[(i * 31) % 10];
    
end:
    return result;
}

/* Main function that orchestrates all stress tests */
int main() {
    double* array1 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* array2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    ComplexData* complex_array = (ComplexData*)malloc(1000 * sizeof(ComplexData));
    
    int i, j;
    double total_result = 0.0;
    int int_result = 0;
    
    /* Initialize with random-ish data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)rand() / RAND_MAX * 100.0;
        array2[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 8; j++) {
            complex_array[i].data[j] = rand() % 1000;
        }
        for (j = 0; j < 4; j++) {
            complex_array[i].weights[j] = (double)rand() / RAND_MAX;
        }
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up phase for profile feedback */
    for (i = 0; i < WARMUP_ITERATIONS; i++) {
        double warmup_result = stress_loop_nesting(array1, 1000);
        global_accumulator += warmup_result;
        
        /* Memory barrier to prevent optimization across iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Main stress test iterations */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Deep loop nesting */
        total_result += stress_loop_nesting(array1, ARRAY_SIZE / 2);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch statements */
        int_result += stress_switch(i, array1, ARRAY_SIZE);
        
        /* Test 3: Inline assembly with register constraints */
        total_result += stress_asm_constraints(
            array1[i % ARRAY_SIZE],
            array2[(i + 1) % ARRAY_SIZE],
            array1[(i + 2) % ARRAY_SIZE],
            array2[(i + 3) % ARRAY_SIZE],
            array1[(i + 4) % ARRAY_SIZE],
            array2[(i + 5) % ARRAY_SIZE],
            array1[(i + 6) % ARRAY_SIZE],
            array2[(i + 7) % ARRAY_SIZE]
        );
        
        /* Test 4: Mixed data types */
        int idx = i % 1000;
        ComplexData data_result = stress_mixed_types(
            (char)(i % 256),
            (short)(i % 1000),
            i * 2,
            (long)i * 1000,
            (float)array1[idx],
            array2[idx],
            &complex_array[idx].data[0],
            &complex_array[(idx + 1) % 1000]
        );
        
        /* Use result to prevent optimization */
        total_result += data_result.weights[0] + data_result.weights[1];
        
        /* Test 5: SIMD operations */
        stress_simd_operations(array1, array2, ARRAY_SIZE);
        total_result += array2[i % ARRAY_SIZE];
        
        /* Test 6: Computed goto for irreducible CFG */
        total_result += stress_computed_goto(50 + (i % 50));
        
        /* Progress indicator */
        if (i % 10 == 0) {
            printf("Iteration %d/%d, current result: %f\n", 
                   i, ITERATIONS, total_result);
        }
    }
    
    /* Final computation and verification */
    double checksum = total_result + int_result + global_accumulator;
    
    /* Add contributions from all arrays */
    for (i = 0; i < ARRAY_SIZE; i += 100) {
        checksum += array1[i] + array2[i];
    }
    
    printf("\nFinal checksum: %.15e\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(complex_array);
    
    return 0;
}
