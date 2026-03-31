/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_accumulator = 0.0;

/* Complex expression with many intermediate values */
double complex_expression(double a, double b, double c, double d, double e) {
    double t1 = a * b + c;
    double t2 = d * e - a;
    double t3 = t1 / (t2 + 1.0);
    double t4 = sin(t1) * cos(t2);
    double t5 = t3 * t4 + tan(t1 + t2);
    double t6 = sqrt(fabs(t5)) + log(fabs(t3) + 1.0);
    double t7 = t6 * exp(-fabs(t4));
    double t8 = t7 + pow(t3, 2.0) + pow(t4, 3.0);
    double t9 = t8 / (1.0 + fabs(t5));
    double t10 = asin(fmin(0.99, fabs(t9))) + acos(fmin(0.99, fabs(t8)));
    
    /* Keep all temporaries alive across computation */
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4), 
                       "+r"(t5), "+r"(t6), "+r"(t7), "+r"(t8));
    
    return t9 + t10 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Function with deeply nested loops and many live ranges */
void nested_loop_pressure(double* arr, int size) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    double prod1 = 1.0, prod2 = 1.0, prod3 = 1.0;
    double diff1, diff2, diff3, diff4;
    
    for (int i = 0; i < size; i++) {
        /* Outer loop computations */
        double x = arr[i];
        double y = x * x - 2.0 * x + 1.0;
        
        for (int j = 0; j < 5; j++) {
            /* Middle loop with more computations */
            double z = y + j * 0.1;
            
            for (int k = 0; k < 3; k++) {
                /* Inner loop with register-intensive operations */
                double w = z * k + sin(z) * cos(y);
                double v = w * w - 2.0 * w * x + x * x;
                
                sum1 += v;
                sum2 += w;
                prod1 *= (v + 1.0);
                
                /* Complex conditional with early continue */
                if (v > 100.0) {
                    double temp = sqrt(v) * log(fabs(w) + 1.0);
                    sum3 += temp;
                    continue;
                }
                
                if (w < -50.0) {
                    double temp = exp(-fabs(w)) * cos(v);
                    sum4 += temp;
                    break;
                }
                
                /* More intermediate values */
                diff1 = v - w;
                diff2 = w - x;
                diff3 = x - y;
                diff4 = y - z;
                
                prod2 *= (diff1 + diff2 + 1.0);
                prod3 *= (diff3 + diff4 + 1.0);
            }
            
            /* Function call within loop creates call-clobbered conflicts */
            double result = complex_expression(x, y, z, sum1, sum2);
            arr[i] += result * 0.01;
        }
        
        /* Pointer aliasing to prevent optimization */
        double* alias = &arr[i];
        *alias = *alias * 0.99 + sum1 * 0.01;
    }
    
    /* Force all variables to be used at the end */
    global_accumulator += sum1 + sum2 + sum3 + sum4 + prod1 + prod2 + prod3 
                        + diff1 + diff2 + diff3 + diff4;
}

