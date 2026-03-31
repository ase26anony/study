/* test_mcf_coverage.c
 * 
 * This program is designed to stress GCC's register allocator and
 * trigger the min-cost flow solver's debug output to reach uncovered
 * lines in mcf.cc's print_edge function.
 * 
 * Compile with: gcc -O3 -fdump-ira-all -fdump-ira-details -fno-omit-frame-pointer -dA -dp -dD test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Memory barrier to prevent cross-function optimization */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Complex expression with many intermediate values */
static inline uint64_t complex_expression(uint64_t a, uint64_t b, uint64_t c, 
                                          uint64_t d, uint64_t e, uint64_t f) {
    /* Force many temporary registers */
    uint64_t t1 = a * b + c;
    uint64_t t2 = d * e + f;
    uint64_t t3 = (a ^ b) | (c & d);
    uint64_t t4 = (e << 3) + (f >> 2);
    uint64_t t5 = t1 * t2 - t3;
    uint64_t t6 = t4 * t5 / (t1 + 1);
    uint64_t t7 = t6 ^ t5 ^ t4 ^ t3;
    uint64_t t8 = t7 * 11400714819323198485ULL;
    
    return t8 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
}

/* Function with deeply nested loops and many live ranges */
void test_deep_nested_loops(int* restrict arr1, int* restrict arr2, 
                           int* restrict arr3, int size) {
    volatile int keep_alive = 0;  /* Force register retention */
    
    /* Declare many variables at function scope */
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1, prod3 = 1;
    float fsum1 = 0.0f, fsum2 = 0.0f;
    double dsum1 = 0.0, dsum2 = 0.0;
    
    /* Complex loop structure */
    for (i = 0; i < size; i++) {
        /* First level nesting */
        sum1 += arr1[i];
        prod1 *= (arr1[i] & 0xFF) + 1;
        
        for (j = i; j < size; j += 128) {
            /* Second level nesting */
            sum2 += arr2[j];
            prod2 *= (arr2[j] & 0x7F) + 1;
            fsum1 += (float)arr2[j] * 0.5f;
            
            for (k = j; k < size; k += 256) {
                /* Third level nesting */
                sum3 += arr3[k];
                prod3 *= (arr3[k] & 0x3F) + 1;
                dsum1 += (double)arr3[k] * 0.25;
                
                /* Complex expression with many temps */
                int temp = complex_expression(
                    arr1[i], arr2[j], arr3[k],
                    sum1, sum2, sum3
                ) & 0xFFFF;
                
                /* Keep variable alive across loop */
                keep_alive = temp;
                
                /* More computations to increase pressure */
                fsum2 += (float)temp * 0.1f;
                dsum2 += (double)temp * 0.05;
                
                /* Early continue creates complex CFG */
                if (temp % 7 == 0) {
                    continue;
                }
                
                /* Nested if with break */
                for (l = 0; l < 8; l++) {
                    if (temp % (l + 2) == 0) {
                        break;
                    }
                    /* Another loop level */
                    for (m = 0; m < 4; m++) {
                        arr1[(i + m) % size] += l * m;
                    }
                }
            }
        }
        
        /* Early return pattern */
        if (sum1 > 1000000) {
            return;
        }
    }
    
    /* Use all computed values to prevent elimination */
    arr1[0] = sum1 + sum2 + sum3;
    arr2[0] = prod1 + prod2 + prod3;
    arr3[0] = (int)(fsum1 + fsum2 + dsum1 + dsum2);
}

/* Function with complex control flow using switch with many cases */
int test_complex_cfg(int x, int* arr, int size) {
    int result = x;
    int i;
    
    /* Switch with many cases to create complex CFG */
    switch (x % 13) {
        case 0:
            for (i = 0; i < size; i++) {
                result += arr[i] & 0xF;
                if (result > 1000) goto early_exit;
            }
            break;
        case 1:
            result = arr[x % size] * 2;
            /* Fall through */
        case 2:
            result += arr[(x + 1) % size];
            /* Fall through */
        case 3:
            result -= arr[(x + 2) % size];
            break;
        case 4:
            for (i = 0; i < size; i += 2) {
                result ^= arr[i];
            }
            break;
        case 5:
            result = ~result;
            break;
        case 6:
            result = result * result;
            break;
        case 7:
            result = result / (x + 1);
            break;
        case 8:
            result = result << (x % 8);
            break;
        case 9:
            result = result >> (x % 8);
            break;
        case 10:
            result = result | 0xAAAAAAAA;
            break;
        case 11:
            result = result & 0x55555555;
            break;
        case 12:
            result = result ^ 0x12345678;
            break;
        default:
            result = 0;
    }
    
    /* Multiple return points */
    if (result < 0) {
        return -result;
    }
    
early_exit:
    return result;
}

