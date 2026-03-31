#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
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
    int* data = (int*)alloca(n * sizeof(int));
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * i - i;
        result += data[i] % 7;
    }
    
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
        
        /* Chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2));
        double t4 = t3 * sin(t2);
        double t5 = t4 + cos(t3);
        
        /* Integer dependency chain mixed in */
        int i1 = (int)t5;
        int i2 = i1 * 37 - 19;
        int i3 = i2 % 17 + i1;
        int i4 = i3 ^ (i2 << 3);
        
        /* Memory operations with addressing modes */
        array[(outer + 4) % ARRAY_SIZE] = t5 + i4;
        checksum += t5 * 0.01 - i4 * 0.001;
        
        /* Scheduling barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % INNER_BASE + 10;
        for (int j = 0; j < inner_limit; ++j) {
            /* Complex addressing with multiple array indices */
            int idx1 = (outer * j) % ARRAY_SIZE;
            int idx2 = (outer + j * 7) % ARRAY_SIZE;
            int idx3 = (j * 13) % ARRAY_SIZE;
            
            /* Mixed integer/FP operations */
            double val1 = array[idx1] * array[idx2];
            double val2 = val1 + sqrt(array[idx3] + 1.0);
            int ival = (int)val2;
            
            /* More dependencies */
            array[idx1] = val2 * 0.9;
            array[idx2] = sin(val2);
            checksum += (ival % 5) * 0.0001;
            
            /* Occasionally insert barrier */
            if (j % 7 == 3) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Call helper with VLA between patterns */
        checksum += use_vla(outer % 20 + 5);
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 1000) == 0; /* 0.1% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path: complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += array[(outer + k) % ARRAY_SIZE] * 
                           array[(outer + k * 2) % ARRAY_SIZE];
                cold_sum = sqrt(fabs(cold_sum));
            }
            
            /* More barriers in cold path */
            asm volatile ("" ::: "memory");
            
            int* cold_data = (int*)alloca(50 * sizeof(int));
            for (int k = 0; k < 50; ++k) {
                cold_data[k] = (int)cold_sum * k;
                checksum += cold_data[k] % 11;
            }
            
            asm volatile ("" ::: "memory");
            checksum += cold_sum * 0.00001;
        }
        
        /* Pattern 4: Another complex block with function calls */
        double x = array[outer % ARRAY_SIZE];
        for (int m = 0; m < 5; ++m) {
            x = sin(x) + cos(x * m);
            x = x * x - sqrt(fabs(x));
            
            /* Call rand() as scheduling barrier */
            int r = rand() % 100;
            x += r * 0.01;
            
            /* Memory store with complex addressing */
            array[(outer + m * 11) % ARRAY_SIZE] = x;
        }
        
        checksum += x;
        
        /* Call alloca helper */
        checksum += use_alloca(outer % 15 + 3);
        
        /* Final barrier in outer loop */
        asm volatile ("" ::: "memory");
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    /* Force one more unlikely path at the end */
    if (__builtin_expect(checksum < -1000000.0, 0)) {
        /* This should almost never execute */
        double* temp = (double*)alloca(100 * sizeof(double));
        for (int i = 0; i < 100; ++i) {
            temp[i] = sqrt(checksum + i);
        }
        printf("Unlikely path taken!\n");
    }
    
    return 0;
}
