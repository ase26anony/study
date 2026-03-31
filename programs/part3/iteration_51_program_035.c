#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper functions to prevent inlining */
__attribute__((noinline)) void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size && i < 10; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" ::: "memory");
}

__attribute__((noinline)) double complex_fp_chain(double a, double b, double c, double d) {
    double t1 = a + b;
    double t2 = t1 * c;
    double t3 = t2 / d;
    double t4 = sqrt(fabs(t3));
    double t5 = sin(t4);
    return t5 * t3;
}

/* Main computation with scheduling-intensive patterns */
int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    double total_checksum = 0.0;
    int int_checksum = 0;
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* PATTERN 1: Large dependency chain basic block */
        double chain_result = fp_data[0];
        for (int i = 1; i < 50; i++) {
            /* Long dependency chain with mixed operations */
            chain_result = chain_result + fp_data[i];
            chain_result = chain_result * 1.01;
            chain_result = chain_result - fp_data[i-1];
            chain_result = chain_result / 1.5;
            chain_result = sqrt(fabs(chain_result));
            
            /* Integer dependency chain interleaved */
            int idx = int_data[i] % ARRAY_SIZE;
            int_checksum += idx;
            int_checksum = int_checksum * 3;
            int_checksum = int_checksum - (idx / 2);
            int_checksum = int_checksum % 10007;
        }
        total_checksum += chain_result;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % 50 + 10;
        for (int j = 0; j < inner_loops; j++) {
            /* Complex addressing modes */
            int base_idx = (int_checksum + j) % ARRAY_SIZE;
            for (int k = 0; k < 5; k++) {
                int idx = (base_idx + k * 7) % ARRAY_SIZE;
                
                /* Mixed integer/FP operations */
                double temp = fp_data[idx] * 2.0;
                fp_data[idx] = sin(temp) + cos(temp);
                
                int_data[idx] = (int_data[idx] * 3 + k) % 1000;
                
                /* Memory access with pointer arithmetic */
                int *ptr = &int_data[idx];
                *ptr += (*ptr % 17);
            }
            
            /* Function call as scheduling barrier */
            double rand_val = (double)rand() / RAND_MAX;
            fp_data[base_idx] += complex_fp_chain(rand_val, 1.0, 2.0, 3.0);
        }
        
        /* VLA helper between patterns */
        use_vla((outer % 20) + 5);
        
        /* PATTERN 3: __builtin_expect with cold path */
        int rare_condition = (outer == 37 || outer == 73); /* Rare cases */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            for (int i = 0; i < 100; i++) {
                cold_result += fp_data[i % ARRAY_SIZE];
                cold_result = cold_result * 1.1;
                cold_result = fmod(cold_result, 100.0);
                
                /* Alloca in cold path */
                int *dynamic = alloca(sizeof(int) * 10);
                for (int d = 0; d < 10; d++) {
                    dynamic[d] = int_data[(i + d) % ARRAY_SIZE];
                    int_checksum += dynamic[d];
                }
            }
            total_checksum += cold_result * 0.5;
            
            /* Multiple assembly barriers in cold path */
            asm volatile ("" ::: "memory");
            asm volatile ("" ::: "memory");
        } else {
            /* Hot path - simpler operations */
            total_checksum += fp_data[outer % ARRAY_SIZE];
        }
        
        /* PATTERN 4: Mixed operations with barriers */
        for (int i = 0; i < 20; i++) {
            int idx1 = (int_checksum + i) % ARRAY_SIZE;
            int idx2 = (idx1 * 13) % ARRAY_SIZE;
            
            /* Integer arithmetic chain */
            int_checksum = int_checksum + int_data[idx1];
            int_checksum = int_checksum * 2 - int_data[idx2];
            int_checksum = int_checksum % 99991;
            
            /* FP arithmetic chain */
            fp_data[idx1] = fp_data[idx1] + fp_data[idx2];
            fp_data[idx1] = fp_data[idx1] * 0.99;
            
            /* Barrier every 5 iterations */
            if (i % 5 == 0) {
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Final VLA use in each outer iteration */
        use_vla(15);
    }
    
    /* Compute final result to prevent elimination */
    double final_result = total_checksum + (double)int_checksum;
    
    /* Additional complex block at end */
    for (int i = 0; i < 30; i++) {
        final_result = final_result * 1.01;
        final_result = sqrt(fabs(final_result));
        final_result = final_result + (double)(int_data[i] % 100);
    }
    
    printf("Final result: %f\n", final_result);
    printf("Checksums - FP: %f, Int: %d\n", total_checksum, int_checksum);
    
    return (final_result > 0) ? 0 : 1;
}
