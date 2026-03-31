#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper functions to prevent inlining */
__attribute__((noinline)) 
static void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 7);
    }
    asm volatile ("" ::: "memory");
}

__attribute__((noinline))
static double complex_fp_chain(double a, double b, double c, double d) {
    double t1 = a + b * c;
    double t2 = sin(t1) * d;
    double t3 = sqrt(fabs(t2)) + c;
    double t4 = t3 / (d + 1.0);
    return t4 * t2 - t1;
}

/* Main computation with scheduling stress */
int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    int int_data[ARRAY_SIZE];
    double fp_data[ARRAY_SIZE];
    volatile int checksum = 0;
    volatile double fp_checksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        fp_data[i] = (double)(rand() % 1000) / 3.14;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        int dep_chain_result = 0;
        double fp_dep_result = 0.0;
        
        for (int i = 1; i < ARRAY_SIZE - 10; i++) {
            /* Integer dependency chain */
            int a = int_data[i];
            int b = int_data[i+1] + outer;
            int c = a * b - int_data[i+2];
            int d = c % (abs(b) + 1);
            int e = (d << 3) | (d >> 5);
            int f = e + int_data[i+3] * int_data[i+4];
            dep_chain_result += f;
            
            /* Floating-point dependency chain */
            double fa = fp_data[i];
            double fb = fp_data[i+1] * 1.1;
            double fc = fa + fb * sin(fa);
            double fd = fc / (fabs(fb) + 0.5);
            double fe = sqrt(fabs(fd)) * cos(fb);
            fp_dep_result += fe;
            
            /* Memory store with addressing mode variation */
            int_data[i-1] = dep_chain_result % 1000;
            fp_data[i-1] = fp_dep_result / 1000.0;
        }
        
        /* Scheduling barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % 50 + 10;
        for (int j = 0; j < inner_loops; j++) {
            int inner_sum = 0;
            double inner_fp = 0.0;
            
            /* Data-dependent inner loop */
            int inner_bound = (rand() % 20) + 5;
            for (int k = 0; k < inner_bound; k++) {
                /* Mixed operations */
                int idx = (j * 17 + k * 31) % ARRAY_SIZE;
                int_data[idx] = int_data[idx] * 3 + k;
                fp_data[idx] = fp_data[idx] * 1.01 + sin(k * 0.1);
                
                /* Complex addressing */
                int* ptr = &int_data[(idx + outer) % ARRAY_SIZE];
                *ptr = (*ptr + j) % 777;
                
                inner_sum += *ptr;
                inner_fp += fp_data[idx];
            }
            
            checksum += inner_sum;
            fp_checksum += inner_fp;
            
            /* Another scheduling barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42) || (rand() % 1000 == 0);
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_result = 0.0;
            for (int i = 0; i < ARRAY_SIZE / 2; i += 2) {
                cold_result += complex_fp_chain(
                    fp_data[i], 
                    fp_data[i+1],
                    fp_data[(i*3) % ARRAY_SIZE],
                    fp_data[(i*7) % ARRAY_SIZE]
                );
                
                /* More barriers in cold path */
                asm volatile ("" ::: "memory");
                
                /* Integer operations with dependencies */
                int chain = int_data[i];
                for (int step = 0; step < 8; step++) {
                    chain = chain * 1103515245 + 12345;
                    chain = (chain >> 16) & 0x7FFF;
                }
                int_data[i] = chain;
            }
            fp_checksum += cold_result;
        }
        
        /* Pattern 4: VLA usage between patterns */
        use_vla((outer % 20) + 10);
        
        /* Pattern 5: Function calls as scheduling barriers */
        for (int i = 0; i < 5; i++) {
            double rand_val = (double)rand() / RAND_MAX;
            fp_checksum += sqrt(rand_val) * cos(rand_val * 3.14159);
            
            /* Memory operations between calls */
            int_data[(outer * 7 + i) % ARRAY_SIZE] = 
                (int)(fp_checksum * 1000) % 1000;
        }
        
        /* Final complex block with alloca */
        {
            int alloca_size = (outer % 16) + 8;
            int* dyn_array = (int*)alloca(alloca_size * sizeof(int));
            
            for (int i = 0; i < alloca_size; i++) {
                dyn_array[i] = int_data[i] * (i + 1);
                dyn_array[i] = (dyn_array[i] << (i % 4)) | 
                               (dyn_array[i] >> (32 - (i % 4)));
                
                /* Dependency across alloca'd elements */
                if (i > 0) {
                    dyn_array[i] += dyn_array[i-1] * 3;
                }
                
                checksum += dyn_array[i] % 255;
            }
            
            /* Force spill/reload */
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Final result computation to prevent elimination */
    double final_result = (double)checksum + fp_checksum;
    
    /* Use the results */
    printf("Final checksum: %d\n", checksum);
    printf("Final FP checksum: %f\n", fp_checksum);
    printf("Combined result: %f\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
