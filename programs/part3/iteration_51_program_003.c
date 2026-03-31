#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
static void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    /* Use the VLA to prevent optimization */
    asm volatile ("" : : "r"(vla) : "memory");
}

/* Another helper with mixed operations */
__attribute__((noinline))
static double complex_math(double a, double b, double c) {
    double t1 = a * b + c;
    double t2 = sin(t1) * cos(b);
    double t3 = sqrt(fabs(t2)) + 1.0;
    return t3 * t3 - t2;
}

int main(void) {
    /* Initialize with random data */
    srand(time(NULL));
    
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    double total_sum = 0.0;
    long int_checksum = 0;
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        double chain_result = fp_data[0];
        for (int i = 1; i < 50; i++) {
            /* Long dependency chain with mixed operations */
            chain_result = chain_result + fp_data[i];
            chain_result = chain_result * 1.01;
            chain_result = chain_result - fp_data[i-1];
            chain_result = chain_result / (fp_data[i] + 0.5);
            chain_result = sqrt(fabs(chain_result));
            
            /* Integer chain in parallel */
            int idx = int_data[i] % ARRAY_SIZE;
            int_checksum += int_data[idx];
            int_checksum = int_checksum * 1103515245 + 12345;
            int_checksum = int_checksum % 1000000007;
        }
        total_sum += chain_result;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % INNER_BASE) + 10;
        for (int j = 0; j < inner_limit; j++) {
            double acc = 0.0;
            for (int k = 0; k < 20; k++) {
                /* Memory accesses with varying addressing */
                int index = (j * 31 + k * 17) % ARRAY_SIZE;
                acc += fp_data[index] * int_data[index];
                
                /* More complex addressing */
                double* ptr = &fp_data[(index + 1) % ARRAY_SIZE];
                acc -= *ptr * 0.5;
                
                /* Function call as scheduling barrier */
                acc += complex_math(acc, fp_data[index], 1.0);
            }
            total_sum += acc / inner_limit;
        }
        
        /* Call VLA helper between patterns */
        use_vla((outer % 20) + 5);
        
        /* Pattern 3: Conditional with __builtin_expect */
        int rare_condition = (rand() % 10000) == 0;  /* 0.01% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operation sequence */
            double cold_acc = 0.0;
            for (int i = 0; i < 100; i++) {
                cold_acc += sin(fp_data[i]) * cos(fp_data[ARRAY_SIZE - i - 1]);
                cold_acc = fmod(cold_acc, 100.0);
                
                /* More barriers */
                asm volatile ("" ::: "memory");
                
                cold_acc += sqrt(fabs(cold_acc)) + log(fabs(cold_acc) + 1.0);
                
                /* Memory operations with pointer arithmetic */
                int* int_ptr = &int_data[i];
                cold_acc += *int_ptr;
                
                int_ptr += (i % 10);
                cold_acc -= *int_ptr;
            }
            total_sum += cold_acc;
            
            /* Another VLA in cold path */
            int vla_size = (rand() % 30) + 10;
            int cold_vla[vla_size];
            for (int i = 0; i < vla_size; i++) {
                cold_vla[i] = i * int_data[i % ARRAY_SIZE];
            }
            asm volatile ("" : : "r"(cold_vla) : "memory");
        } else {
            /* Hot path - simpler operations */
            double hot_acc = 0.0;
            for (int i = 0; i < 20; i++) {
                hot_acc += fp_data[i] * 2.0 - 1.0;
            }
            total_sum += hot_acc;
        }
        
        /* Pattern 4: Mixed operations with alloca */
        {
            int block_size = (outer % 40) + 10;
            int* dyn_array = (int*)alloca(block_size * sizeof(int));
            
            for (int i = 0; i < block_size; i++) {
                dyn_array[i] = int_data[(outer + i) % ARRAY_SIZE] * i;
                dyn_array[i] = (dyn_array[i] + 12345) % 1000;
                
                /* Floating point in the middle */
                total_sum += (double)dyn_array[i] / 1000.0;
            }
            
            /* Use the alloca'd array */
            asm volatile ("" : : "r"(dyn_array) : "memory");
        }
        
        /* Pattern 5: Loop with varying trip counts and complex exit */
        int dynamic_limit = (rand() % 60) + 20;
        double loop_acc = 0.0;
        int counter = 0;
        
        while (counter < dynamic_limit) {
            /* Mixed integer and FP in loop */
            int idx1 = (counter * 13) % ARRAY_SIZE;
            int idx2 = (counter * 7) % ARRAY_SIZE;
            
            loop_acc += (fp_data[idx1] * int_data[idx1]) / 
                       (fp_data[idx2] + 1.0);
            
            /* Modulo operation as potential scheduling barrier */
            loop_acc = fmod(loop_acc, 1000.0);
            
            /* Periodic barrier */
            if (counter % 7 == 0) {
                asm volatile ("" ::: "memory");
            }
            
            counter++;
            
            /* Early exit based on computation */
            if (loop_acc > 500.0 && (counter % 3 == 0)) {
                dynamic_limit += 5;  /* Change loop bound dynamically */
            }
        }
        total_sum += loop_acc;
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = total_sum + (double)int_checksum / 1000000000.0;
    
    /* Use result in non-eliminable way */
    printf("Result: %.15f\n", final_result);
    printf("Checksum: %ld\n", int_checksum);
    
    return (final_result > 0.0) ? 0 : 1;
}