/* Function with inline assembly forcing specific register usage */
void test_asm_register_pressure(int* arr, int size) {
    int i;
    uint64_t a, b, c, d, e, f;
    
    /* Force use of specific registers */
    for (i = 0; i < size; i++) {
        /* Compete for RAX */
        asm volatile (
            "mov %[val], %%rax\n\t"
            "imul $11400714819323198485, %%rax, %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=r" (a)
            : [val] "r" (arr[i])
            : "rax", "rdx"  /* Clobber rax and rdx */
        );
        
        /* Compete for RBX */
        asm volatile (
            "mov %[val], %%rbx\n\t"
            "ror $17, %%rbx\n\t"
            "mov %%rbx, %[out]"
            : [out] "=r" (b)
            : [val] "r" (arr[i + 1 % size])
            : "rbx"
        );
        
        /* More assembly blocks competing for registers */
        asm volatile (
            "xchg %%rax, %[val]\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=r" (c), [val] "+r" (a)
            :
            : "rax", "rbx", "cc"
        );
        
        /* Memory clobber forces spills */
        asm volatile (
            "mfence\n\t"
            ::: "memory"
        );
        
        /* Use results to prevent elimination */
        arr[i] = (int)((a + b + c) & 0xFFFFFFFF);
    }
}

/* Function with mixed data types stressing different register classes */
void test_mixed_data_types(double* darr, float* farr, 
                          int* iarr, short* sarr, char* carr, int size) {
    int i;
    double dacc = 0.0;
    float facc = 0.0f;
    int iacc = 0;
    short sacc = 0;
    char cacc = 0;
    
    /* Mixed type computations */
    for (i = 0; i < size; i++) {
        /* Use all different types in complex expressions */
        dacc += darr[i] * 1.5 + (double)farr[i];
        facc += farr[i] * 2.0f + (float)iarr[i];
        iacc += iarr[i] * 3 + (int)sarr[i];
        sacc += sarr[i] * 4 + (short)carr[i];
        cacc += carr[i] * 5;
        
        /* Type conversions add pressure */
        darr[i] = (double)iarr[i] / 256.0;
        farr[i] = (float)sarr[i] / 128.0f;
        iarr[i] = (int)carr[i] * 2;
        
        /* Pointer aliasing prevents optimizations */
        volatile char* alias = (volatile char*)&iarr[i];
        *alias = carr[i];
    }
    
    /* Store results */
    darr[0] = dacc;
    farr[0] = facc;
    iarr[0] = iacc;
    sarr[0] = sacc;
    carr[0] = cacc;
}

/* Function with many function calls in loops */
void test_function_call_pressure(int* arr, int size) {
    int i, j;
    
    /* Many calls in loop */
    for (i = 0; i < size; i++) {
        /* Call complex_expression multiple times */
        uint64_t r1 = complex_expression(arr[i], arr[(i+1)%size], 
                                        arr[(i+2)%size], arr[(i+3)%size],
                                        arr[(i+4)%size], arr[(i+5)%size]);
        
        uint64_t r2 = complex_expression(arr[(i+6)%size], arr[(i+7)%size],
                                        arr[(i+8)%size], arr[(i+9)%size],
                                        arr[(i+10)%size], arr[(i+11)%size]);
        
        /* Recursive-like pattern */
        for (j = 0; j < 4; j++) {
            r1 = complex_expression(r1 & 0xFFFF, r2 & 0xFFFF,
                                   arr[(i+j)%size], arr[(i+j+1)%size],
                                   j, i);
        }
        
        arr[i] = (int)(r1 & 0x7FFFFFFF);
    }
}

