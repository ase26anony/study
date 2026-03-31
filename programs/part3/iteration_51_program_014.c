#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
void vla_helper(int size, int iter) {
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (i * iter) % 256;
    }
    
    /* Use the VLA to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += vla[i];
    }
    (void)sum;
}

/* Another noinline helper with mixed operations */
__attribute__((noinline))
double complex_math(double a, double b, double c, int iterations) {
    double result = a;
    for (int i = 0; i < iterations; ++i) {
        result = sin(result) * b + cos(result) * c;
        result = sqrt(fabs(result)) + 1.0;
    }
    return result;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    double array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
        int_array[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    long long int_sum = 0;
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        
        /* PATTERN 1: Large dependency-chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        a = b + c;
        d = a * d;
        a = d / (c + 1.0);
        d = sin(a) * cos(b);
        a = sqrt(fabs(d)) + tan(c);
        d = a * a - b * b;
        a = d / (c * c + 1.0);
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More dependent integer operations */
        int x = int_array[outer % ARRAY_SIZE];
        int y = int_array[(outer + 1) % ARRAY_SIZE];
        int z = int_array[(outer + 2) % ARRAY_SIZE];
        
        x = y + z;
        y = x * z;
        z = y % (x + 1);
        x = z * z - y * y;
        y = (x << 3) | (z & 0xFF);
        z = y * 7 + x / 3;
        
        total_sum += a + d;
        int_sum += x + y + z;
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 20) + 10, outer);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
        
        for (int i = 0; i < 5; ++i) { /* Fixed outer */
            for (int j = 0; j < inner_limit; ++j) { /* Variable inner */
                /* Mixed operations within nested loop */
                double* ptr = &array[(i * j) % ARRAY_SIZE];
                *ptr = *ptr * 1.01 + sin((double)j);
                
                int* iptr = &int_array[(i + j) % ARRAY_SIZE];
                *iptr = (*iptr * 3 + 7) % 1000;
                
                /* Memory access with varying addressing */
                array[(i * 17 + j * 13) % ARRAY_SIZE] += 
                    array[(j * 19) % ARRAY_SIZE] * 0.5;
            }
            
            /* Another assembly barrier inside loop */
            asm volatile("" ::: "memory");
        }
        
        /* PATTERN 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42); /* Rarely true */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operation sequence */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += complex_math(array[k], array[k+1], 
                                       array[k+2], 5);
            }
            
            /* Use alloca inside cold path */
            int* dynamic = (int*)alloca(sizeof(int) * 50);
            for (int k = 0; k < 50; ++k) {
                dynamic[k] = k * outer;
                cold_sum += dynamic[k];
            }
            
            total_sum += cold_sum * 0.01;
            
            /* Multiple assembly barriers in cold path */
            asm volatile("" ::: "memory");
            asm volatile("" ::: "memory");
        }
        
        /* PATTERN 4: Mixed loads/stores with pointer chasing */
        double* dp = array;
        int* ip = int_array;
        
        for (int i = 0; i < 100; ++i) {
            *dp = *dp + *ip * 0.5;
            *(dp + 1) = sin(*dp) * cos(*(dp + 2));
            dp += 3;
            ip += 2;
            
            /* Interleave integer math */
            int_sum += (*ip % 17) * (*ip / 13);
        }
        
        /* Final VLA call in iteration */
        vla_helper((outer % 15) + 5, outer * 2);
    }
    
    /* Ensure result depends on all computations */
    double final_result = total_sum + int_sum;
    
    /* Mix in some trig functions as scheduling barriers */
    final_result = sin(final_result) + cos(int_sum % 100);
    final_result = sqrt(fabs(final_result));
    
    printf("Final result: %f\n", final_result);
    printf("Checksum: %lld\n", int_sum);
    
    return 0;
}