/* Function with complex control flow and switch statement */
int complex_control_flow(int x, int* results) {
    int ret = 0;
    
    /* Deep if-else chain */
    if (x < 10) {
        ret = x * 2;
        if (x < 5) {
            ret += 10;
            if (x < 2) {
                ret *= 3;
                goto early_exit;
            } else {
                ret -= 5;
            }
        } else {
            ret /= 2;
        }
    } else if (x < 20) {
        ret = x + 100;
        if (x < 15) {
            ret <<= 2;
        } else {
            ret >>= 1;
        }
    } else if (x < 30) {
        ret = x - 50;
        for (int i = 0; i < 3; i++) {
            ret += i * x;
            if (ret > 200) break;
        }
    } else {
        ret = 0;
    }
    
early_exit:
    
    /* Large switch statement with fall-through */
    switch (x % SWITCH_CASES) {
        case 0:
            ret += 1;
            /* fall through */
        case 1:
            ret *= 2;
            break;
        case 2:
            ret -= 3;
            /* fall through */
        case 3:
        case 4:
            ret /= 4;
            break;
        case 5:
            ret <<= 1;
            /* fall through */
        case 6:
            ret >>= 2;
            break;
        case 7:
            ret = ~ret;
            /* fall through */
        case 8:
            ret &= 0xFF;
            break;
        case 9:
            ret |= 0x80;
            /* fall through */
        case 10:
            ret ^= 0x55;
            break;
        case 11:
            ret = (ret * 3) / 2;
            break;
        case 12:
            ret = ret > 0 ? ret : -ret;
            break;
        case 13:
            ret = ret % 100;
            break;
        case 14:
            ret = ret * ret;
            break;
        default:
            ret = 0;
    }
    
    /* Loop with break/continue at different levels */
    for (int i = 0; i < 10; i++) {
        if (i == ret % 3) continue;
        
        for (int j = 0; j < 5; j++) {
            if (j == ret % 2) break;
            
            for (int k = 0; k < 3; k++) {
                if (k == ret % 4) goto inner_break;
                results[i * 5 + j] += ret * i * j * k;
            }
            inner_break:;
        }
        
        if (i == ret % 5) break;
    }
    
    return ret;
}

/* Function with inline assembly and register constraints */
void register_constraint_test(int* data, int size) {
    int a, b, c, d, e, f, g, h;
    
    for (int i = 0; i < size; i += 8) {
        /* Competing for specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i]), "r" (data[i+1])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i+2]), "r" (data[i+3])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl %2, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i+4]), "r" (data[i+5])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl %2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i+6]), "r" (data[i+7])
            : "%edx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movl %1, %%esi\n\t"
            "movl %2, %%edi\n\t"
            "addl %%esi, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (e)
            : "r" (a), "r" (b)
            : "%esi", "%edi", "memory"
        );
        
        asm volatile (
            "movl %1, %%r8d\n\t"
            "movl %2, %%r9d\n\t"
            "subl %%r9d, %%r8d\n\t"
            "movl %%r8d, %0\n\t"
            : "=r" (f)
            : "r" (c), "r" (d)
            : "%r8", "%r9", "memory"
        );
        
        /* Vector/SIMD operations */
        asm volatile (
            "movdqu %1, %%xmm0\n\t"
            "movdqu %2, %%xmm1\n\t"
            "paddd %%xmm1, %%xmm0\n\t"
            "movdqu %%xmm0, %0\n\t"
            : "=m" (data[i])
            : "m" (data[i]), "m" (data[i+4])
            : "%xmm0", "%xmm1", "memory"
        );
        
        /* Use all computed values */
        g = (a + b) * (c - d);
        h = (e | f) & (a ^ b);
        
        data[i] = a + b + c + d + e + f + g + h;
    }
}

/* Function with mixed data types */
void mixed_type_test(void) {
    char c1 = 'A', c2 = 'Z', c3;
    short s1 = 1000, s2 = 2000, s3;
    int i1 = 100000, i2 = 200000, i3;
    long l1 = 1000000L, l2 = 2000000L, l3;
    float f1 = 3.14159f, f2 = 2.71828f, f3;
    double d1 = 3.1415926535, d2 = 2.7182818284, d3;
    
    /* Operations requiring different register classes */
    c3 = c1 + c2 - 'B';
    s3 = s1 * s2 / 3;
    i3 = i1 << 3 | i2 >> 2;
    l3 = l1 % 17 + l2 % 23;
    f3 = f1 * f2 + f1 / f2 - f1;
    d3 = sin(d1) * cos(d2) + tan(d1 + d2);
    
    /* Address calculations with multiple indexing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i;
        array[(i * 3) % 100] += c3;
        array[(i * 7) % 100] += s3;
        array[(i * 11) % 100] += i3;
        array[(i * 13) % 100] += l3;
        array[(i * 17) % 100] += (int)f3;
        array[(i * 19) % 100] += (int)d3;
    }
    
    /* Force all variables to be live */
    asm volatile("" : "+r"(c3), "+r"(s3), "+r"(i3), "+r"(l3), "+r"(f3), "+r"(d3));
    global_accumulator += c3 + s3 + i3 + l3 + f3 + d3;
}

