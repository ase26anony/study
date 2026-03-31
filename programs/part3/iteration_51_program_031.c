#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define MAX_INNER_LOOPS 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
static void use_vla(int size) {
    double vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    /* Prevent optimization */
    asm volatile ("" : : "r"(vla) : "memory");
}

/* Another helper with alloca */
__attribute__((noinline))
static void use_alloca(int size) {
    double* dyn = (double*)alloca(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        dyn[i] = sqrt(i + 1.0) * log(i + 2.0);
    }
    asm volatile ("" : : "r"(dyn) : "memory");
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        double e = array[(outer + 4) % ARRAY_SIZE];
        double f = array[(outer + 5) % ARRAY_SIZE];
        
        /* Long dependency chain with mixed operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sin(t2) * cos(t1);
        double t4 = sqrt(fabs(t3)) + exp(t2 * 0.1);
        double t5 = t4 * t3 - t2 / t1;
        double t6 = t5 + pow(t4, 2.0) - log(fabs(t3) + 1.0);
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Continue dependency chain */
        double t7 = t6 * array[(outer + 10) % ARRAY_SIZE];
        double t8 = t7 + array[(outer + 20) % ARRAY_SIZE] * 0.5;
        double t9 = t8 - array[(outer + 30) % ARRAY_SIZE] / 2.0;
        double t10 = sin(t9) + cos(t8) * tan(t7 * 0.01);
        
        checksum += t10;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % MAX_INNER_LOOPS + 10;
        for (int i = 0; i < inner_loops; i++) {
            int inner_inner = rand() % 20 + 5;
            for (int j = 0; j < inner_inner; j++) {
                /* Mixed integer and FP operations */
                int idx = (i * j + outer) % ARRAY_SIZE;
                double val = array[idx];
                
                /* Complex addressing modes */
                double* ptr1 = &array[(idx + j) % ARRAY_SIZE];
                double* ptr2 = &array[(idx * 2) % ARRAY_SIZE];
                double* ptr3 = &array[(idx / 2) % ARRAY_SIZE];
                
                *ptr1 = val * sin(j * 0.1) + cos(i * 0.05);
                *ptr2 = *ptr1 / (val + 1.0) * sqrt(fabs(*ptr1));
                *ptr3 = *ptr2 - *ptr1 * 0.5 + pow(val, 1.5);
                
                /* Integer arithmetic chain */
                int k = idx % 100;
                k = k * 3 + 7;
                k = k % 50 - 25;
                k = (k * k) / (abs(k) + 1);
                array[(idx + k) % ARRAY_SIZE] += 0.001 * k;
            }
            
            /* Function call as scheduling barrier */
            double rnd = (double)rand() / RAND_MAX;
            array[i % ARRAY_SIZE] *= (1.0 + sin(rnd * 3.14159));
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 1000) == 0;  /* 0.1% probability */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int i = 0; i < 100; i++) {
                double x = array[(outer + i) % ARRAY_SIZE];
                double y = array[(outer + i + 50) % ARRAY_SIZE];
                
                /* Heavy FP chain */
                double z = x * y + sin(x) * cos(y);
                z = z / (fabs(x - y) + 1.0);
                z = sqrt(z * z + 1.0) * log(z + 2.0);
                z = pow(z, 1.5) - exp(z * 0.01);
                
                cold_sum += z;
                
                /* Memory operations with different strides */
                array[(i * 7) % ARRAY_SIZE] += z * 0.01;
                array[(i * 13) % ARRAY_SIZE] -= z * 0.005;
                array[(i * 29) % ARRAY_SIZE] *= 1.0 + z * 0.001;
            }
            checksum += cold_sum * 0.001;
            
            /* Another assembly barrier in cold path */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 4: Mixed operations with barriers */
        double mix_acc = 0.0;
        for (int i = 0; i < 20; i++) {
            int idx = (outer * 31 + i * 17) % ARRAY_SIZE;
            mix_acc += array[idx] * i;
            
            /* Barrier every 5 iterations */
            if (i % 5 == 0) {
                asm volatile ("" ::: "memory");
            }
            
            /* Integer operations interleaved */
            int int_val = idx % 100;
            int_val = (int_val * 3 + 7) % 97;
            int_val = int_val * int_val - int_val;
            array[(idx + 1) % ARRAY_SIZE] += int_val * 0.0001;
        }
        checksum += mix_acc;
        
        /* Call helper functions between patterns */
        use_vla((outer % 20) + 10);
        use_alloca((outer % 15) + 5);
        
        /* More complex dependency chains */
        double chain_start = array[outer % ARRAY_SIZE];
        for (int step = 0; step < 15; step++) {
            chain_start = chain_start * 1.1 + sin(chain_start);
            chain_start = sqrt(fabs(chain_start)) + 0.5;
            chain_start = chain_start / (1.0 + fabs(cos(chain_start)));
            
            /* Store intermediate results */
            array[(outer + step) % ARRAY_SIZE] = 
                array[(outer + step) % ARRAY_SIZE] * 0.9 + chain_start * 0.1;
        }
        checksum += chain_start;
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += array[i] * (i % 7 + 1);
    }
    final_result = sin(final_result * 0.0001) * 1000.0;
    
    printf("Result: %f\n", final_result);
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
