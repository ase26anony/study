/* test_mcf_coverage.c
 * 
 * This program is designed to stress GCC's register allocator and
 * trigger the min-cost flow solver's debug output, specifically
 * aiming to reach the print_edge function's uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_result = 0;

/* Function 1: Deeply nested loops with many live ranges */
int complex_nested_loops(int *data, int size) {
    int sum = 0;
    int i, j, k, l, m;
    
    /* Multiple nested loops creating many live ranges */
    for (i = 0; i < size / 10; i++) {
        int temp1 = data[i];
        int temp2 = data[i + 1];
        int temp3 = data[i + 2];
        
        for (j = 0; j < 5; j++) {
            int inner1 = temp1 * j;
            int inner2 = temp2 + j;
            
            for (k = 0; k < 3; k++) {
                int deeper1 = inner1 * k;
                int deeper2 = inner2 - k;
                int deeper3 = temp3 + k;
                
                for (l = 0; l < 2; l++) {
                    int deepest1 = deeper1 >> l;
                    int deepest2 = deeper2 << l;
                    int deepest3 = deeper3 ^ l;
                    int deepest4 = deepest1 + deepest2;
                    int deepest5 = deepest3 * deepest4;
                    
                    for (m = 0; m < 2; m++) {
                        /* Complex expression with many intermediates */
                        int result = (deepest5 * m) + 
                                    (deepest4 / (m + 1)) -
                                    (deepest3 << m) +
                                    (deepest2 >> m) +
                                    (deepest1 ^ m);
                        sum += result;
                    }
                }
            }
        }
    }
    return sum;
}

