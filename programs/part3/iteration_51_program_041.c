#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * (rand() % 10);
        sum += data[i];
    }
    
    asm volatile("" ::: "memory");
    return sum;
}

int main() {
    double* array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
        int_array[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_total = 0;
    
    /* Primary outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency-chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2));
        double t4 = sin(t3) * cos(t2);
        double t5 = t4 * t4 + t3 * t3;
        double t6 = exp(log(fabs(t5) + 1.0));
        double t7 = t6 * M_PI / 180.0;
        
        /* Mixed integer operations */
        int x = int_array[outer % ARRAY_SIZE];
        int y = int_array[(outer + 1) % ARRAY_SIZE];
        int z = int_array[(outer + 2) % ARRAY_SIZE];
        
        int r1 = x + y * z;
        int r2 = r1 % (abs(y) + 1);
        int r3 = r2 * r2 - r1;
        int r4 = r3 / (abs(z) + 1) + x % 7;
        
        /* Memory operations with varying addressing */
        array[(outer + 4) % ARRAY_SIZE] = t7;
        int_array[(outer + 4) % ARRAY_SIZE] = r4;
        
        total_sum += t7;
        int_total += r4;
        
        /* Call VLA helper between patterns */
        double vla_result = use_vla((outer % 20) + 10);
        total_sum += vla_result;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = (rand() % INNER_BASE) + 10; /* Data-dependent */
        for (int j = 0; j < inner_bound; ++j) {
            /* Complex addressing within loop */
            int idx = (outer * j) % ARRAY_SIZE;
            double val = array[idx];
            
            /* Mixed operations in loop */
            val = val * 1.01 + sin(j * 0.01);
            val = sqrt(fabs(val)) + cos(val);
            
            /* Integer computations */
            int iidx = (idx + j) % ARRAY_SIZE;
            int ival = int_array[iidx];
            ival = (ival * 3 + j) % 100;
            
            /* Store results */
            array[idx] = val;
            int_array[iidx] = ival;
            
            total_sum += val;
            int_total += ival;
        }
        
        /* Pattern 3: Inline assembly barriers */
        double barrier_var = total_sum * 0.5;
        
        asm volatile("" ::: "memory");
        
        barrier_var = barrier_var * barrier_var + 1.0;
        barrier_var = sin(barrier_var) * cos(barrier_var);
        
        asm volatile("" ::: "memory");
        
        barrier_var = sqrt(fabs(barrier_var));
        barrier_var = log(barrier_var + 1.0);
        
        asm volatile("" ::: "memory");
        
        total_sum += barrier_var;
        
        /* Pattern 4: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* Rare condition */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += sqrt(array[(outer + k) % ARRAY_SIZE]);
                cold_sum *= 1.0001;
            }
            
            /* More complex math in cold path */
            cold_sum = exp(sin(cold_sum)) + cos(cold_sum * M_PI);
            total_sum += cold_sum;
            
            /* Use alloca in cold path */
            int alloca_res = use_alloca(50);
            int_total += alloca_res;
        }
        
        /* More dependency chains */
        double chain_start = array[outer % ARRAY_SIZE];
        for (int link = 0; link < 5; ++link) {
            chain_start = chain_start * 1.5 - sin(chain_start);
            chain_start = sqrt(fabs(chain_start)) + 0.1;
            asm volatile("" ::: "memory"); /* Barrier in chain */
        }
        total_sum += chain_start;
        
        /* Function calls as scheduling barriers */
        double rand_val = (double)rand() / RAND_MAX;
        total_sum += sqrt(rand_val);
        total_sum += sin(total_sum * 0.001);
    }
    
    /* Final checksum computation */
    double final_checksum = total_sum + int_total;
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %f\n", final_checksum);
    printf("Total sum: %f, Int total: %d\n", total_sum, int_total);
    
    free(array);
    free(int_array);
    
    return 0;
}
