#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * i - i;
        sum += data[i] % 17;
    }
    
    asm volatile ("" ::: "memory");
    return sum;
}

int main(void) {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Primary outer loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        double e = array[(outer + 4) % ARRAY_SIZE];
        
        /* Chain of dependent FP operations */
        double t1 = a + b * c;
        double t2 = t1 / (d + 1.0);
        double t3 = sqrt(fabs(t2)) + sin(e);
        double t4 = t3 * t3 - t2 * t1;
        double t5 = t4 / (a + b + c + d + e);
        
        /* Integer dependency chain mixed in */
        int i1 = (int)t1;
        int i2 = i1 * 7 - 13;
        int i3 = i2 % 31 + i1;
        int i4 = i3 * i3 - i2 * i2;
        
        /* Memory operations with addressing */
        array[(outer + 5) % ARRAY_SIZE] = t5;
        array[(outer + 6) % ARRAY_SIZE] = (double)i4;
        
        checksum += t5 + i4;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % INNER_BASE + 10;
        for (int i = 0; i < inner_limit; ++i) {
            /* Complex addressing */
            int idx1 = (outer * i) % ARRAY_SIZE;
            int idx2 = (outer + i * 7) % ARRAY_SIZE;
            int idx3 = (i * 13) % ARRAY_SIZE;
            
            /* Mixed operations */
            double val1 = array[idx1] * array[idx2];
            double val2 = val1 + sqrt(array[idx3]);
            int ival = (int)val2;
            
            /* More dependencies */
            array[idx1] = val2 * 0.9;
            array[idx2] = sin(val2);
            array[idx3] = (double)(ival % 100);
            
            checksum += val1 + val2 + ival;
        }
        
        /* Call helper with VLA */
        double vla_result = use_vla((outer % 20) + 5);
        checksum += vla_result;
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* 0.01% probability */
        
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                double x = array[(outer + k) % ARRAY_SIZE];
                double y = array[(outer + k + 50) % ARRAY_SIZE];
                
                /* Long dependency chain in cold path */
                double r1 = x * y + sin(x) * cos(y);
                double r2 = r1 * r1 - x * y;
                double r3 = sqrt(fabs(r2)) + log(fabs(r1) + 1.0);
                double r4 = r3 / (x + y + 1.0);
                
                cold_sum += r4;
                
                /* Memory barrier in cold path */
                if (k % 10 == 0) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            array[outer % ARRAY_SIZE] = cold_sum;
            checksum += cold_sum * 0.5;
            
            /* Call alloca in cold path */
            int alloca_res = use_alloca((outer % 15) + 3);
            checksum += alloca_res;
        }
        
        /* Pattern 4: More mixed operations with barriers */
        for (int j = 0; j < 5; ++j) {
            /* Group 1 */
            double g1 = array[(outer + j * 2) % ARRAY_SIZE];
            double g2 = array[(outer + j * 2 + 1) % ARRAY_SIZE];
            double g3 = g1 * g2 + g1 / (g2 + 0.001);
            
            /* Barrier between groups */
            asm volatile ("" ::: "memory");
            
            /* Group 2 */
            int ig1 = (int)g3;
            int ig2 = ig1 * 11 - 7;
            int ig3 = ig2 % 19 + ig1 * 3;
            
            /* Another barrier */
            asm volatile ("" ::: "memory");
            
            /* Group 3 - FP function calls */
            double f1 = sin(g3);
            double f2 = cos(g3 * 0.5);
            double f3 = f1 * f1 + f2 * f2;
            
            array[(outer + j * 3) % ARRAY_SIZE] = f3 + ig3;
            checksum += f3 + ig3;
        }
        
        /* Final helper call */
        double final_vla = use_vla((outer % 10) + 2);
        checksum += final_vla;
    }
    
    /* Prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    
    /* Force rare condition to be possible */
    if (checksum < -1000000.0) {
        /* This should never happen, but makes the cold path reachable */
        printf("Impossible condition reached!\n");
    }
    
    return 0;
}
