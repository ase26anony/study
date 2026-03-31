#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define MAX_INNER 60

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size, double seed) {
    double vla[size];
    double result = seed;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = seed * i + (i % 7);
        result += vla[i] * 0.5;
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return result;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n, int base) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = base;
    
    for (int i = 0; i < n; ++i) {
        data[i] = (base + i) * 3;
        sum += data[i] % 17;
    }
    
    return sum;
}

int main(void) {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        double temp = 0.0;
        
        /* PATTERN 1: Large dependency chain basic block */
        {
            double a = array[outer % ARRAY_SIZE];
            double b = array[(outer + 1) % ARRAY_SIZE];
            double c = array[(outer + 2) % ARRAY_SIZE];
            double d = array[(outer + 3) % ARRAY_SIZE];
            double e = array[(outer + 4) % ARRAY_SIZE];
            double f = array[(outer + 5) % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            double t1 = a + b * c;
            double t2 = t1 / (d + 1.0);
            double t3 = sqrt(fabs(t2)) + sin(e * 0.01);
            double t4 = t3 * t3 - t2 * t1;
            double t5 = t4 / (f + 0.5) + cos(t3);
            double t6 = t5 * 2.5 + t4 * 0.75;
            double t7 = t6 - t5 / 3.0 + sqrt(t4);
            double t8 = sin(t7) * cos(t6) + tan(t5 * 0.1);
            double t9 = t8 * t7 / (t6 + 0.001) + log(fabs(t5) + 1.0);
            double t10 = t9 * array[(outer + 6) % ARRAY_SIZE] + 
                        array[(outer + 7) % ARRAY_SIZE] / (t9 + 0.1);
            
            temp += t10;
            
            /* Integer operations mixed in */
            int i1 = (int)(t10 * 100);
            int i2 = i1 * 3 + (outer % 17);
            int i3 = i2 / (abs(i1 % 13) + 1);
            int i4 = i3 * i2 - i1;
            temp += (double)(i4 % 100) * 0.01;
        }
        
        /* Insert VLA helper between patterns */
        temp += use_vla((outer % 20) + 5, temp);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_bound = rand() % MAX_INNER + 10;
            double loop_acc = 0.0;
            
            for (int i = 0; i < 5; ++i) {
                /* Data-dependent inner loop */
                for (int j = 0; j < (rand() % 30 + inner_bound); ++j) {
                    /* Complex addressing and operations */
                    int idx = (i * 17 + j * 13 + outer) % ARRAY_SIZE;
                    double val = array[idx];
                    
                    /* Mixed operations with dependencies */
                    val = val * 1.5 + sin(val * 0.01);
                    val = val / (cos(val * 0.02) + 2.0);
                    val = sqrt(fabs(val)) + val * 0.3;
                    
                    loop_acc += val * (j + 1);
                    
                    /* Integer operations on the side */
                    int mod = (int)val % 19;
                    loop_acc += (mod * 0.01);
                }
                
                /* Memory barrier in the middle of loops */
                asm volatile ("" ::: "memory");
            }
            
            temp += loop_acc * 0.1;
        }
        
        /* Use alloca helper */
        temp += use_alloca((outer % 15) + 3, (int)temp);
        
        /* PATTERN 3: Block with inline assembly barriers */
        {
            double b1 = array[outer * 3 % ARRAY_SIZE];
            double b2 = array[outer * 7 % ARRAY_SIZE];
            
            b1 = b1 + b2 * 2.5;
            b1 = b1 / (sqrt(fabs(b2)) + 1.0);
            
            /* First barrier */
            asm volatile ("" ::: "memory");
            
            b2 = sin(b1) * 3.14 + cos(b2);
            double b3 = b1 * b2 - b1 / (b2 + 0.1);
            
            /* Second barrier */
            asm volatile ("" ::: "memory");
            
            b3 = b3 + log(fabs(b1) + fabs(b2) + 1.0);
            b3 = b3 * array[(outer * 11) % ARRAY_SIZE];
            
            temp += b3;
        }
        
        /* PATTERN 4: __builtin_expect with cold path */
        {
            int rare_condition = (outer == 37 || outer == 73); /* Rare cases */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_sum = 0.0;
                for (int k = 0; k < 25; ++k) {
                    cold_sum += sqrt(array[(outer + k * 7) % ARRAY_SIZE]);
                    cold_sum = cold_sum * 1.1 - cos(cold_sum * 0.05);
                }
                
                /* More complex cold path operations */
                double x = cold_sum;
                for (int m = 0; m < 10; ++m) {
                    x = sin(x) + cos(x * m * 0.1);
                    x = x * x - x / (m + 2.0);
                }
                
                temp += x * 0.5;
            } else {
                /* Hot path - simpler operations */
                temp += array[outer % ARRAY_SIZE] * 0.01;
            }
        }
        
        /* Final complex block with function calls */
        {
            double f1 = temp;
            double f2 = array[(outer + 100) % ARRAY_SIZE];
            
            f1 = f1 + sqrt(fabs(f2)) * 2.0;
            f1 = sin(f1 * 0.01) + cos(f2 * 0.02);
            f1 = f1 * f1 - f2 / (f1 + 0.001);
            
            /* Call math functions as scheduling barriers */
            f1 = f1 + sin(f2) * 0.5;
            f1 = f1 + cos(temp) * 0.3;
            
            /* Integer remainder operation */
            int int_part = (int)f1;
            f1 = f1 + (double)(int_part % 23) * 0.01;
            
            temp = f1;
        }
        
        checksum += temp;
        
        /* Occasionally use VLA again */
        if (outer % 7 == 0) {
            checksum += use_vla(10, checksum);
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
