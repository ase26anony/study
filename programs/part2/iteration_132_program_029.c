/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define NUM_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile double global_sum = 0.0;

/* Complex data structure to stress register allocation */
typedef struct {
    int data[8];
    double weights[4];
    char metadata[16];
    short indices[12];
    long counters[6];
    float temps[10];
} ComplexStruct;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *arr, int size) {
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = arr[i] * 2;
        sum1 += temp1;
        acc1 += sqrt(fabs(temp1));
        
        for (j = i; j < size / 8; j++) {
            temp2 = arr[j] + temp1;
            sum2 += temp2;
            acc2 += log(fabs(temp2) + 1.0);
            
            for (k = j; k < size / 16; k++) {
                temp3 = arr[k] ^ temp2;
                sum3 += temp3;
                acc3 += sin(temp3) * cos(temp3);
                
                for (l = k; l < size / 32; l++) {
                    temp4 = arr[l] | temp3;
                    sum4 += temp4;
                    acc4 += exp(fabs(temp4 % 100));
                    
                    /* Complex expression with many intermediates */
                    temp5 = (temp1 * temp2) + (temp3 << 2) - (temp4 / 3);
                    temp6 = (temp2 ^ temp3) | (temp4 & temp1);
                    temp7 = (temp5 * 7) % 13 + temp6;
                    temp8 = (temp7 << 1) ^ (temp5 >> 1);
                    
                    /* Force register pressure with many operations */
                    arr[l] = (temp5 + temp6 + temp7 + temp8) % 256;
                }
            }
        }
    }
    
    /* Use all computed values to prevent dead code elimination */
    global_sum += sum1 + sum2 + sum3 + sum4 + acc1 + acc2 + acc3 + acc4;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *arr, int size) {
    int result = 0;
    int i, state = 0;
    
    for (i = 0; i < size; i++) {
        /* Large switch with fall-through cases */
        switch (arr[i] % NUM_CASES) {
            case 0:
                result += arr[i] * 2;
                if (result > 1000000) return result; /* Early return */
                /* Fall through */
            case 1:
                result -= arr[i] / 3;
                break;
            case 2:
                result ^= arr[i];
                if (i % 7 == 0) continue; /* Skip to next iteration */
                /* Fall through */
            case 3:
                result |= arr[i] << 2;
                break;
            case 4:
                result &= arr[i];
                if (result < 0) goto cleanup;
                /* Fall through */
            case 5:
                result = result * 3 + arr[i];
                break;
            case 6:
                result = result / 2 - arr[i];
                /* Nested if-else chain */
                if (result > 0) {
                    if (arr[i] % 3 == 0) {
                        result += 100;
                    } else if (arr[i] % 3 == 1) {
                        result -= 50;
                    } else {
                        result *= 2;
                    }
                } else {
                    if (arr[i] % 2 == 0) {
                        result = -result;
                    } else {
                        result = 0;
                    }
                }
                break;
            case 7:
                result = ~result;
                break;
            case 8:
                result = result >> (arr[i] % 8);
                break;
            case 9:
                result = result << (arr[i] % 8);
                break;
            case 10:
                result = (result + arr[i]) % 1337;
                break;
            case 11:
                result = result ^ ~arr[i];
                if (i % 13 == 0) break;
                /* Fall through */
            case 12:
                result = result | 0xFF00;
                break;
            case 13:
                result = result & 0x00FF;
                break;
            case 14:
                result = result + (arr[i] * arr[i-1]) / 2;
                break;
            default:
                result = -1;
        }
        
        /* Loop with break at different levels */
        for (int j = 0; j < 5; j++) {
            if (arr[i] % (j + 2) == 0) {
                result += j;
                if (j == 3) break;
                for (int k = 0; k < 3; k++) {
                    if ((result + k) % 7 == 0) {
                        result -= k;
                        continue;
                    }
                }
            }
        }
        
        state = (state + 1) % 4;
    }
    
cleanup:
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_constraints(ComplexStruct *cs, int count) {
    int i;
    long long acc = 0;
    double fp_acc = 0.0;
    
    for (i = 0; i < count; i++) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "mov %[val1], %%rax\n\t"
            "imul %[val2], %%rax\n\t"
            "add %%rax, %[acc]\n\t"
            : [acc] "+r" (acc)
            : [val1] "r" (cs[i].data[0]),
              [val2] "r" (cs[i].data[1])
            : "rax", "cc", "memory"
        );
        
        asm volatile (
            "mov %[val3], %%rbx\n\t"
            "xor %[val4], %%rbx\n\t"
            "or %%rbx, %[acc]\n\t"
            : [acc] "+r" (acc)
            : [val3] "r" (cs[i].data[2]),
              [val4] "r" (cs[i].data[3])
            : "rbx", "cc"
        );
        
        /* Floating point with fixed registers */
        double x = cs[i].weights[0];
        double y = cs[i].weights[1];
        asm volatile (
            "movsd %[x], %%xmm0\n\t"
            "mulsd %[y], %%xmm0\n\t"
            "addsd %%xmm0, %[fp_acc]\n\t"
            : [fp_acc] "+t" (fp_acc)
            : [x] "fm" (x),
              [y] "fm" (y)
            : "xmm0"
        );
        
        /* More register pressure */
        asm volatile (
            "mov %[idx], %%rcx\n\t"
            "shl $2, %%rcx\n\t"
            "add %%rcx, %[acc]\n\t"
            : [acc] "+r" (acc)
            : [idx] "r" (cs[i].indices[0])
            : "rcx"
        );
    }
    
    global_sum += acc + fp_acc;
}

