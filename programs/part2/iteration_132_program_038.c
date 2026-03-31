/* test_mcf_coverage.c - Program to stress GCC's min-cost flow solver */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile long global_checksum = 0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l;
    int temp1, temp2, temp3, temp4, temp5;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    
    /* Complex loop structure creating many basic blocks */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i] * 3;
        for (j = i; j < size / 8; j++) {
            temp2 = data[j] + temp1;
            if (temp2 > 1000) {
                for (k = j; k < size / 16; k++) {
                    temp3 = data[k] - temp2;
                    if (temp3 < 0) {
                        for (l = k; l < size / 32; l++) {
                            temp4 = data[l] * temp3;
                            temp5 = temp4 / (temp2 + 1);
                            acc1 += temp5;
                            acc2 += temp4;
                        }
                    } else {
                        temp4 = temp3 * 2;
                        acc3 += temp4;
                    }
                }
            } else {
                temp3 = temp2 * 7;
                acc4 += temp3;
            }
            acc5 += temp1 + temp2;
        }
    }
    
    /* Force all accumulators to be used */
    asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3), "r"(acc4), "r"(acc5));
    global_checksum += acc1 + acc2 + acc3 + acc4 + acc5;
}

/* Function 2: Complex switch with fall-through cases */
int test_complex_switch(int value, int *data, int size) {
    int result = 0;
    int i;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[0] * 2;
            /* Fall through */
        case 1:
            result += data[1] * 3;
            break;
        case 2:
            result = data[2] - data[3];
            /* Fall through */
        case 3:
        case 4:
            result *= data[4];
            break;
        case 5:
            for (i = 0; i < size / 2; i++) {
                result += data[i] * i;
            }
            break;
        case 6:
            result = data[5] / (data[6] + 1);
            /* Fall through */
        case 7:
            result += 1000;
            break;
        case 8:
        case 9:
        case 10:
            result = data[7] * data[8] * data[9];
            break;
        case 11:
            result = -data[10];
            /* Fall through */
        case 12:
            result += data[11] * 2;
            /* Fall through */
        case 13:
            result -= data[12];
            break;
        case 14:
            result = data[13] | data[14];
            break;
        default:
            result = 1;
    }
    
    /* Multiple early returns in different paths */
    if (result > 1000000) {
        return result % 1000;
    }
    
    if (value % 3 == 0) {
        return result + 500;
    }
    
    /* Complex expression with many temporaries */
    int a = data[15] * 3;
    int b = data[16] + a;
    int c = data[17] - b;
    int d = data[18] * c;
    int e = data[19] / (d + 1);
    int f = a + b + c + d + e;
    
    return result + f;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_register_pressure(int *data, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Force specific registers with inline asm */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "imul %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            : [out1] "=r" (a)
            : [in1] "r" (data[i]), [in2] "r" (data[i+1])
            : "eax", "cc"
        );
        
        asm volatile (
            "mov %[in1], %%ebx\n\t"
            "add %[in2], %%ebx\n\t"
            "mov %%ebx, %[out1]\n\t"
            : [out1] "=r" (b)
            : [in1] "r" (data[i+2]), [in2] "r" (data[i+3])
            : "ebx", "cc"
        );
        
        asm volatile (
            "mov %[in1], %%ecx\n\t"
            "sub %[in2], %%ecx\n\t"
            "mov %%ecx, %[out1]\n\t"
            : [out1] "=r" (c)
            : [in1] "r" (data[i+4]), [in2] "r" (data[i+5])
            : "ecx", "cc"
        );
        
        asm volatile (
            "mov %[in1], %%edx\n\t"
            "xor %[in2], %%edx\n\t"
            "mov %%edx, %[out1]\n\t"
            : [out1] "=r" (d)
            : [in1] "r" (data[i+6]), [in2] "r" (data[i+7])
            : "edx", "cc"
        );
        
        /* Use all results to keep them live */
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (a + 1);
        
        /* Memory clobber to force spills */
        asm volatile("" : : "r"(h) : "memory");
        
        global_checksum += a + b + c + d + e + f + g + h;
    }
}

/* Function 4: Mixed data types and vector-like operations */
void test_mixed_types(double *ddata, float *fdata, short *sdata, 
                      char *cdata, int size) {
    int i;
    double dacc = 0.0;
    float facc = 0.0f;
    long long llacc = 0;
    
    for (i = 0; i < size; i++) {
        /* Mixed type computations */
        double d1 = ddata[i] * 1.5;
        float f1 = fdata[i] + 2.5f;
        short s1 = sdata[i] * 3;
        char c1 = cdata[i] + 5;
        
        /* Force conversions and use different register classes */
        dacc += d1 + (double)f1 + (double)s1 + (double)c1;
        facc += f1 + (float)d1 + (float)s1 + (float)c1;
        llacc += (long long)d1 + (long long)f1 + s1 + c1;
        
        /* Complex addressing modes */
        int idx1 = i * 2;
        int idx2 = i * 3;
        int idx3 = i * 5;
        
        ddata[idx1 % size] = dacc / (i + 1);
        fdata[idx2 % size] = facc * 0.5f;
        sdata[idx3 % size] = (short)(llacc % 1000);
        cdata[i] = (char)(((int)dacc + (int)facc + (int)llacc) % 256);
    }
    
    /* Use all accumulators */
    asm volatile("" : : "r"(dacc), "r"(facc), "r"(llacc));
    global_checksum += (long)dacc + (long)facc + (long)llacc;
}

