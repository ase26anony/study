/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_temp = 0.0;

/* Complex expression with many intermediate values */
double complex_expression(double a, double b, double c, double d, double e) {
    /* Many intermediate calculations requiring registers */
    double t1 = a * b + c;
    double t2 = d * e - a;
    double t3 = t1 * t2 / (b + 1.0);
    double t4 = sin(t1) * cos(t2);
    double t5 = t3 * t4 + tan(t1 + t2);
    double t6 = sqrt(fabs(t5)) + log(fabs(t4) + 1.0);
    double t7 = t6 * t3 / (t4 + 0.5);
    double t8 = t7 * t5 + t6 * t4;
    double t9 = t8 * t3 / (t5 + 0.25);
    double t10 = t9 + t8 + t7 + t6 + t5;
    
    return t10 * global_temp;
}

/* Function with deeply nested loops and many live ranges */
void nested_loop_stress(int *data, int size) {
    int i, j, k, l;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    volatile double keep_alive[10]; /* Force register spills */
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        int base = data[i];
        double temp1 = base * 1.1;
        double temp2 = base * 2.2;
        
        for (j = 0; j < size / 8; j++) {
            int idx = data[j] % 100;
            double temp3 = temp1 * idx;
            double temp4 = temp2 / (idx + 1);
            
            /* Intermediate calculations that must stay in registers */
            double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
            for (k = 0; k < 50; k++) {
                /* Complex expression with many temporaries */
                double x = temp3 * k + temp4;
                double y = temp4 * k - temp3;
                double z = x * y + x / (y + 1.0);
                
                sum1 += z;
                sum2 += z * z;
                sum3 += sqrt(fabs(z));
                
                /* Keep variables alive across loop iterations */
                keep_alive[k % 10] = sum1 + sum2 + sum3;
            }
            
            acc1 += sum1;
            acc2 += sum2;
            acc3 += sum3;
            
            /* Another inner loop with different live ranges */
            for (l = 0; l < 20; l++) {
                double a = acc1 * l;
                double b = acc2 / (l + 1);
                double c = acc3 + l;
                
                /* Force register pressure with many calculations */
                double r1 = a * b + c;
                double r2 = a * c - b;
                double r3 = b * c + a;
                double r4 = r1 * r2 / (r3 + 1.0);
                double r5 = r2 * r3 - r1;
                double r6 = r3 * r1 + r2;
                
                acc4 += r4 + r5 + r6;
                
                /* Inline assembly with fixed register constraints */
                asm volatile (
                    "mov %[val1], %%eax\n\t"
                    "imul %[val2], %%eax\n\t"
                    "add %%eax, %[out]\n\t"
                    : [out] "+r" (acc4)
                    : [val1] "r" ((int)r4), [val2] "r" ((int)r5)
                    : "%eax", "cc", "memory"
                );
            }
        }
        
        /* Early return in some cases to create complex CFG */
        if (i % 100 == 0 && acc1 > 1000000.0) {
            return;
        }
    }
    
    global_temp = acc1 + acc2 + acc3 + acc4;
}

/* Function with complex switch statement creating many basic blocks */
int switch_stress(int value) {
    int result = 0;
    
    switch (value % SWITCH_CASES) {
        case 0: {
            double a = value * 1.1;
            double b = value * 2.2;
            result = (int)(complex_expression(a, b, a+b, a-b, a*b));
            /* Fall through */
        }
        case 1:
            result += value * 3;
            /* Intentional fall through */
        case 2: {
            int temp = value * 5;
            asm volatile (
                "movl %[in], %%ecx\n\t"
                "leal (%%ecx,%%ecx,2), %%eax\n\t"
                "movl %%eax, %[out]\n\t"
                : [out] "=r" (result)
                : [in] "r" (temp)
                : "%eax", "%ecx", "cc"
            );
            break;
        }
        case 3:
            result = value << 2;
            break;
        case 4: {
            volatile int x = value;
            for (int i = 0; i < 10; i++) {
                x = x * 3 + i;
            }
            result = x;
            break;
        }
        case 5:
        case 6:
            result = value * value;
            if (value > 100) {
                result += 1000;
            } else {
                result -= 1000;
            }
            break;
        case 7: {
            double arr[10];
            for (int i = 0; i < 10; i++) {
                arr[i] = value * i * 0.1;
            }
            result = (int)arr[5];
            break;
        }
        case 8:
            return value * 8;  /* Early return */
        case 9:
            result = value / 3;
            goto special_case;  /* Computed goto-like flow */
        case 10:
            result = value % 7;
            break;
        case 11: {
            /* Multiple inline asm statements competing for registers */
            int a = value, b = value * 2, c = value * 3;
            asm volatile (
                "mov %1, %%eax\n\t"
                "add %2, %%eax\n\t"
                "mov %%eax, %0\n\t"
                : "=r" (result)
                : "r" (a), "r" (b)
                : "%eax"
            );
            asm volatile (
                "mov %1, %%ebx\n\t"
                "imul %2, %%ebx\n\t"
                "add %%ebx, %0\n\t"
                : "+r" (result)
                : "r" (c), "r" (value)
                : "%ebx"
            );
            break;
        }
        case 12:
            result = -value;
            break;
        case 13:
            result = value | 0xFF;
            break;
        case 14:
            result = value & 0x7F;
            break;
        default:
            result = value;
    }
    
special_case:
    return result + global_seed;
}

