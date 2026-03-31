#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int *arr = (int *)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        arr[i] = i * i - i;
        sum += arr[i] % 17;
    }
    
    return sum;
}

int main() {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = data[outer % ARRAY_SIZE];
        double b = data[(outer + 1) % ARRAY_SIZE];
        double c = data[(outer + 2) % ARRAY_SIZE];
        double d = data[(outer + 3) % ARRAY_SIZE];
        double e = data[(outer + 4) % ARRAY_SIZE];
        double f = data[(outer + 5) % ARRAY_SIZE];
        
        /* Long dependency chain with mixed operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2)) + sin(e);
        double t4 = t3 * t3 - t2 * t1;
        double t5 = t4 / (f + 0.5) * cos(t3);
        double t6 = t5 + exp(t4 * 0.01);
        double t7 = log(fabs(t6) + 1.0) * t3;
        double t8 = t7 * t7 - t6 * t5 + t4;
        
        /* Memory barrier between dependency groups */
        asm volatile ("" ::: "memory");
        
        /* Second dependency chain */
        double u1 = t8 + a * b;
        double u2 = u1 - c / d;
        double u3 = u2 * u2 + e * f;
        double u4 = sin(u3) * cos(u2);
        double u5 = u4 / (t8 + 1.0) + sqrt(u3);
        
        checksum += u5;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % INNER_BASE + 10;
        for (int i = 0; i < inner_limit; i++) {
            int j_limit = rand() % 20 + 5;
            for (int j = 0; j < j_limit; j++) {
                /* Mixed operations with memory accesses */
                int idx = (i * j + outer) % ARRAY_SIZE;
                data[idx] = data[idx] * 1.01 + sin(j * 0.1);
                data[(idx + 1) % ARRAY_SIZE] += cos(i * 0.05);
                
                /* Integer operations */
                int int_val = i * j - i + j;
                data[idx] += (int_val % 13) * 0.1;
                
                /* Floating division - expensive operation */
                if (j > 0) {
                    data[idx] /= (1.0 + fabs(data[(idx - 1) % ARRAY_SIZE]));
                }
            }
            
            /* Inline assembly barrier in loop */
            if (i % 7 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Call helper with VLA between patterns */
        checksum += use_vla((outer % 20) + 5);
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (outer == 42 || outer == 77); /* Rare cases */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_sum += sqrt(data[k] * data[k] + 1.0);
                cold_sum -= log(fabs(data[k]) + 1.0);
                
                /* Dependency chain in cold path */
                double x = cold_sum;
                double y = x * x - x + 1.0;
                double z = y / (x + 2.0) * sin(x);
                cold_sum = z * 0.5 + cold_sum * 0.5;
                
                /* Memory barrier in cold path */
                if (k % 13 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            checksum += cold_sum * 0.01;
        }
        
        /* Pattern 4: Mixed integer/FP with pointer chasing */
        double *ptr = &data[outer % ARRAY_SIZE];
        for (int m = 0; m < 25; m++) {
            *ptr = *ptr * 1.5 + sin(*ptr);
            ptr = &data[((int)(*ptr)) % ARRAY_SIZE];
            
            /* Integer arithmetic */
            int int_op = (m * outer) % 97;
            int_op = int_op * int_op - int_op;
            *ptr += (int_op % 19) * 0.01;
        }
        
        /* Call helper with alloca */
        checksum += use_alloca((outer % 15) + 3) * 0.001;
        
        /* Pattern 5: Another large basic block with barriers */
        double v1 = data[outer % ARRAY_SIZE];
        double v2 = data[(outer + 10) % ARRAY_SIZE];
        
        v1 = v1 + v2 * 2.5;
        v2 = v1 / (v2 + 0.1) - sin(v1);
        
        asm volatile ("" ::: "memory");
        
        v1 = v1 * v1 - v2 * v2;
        v2 = sqrt(fabs(v1)) + cos(v2 * 3.14);
        
        asm volatile ("" ::: "memory");
        
        v1 = v1 + v2 * exp(v1 * 0.01);
        v2 = log(fabs(v2) + 1.0) * tan(v1 * 0.1);
        
        checksum += v1 + v2;
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += data[i] * 0.001;
    }
    final_result += checksum;
    
    printf("Result: %f\n", final_result);
    
    return 0;
}
