#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Prevent inlining to create scheduling barriers */
__attribute__((noinline)) 
void use_vla(int size) {
    /* Variable-length array forces stack adjustments */
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    
    /* Use the VLA to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += vla[i];
    }
}

/* Another noinline function with complex operations */
__attribute__((noinline))
double complex_math_sequence(double a, double b, double c) {
    double t1 = a * b + c;
    double t2 = sin(t1) * cos(b);
    double t3 = sqrt(fabs(t2)) + log(fabs(a) + 1.0);
    return t3 * t2 / (a + 1.0);
}

int main() {
    /* Initialize with fixed seed for reproducibility */
    srand(42);
    
    /* Large arrays for memory operations */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    double total_result = 0.0;
    long long int_checksum = 0;
    
    /* Outer loop - driver for scheduling context creation */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* PATTERN 1: Large basic block with dependency chains */
        double a = double_array[outer % ARRAY_SIZE];
        double b = double_array[(outer + 1) % ARRAY_SIZE];
        double c = double_array[(outer + 2) % ARRAY_SIZE];
        
        /* Long dependency chain with mixed operations */
        double d1 = a + b;
        double d2 = d1 * c;
        double d3 = d2 / (a + 1.0);
        double d4 = sin(d3) + cos(d2);
        double d5 = d4 * d3 - d2;
        double d6 = sqrt(fabs(d5)) + log(fabs(d4) + 1.0);
        double d7 = d6 * d5 / (d4 + 1.0);
        double d8 = d7 + d6 - d5;
        double d9 = d8 * exp(-fabs(d7));
        double d10 = d9 + sin(d8) * cos(d9);
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" ::: "memory");
        
        /* Continue the dependency chain */
        double d11 = d10 * 2.0 - d9;
        double d12 = d11 / (d10 + 0.5);
        double d13 = pow(d12, 2.0) + pow(d11, 1.5);
        double d14 = d13 * tan(d12) + atan(d13);
        
        total_result += d14;
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10;  /* Data-dependent */
        
        for (int i = 0; i < inner_limit; i++) {
            int idx = (outer * i) % ARRAY_SIZE;
            
            /* Mixed integer operations with dependencies */
            int x = int_array[idx];
            int y = int_array[(idx + 1) % ARRAY_SIZE];
            int z = int_array[(idx + 2) % ARRAY_SIZE];
            
            int r1 = x + y;
            int r2 = r1 * z;
            int r3 = r2 % (abs(x) + 1);
            int r4 = r3 - y + z;
            int r5 = r4 * r3 / (abs(r2) + 1);
            int r6 = r5 ^ r4 | r3;
            
            int_checksum += r6;
            
            /* Floating point in the same loop */
            double f1 = double_array[idx];
            double f2 = double_array[(idx + 3) % ARRAY_SIZE];
            double f3 = f1 * f2 + sin(f1) - cos(f2);
            
            total_result += f3;
        }
        
        /* Call VLA function between patterns */
        use_vla((outer % 20) + 5);
        
        /* PATTERN 3: Conditional with __builtin_expect */
        int condition = rand() % 1000;
        
        /* Cold path prediction */
        if (__builtin_expect(condition < 5, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            
            /* Another scheduling barrier */
            asm volatile ("" ::: "memory");
            
            for (int k = 0; k < 25; k++) {
                int idx1 = (outer + k * 7) % ARRAY_SIZE;
                int idx2 = (outer + k * 13) % ARRAY_SIZE;
                
                /* Complex dependency chain in cold path */
                double val1 = double_array[idx1];
                double val2 = double_array[idx2];
                
                double chain1 = val1 * val2;
                double chain2 = chain1 + sin(val1) * cos(val2);
                double chain3 = sqrt(fabs(chain2)) * exp(-fabs(chain1));
                double chain4 = chain3 / (val1 + val2 + 1.0);
                double chain5 = pow(chain4, 1.5) + log(fabs(chain3) + 1.0);
                
                cold_result += chain5;
            }
            
            total_result += cold_result * 0.01;
            
            /* Use alloca for dynamic allocation in cold path */
            int* dynamic = (int*)alloca(sizeof(int) * 10);
            for (int m = 0; m < 10; m++) {
                dynamic[m] = m * condition;
                int_checksum += dynamic[m];
            }
        }
        
        /* PATTERN 4: Mixed operations with memory accesses */
        for (int j = 0; j < 15; j++) {
            /* Pointer arithmetic with different addressing modes */
            int* ptr = &int_array[(outer + j * 31) % ARRAY_SIZE];
            double* dptr = &double_array[(outer + j * 17) % ARRAY_SIZE];
            
            /* Load and immediate operations */
            int loaded = *ptr;
            *ptr = loaded + j * 3 - outer;
            
            double dloaded = *dptr;
            *dptr = dloaded * 1.1 + sin(j * 0.1) * 0.5;
            
            /* More arithmetic */
            int_checksum += loaded * j;
            total_result += dloaded * j * 0.01;
        }
        
        /* Final scheduling barrier in each outer iteration */
        asm volatile ("" ::: "memory");
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    printf("Integer checksum: %lld\n", int_checksum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    
    return 0;
}