/* Function with mixed data types stressing different register classes */
void mixed_types_stress(char *cdata, short *sdata, int *idata, 
                        long *ldata, float *fdata, double *ddata, int size) {
    /* Declare many variables at function scope */
    char c1, c2, c3;
    short s1, s2, s3;
    int i1, i2, i3, i4, i5;
    long l1, l2;
    float f1, f2, f3, f4;
    double d1, d2, d3, d4, d5, d6;
    
    /* Complex addressing modes */
    for (int i = 0; i < size; i++) {
        /* Multiple index calculations */
        int idx1 = i * 2;
        int idx2 = i * 3;
        int idx3 = i * 5;
        int idx4 = i * 7;
        
        /* Mixed type operations */
        c1 = cdata[idx1 % size];
        s1 = sdata[idx2 % size];
        i1 = idata[idx3 % size];
        l1 = ldata[idx4 % size];
        f1 = fdata[i];
        d1 = ddata[i];
        
        /* Many intermediate calculations */
        c2 = c1 * 2 + i;
        c3 = c2 - s1 / 256;
        
        s2 = s1 * 3 + c1;
        s3 = s2 - i1 / 65536;
        
        i2 = i1 * 5 + s1;
        i3 = i2 - l1 / 1000;
        i4 = i3 * 7 + c2;
        i5 = i4 / (s3 + 1);
        
        l2 = l1 * 11 + i3;
        
        f2 = f1 * 1.5f + i2;
        f3 = f2 * 2.0f - s2;
        f4 = f3 / (fabs(d1) + 1.0f);
        
        d2 = d1 * 2.5 + l1;
        d3 = d2 * 3.0 - f1;
        d4 = d3 / (i4 + 1.0);
        d5 = d4 * d2 + d3;
        d6 = d5 - d4 * 0.5;
        
        /* Store results back, creating write-after-read dependencies */
        cdata[i] = c3;
        sdata[i] = s3;
        idata[i] = i5;
        ldata[i] = l2;
        fdata[i] = f4;
        ddata[i] = d6;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Function with many parameters stressing calling convention */
long many_parameters(int a, int b, int c, int d, int e,
                     int f, int g, int h, int i, int j,
                     int k, int l, int m, int n, int o) {
    /* Use all parameters in complex ways */
    long sum = a + b + c + d + e;
    sum *= f + g + h + i + j;
    sum /= (k + l + m + n + o + 1);
    
    /* Additional calculations with many temporaries */
    int t1 = a * b - c;
    int t2 = d * e + f;
    int t3 = g * h / (i + 1);
    int t4 = j * k - l;
    int t5 = m * n + o;
    
    double dt1 = t1 * 1.1;
    double dt2 = t2 * 2.2;
    double dt3 = t3 * 3.3;
    double dt4 = t4 * 4.4;
    double dt5 = t5 * 5.5;
    
    /* Complex expression tree */
    double result = dt1 * dt2 + dt3 * dt4 - dt5;
    result = result * dt1 / (dt2 + 1.0) + dt3 * dt5;
    
    return sum + (long)result;
}

/* Main test driver */
int main() {
    /* Initialize large arrays with random data */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    
    srand(global_seed);
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 10000;
        long_data[i] = rand() % 1000000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    long total_checksum = 0;
    
    /* Warm-up iterations */
    for (int warmup = 0; warmup < 10; warmup++) {
        nested_loop_stress(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loop stress */
        nested_loop_stress(int_data, ARRAY_SIZE / 4);
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch statement stress */
        for (int i = 0; i < ARRAY_SIZE / 100; i++) {
            int_data[i] = switch_stress(int_data[i]);
        }
        
        /* Test 3: Mixed types stress */
        mixed_types_stress(char_data, short_data, int_data,
                          long_data, float_data, double_data,
                          ARRAY_SIZE / 2);
        
        /* Test 4: Many parameters stress */
        for (int i = 0; i < ARRAY_SIZE / 200; i++) {
            int base = int_data[i];
            long result = many_parameters(
                base, base+1, base+2, base+3, base+4,
                base+5, base+6, base+7, base+8, base+9,
                base+10, base+11, base+12, base+13, base+14
            );
            total_checksum += result;
        }
        
        /* Update global seed to change patterns */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Compute final checksum */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total_checksum += int_data[i] + char_data[i] + short_data[i];
        total_checksum += long_data[i] + (long)float_data[i] + (long)double_data[i];
    }
    
    printf("Final checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    free(int_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
