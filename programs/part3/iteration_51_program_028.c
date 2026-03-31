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
    /* VLA forces stack adjustments affecting scheduling */
    double vla_array[size];
    for (int i = 0; i < size; i++) {
        vla_array[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(vla_array) : "memory");
}

/* Another helper with alloca */
__attribute__((noinline))
void use_alloca(int size) {
    double* dyn_array = (double*)alloca(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        dyn_array[i] = sqrt(i + 1.0);
    }
    asm volatile("" : : "r"(dyn_array) : "memory");
}

/* Complex dependency chain in a single basic block */
__attribute__((noinline))
double complex_block(double* arr, int idx, int n) {
    double a, b, c, d, e, f, g, h, result;
    
    /* Long dependency chain with mixed operations */
    a = arr[idx] * 1.5;
    b = sin(a) + cos(arr[idx + 1]);
    c = b * 2.71828;
    d = c / (arr[idx + 2] + 1.0);
    
    /* Inline assembly as scheduling barrier */
    asm volatile("" ::: "memory");
    
    e = sqrt(fabs(d)) + arr[idx + 3];
    f = e * e - d * d;
    g = log(fabs(f) + 1.0);
    
    /* Another scheduling barrier */
    asm volatile("" ::: "memory");
    
    h = g * arr[idx + 4] / (arr[idx + 5] + 0.5);
    result = h + sin(h) * cos(h);
    
    return result;
}

/* Function with __builtin_expect cold path */
__attribute__((noinline))
double unlikely_path_operation(double* arr, int idx, int threshold) {
    double result = arr[idx];
    
    /* Hot path - usually taken */
    if (__builtin_expect(result < threshold, 1)) {
        for (int i = 0; i < 5; i++) {
            result = result * 1.1 + sin(result);
        }
    } 
    /* Cold path - complex operations if taken */
    else {
        /* This should rarely execute, but when it does,
           it creates new scheduling context */
        double temp[10];
        for (int i = 0; i < 10; i++) {
            temp[i] = sqrt(arr[idx + i] * arr[idx + i + 1]);
        }
        
        /* Complex dependency chain in cold path */
        double sum = 0.0;
        for (int i = 0; i < 9; i++) {
            sum += temp[i] * temp[i + 1] - cos(temp[i]);
        }
        
        /* Inline assembly barrier in cold path */
        asm volatile("" ::: "memory");
        
        result = sum / 10.0 + exp(result * 0.01);
    }
    
    return result;
}

int main() {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)(rand() % 1000) / 100.0 + 0.1;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency-chain basic block */
        for (int i = 0; i < ARRAY_SIZE - 20; i += 10) {
            double r1 = complex_block(data, i, 10);
            double r2 = complex_block(data, i + 5, 10);
            checksum += r1 * r2 - sin(r1 + r2);
        }
        
        /* Use VLA helper - affects stack/scheduling */
        use_vla(outer % 100 + 10);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 20; i++) {
            /* Data-dependent inner loop bound */
            int inner_bound = (rand() % INNER_BASE) + 10;
            
            for (int j = 0; j < inner_bound; j++) {
                /* Mixed operations within nested loop */
                int idx = (i * 17 + j * 13) % (ARRAY_SIZE - 5);
                
                double* ptr = &data[idx];
                double val1 = ptr[0] + ptr[1] * ptr[2];
                double val2 = ptr[3] - ptr[4] / (ptr[5] + 1.0);
                
                /* Memory store with dependency */
                ptr[0] = val1 * val2 + sin(val1) * cos(val2);
                
                /* Integer arithmetic mixed in */
                int int_val = (int)(val1 * 100) % 97;
                checksum += (int_val * 0.01) + val2;
            }
        }
        
        /* Use alloca helper */
        use_alloca(outer % 50 + 5);
        
        /* Pattern 3: Blocks with inline assembly barriers */
        {
            double barrier_sum = 0.0;
            for (int i = 0; i < ARRAY_SIZE - 1; i++) {
                double x = data[i];
                double y = data[i + 1];
                
                /* Group 1 of dependent operations */
                double t1 = x * y + sqrt(x * x + y * y);
                double t2 = sin(t1) * cos(t1);
                
                /* Scheduling barrier */
                asm volatile("" ::: "memory");
                
                /* Group 2 after barrier */
                double t3 = t2 * 0.5 + exp(-fabs(t2));
                double t4 = log(fabs(t3) + 1.0);
                
                /* Another barrier */
                asm volatile("" ::: "memory");
                
                /* Group 3 */
                barrier_sum += t4 * (i % 7 + 1);
            }
            checksum += barrier_sum / ARRAY_SIZE;
        }
        
        /* Pattern 4: __builtin_expect with rare cold path */
        /* Occasionally trigger cold path by making threshold very low */
        double threshold = (outer == OUTER_LOOPS / 2) ? -1000.0 : 10000.0;
        
        for (int i = 0; i < ARRAY_SIZE - 10; i += 5) {
            double res = unlikely_path_operation(data, i, (int)threshold);
            checksum += res * 0.001;
        }
        
        /* Additional mixed operations to stress scheduler queues */
        for (int i = 0; i < ARRAY_SIZE - 5; i++) {
            /* Integer arithmetic */
            int int_op = ((i * 3) % 17) + ((i * 7) % 13);
            int_op = (int_op * int_op) % 31;
            
            /* Floating point */
            double fp_op = data[i] * 1.2345 - data[i + 1] / 2.3456;
            fp_op = fp_op + sin(data[i + 2]) * cos(data[i + 3]);
            
            /* Memory operation */
            data[i] = fp_op * 0.9 + int_op * 0.01;
            
            /* Function call as scheduling barrier */
            checksum += sqrt(fabs(data[i])) * 0.001;
        }
    }
    
    /* Final result depends on all computations */
    printf("Final checksum: %.15f\n", checksum);
    
    /* Additional computation to ensure all paths matter */
    double final = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final += data[i] * (i % 3 + 1);
    }
    printf("Array sum: %.15f\n", final / ARRAY_SIZE);
    
    return 0;
}
