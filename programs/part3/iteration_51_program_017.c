#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline)) 
void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    /* Memory barrier to prevent reordering */
    asm volatile ("" ::: "memory");
}

/* Another helper with complex operations */
__attribute__((noinline))
double complex_math(double a, double b, double c) {
    double result = sin(a) * cos(b) + tan(c);
    result = sqrt(fabs(result)) + log(fabs(result) + 1.0);
    return result;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    double array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
        int_array[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    long long int_checksum = 0;
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Long chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sin(t2) * cos(t1);
        double t4 = sqrt(fabs(t3)) + log(fabs(t2) + 1.0);
        double t5 = t4 * t3 - t2 / t1;
        double t6 = tan(t5) + atan(t4);
        double t7 = t6 * exp(-fabs(t5));
        double t8 = t7 + pow(t6, 2.0) - pow(t5, 1.5);
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" ::: "memory");
        
        /* Continue the dependency chain */
        double t9 = t8 * 0.5 + t7 * 0.3 - t6 * 0.2;
        double t10 = sin(t9) + cos(t9 * 2.0) + tan(t9 / 3.0);
        double t11 = t10 * array[(outer + 10) % ARRAY_SIZE];
        double t12 = t11 / (array[(outer + 20) % ARRAY_SIZE] + 0.001);
        
        total_sum += t12;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10;  /* Data-dependent */
        for (int i = 0; i < inner_limit; i++) {
            int idx = (outer * i) % ARRAY_SIZE;
            
            /* Mixed integer operations with dependencies */
            int val1 = int_array[idx];
            int val2 = int_array[(idx + 1) % ARRAY_SIZE];
            int val3 = val1 + val2 * 3;
            int val4 = val3 % 97 + val2 / 5;
            int val5 = (val4 << 3) | (val3 & 0xFF);
            int val6 = val5 ^ val4 + val3 * val2;
            
            /* Memory store with addressing mode variation */
            int_array[(idx + i) % ARRAY_SIZE] = val6;
            
            /* Floating-point in the same loop */
            double f1 = array[idx];
            double f2 = array[(idx + 2) % ARRAY_SIZE];
            double f3 = f1 * f2 - sqrt(fabs(f1 - f2));
            array[idx] = f3 + sin(f2) * 0.1;
            
            /* Another scheduling barrier */
            if (i % 7 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 10000) == 0;  /* Rare condition */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; k++) {
                double x = (double)k / 100.0;
                cold_sum += sin(x * M_PI) * cos(x * 2.0 * M_PI);
                cold_sum = sqrt(fabs(cold_sum)) + log(fabs(cold_sum) + 1.0);
                
                /* More dependency chains */
                double y = cold_sum * 0.5;
                double z = y * y - y + 1.0;
                cold_sum = z / (y + 0.001) * tan(z);
                
                /* Memory access pattern */
                array[(outer + k) % ARRAY_SIZE] += cold_sum * 0.01;
            }
            total_sum += cold_sum;
            
            /* Call to function with VLA in cold path */
            use_vla(50 + (rand() % 20));
        }
        
        /* Pattern 4: Mixed operations with function calls */
        for (int j = 0; j < 5; j++) {
            int idx = (outer + j * 13) % ARRAY_SIZE;
            
            /* Function call as scheduling barrier */
            double func_result = complex_math(
                array[idx],
                array[(idx + 5) % ARRAY_SIZE],
                array[(idx + 10) % ARRAY_SIZE]
            );
            
            /* Integer operations around function call */
            int base = int_array[idx];
            int modified = base * 3 + (base % 17) - (base / 23);
            int_array[idx] = modified ^ (int)(func_result * 1000);
            
            /* Update floating array */
            array[idx] = func_result * 0.5 + array[idx] * 0.5;
        }
        
        /* Call helper with VLA between patterns */
        use_vla(20 + (outer % 10));
        
        /* More integer dependency chains */
        int chain_start = int_array[outer % ARRAY_SIZE];
        for (int chain = 0; chain < 8; chain++) {
            chain_start = chain_start * 3 + 1;
            chain_start = (chain_start % 1023) ^ (chain_start >> 5);
            chain_start = chain_start - (chain_start / 7) * 7;
        }
        int_array[outer % ARRAY_SIZE] = chain_start;
        int_checksum += chain_start;
    }
    
    /* Final computation to prevent elimination */
    double final_result = total_sum / (OUTER_LOOPS + 1);
    int_checksum = int_checksum % 1000000007;
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f, Checksum: %lld\n", final_result, int_checksum);
    
    /* Additional complex cleanup pattern */
    {
        /* Use alloca within a block */
        int dynamic_size = 100 + (rand() % 50);
        int* dynamic_array = (int*)alloca(dynamic_size * sizeof(int));
        
        for (int i = 0; i < dynamic_size; i++) {
            dynamic_array[i] = i * i - i + 1;
            if (i % 3 == 0) {
                dynamic_array[i] = dynamic_array[i] ^ (dynamic_array[i] >> 2);
            }
        }
        
        /* Process dynamic array */
        int local_sum = 0;
        for (int i = 0; i < dynamic_size; i++) {
            local_sum += dynamic_array[i] * (i % 5 + 1);
        }
        printf("Dynamic sum: %d\n", local_sum);
    }
    
    return 0;
}