/* Function with many arguments */
int many_arguments(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   double f1, double f2, double f3, double f4) {
    /* Use all arguments in complex ways */
    int sum_int = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double sum_double = f1 + f2 + f3 + f4;
    
    for (int i = 0; i < 10; i++) {
        sum_int += (a1 * i + a2 / (i+1) - a3 % (i+2)) 
                 * (a4 << i | a5 >> i) 
                 + (a6 & i) * (a7 | i) 
                 - (a8 ^ i) + (a9 - i) * (a10 + i);
        
        sum_double += sin(f1 * i) * cos(f2 / (i+1)) 
                    + tan(f3 + i) * exp(f4 - i);
    }
    
    return sum_int + (int)sum_double;
}

/* Main test driver */
int main(void) {
    printf("Starting MCF stress test...\n");
    
    /* Initialize with random data */
    double* array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* results = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
        int_array[i] = rand() % 1000;
        results[i] = 0;
    }
    
    /* Warm-up iterations */
    printf("Warm-up phase...\n");
    for (int warmup = 0; warmup < 5; warmup++) {
        nested_loop_pressure(array, 1000);
        memory_barrier();
    }
    
    /* Main stress test */
    printf("Main stress phase...\n");
    uint64_t checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loop pressure */
        nested_loop_pressure(array, ARRAY_SIZE / 10);
        memory_barrier();
        
        /* Test 2: Complex control flow */
        for (int i = 0; i < ARRAY_SIZE / 100; i++) {
            results[i] = complex_control_flow(int_array[i], results);
        }
        memory_barrier();
        
        /* Test 3: Register constraints */
        register_constraint_test(int_array, ARRAY_SIZE / 20);
        memory_barrier();
        
        /* Test 4: Mixed types */
        mixed_type_test();
        memory_barrier();
        
        /* Test 5: Many arguments */
        int arg_result = many_arguments(
            iter, iter*2, iter*3, iter*4, iter*5,
            iter*6, iter*7, iter*8, iter*9, iter*10,
            array[iter], array[iter+1], array[iter+2], array[iter+3]
        );
        checksum += arg_result;
        
        /* Update checksum */
        for (int i = 0; i < 100; i++) {
            checksum += (uint64_t)(array[i] * 1000);
            checksum += int_array[i];
            checksum += results[i];
        }
        
        if (iter % 10 == 0) {
            printf("Iteration %d, checksum so far: %lu\n", iter, checksum);
        }
    }
    
    /* Final computation */
    double final_sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i] + int_array[i] + results[i];
    }
    
    checksum += (uint64_t)(final_sum * 1000);
    
    printf("Final checksum: %lu\n", checksum);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Cleanup */
    free(array);
    free(int_array);
    free(results);
    
    return 0;
}

/* Memory barrier function */
void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Additional function to create irreducible CFG */
void irreducible_cfg(int n) {
    void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int i = 0;
    
    label0:
        i += 1;
        goto *labels[i % 10];
    
    label1:
        i *= 2;
        if (i > n) goto label5;
        else goto label3;
    
    label2:
        i -= 3;
        goto label7;
    
    label3:
        i += 5;
        goto label9;
    
    label4:
        i /= 2;
        goto label0;
    
    label5:
        i %= 7;
        goto label2;
    
    label6:
        i <<= 1;
        goto label4;
    
    label7:
        i >>= 2;
        goto label6;
    
    label8:
        i |= 0xFF;
        goto label1;
    
    label9:
        i &= 0x7F;
        if (i < 100) goto label8;
        else return;
}
