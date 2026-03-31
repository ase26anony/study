/* mcf_stress_test.c - Stress test for GCC's min-cost flow register allocator */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Complex expression with many temporaries */
double complex_expression(double a, double b, double c, double d, double e, double f) {
    /* Many intermediate values requiring registers */
    double t1 = a * b + c;
    double t2 = d / e - f;
    double t3 = sin(a) * cos(b);
    double t4 = tan(c) * atan(d);
    double t5 = exp(e) * log(fabs(f) + 1.0);
    double t6 = t1 * t2 + t3;
    double t7 = t4 / t5 - t6;
    double t8 = sqrt(t7 * t7 + 1.0);
    double t9 = pow(t8, 2.5) * M_PI;
    double t10 = t9 / (t1 + t2) * (t3 - t4);
    
    /* Nested calculations extending live ranges */
    for (int i = 0; i < 5; i++) {
        t10 += sin(t10 * i) * cos(t10 / (i + 1));
    }
    
    return t10 * a * b * c * d * e * f;
}

/* Function with deeply nested loops and many live ranges */
void nested_loop_stress(int* data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1, prod3 = 1, prod4 = 1;
    float fsum1 = 0.0f, fsum2 = 0.0f;
    double dsum1 = 0.0, dsum2 = 0.0;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        int idx1 = data[i] % size;
        int idx2 = data[i + 1] % size;
        
        for (j = 0; j < size / 8; j++) {
            int val1 = data[idx1] ^ data[idx2];
            int val2 = data[j] | data[size - j - 1];
            
            /* Middle loop with complex expressions */
            for (k = 0; k < 10; k++) {
                double temp1 = complex_expression(val1, val2, i, j, k, size);
                double temp2 = complex_expression(val2, val1, j, i, k, size);
                
                /* Innermost loop with register-intensive operations */
                for (l = 0; l < 5; l++) {
                    /* Many intermediate calculations */
                    int t1 = val1 * l + val2;
                    int t2 = val2 * l - val1;
                    int t3 = t1 ^ t2;
                    int t4 = t1 & t2;
                    int t5 = t3 | t4;
                    
                    float ft1 = sinf(t1) * cosf(t2);
                    float ft2 = tanf(t3) * atanf(t4);
                    
                    sum1 += t1 + t5;
                    sum2 += t2 * t3;
                    sum3 += t4 ^ t5;
                    sum4 += (t1 << 2) | (t2 >> 3);
                    
                    prod1 *= (t1 + 1);
                    prod2 *= (t2 + 1);
                    prod3 *= (t3 + 1);
                    prod4 *= (t4 + 1);
                    
                    fsum1 += ft1 * ft2;
                    fsum2 += ft1 / (ft2 + 1.0f);
                    
                    dsum1 += temp1 * l;
                    dsum2 += temp2 / (l + 1);
                }
            }
            
            /* Early continue creates complex CFG */
            if ((j % 7) == 0) {
                continue;
            }
            
            /* Break at different nesting levels */
            if ((sum1 > 1000000) && (k > 5)) {
                break;
            }
        }
        
        /* Multiple early returns */
        if (i > 100 && sum2 < 0) {
            return;
        }
    }
    
    /* Store results to volatile to prevent dead code elimination */
    g_volatile_counter = sum1 + sum2 + sum3 + sum4;
    g_volatile_double = dsum1 + dsum2 + fsum1 + fsum2;
}

