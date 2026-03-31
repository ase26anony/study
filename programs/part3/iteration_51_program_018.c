#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
void vla_helper(int size, int seed) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = (i * seed) % 100;
    }
    /* Use the VLA to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += vla[i];
    }
}

/* Another noinline helper with mixed operations */
__attribute__((noinline))
double compute_polynomial(double x, int terms) {
    double result = 0.0;
    double power = 1.0;
    for (int i = 0; i < terms; i++) {
        result += power * (i % 5 + 1);
        power *= x;
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
    return result;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    long long checksum = 0;
    
    /* Fill arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        double fp_acc = fp_array[outer % ARRAY_SIZE];
        int int_acc = int_array[outer % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        fp_acc = fp_acc + fp_array[(outer + 1) % ARRAY_SIZE];
        fp_acc = fp_acc * 1.2345;
        fp_acc = sin(fp_acc);
        fp_acc = fp_acc / (fp_array[(outer + 2) % ARRAY_SIZE] + 0.001);
        fp_acc = sqrt(fabs(fp_acc));
        
        /* Chain of dependent integer operations */
        int_acc = int_acc + int_array[(outer + 3) % ARRAY_SIZE];
        int_acc = int_acc * 3;
        int_acc = int_acc - int_array[(outer + 4) % ARRAY_SIZE];
        int_acc = int_acc % 997;
        int_acc = int_acc * int_acc;
        
        /* Mixed operations with memory access */
        for (int i = 0; i < 10; i++) {
            int idx = (outer + i) % ARRAY_SIZE;
            fp_array[idx] = fp_array[idx] * 0.99 + sin(int_array[idx] * 0.01);
            int_array[idx] = (int_array[idx] + i) % 1000;
        }
        
        /* Inline assembly barrier between operation groups */
        asm volatile("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % INNER_BASE + 10;
        for (int j = 0; j < inner_loops; j++) {
            double temp = 0.0;
            for (int k = 0; k < 5; k++) {
                int idx = (j * k + outer) % ARRAY_SIZE;
                temp += fp_array[idx] * int_array[idx];
                /* Complex addressing modes */
                int_array[idx] += (j % 2 == 0) ? k : -k;
                fp_array[(idx + 1) % ARRAY_SIZE] = 
                    fp_array[(idx + 1) % ARRAY_SIZE] * 0.95 + temp * 0.05;
            }
            checksum += (long long)(temp * 1000);
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (outer == 37 || outer == 73); /* Rare values */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            for (int i = 0; i < 20; i++) {
                cold_result += compute_polynomial(fp_array[i], 8);
                /* Memory operations with pointer arithmetic */
                int* ptr = &int_array[i];
                *ptr = (*ptr * 17 + 23) % 1000;
                ptr++;
                *ptr = (*ptr - 7) % 1000;
            }
            checksum += (long long)(cold_result * 100);
            
            /* Another assembly barrier */
            asm volatile("" ::: "memory");
            
            /* More cold path computations */
            for (int i = 0; i < 15; i++) {
                fp_array[i] = tan(fp_array[i] * 0.1);
                int_array[i] = (int_array[i] << 2) | (int_array[i] >> 6);
            }
        }
        
        /* Pattern 4: Mixed operations with function calls */
        for (int i = 0; i < 8; i++) {
            int idx = (outer * i) % ARRAY_SIZE;
            double rnd = (double)rand() / RAND_MAX;
            fp_array[idx] = fp_array[idx] * rnd + cos(int_array[idx] * 0.01);
            int_array[idx] = (int_array[idx] + rand() % 100) % 1000;
        }
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 20) + 10, int_array[outer % ARRAY_SIZE]);
        
        /* Additional complex block with alloca */
        {
            int alloca_size = (outer % 15) + 5;
            int* dyn_array = (int*)alloca(alloca_size * sizeof(int));
            for (int i = 0; i < alloca_size; i++) {
                dyn_array[i] = int_array[(outer + i) % ARRAY_SIZE] * i;
                checksum += dyn_array[i];
            }
        }
        
        /* Final dependency chain in the block */
        fp_acc = fp_acc * 2.0 - 1.0;
        int_acc = (int_acc * 3 + 7) % 1000;
        checksum += (long long)(fp_acc * 1000) + int_acc;
        
        /* Memory store to force scheduling considerations */
        volatile int* volatile_ptr = &int_array[outer % ARRAY_SIZE];
        *volatile_ptr = int_acc;
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional computation to ensure scheduler sees complex exit block */
    double final_fp = 0.0;
    for (int i = 0; i < 100; i++) {
        final_fp += fp_array[i] * int_array[i];
    }
    printf("Final FP result: %f\n", final_fp);
    
    return 0;
}