/* Function with many arguments to stress register/stack passing */
int test_many_arguments(int a1, int a2, int a3, int a4, int a5,
                       int a6, int a7, int a8, int a9, int a10,
                       int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int sum = a1 + a2 + a3 + a4 + a5;
    int prod = a6 * a7 * a8 * a9 * a10;
    int xor_result = a11 ^ a12 ^ a13 ^ a14 ^ a15;
    
    /* Many intermediate calculations */
    int t1 = sum * prod;
    int t2 = xor_result * sum;
    int t3 = prod * xor_result;
    int t4 = t1 + t2 + t3;
    int t5 = t4 * 2 - t1;
    int t6 = t5 / (prod + 1);
    int t7 = t6 ^ t5 ^ t4;
    
    /* Complex control flow */
    switch (t7 % 8) {
        case 0: return t1;
        case 1: return t2;
        case 2: return t3;
        case 3: return t4;
        case 4: return t5;
        case 5: return t6;
        case 6: return t7;
        default: return sum + prod + xor_result;
    }
}

/* Main test driver */
int main() {
    int i, iter;
    uint64_t checksum = 0;
    
    /* Allocate large arrays with different types */
    int* arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* darr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* farr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    short* sarr = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char* carr = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!arr1 || !arr2 || !arr3 || !darr || !farr || !sarr || !carr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand();
        arr2[i] = rand();
        arr3[i] = rand();
        darr[i] = (double)rand() / RAND_MAX;
        farr[i] = (float)rand() / RAND_MAX;
        sarr[i] = (short)rand();
        carr[i] = (char)rand();
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up phase for profile feedback */
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_deep_nested_loops(arr1, arr2, arr3, ARRAY_SIZE / 10);
        MEMORY_BARRIER();
    }
    
    /* Main test iterations */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Deep nested loops */
        test_deep_nested_loops(arr1, arr2, arr3, ARRAY_SIZE / 20);
        MEMORY_BARRIER();
        
        /* Test 2: Complex CFG */
        for (i = 0; i < 100; i++) {
            arr1[i] = test_complex_cfg(arr1[i], arr2, ARRAY_SIZE);
        }
        MEMORY_BARRIER();
        
        /* Test 3: Inline assembly register pressure */
        test_asm_register_pressure(arr3, ARRAY_SIZE / 50);
        MEMORY_BARRIER();
        
        /* Test 4: Mixed data types */
        test_mixed_data_types(darr, farr, arr1, sarr, carr, ARRAY_SIZE / 40);
        MEMORY_BARRIER();
        
        /* Test 5: Function call pressure */
        test_function_call_pressure(arr2, ARRAY_SIZE / 30);
        MEMORY_BARRIER();
        
        /* Test 6: Many arguments */
        for (i = 0; i < 50; i++) {
            arr1[i % ARRAY_SIZE] += test_many_arguments(
                arr1[i], arr2[i], arr3[i], i, iter,
                arr1[i+1], arr2[i+1], arr3[i+1], i+1, iter+1,
                arr1[i+2], arr2[i+2], arr3[i+2], i+2, iter+2
            );
        }
        MEMORY_BARRIER();
        
        /* Update checksum */
        checksum ^= arr1[iter % ARRAY_SIZE];
        checksum ^= arr2[iter % ARRAY_SIZE];
        checksum ^= arr3[iter % ARRAY_SIZE];
        checksum = (checksum << 1) | (checksum >> 63);
    }
    
    /* Final computation and output */
    uint64_t final_result = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_result += arr1[i] ^ arr2[i] ^ arr3[i];
        final_result += (uint64_t)(darr[i] * 1000);
        final_result += (uint32_t)(farr[i] * 1000);
        final_result += sarr[i];
        final_result += carr[i];
    }
    
    final_result ^= checksum;
    
    printf("Test completed. Final checksum: 0x%016llX\n", 
           (unsigned long long)final_result);
    printf("Expected range: non-zero value\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr);
    free(farr);
    free(sarr);
    free(carr);
    
    return 0;
}
