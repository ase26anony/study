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

/* Complex expression with many intermediate values */
double complex_expression(double a, double b, double c, double d, double e, double f) {
    double t1 = a * b + c;
    double t2 = d * e - f;
    double t3 = t1 / (t2 + 1.0);
    double t4 = sin(t1) * cos(t2);
    double t5 = t3 * t4 + a * d;
    double t6 = t5 - b * e + c * f;
    double t7 = sqrt(fabs(t6)) + log(fabs(t3) + 1.0);
    double t8 = t7 * t4 / (t1 + t2);
    double t9 = t8 + tan(t3) * atan(t5);
    double t10 = t9 * exp(-fabs(t6));
    
    /* Keep all temporaries alive across complex control flow */
    volatile double preserve = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    (void)preserve;
    
    return t10;
}

/* Function with deeply nested loops and many live ranges */
void nested_loop_stress(double* data, int size) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    double acc5 = 0.0, acc6 = 0.0, acc7 = 0.0, acc8 = 0.0;
    
    for (int i = 0; i < size; i++) {
        double x = data[i];
        
        /* First level of nesting */
        for (int j = 0; j < 5; j++) {
            double y = x * j + acc1;
            
            /* Second level */
            for (int k = 0; k < 3; k++) {
                double z = y * k + acc2;
                
                /* Third level with conditional */
                for (int l = 0; l < 2; l++) {
                    double w = z * l + acc3;
                    
                    /* Complex expression using all accumulators */
                    acc1 += complex_expression(x, y, z, w, acc4, acc5);
                    acc2 += complex_expression(y, z, w, x, acc6, acc7);
                    acc3 += complex_expression(z, w, x, y, acc8, acc1);
                    acc4 += complex_expression(w, x, y, z, acc2, acc3);
                    
                    /* Early continue creates complex CFG */
                    if (w > 1000.0) continue;
                    
                    acc5 += sin(w) * cos(x);
                    acc6 += tan(y) * atan(z);
                    
                    /* Break from inner loop */
                    if (acc6 > 10000.0) break;
                }
                
                /* Multiple accumulators create register pressure */
                acc7 += acc1 * acc2 - acc3 * acc4 + acc5 * acc6;
                acc8 += acc2 * acc3 - acc4 * acc5 + acc6 * acc1;
            }
            
            /* Conditional return creates exit edges */
            if (acc7 > 1e6) return;
        }
        
        /* Mix data types to stress different register classes */
        float f1 = (float)acc1;
        float f2 = (float)acc2;
        short s1 = (short)(acc3 * 100);
        short s2 = (short)(acc4 * 100);
        char c1 = (char)(acc5 * 10);
        char c2 = (char)(acc6 * 10);
        
        /* Use all mixed types in computation */
        data[i] = (double)(f1 * f2) + (double)(s1 * s2) + (double)(c1 * c2);
    }
    
    global_accumulator += acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
}

/* Function with switch statement creating complex CFG */
int switch_stress(int x, double* data, int size) {
    int result = 0;
    
    switch (x % 15) {
        case 0: {
            double temp = data[0];
            for (int i = 1; i < size; i++) {
                temp += data[i] * i;
                result += (int)temp;
            }
            break;
        }
        case 1:
            /* Fall through */
        case 2: {
            double temp = 1.0;
            for (int i = 0; i < size; i++) {
                temp *= data[i] + 1.0;
                if (temp > 1e100) {
                    result = 1;
                    break;
                }
            }
            if (temp > 0) result = 2;
            break;
        }
        case 3:
        case 4:
        case 5: {
            float f1 = 0.0f, f2 = 0.0f;
            for (int i = 0; i < size; i += 2) {
                f1 += (float)data[i];
                f2 += (float)data[i + 1];
                result += (int)(f1 * f2);
            }
            /* Fall through to case 6 */
        }
        case 6: {
            result *= 2;
            break;
        }
        case 7: {
            long l1 = 0, l2 = 0;
            for (int i = 0; i < size; i++) {
                l1 += (long)data[i];
                l2 += (long)(data[i] * data[i]);
            }
            result = (int)(l1 % 1000) + (int)(l2 % 1000);
            break;
        }
        case 8:
        case 9:
        case 10:
        case 11: {
            /* Multiple nested loops */
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    result += data[(i * j) % size] > 0.5 ? 1 : -1;
                }
            }
            break;
        }
        case 12: {
            /* Early return */
            return x * 2;
        }
        case 13: {
            /* Complex expression */
            result = (int)(sin(data[0]) * 1000 + cos(data[1]) * 1000);
            break;
        }
        case 14: {
            /* Default with loop */
            for (int i = 0; i < size; i++) {
                data[i] = sqrt(fabs(data[i]));
                result += (int)data[i];
            }
            break;
        }
    }
    
    return result;
}