/* Function 5: Function with many arguments (register + stack) */
long test_many_args(int a1, int a2, int a3, int a4, int a5,
                    int a6, int a7, int a8, int a9, int a10,
                    int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int b1 = a1 * a2;
    int b2 = a3 + a4;
    int b3 = a5 - a6;
    int b4 = a7 * a8;
    int b5 = a9 / (a10 + 1);
    int b6 = a11 | a12;
    int b7 = a13 & a14;
    int b8 = a15 ^ b1;
    
    /* Nested conditionals */
    if (b1 > b2) {
        if (b3 < b4) {
            b5 = b6 * b7;
        } else {
            b5 = b8 + b1;
        }
    } else if (b2 > b3) {
        b6 = b4 - b5;
    } else {
        b7 = b1 * b2 * b3;
    }
    
    /* Loop with many live variables */
    int i;
    long result = 0;
    for (i = 0; i < 100; i++) {
        int t1 = b1 + i;
        int t2 = b2 * i;
        int t3 = b3 - i;
        int t4 = b4 / (i + 1);
        int t5 = b5 | i;
        int t6 = b6 & i;
        int t7 = b7 ^ i;
        int t8 = b8 + t1;
        
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
    }
    
    return result;
}

/* Function 6: Pointer aliasing and volatile accesses */
void test_pointer_aliasing(int *data, int size) {
    int *ptr1 = data;
    int *ptr2 = data + size / 2;
    int *ptr3 = data + size / 4;
    int *ptr4 = data + size * 3 / 4;
    
    volatile int *volatile_ptr = &data[size / 3];
    
    int i;
    for (i = 0; i < size / 4; i++) {
        /* Create aliasing */
        *ptr1 = *ptr2 + *ptr3;
        *ptr2 = *ptr1 - *ptr4;
        *ptr3 = *ptr2 * *ptr1;
        *ptr4 = *ptr3 / (*ptr1 + 1);
        
        /* Volatile access forces memory operations */
        *volatile_ptr = *ptr1 + *ptr2 + *ptr3 + *ptr4;
        
        /* Pointer arithmetic keeps values live */
        ptr1++;
        ptr2--;
        ptr3 += 2;
        ptr4 -= 2;
        
        /* Prevent optimization of pointer values */
        asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4));
    }
    
    /* Use final values */
    global_checksum += *data + data[size-1];
}

/* Main function that orchestrates all tests */
int main() {
    int i, j;
    
    /* Allocate and initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!int_data || !double_data || !float_data || !short_data || !char_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (double)(rand() % 1000) / 10.0;
        float_data[i] = (float)(rand() % 1000) / 10.0f;
        short_data[i] = (short)(rand() % 1000);
        char_data[i] = (char)(rand() % 256);
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < 5; j++) {
        test_nested_loops(int_data, ARRAY_SIZE / (j + 1));
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops */
        test_nested_loops(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch */
        int switch_result = test_complex_switch(i, int_data, ARRAY_SIZE);
        global_checksum += switch_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_register_pressure(int_data, ARRAY_SIZE / 2);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types */
        test_mixed_types(double_data, float_data, short_data, char_data, 
                         ARRAY_SIZE / 4);
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        long arg_result = test_many_args(
            int_data[0], int_data[1], int_data[2], int_data[3], int_data[4],
            int_data[5], int_data[6], int_data[7], int_data[8], int_data[9],
            int_data[10], int_data[11], int_data[12], int_data[13], int_data[14]
        );
        global_checksum += arg_result;
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        test_pointer_aliasing(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Modify data slightly each iteration */
        for (j = 0; j < ARRAY_SIZE; j += 7) {
            int_data[j] = (int_data[j] * 3 + i) % 1000;
        }
    }
    
    /* Final computation for verification */
    long final_checksum = global_checksum;
    for (i = 0; i < ARRAY_SIZE; i += 13) {
        final_checksum += int_data[i] + (long)double_data[i] + 
                         (long)float_data[i] + short_data[i] + char_data[i];
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    printf("Tests completed successfully.\n");
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(float_data);
    free(short_data);
    free(char_data);
    
    return 0;
}