/* Function with complex switch statement and computed goto */
int switch_complex_cfg(int value, int* data, int size) {
    int result = 0;
    
    /* Large switch with many cases */
    switch (value % SWITCH_CASES) {
        case 0: {
            /* Case with inline assembly and fixed registers */
            int a, b;
            asm volatile (
                "movl %1, %%eax\n\t"
                "movl %2, %%ebx\n\t"
                "addl %%ebx, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r" (a)
                : "r" (value), "r" (data[0])
                : "%eax", "%ebx", "memory"
            );
            result = a * 2;
            /* Fall through */
        }
        case 1:
            result += data[1] * 3;
            break;
        case 2: {
            /* More inline assembly with register constraints */
            int x, y;
            asm volatile (
                "imull %2, %1\n\t"
                "addl %1, %0\n\t"
                : "+r" (result), "=r" (x)
                : "r" (data[2]), "1" (value)
                : "cc", "memory"
            );
            y = x ^ result;
            result = y;
            break;
        }
        case 3:
        case 4:
            /* Combined cases */
            result = data[3] + data[4] - value;
            if (result > 0) {
                return result;  /* Early return */
            }
            /* Fall through */
        case 5:
            result *= 2;
            break;
        case 6: {
            /* Vector-like operations using multiple registers */
            long long ll1 = (long long)data[5] * data[6];
            long long ll2 = (long long)data[7] * data[8];
            long long ll3 = ll1 + ll2;
            long long ll4 = ll1 - ll2;
            result = (int)((ll3 + ll4) % 1000000);
            break;
        }
        case 7:
            /* Nested switch */
            switch (value % 5) {
                case 0: result = 1; break;
                case 1: result = 2; break;
                case 2: result = 3; break;
                case 3: result = 4; break;
                case 4: result = 5; break;
            }
            break;
        case 8:
            result = complex_expression(value, data[9], data[10], 
                                       data[11], data[12], data[13]);
            break;
        case 9:
            /* Loop with break at different levels */
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (i * j > 50) break;
                    result += i * j;
                }
                if (result > 100) break;
            }
            break;
        case 10:
            /* Computed goto simulation */
            {
                static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
                goto *labels[value % 4];
                
                label0:
                    result = 100;
                    goto end;
                label1:
                    result = 200;
                    goto end;
                label2:
                    result = 300;
                    goto end;
                label3:
                    result = 400;
                    goto end;
                end:;
            }
            break;
        case 11:
        case 12:
        case 13:
        case 14:
            /* Range of cases with complex math */
            {
                double d1 = sin(value) * cos(data[14]);
                double d2 = exp(data[15] / 100.0);
                double d3 = log(fabs(d1) + fabs(d2) + 1.0);
                result = (int)(d3 * 1000.0);
            }
            break;
        default:
            result = -1;
    }
    
    return result;
}

/* Function with many arguments stressing calling convention */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   float f1, float f2, float f3, double d1, double d2) {
    /* Mix of operations on all arguments */
    int sum_int = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    float sum_float = f1 + f2 + f3;
    double sum_double = d1 + d2;
    
    /* Complex expression using all arguments */
    double result = (sum_int * sum_float) / (sum_double + 1.0);
    
    /* Inline assembly using multiple argument registers */
    int output;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %3, %%ecx\n\t"
        "subl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (output)
        : "r" (a1), "r" (a2), "r" (a3)
        : "%eax", "%ebx", "%ecx", "memory"
    );
    
    return output + (int)result;
}