/* Function with inline assembly creating register pressure */
void asm_register_pressure(double* data, int size) {
    double a, b, c, d, e, f, g, h;
    
    /* Initialize with data */
    a = data[0];
    b = data[1];
    c = data[2];
    d = data[3];
    e = data[4];
    f = data[5];
    g = data[6];
    h = data[7];
    
    /* Inline assembly with fixed register constraints */
    for (int i = 0; i < size / 8; i++) {
        /* Force use of specific registers */
        asm volatile (
            "movq %[a], %%rax\n\t"
            "movq %[b], %%rbx\n\t"
            "movq %[c], %%rcx\n\t"
            "movq %[d], %%rdx\n\t"
            "addq %%rbx, %%rax\n\t"
            "imulq %%rcx, %%rax\n\t"
            "subq %%rdx, %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=m" (data[i*8])
            : [a] "m" (a), [b] "m" (b), [c] "m" (c), [d] "m" (d)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* More assembly with different constraints */
        asm volatile (
            "movsd %[e], %%xmm0\n\t"
            "movsd %[f], %%xmm1\n\t"
            "movsd %[g], %%xmm2\n\t"
            "movsd %[h], %%xmm3\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "mulsd %%xmm2, %%xmm0\n\t"
            "subsd %%xmm3, %%xmm0\n\t"
            "movsd %%xmm0, %[result2]\n\t"
            : [result2] "=m" (data[i*8 + 1])
            : [e] "m" (e), [f] "m" (f), [g] "m" (g), [h] "m" (h)
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Update variables to create dependencies */
        a = data[i*8];
        b = data[i*8 + 1];
        c = a * b;
        d = c + e;
        e = d - f;
        f = e * g;
        g = f / h;
        h = g + a;
    }
}

/* Function with many arguments to stress register/stack passing */
double many_args_stress(double a1, double a2, double a3, double a4, double a5,
                        double a6, double a7, double a8, double a9, double a10,
                        double a11, double a12, double a13, double a14, double a15) {
    /* Use all arguments in complex expressions */
    double t1 = a1 + a2 * a3 - a4 / (a5 + 1.0);
    double t2 = a6 * sin(a7) + a8 * cos(a9) - a10 * tan(a11);
    double t3 = a12 * log(fabs(a13) + 1.0) + a14 * exp(-fabs(a15));
    
    /* Nested conditional */
    if (t1 > t2) {
        if (t2 > t3) {
            t1 = t3 * t2 - t1;
        } else if (t1 > t3) {
            t2 = t1 * t3 - t2;
        } else {
            t3 = t2 * t1 - t3;
        }
    } else {
        if (t1 > t3) {
            t2 = t3 * t1 - t2;
        } else if (t2 > t3) {
            t1 = t2 * t3 - t1;
        } else {
            t3 = t1 * t2 - t3;
        }
    }
    
    /* Loop with all variables live */
    for (int i = 0; i < 10; i++) {
        t1 += a1 * i - a2 / (i + 1);
        t2 += a3 * sin(i) - a4 * cos(i);
        t3 += a5 * log(i + 1) - a6 * exp(-i);
        
        /* Early continue */
        if (t1 > 1000) continue;
        
        t1 += a7 * t2 - a8 * t3;
        t2 += a9 * t1 - a10 * t2;
        t3 += a11 * t2 - a12 * t1;
        
        /* Break */
        if (t3 > 10000) break;
    }
    
    return t1 + t2 + t3 + a13 + a14 + a15;
}

/* Function with pointer aliasing to prevent optimization */
void pointer_aliasing_stress(double* data, int size) {
    double* ptr1 = data;
    double* ptr2 = data + size / 2;
    double* ptr3 = data + size / 4;
    double* ptr4 = data + 3 * size / 4;
    
    volatile double* vptr = data; /* volatile pointer */
    
    for (int i = 0; i < size / 4; i++) {
        /* Create aliasing */
        *ptr1 = *ptr2 + *ptr3;
        *ptr2 = *ptr1 - *ptr4;
        *ptr3 = *ptr2 * *ptr1;
        *ptr4 = *ptr3 / (*ptr1 + 1.0);
        
        /* Use volatile to force memory access */
        *vptr = *ptr1 + *ptr2 + *ptr3 + *ptr4;
        
        /* Update pointers with overlap */
        ptr1++;
        ptr2--;
        ptr3 += 2;
        ptr4 -= 2;
        vptr = (double*)((uintptr_t)vptr ^ 0x1000); /* Force different cache line */
    }
}

/* Main test driver */
int main() {
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up iterations for profile feedback */
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        nested_loop_stress(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    double checksum = 0.0;
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call different stress functions */
        nested_loop_stress(data, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
        
        int switch_result = switch_stress(iter, data, ARRAY_SIZE / 20);
        checksum += switch_result;
        asm volatile("" ::: "memory");
        
        asm_register_pressure(data, ARRAY_SIZE / 8);
        asm volatile("" ::: "memory");
        
        /* Call with many arguments */
        double args_result = many_args_stress(
            data[0], data[1], data[2], data[3], data[4],
            data[5], data[6], data[7], data[8], data[9],
            data[10], data[11], data[12], data[13], data[14]
        );
        checksum += args_result;
        asm volatile("" ::: "memory");
        
        pointer_aliasing_stress(data, ARRAY_SIZE / 2);
        asm volatile("" ::: "memory");
        
        /* Update data to create new patterns */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = sin(data[i] * 0.01) + cos(iter * 0.1);
        }
    }
    
    /* Final checksum computation */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Global accumulator: %f\n", (double)global_accumulator);
    
    free(data);
    return 0;
}