/* Function 2: Complex control flow with many basic blocks */
int complex_control_flow(int *data, int size) {
    int result = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex if-else chain creating many basic blocks */
        if (data[i] < 100) {
            if (data[i] < 50) {
                if (data[i] < 25) {
                    if (data[i] < 10) {
                        result += data[i] * 2;
                    } else {
                        result += data[i] * 3;
                    }
                } else {
                    if (data[i] < 37) {
                        result += data[i] * 4;
                    } else {
                        result += data[i] * 5;
                    }
                }
            } else {
                if (data[i] < 75) {
                    if (data[i] < 62) {
                        result += data[i] * 6;
                    } else {
                        result += data[i] * 7;
                    }
                } else {
                    if (data[i] < 87) {
                        result += data[i] * 8;
                    } else {
                        result += data[i] * 9;
                    }
                }
            }
        } else {
            if (data[i] < 200) {
                if (data[i] < 150) {
                    result += data[i] * 10;
                } else {
                    result += data[i] * 11;
                }
            } else {
                if (data[i] < 300) {
                    result += data[i] * 12;
                } else {
                    result += data[i] * 13;
                }
            }
        }
        
        /* Early returns in the middle of loops */
        if (result > 1000000) {
            return result / 2;
        }
        
        /* Continue with more complex logic */
        switch (data[i] % 15) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
            case 3: result += 4; break;
            case 4: result += 5; break;
            case 5: result += 6; break;
            case 6: result += 7; break;
            case 7: result += 8; break;
            case 8: result += 9; break;
            case 9: result += 10; break;
            case 10: result += 11; break;
            case 11: result += 12; break;
            case 12: result += 13; break;
            case 13: result += 14; break;
            case 14: result += 15; break;
            default: result += 16; break;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
int inline_asm_stress(int *data, int size) {
    int i;
    int result = 0;
    
    for (i = 0; i < size; i++) {
        int a, b, c, d, e, f;
        
        /* Force specific register allocations */
        asm volatile (
            "movl %[input], %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[a]\n\t"
            : [a] "=r" (a)
            : [input] "r" (data[i])
            : "eax", "memory"
        );
        
        asm volatile (
            "movl %[input], %%ebx\n\t"
            "subl $2, %%ebx\n\t"
            "movl %%ebx, %[b]\n\t"
            : [b] "=r" (b)
            : [input] "r" (a)
            : "ebx", "memory"
        );
        
        asm volatile (
            "movl %[input], %%ecx\n\t"
            "imull $3, %%ecx\n\t"
            "movl %%ecx, %[c]\n\t"
            : [c] "=r" (c)
            : [input] "r" (b)
            : "ecx", "memory"
        );
        
        asm volatile (
            "movl %[input], %%edx\n\t"
            "xorl $0x55, %%edx\n\t"
            "movl %%edx, %[d]\n\t"
            : [d] "=r" (d)
            : [input] "r" (c)
            : "edx", "memory"
        );
        
        asm volatile (
            "movl %[input], %%esi\n\t"
            "shrl $2, %%esi\n\t"
            "movl %%esi, %[e]\n\t"
            : [e] "=r" (e)
            : [input] "r" (d)
            : "esi", "memory"
        );
        
        asm volatile (
            "movl %[input], %%edi\n\t"
            "andl $0xFF, %%edi\n\t"
            "movl %%edi, %[f]\n\t"
            : [f] "=r" (f)
            : [input] "r" (e)
            : "edi", "memory"
        );
        
        result += f;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Function 4: Mixed data types and many function arguments */
double mixed_data_types(char *cdata, short *sdata, int *idata, 
                       long *ldata, float *fdata, double *ddata, 
                       int size) {
    double total = 0.0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Mix different data types requiring different register classes */
        char c = cdata[i];
        short s = sdata[i];
        int i_val = idata[i];
        long l = ldata[i];
        float f = fdata[i];
        double d = ddata[i];
        
        /* Complex expression mixing types */
        double temp = (double)c + (double)s + (double)i_val + 
                     (double)l + (double)f + d;
        
        /* Trigonometric functions that use FP registers */
        temp = sin(temp) * cos(temp) + tan(temp / 2.0);
        
        /* Power function */
        temp = pow(temp, 1.5);
        
        total += temp;
    }
    
    return total;
}

/* Function 5: Function with many arguments (stress calling convention) */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Complex computation using all arguments */
    int result = a1 + a2 - a3 * a4 / (a5 + 1);
    result += a6 << 2;
    result += a7 >> 1;
    result += a8 & 0xFF;
    result += a9 | 0x55;
    result += a10 ^ a11;
    result += a12 * a13;
    result += a14 / (a15 + 1);
    
    /* Nested function call with many arguments */
    result += many_arguments_helper(a1, a2, a3, a4, a5, a6, a7);
    
    return result;
}

/* Helper function for many_arguments */
int many_arguments_helper(int b1, int b2, int b3, int b4,
                         int b5, int b6, int b7) {
    return b1 + b2 + b3 + b4 + b5 + b6 + b7;
}

/* Function 6: Pointer aliasing and volatile variables */
int pointer_aliasing_stress(int *data, int size) {
    volatile int vol1, vol2, vol3;
    int *ptr1 = data;
    int *ptr2 = data + size / 2;
    int *ptr3 = data + size / 4;
    int result = 0;
    int i;
    
    for (i = 0; i < size / 2; i++) {
        /* Create aliasing pointers */
        int *alias1 = (i % 2) ? ptr1 : ptr2;
        int *alias2 = (i % 3) ? ptr2 : ptr3;
        int *alias3 = (i % 5) ? ptr3 : ptr1;
        
        /* Complex pointer arithmetic */
        int val1 = *alias1++;
        int val2 = *alias2++;
        int val3 = *alias3++;
        
        /* Volatile operations force memory accesses */
        vol1 = val1;
        vol2 = val2;
        vol3 = val3;
        
        /* Use volatile values */
        result += vol1 * vol2 - vol3;
        
        /* Update pointers in complex way */
        if (i % 7 == 0) {
            ptr1 = alias1;
        }
        if (i % 11 == 0) {
            ptr2 = alias2;
        }
        if (i % 13 == 0) {
            ptr3 = alias3;
        }
    }
    
    return result;
}

/* Function 7: Vector-like operations using multiple registers */
void vector_operations(int *src1, int *src2, int *dst, int size) {
    int i;
    
    for (i = 0; i < size; i += 4) {
        /* Simulate 4-element vector operations */
        int a0 = src1[i];
        int a1 = src1[i + 1];
        int a2 = src1[i + 2];
        int a3 = src1[i + 3];
        
        int b0 = src2[i];
        int b1 = src2[i + 1];
        int b2 = src2[i + 2];
        int b3 = src2[i + 3];
        
        /* Multiple parallel computations */
        int c0 = a0 + b0;
        int c1 = a1 - b1;
        int c2 = a2 * b2;
        int c3 = a3 / (b3 + 1);
        
        /* More computations */
        int d0 = c0 << 2;
        int d1 = c1 >> 1;
        int d2 = c2 & 0xFF;
        int d3 = c3 | 0x55;
        
        /* Cross-element operations */
        int e0 = d0 + d1;
        int e1 = d1 + d2;
        int e2 = d2 + d3;
        int e3 = d3 + d0;
        
        /* Store results */
        dst[i] = e0;
        dst[i + 1] = e1;
        dst[i + 2] = e2;
        dst[i + 3] = e3;
    }
}

/* Main test driver */
int main() {
    int i, j;
    int total_result = 0;
    
    /* Allocate and initialize test data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *cdata = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *sdata = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long *ldata = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *fdata = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *ddata = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        cdata[i] = rand() % 256;
        sdata[i] = rand() % 65536;
        ldata[i] = rand() % 10000;
        fdata[i] = (float)rand() / RAND_MAX * 100.0f;
        ddata[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up phase (allow GCC profile feedback if enabled) */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (j = 0; j < WARMUP_ITERATIONS; j++) {
        int warm_result = complex_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
        global_result ^= warm_result;
    }
    
    /* Main test phase */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    for (j = 0; j < ITERATIONS; j++) {
        int iter_result = 0;
        
        /* Test 1: Complex nested loops */
        iter_result += complex_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        iter_result += complex_control_flow(data2, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly stress */
        iter_result += inline_asm_stress(data3, ARRAY_SIZE / 20);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed data types */
        double dresult = mixed_data_types(cdata, sdata, data1, ldata, 
                                         fdata, ddata, ARRAY_SIZE / 50);
        iter_result += (int)dresult;
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        iter_result += many_arguments(j, j+1, j+2, j+3, j+4,
                                     j+5, j+6, j+7, j+8, j+9,
                                     j+10, j+11, j+12, j+13, j+14);
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        iter_result += pointer_aliasing_stress(data1, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 7: Vector operations */
        int *temp_dst = (int*)malloc(ARRAY_SIZE * sizeof(int));
        vector_operations(data1, data2, temp_dst, ARRAY_SIZE);
        for (i = 0; i < ARRAY_SIZE / 100; i++) {
            iter_result += temp_dst[i];
        }
        free(temp_dst);
        asm volatile("" ::: "memory");
        
        total_result += iter_result;
        
        /* Progress indicator */
        if ((j + 1) % (ITERATIONS / 10) == 0) {
            printf("  Completed %d%%\n", (j + 1) * 100 / ITERATIONS);
        }
    }
    
    /* Clean up */
    free(data1);
    free(data2);
    free(data3);
    free(cdata);
    free(sdata);
    free(ldata);
    free(fdata);
    free(ddata);
    
    printf("Test completed. Final result: %d\n", total_result);
    printf("Checksum: 0x%08x\n", total_result);
    
    return 0;
}