/* Function 4: Many function calls with register arguments */
double test_function_calls(double *arr, int size) {
    double result = 0.0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Call math functions that use floating point registers */
        result += sin(arr[i]) * cos(arr[i] * 2.0);
        result += tan(arr[i] / 3.0) - atan(arr[i]);
        result += exp(arr[i] / 10.0) + log(fabs(arr[i]) + 1.0);
        result += pow(arr[i], 2.5) / sqrt(fabs(arr[i]) + 0.5);
        
        /* Mix with integer operations */
        int int_part = (int)arr[i];
        result += (int_part * int_part) / 1000.0;
    }
    
    return result;
}

/* Function 5: Vector operations using multiple SIMD registers */
void test_vector_ops(float *arr, int size) {
    int i;
    float sum[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float prod[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    /* Process 4 elements at a time */
    for (i = 0; i < size - 3; i += 4) {
        /* Many SIMD-like operations creating register pressure */
        sum[0] += arr[i] * 1.1f;
        sum[1] += arr[i+1] * 1.2f;
        sum[2] += arr[i+2] * 1.3f;
        sum[3] += arr[i+3] * 1.4f;
        
        prod[0] *= arr[i] + 0.1f;
        prod[1] *= arr[i+1] + 0.2f;
        prod[2] *= arr[i+2] + 0.3f;
        prod[3] *= arr[i+3] + 0.4f;
        
        /* Cross-element operations */
        float temp0 = sum[0] * prod[1];
        float temp1 = sum[1] * prod[2];
        float temp2 = sum[2] * prod[3];
        float temp3 = sum[3] * prod[0];
        
        arr[i] = temp0 - temp1;
        arr[i+1] = temp1 - temp2;
        arr[i+2] = temp2 - temp3;
        arr[i+3] = temp3 - temp0;
    }
    
    global_sum += sum[0] + sum[1] + sum[2] + sum[3];
}

/* Function 6: Mixed data types stressing register classes */
long test_mixed_types(char *carr, short *sarr, int *iarr, 
                      float *farr, double *darr, int size) {
    long total = 0;
    double dtotal = 0.0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Operations on different data types requiring different registers */
        char c = carr[i];
        short s = sarr[i];
        int i1 = iarr[i];
        float f = farr[i];
        double d = darr[i];
        
        /* Mixed-type computations */
        total += (long)c * s + i1;
        dtotal += (double)f * d + (double)i1 / 256.0;
        
        /* Type conversions */
        carr[i] = (char)((i1 + s) % 256);
        sarr[i] = (short)((i1 * 3 + c) % 32768);
        iarr[i] = (int)(dtotal * 1000.0);
        farr[i] = (float)(total % 10000) / 100.0f;
        darr[i] = dtotal / (i + 1.0);
    }
    
    return total + (long)dtotal;
}

/* Function 7: Pointer aliasing extending live ranges */
void test_pointer_aliasing(int *arr1, int *arr2, int *arr3, int size) {
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    int *ptr3 = arr3;
    int i, j;
    
    /* Complex pointer arithmetic keeping values live */
    for (i = 0; i < size; i++) {
        int val1 = *ptr1;
        int val2 = *ptr2;
        int val3 = *ptr3;
        
        /* Long dependency chain */
        for (j = 0; j < 8; j++) {
            val1 = (val1 * 1103515245 + 12345) & 0x7fffffff;
            val2 = (val2 * 1664525 + 1013904223) & 0x7fffffff;
            val3 = val1 ^ val2;
            
            /* Pointer updates create aliasing concerns */
            if (j % 3 == 0) {
                *ptr1 = val3;
                ptr1++;
            }
            if (j % 4 == 0) {
                *ptr2 = val1;
                ptr2++;
            }
            if (j % 5 == 0) {
                *ptr3 = val2;
                ptr3++;
            }
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Main function orchestrating all tests */
int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate large arrays */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    char *char_array = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_array = (short*)malloc(ARRAY_SIZE * sizeof(short));
    ComplexStruct *struct_array = (ComplexStruct*)malloc(
        (ARRAY_SIZE/10) * sizeof(ComplexStruct));
    
    if (!int_array || !double_array || !float_array || 
        !char_array || !short_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 10000;
        double_array[i] = (double)(rand() % 10000) / 100.0;
        float_array[i] = (float)(rand() % 10000) / 100.0f;
        char_array[i] = (char)(rand() % 256);
        short_array[i] = (short)(rand() % 32768);
    }
    
    for (i = 0; i < ARRAY_SIZE/10; i++) {
        for (j = 0; j < 8; j++) {
            struct_array[i].data[j] = rand() % 1000;
        }
        for (j = 0; j < 4; j++) {
            struct_array[i].weights[j] = (double)(rand() % 1000) / 10.0;
        }
    }
    
    printf("Starting register pressure tests...\n");
    start = clock();
    
    /* Warm-up phase for profile feedback */
    for (i = 0; i < ITERATIONS/10; i++) {
        test_nested_loops(int_array, ARRAY_SIZE/4);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Run all tests multiple times */
    long total_result = 0;
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops */
        test_nested_loops(int_array, ARRAY_SIZE/2);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG */
        total_result += test_complex_cfg(int_array, ARRAY_SIZE/3);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_constraints(struct_array, ARRAY_SIZE/100);
        asm volatile("" ::: "memory");
        
        /* Test 4: Function calls */
        global_sum += test_function_calls(double_array, ARRAY_SIZE/5);
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_ops(float_array, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 6: Mixed types */
        total_result += test_mixed_types(char_array, short_array, 
                                        int_array, float_array, 
                                        double_array, ARRAY_SIZE/10);
        asm volatile("" ::: "memory");
        
        /* Test 7: Pointer aliasing */
        test_pointer_aliasing(int_array, 
                             int_array + ARRAY_SIZE/4, 
                             int_array + ARRAY_SIZE/2, 
                             ARRAY_SIZE/8);
        asm volatile("" ::: "memory");
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
        checksum += (unsigned long long)(double_array[i] * 1000.0);
        checksum += (unsigned long long)(float_array[i] * 1000.0f);
    }
    
    checksum += total_result + (unsigned long long)global_sum;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %llu\n", checksum);
    printf("Total result: %ld\n", total_result);
    printf("Global sum: %f\n", global_sum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(char_array);
    free(short_array);
    free(struct_array);
    
    return 0;
}
