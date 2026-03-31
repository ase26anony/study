#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline))
void vla_helper(int size, int seed) {
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (i * seed) % 256;
    }
    /* Use the VLA to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += vla[i];
    }
    (void)sum;
}

/* Another helper with complex operations */
__attribute__((noinline))
double fp_compute(double a, double b, double c) {
    double t1 = a * b + c;
    double t2 = sin(t1) * cos(b);
    double t3 = sqrt(fabs(t2)) + 1.0;
    return t3 * t3 - 2.0 * t2;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    double total_result = 0.0;
    long long int_checksum = 0;
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        
        /* PATTERN 1: Large dependency-chain basic block */
        double chain_result = fp_array[0];
        for (int i = 1; i < 50; ++i) {
            /* Long dependency chain with mixed operations */
            double a = fp_array[i] * 1.1;
            double b = sin(a) + 2.0;
            double c = b * fp_array[i-1];
            double d = sqrt(fabs(c)) + 3.14;
            double e = d / (fp_array[i] + 1.0);
            chain_result = chain_result * 0.9 + e;
            
            /* Integer chain in parallel */
            int x = int_array[i] + 777;
            int y = x * 3 - 456;
            int z = y % 123 + int_array[i-1];
            int_checksum += z * (i + 1);
        }
        total_result += chain_result;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Call VLA helper between patterns */
        vla_helper((rand() % 64) + 16, outer);
        
        /* PATTERN 2: Nested loops with data-dependent inner bound */
        for (int i = 0; i < 20; ++i) {
            int inner_bound = (rand() % 50) + 10; /* Data-dependent */
            for (int j = 0; j < inner_bound; ++j) {
                /* Mixed memory accesses with varying addressing */
                int idx = (i * 17 + j * 13) % ARRAY_SIZE;
                double temp = fp_array[idx] * 2.0;
                
                /* Complex addressing modes */
                fp_array[(idx + 1) % ARRAY_SIZE] = temp + sin(fp_array[idx]);
                int_array[(idx + 2) % ARRAY_SIZE] = 
                    (int_array[idx] * 3 + int_array[(idx + 1) % ARRAY_SIZE]) % 1000;
                
                /* Function call as scheduling barrier */
                double computed = fp_compute(temp, fp_array[idx], (double)j);
                total_result += computed * 0.01;
            }
        }
        
        /* Another assembly barrier */
        asm volatile("" ::: "memory");
        
        /* PATTERN 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* 0.01% probability */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            for (int i = 0; i < ARRAY_SIZE; i += 4) {
                /* Unrolled complex FP chain */
                double a1 = fp_array[i] * fp_array[i+1];
                double a2 = fp_array[i+2] / (fp_array[i+3] + 1.0);
                double a3 = sin(a1) * cos(a2);
                double a4 = sqrt(a1 * a1 + a2 * a2);
                cold_result += a3 + a4;
                
                /* Memory store with barrier */
                fp_array[i] = a3;
                asm volatile("" ::: "memory");
                fp_array[i+1] = a4;
            }
            total_result += cold_result * 0.001;
            
            /* Additional VLA in cold path */
            int cold_vla_size = 32 + (rand() % 32);
            int cold_vla[cold_vla_size];
            for (int i = 0; i < cold_vla_size; ++i) {
                cold_vla[i] = int_array[i % ARRAY_SIZE] * i;
            }
            /* Use VLA */
            for (int i = 0; i < cold_vla_size; ++i) {
                int_checksum += cold_vla[i];
            }
        } else {
            /* Hot path - simpler operations */
            for (int i = 0; i < 100; ++i) {
                int idx = (outer * i) % ARRAY_SIZE;
                total_result += fp_array[idx] * 0.0001;
                int_checksum += int_array[idx];
            }
        }
        
        /* PATTERN 4: Mixed operations with alloca */
        if (outer % 10 == 0) {
            /* Use alloca for dynamic stack allocation */
            int* dyn_array = (int*)alloca(sizeof(int) * 64);
            for (int i = 0; i < 64; ++i) {
                dyn_array[i] = int_array[i % ARRAY_SIZE] + outer;
                /* Complex integer math chain */
                dyn_array[i] = (dyn_array[i] * 3) % 65537;
                dyn_array[i] = dyn_array[i] ^ (dyn_array[i] >> 8);
                int_checksum += dyn_array[i];
            }
            
            /* FP operations interleaved */
            for (int i = 0; i < 64; i += 2) {
                double f1 = (double)dyn_array[i] / 256.0;
                double f2 = (double)dyn_array[i+1] / 512.0;
                total_result += sin(f1) * cos(f2);
            }
        }
        
        /* Final barrier in outer loop */
        asm volatile("" ::: "memory");
    }
    
    /* Ensure results are used */
    printf("Total result: %f\n", total_result);
    printf("Integer checksum: %lld\n", int_checksum);
    
    return 0;
}