/* Function with mixed data types stressing register classes */
void mixed_data_types(char* cdata, short* sdata, int* idata, 
                      long* ldata, float* fdata, double* ddata, int size) {
    char c1, c2, c3;
    short s1, s2, s3;
    int i1, i2, i3;
    long l1, l2, l3;
    float f1, f2, f3;
    double d1, d2, d3;
    
    /* Complex addressing calculations */
    for (int idx = 0; idx < size; idx++) {
        /* Different index calculations */
        int idx1 = (idx * 3) % size;
        int idx2 = (idx * 7) % size;
        int idx3 = (idx * 11) % size;
        int idx4 = (idx * 13) % size;
        int idx5 = (idx * 17) % size;
        int idx6 = (idx * 19) % size;
        
        /* Load all data types - each requires different register class */
        c1 = cdata[idx1];
        c2 = cdata[idx2];
        c3 = cdata[idx3];
        
        s1 = sdata[idx1];
        s2 = sdata[idx2];
        s3 = sdata[idx3];
        
        i1 = idata[idx1];
        i2 = idata[idx2];
        i3 = idata[idx3];
        
        l1 = ldata[idx1];
        l2 = ldata[idx2];
        l3 = ldata[idx3];
        
        f1 = fdata[idx1];
        f2 = fdata[idx2];
        f3 = fdata[idx3];
        
        d1 = ddata[idx1];
        d2 = ddata[idx2];
        d3 = ddata[idx3];
        
        /* Mixed type computations */
        i1 = (int)c1 * (int)s1 + i1;
        i2 = (int)c2 * (int)s2 + i2;
        i3 = (int)c3 * (int)s3 + i3;
        
        l1 = (long)i1 * (long)i2 + l1;
        l2 = (long)i2 * (long)i3 + l2;
        l3 = (long)i3 * (long)i1 + l3;
        
        f1 = (float)i1 * 0.5f + f1;
        f2 = (float)i2 * 0.3f + f2;
        f3 = (float)i3 * 0.7f + f3;
        
        d1 = (double)l1 * 0.25 + d1;
        d2 = (double)l2 * 0.75 + d2;
        d3 = (double)l3 * 1.25 + d3;
        
        /* Store back with complex addressing */
        idata[idx4] = i1 + i2 + i3;
        ldata[idx5] = l1 ^ l2 ^ l3;
        fdata[idx6] = f1 + f2 + f3;
        ddata[idx % size] = d1 * d2 * d3;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Main test driver */
int main() {
    /* Initialize large arrays with random data */
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char* char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short* short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long* long_data = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        long_data[i] = rand() % 10000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    int checksum = 0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 10; warmup++) {
        nested_loop_stress(int_data, ARRAY_SIZE / 10);
    }
    
    /* Memory barrier between tests */
    asm volatile("" ::: "memory");
    
    /* Run multiple test patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops with register pressure */
        nested_loop_stress(int_data, ARRAY_SIZE / (iter % 10 + 1));
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex switch CFG */
        int switch_result = switch_complex_cfg(iter, int_data, ARRAY_SIZE);
        checksum ^= switch_result;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Many arguments */
        int arg_result = many_arguments(
            int_data[iter % ARRAY_SIZE],
            int_data[(iter + 1) % ARRAY_SIZE],
            int_data[(iter + 2) % ARRAY_SIZE],
            int_data[(iter + 3) % ARRAY_SIZE],
            int_data[(iter + 4) % ARRAY_SIZE],
            int_data[(iter + 5) % ARRAY_SIZE],
            int_data[(iter + 6) % ARRAY_SIZE],
            int_data[(iter + 7) % ARRAY_SIZE],
            int_data[(iter + 8) % ARRAY_SIZE],
            int_data[(iter + 9) % ARRAY_SIZE],
            float_data[iter % ARRAY_SIZE],
            float_data[(iter + 1) % ARRAY_SIZE],
            float_data[(iter + 2) % ARRAY_SIZE],
            double_data[iter % ARRAY_SIZE],
            double_data[(iter + 1) % ARRAY_SIZE]
        );
        checksum += arg_result;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed data types */
        if (iter % 5 == 0) {
            mixed_data_types(char_data, short_data, int_data,
                            long_data, float_data, double_data,
                            ARRAY_SIZE / 10);
        }
        
        /* Update volatile to prevent optimization */
        g_volatile_counter += iter;
        g_volatile_double += (double)iter * 0.1;
    }
    
    /* Final complex expression */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE / 100; i++) {
        final_result += complex_expression(
            int_data[i], int_data[i + 1], int_data[i + 2],
            int_data[i + 3], int_data[i + 4], int_data[i + 5]
        );
    }
    
    checksum += (int)final_result;
    
    /* Print verifiable result */
    printf("Final checksum: %d\n", checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile double: %f\n", g_volatile_double);
    
    /* Cleanup */
    free(int_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
