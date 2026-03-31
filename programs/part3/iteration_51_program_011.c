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
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.01);
        sum += vla[i];
    }
    
    /* Inline assembly barrier */
    asm volatile ("" ::: "memory");
    
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int *arr = (int *)alloca(n * sizeof(int));
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        arr[i] = i * (i % 7);
        result += arr[i];
    }
    
    asm volatile ("" ::: "memory");
    return result;
}

int main(void) {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    int i, j, k;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        double temp = 0.0;
        
        /* PATTERN 1: Large dependency chain basic block */
        {
            double a = data[outer % ARRAY_SIZE];
            double b = data[(outer + 1) % ARRAY_SIZE];
            double c = data[(outer + 2) % ARRAY_SIZE];
            double d = data[(outer + 3) % ARRAY_SIZE];
            double e = data[(outer + 4) % ARRAY_SIZE];
            double f = data[(outer + 5) % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            a = b + c;
            d = a * e;
            f = d / (c + 1.0);
            a = sqrt(fabs(f));
            d = sin(a) * cos(b);
            f = d * tan(c * 0.01);
            a = f + exp(d * 0.1);
            b = log(fabs(a) + 1.0);
            c = pow(b, 2.0);
            d = c * M_PI;
            f = d / M_E;
            a = fmod(f, 3.14159);
            
            temp += a + b + c + d + f;
            
            /* Memory store with complex addressing */
            data[(outer * 7) % ARRAY_SIZE] = temp;
        }
        
        /* Call helper with VLA between patterns */
        checksum += use_vla((outer % 20) + 10);
        
        /* PATTERN 2: Nested loops with data-dependent inner bound */
        {
            int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
            double local_sum = 0.0;
            
            for (j = 0; j < inner_limit; ++j) {
                /* Mixed integer and FP in inner loop */
                int idx = (outer * j) % ARRAY_SIZE;
                double val = data[idx];
                
                /* Complex addressing modes */
                val += data[(idx + 1) % ARRAY_SIZE] * 0.5;
                val -= data[(idx + 2) % ARRAY_SIZE] * 0.3;
                val *= data[(idx + 3) % ARRAY_SIZE];
                val /= (data[(idx + 4) % ARRAY_SIZE] + 1.0);
                
                /* Integer arithmetic mixed in */
                int int_part = (int)val;
                int_part = int_part * (j % 17) + (j % 23);
                int_part = int_part / ((j % 5) + 1);
                
                val += sin(int_part * 0.01);
                local_sum += val;
                
                /* Store back with different index */
                data[(idx + j) % ARRAY_SIZE] = val;
            }
            
            temp += local_sum / inner_limit;
        }
        
        /* Call helper with alloca */
        checksum += use_alloca((outer % 15) + 5);
        
        /* PATTERN 3: Block with inline assembly barriers */
        {
            double x = data[outer % ARRAY_SIZE];
            double y = data[(outer + 10) % ARRAY_SIZE];
            double z = 0.0;
            
            /* Group 1 with barrier */
            x = x * y + sin(x);
            y = y / (cos(x) + 2.0);
            asm volatile ("" ::: "memory");
            
            /* Group 2 with barrier */
            z = sqrt(fabs(x * y));
            z = z * tan(y * 0.01);
            asm volatile ("" ::: "memory");
            
            /* Group 3 with barrier */
            x = exp(z * 0.1);
            y = log(fabs(x) + 1.0);
            asm volatile ("" ::: "memory");
            
            temp += x + y + z;
        }
        
        /* PATTERN 4: __builtin_expect with cold path */
        {
            int rare_condition = (outer == 42); /* Rare condition */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_sum = 0.0;
                for (k = 0; k < 100; ++k) {
                    cold_sum += sqrt(data[(outer + k) % ARRAY_SIZE]);
                    cold_sum *= 1.0001;
                    cold_sum -= sin(k * 0.01);
                }
                
                /* More complex cold path operations */
                double *ptr = &data[outer % ARRAY_SIZE];
                for (int m = 0; m < 20; ++m) {
                    *ptr = *ptr * 0.99 + cos(m * 0.1);
                    ptr = &data[(outer + m) % ARRAY_SIZE];
                }
                
                temp += cold_sum;
            } else {
                /* Hot path - simpler operations */
                temp += data[outer % ARRAY_SIZE] * 0.5;
            }
        }
        
        /* Final accumulation with memory barrier */
        checksum += temp;
        asm volatile ("" ::: "memory");
        
        /* Occasional function call as scheduling barrier */
        if (outer % 23 == 0) {
            checksum += sqrt(fabs(checksum));
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    /* Additional complex exit path */
    {
        double exit_sum = 0.0;
        for (i = 0; i < 100; ++i) {
            exit_sum += pow(data[i], 1.5);
            exit_sum = fmod(exit_sum, 1000.0);
        }
        printf("Exit sum: %f\n", exit_sum);
    }
    
    return 0;
}
