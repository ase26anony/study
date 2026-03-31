#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static void vla_helper(int size, int iter) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = (i * iter) % 256;
    }
    asm volatile ("" ::: "memory");
}

/* Another helper with complex operations */
__attribute__((noinline))
static double compute_heavy(double a, double b, int count) {
    double result = a;
    for (int i = 0; i < count; i++) {
        result = sin(result) * cos(b) + sqrt(fabs(result));
        result = result * 0.99 + 0.01;
    }
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    volatile double checksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double chain_result = fp_data[0];
        int chain_int = int_data[0];
        
        /* Long chain of dependent FP operations */
        chain_result = chain_result + fp_data[1] * 1.1;
        chain_result = chain_result / (fp_data[2] + 0.5);
        chain_result = sqrt(fabs(chain_result));
        chain_result = chain_result * fp_data[3] - fp_data[4];
        chain_result = sin(chain_result) + cos(chain_result * 0.5);
        chain_result = chain_result * 2.0 / 3.0;
        
        /* Mixed with integer operations */
        chain_int = chain_int + int_data[1] * 2;
        chain_int = chain_int % 997 + int_data[2];
        chain_int = chain_int * 3 - int_data[3];
        chain_int = (chain_int << 2) | (chain_int >> 30);
        
        /* Memory operations in the chain */
        for (int i = 5; i < 20; i++) {
            chain_result += fp_data[i] * (i % 3 + 1);
            chain_int ^= int_data[i];
        }
        
        checksum += chain_result + chain_int;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = (rand() % 50) + 10;  /* Data-dependent */
        for (int i = 0; i < 5; i++) {
            double temp = fp_data[i];
            for (int j = 0; j < inner_limit; j++) {
                /* Complex addressing modes */
                temp = temp * fp_data[(i * j) % ARRAY_SIZE] 
                       + int_data[(i + j * 3) % ARRAY_SIZE];
                temp = temp / (1.0 + fabs(fp_data[(j * 7) % ARRAY_SIZE]));
                
                /* Memory store with barrier */
                if (j % 7 == 0) {
                    fp_data[(i * 11) % ARRAY_SIZE] = temp;
                    asm volatile ("" ::: "memory");
                }
            }
            checksum += temp;
        }
        
        /* Call VLA helper */
        vla_helper((outer % 20) + 10, outer);
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42);  /* Rarely true */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            for (int i = 0; i < 100; i++) {
                cold_result += compute_heavy(fp_data[i], fp_data[i+1], 3);
                cold_result = fmod(cold_result, 1000.0);
                
                /* Use alloca within loop */
                int* dynamic = (int*)alloca(sizeof(int) * 4);
                for (int k = 0; k < 4; k++) {
                    dynamic[k] = int_data[i + k] * k;
                    cold_result += dynamic[k];
                }
            }
            checksum += cold_result * 0.1;
        } else {
            /* Hot path - simpler but still complex */
            for (int i = 0; i < 30; i++) {
                checksum += fp_data[(outer + i) % ARRAY_SIZE] 
                          * int_data[(outer * i) % ARRAY_SIZE];
            }
        }
        
        /* Pattern 4: Mixed operations with function calls */
        double func_result = 0.0;
        for (int i = 0; i < 25; i++) {
            func_result += sqrt(fp_data[i * 2]) 
                         * sin(fp_data[i * 2 + 1]);
            
            /* Barrier every 5 iterations */
            if (i % 5 == 0) {
                asm volatile ("" ::: "memory");
            }
            
            /* Integer operations interleaved */
            int idx = (i * 13 + outer) % ARRAY_SIZE;
            int_data[idx] = (int_data[idx] * 1103515245 + 12345) & 0x7fffffff;
        }
        checksum += func_result;
        
        /* Another VLA call */
        vla_helper((outer % 15) + 5, outer * 2);
        
        /* Pattern 5: Pointer chasing with dependencies */
        double* ptr1 = &fp_data[0];
        int* ptr2 = &int_data[0];
        double ptr_result = 0.0;
        
        for (int i = 0; i < 40; i++) {
            ptr_result += *ptr1 * (*ptr2);
            ptr1 = &fp_data[(*ptr2) % ARRAY_SIZE];
            ptr2 = &int_data[((int)ptr_result) % ARRAY_SIZE];
            
            /* Complex integer math */
            *ptr2 = (*ptr2 * 6364136223846793005ULL + 1) & 0x7fffffff;
        }
        checksum += ptr_result;
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += fp_data[i] * 0.01 + int_data[i] * 0.001;
    }
    checksum += final_result;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
