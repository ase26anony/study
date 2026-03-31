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
        vla[i] = sin(i * 0.1) * cos(i * 0.05);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int *arr = (int*)alloca(n * sizeof(int));
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        arr[i] = i * i - i;
        result += arr[i] % 17;
    }
    
    asm volatile ("" ::: "memory");
    return result;
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Complex FP dependency chain */
        double t1 = a + b * c;
        double t2 = sqrt(fabs(t1)) + sin(d);
        double t3 = t2 * t2 - cos(t1);
        double t4 = t3 / (1.0 + fabs(t2));
        double t5 = exp(t4 * 0.1) + log(fabs(t3) + 1.0);
        double t6 = t5 * t4 - t3 * t2;
        double t7 = t6 + sqrt(t5 * t5 + t4 * t4);
        double t8 = sin(t7) * cos(t6) + tan(t5 * 0.01);
        
        checksum += t8;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10;
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < inner_limit; ++j) {
                /* Mixed integer/FP operations */
                int idx = (i * inner_limit + j) % ARRAY_SIZE;
                array[idx] = array[idx] * 1.01 + sin(j * 0.1);
                
                /* Integer arithmetic chain */
                int k = i * j + outer;
                k = (k * 13) % 17;
                k = k + (k >> 2) - (k << 1);
                k = k % 7 + 1;
                
                checksum += k * 0.001;
            }
            
            /* Call function with VLA between loop iterations */
            checksum += use_vla(20 + i) * 0.0001;
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 10000) == 0; /* 0.01% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int i = 0; i < 100; ++i) {
                cold_sum += sqrt(array[i % ARRAY_SIZE] + i);
                cold_sum *= 1.0001;
                
                /* More dependency chains */
                double x = cold_sum * 0.5;
                x = x * x - x + 1.0;
                x = 1.0 / (x + 0.001);
                cold_sum += x;
            }
            
            checksum += cold_sum * 0.00001;
            
            /* Use alloca in cold path */
            int alloca_res = use_alloca(30 + (rand() % 20));
            checksum += alloca_res * 0.000001;
        }
        
        /* Pattern 4: Another large basic block with barriers */
        double acc = 0.0;
        for (int i = 0; i < 10; ++i) {
            acc += array[(outer + i) % ARRAY_SIZE] * i;
        }
        
        /* Barrier between computation phases */
        asm volatile ("" ::: "memory");
        
        double acc2 = 0.0;
        for (int i = 0; i < 10; ++i) {
            acc2 += sqrt(acc + i) * cos(i * 0.3);
        }
        
        /* Another barrier */
        asm volatile ("" ::: "memory");
        
        double final_val = acc * 0.3 + acc2 * 0.7;
        checksum += final_val;
        
        /* Integer operations with dependencies */
        int int_val = outer;
        int_val = int_val * 3 + 1;
        int_val = (int_val % 97) * (int_val % 41);
        int_val = int_val - (int_val / 7) * 7;
        
        checksum += int_val * 0.0000001;
        
        /* Call VLA function periodically */
        if (outer % 10 == 0) {
            checksum += use_vla(15 + (outer % 5)) * 0.00001;
        }
        
        /* Memory operations with varying addressing */
        for (int i = 0; i < 3; ++i) {
            double *ptr = &array[(outer + i * 7) % ARRAY_SIZE];
            *ptr = *ptr * 0.99 + sin(checksum * 0.001);
            
            /* Pointer arithmetic */
            ptr += (i * 3) % 5;
            if (ptr < &array[ARRAY_SIZE]) {
                *ptr = cos(*ptr * 0.01);
            }
        }
    }
    
    /* Final computation to prevent elimination */
    checksum = fmod(fabs(checksum), 1000000.0);
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
