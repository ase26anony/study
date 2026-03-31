#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper with VLA to influence scheduling */
__attribute__((noinline))
static double vla_helper(int size, double seed) {
    double vla[size];
    double result = seed;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = seed * i * 0.1;
        result += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return result;
}

/* Another noinline helper with complex operations */
__attribute__((noinline))
static double complex_chain(double a, double b, double c, double d) {
    double t1 = a + b;
    double t2 = t1 * c;
    double t3 = t2 / (d + 1.0);
    double t4 = sqrt(fabs(t3));
    double t5 = sin(t4);
    
    /* Scheduling barrier */
    asm volatile ("" ::: "memory");
    
    return t5 * t3 + t2 - t1;
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        double local_sum = 0.0;
        
        /* PATTERN 1: Large dependency chain basic block */
        {
            double a = array[outer % ARRAY_SIZE];
            double b = array[(outer + 1) % ARRAY_SIZE];
            double c = array[(outer + 2) % ARRAY_SIZE];
            double d = array[(outer + 3) % ARRAY_SIZE];
            double e = array[(outer + 4) % ARRAY_SIZE];
            double f = array[(outer + 5) % ARRAY_SIZE];
            
            /* Long chain of dependent FP operations */
            double r1 = a + b;
            double r2 = r1 * c;
            double r3 = r2 / (d + 0.001);
            double r4 = sqrt(fabs(r3));
            double r5 = sin(r4);
            double r6 = r5 * e;
            double r7 = r6 - f;
            double r8 = r7 * r7;
            double r9 = r8 + r3;
            double r10 = r9 / (r4 + 1.0);
            
            /* Integer operations mixed in */
            int i1 = (int)r10;
            int i2 = i1 * 7;
            int i3 = i2 % 13;
            int i4 = i3 + i1;
            int i5 = i4 - (int)r7;
            
            /* Memory operations with different addressing */
            array[(outer + i5) % ARRAY_SIZE] = r10;
            double* ptr = &array[(outer + i3) % ARRAY_SIZE];
            *ptr = *ptr * 0.99;
            
            local_sum += r10 + i5;
            
            /* Scheduling barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Call VLA helper between patterns */
        checksum += vla_helper(outer % 20 + 5, local_sum);
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = rand() % INNER_BASE + 10;
            
            for (int i = 0; i < 5; ++i) {
                /* Data-dependent inner loop */
                for (int j = 0; j < inner_limit; ++j) {
                    /* Mixed operations with dependencies */
                    int idx = (i * inner_limit + j) % ARRAY_SIZE;
                    double val = array[idx];
                    
                    val = val * 1.01 + sin(val * 0.1);
                    val = val - cos(val * 0.05);
                    
                    /* Integer arithmetic */
                    int ival = (int)val;
                    ival = (ival * 3 + 7) % 17;
                    
                    /* Store with pointer arithmetic */
                    double* dptr = array + idx;
                    *dptr = val * ival;
                    
                    local_sum += val;
                    
                    /* Occasional scheduling barrier */
                    if (j % 7 == 0) {
                        asm volatile ("" ::: "memory");
                    }
                }
                
                /* Change inner limit occasionally */
                if (i == 2) {
                    inner_limit = rand() % 30 + 5;
                }
            }
        }
        
        /* Another VLA helper call */
        checksum += vla_helper(10, local_sum * 0.1);
        
        /* PATTERN 3: __builtin_expect with cold path */
        {
            int rare_condition = (outer == 42 || outer == 77);
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_sum = 0.0;
                
                /* Use alloca for dynamic allocation */
                int alloca_size = outer % 50 + 10;
                double* dyn_array = (double*)alloca(alloca_size * sizeof(double));
                
                for (int i = 0; i < alloca_size; ++i) {
                    dyn_array[i] = sin(i * 0.1) * cos(i * 0.05);
                    cold_sum += dyn_array[i];
                    
                    /* Chain of dependent operations */
                    for (int k = 0; k < 3; ++k) {
                        dyn_array[i] = sqrt(fabs(dyn_array[i] + k));
                        cold_sum *= 0.99;
                    }
                }
                
                /* Complex chain in cold path */
                cold_sum = complex_chain(cold_sum, local_sum, 
                                        array[outer % ARRAY_SIZE],
                                        array[(outer + 10) % ARRAY_SIZE]);
                
                local_sum += cold_sum;
                
                /* Multiple scheduling barriers */
                asm volatile ("" ::: "memory");
                asm volatile ("" ::: "memory");
            } else {
                /* Hot path - simpler operations */
                local_sum *= 0.999;
                array[outer % ARRAY_SIZE] = local_sum;
            }
        }
        
        /* PATTERN 4: Mixed operations with inline assembly barriers */
        {
            double temp = local_sum;
            
            /* Group 1 with barrier */
            temp = temp + array[(outer + 1) % ARRAY_SIZE];
            temp = temp * 1.5;
            temp = sin(temp);
            asm volatile ("" ::: "memory");
            
            /* Group 2 with barrier */
            temp = temp - array[(outer + 2) % ARRAY_SIZE];
            temp = cos(temp);
            temp = temp * temp;
            asm volatile ("" ::: "memory");
            
            /* Group 3 with barrier */
            int itemp = (int)temp;
            itemp = (itemp * 11) % 19;
            itemp = itemp + (int)array[(outer + 3) % ARRAY_SIZE];
            asm volatile ("" ::: "memory");
            
            temp = temp + itemp;
            local_sum = temp;
        }
        
        checksum += local_sum;
        
        /* Final VLA helper */
        checksum += vla_helper(15, checksum * 0.01);
    }
    
    /* Use alloca in main path as well */
    int final_vla_size = ((int)checksum % 20) + 5;
    double* final_array = (double*)alloca(final_vla_size * sizeof(double));
    
    for (int i = 0; i < final_vla_size; ++i) {
        final_array[i] = checksum * i * 0.01;
        checksum += sin(final_array[i]);
    }
    
    printf("Final checksum: %.15f\n", checksum);
    
    /* Force use of all computed values to prevent DCE */
    volatile double sink = checksum;
    
    return 0;
}
