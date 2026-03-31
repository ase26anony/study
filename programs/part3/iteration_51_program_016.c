#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    return sum;
}

/* Another helper with complex operations */
__attribute__((noinline))
static double process_block(double *data, int start, int end) {
    double result = 0.0;
    
    for (int i = start; i < end; i++) {
        double x = data[i];
        double y = data[(i * 3) % ARRAY_SIZE];
        
        /* Complex dependency chain */
        double t1 = x + y;
        double t2 = t1 * t1;
        double t3 = sqrt(fabs(t2) + 1.0);
        double t4 = sin(t3) + cos(t3);
        double t5 = t4 * t4 - 2.0 * t4 + 1.0;
        
        result += t5;
    }
    
    return result;
}

int main(void) {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large basic block with dependency chains */
        {
            double a = data[outer % ARRAY_SIZE];
            double b = data[(outer * 7) % ARRAY_SIZE];
            double c = data[(outer * 13) % ARRAY_SIZE];
            double d = data[(outer * 19) % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            double r1 = a + b;
            double r2 = r1 * c;
            double r3 = r2 / (d + 1.0);
            double r4 = sqrt(fabs(r3));
            double r5 = sin(r4) * cos(r4);
            double r6 = r5 * r5 - 2.0 * r5 + 1.0;
            double r7 = r6 * exp(-fabs(r6));
            double r8 = r7 + tan(r7 * 0.1);
            double r9 = r8 * r8 + r8 * 2.0 + 1.0;
            double r10 = log(fabs(r9) + 1.0);
            
            checksum += r10;
            
            /* Inline assembly barrier */
            asm volatile ("" ::: "memory");
            
            /* More integer operations */
            int i1 = (int)r10;
            int i2 = i1 * 3 + 7;
            int i3 = i2 % 17;
            int i4 = i3 * i3 - i3;
            int i5 = i4 | (i4 << 3);
            int i6 = i5 ^ (i5 >> 2);
            
            checksum += i6;
        }
        
        /* Call VLA helper between patterns */
        double vla_result = use_vla((outer % 20) + 10);
        checksum += vla_result;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = (rand() % INNER_BASE) + 10;
            
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < inner_limit; j++) {
                    /* Mixed integer and FP in inner loop */
                    int idx = (i * 31 + j * 7) % ARRAY_SIZE;
                    double val = data[idx];
                    
                    /* Complex operations with dependencies */
                    val = val * 1.1 + 0.5;
                    val = sin(val) * cos(val);
                    val = val * val - 2.0 * val + 1.0;
                    
                    /* Integer computation */
                    int ival = (int)(val * 100);
                    ival = (ival * 3) % 97;
                    ival = ival ^ (ival << 3);
                    
                    checksum += val + ival;
                    
                    /* Memory store with addressing */
                    data[(idx + 1) % ARRAY_SIZE] = val * 0.9;
                }
                
                /* Barrier every outer iteration */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Pattern 3: Conditional with __builtin_expect */
        {
            int rare_condition = (outer == 42);  /* Rare case */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                double cold_sum = 0.0;
                
                for (int i = 0; i < 100; i++) {
                    double x = data[i];
                    double y = data[ARRAY_SIZE - 1 - i];
                    
                    /* Very long dependency chain */
                    double t = x + y;
                    t = t * t - 2.0 * t + 1.0;
                    t = sqrt(fabs(t));
                    t = sin(t) + cos(t);
                    t = t * t * t - 3.0 * t * t + 3.0 * t;
                    t = log(fabs(t) + 1.0);
                    t = exp(t * 0.1);
                    
                    cold_sum += t;
                    
                    /* Multiple barriers in cold path */
                    if (i % 10 == 0) {
                        asm volatile ("" ::: "memory");
                    }
                }
                
                checksum += cold_sum * 2.0;
                
                /* Use alloca in cold path */
                int alloca_size = 50 + (outer % 10);
                double *dynamic = (double*)alloca(alloca_size * sizeof(double));
                
                for (int i = 0; i < alloca_size; i++) {
                    dynamic[i] = sin(i * 0.2) * cos(i * 0.1);
                    checksum += dynamic[i];
                }
            } else {
                /* Hot path - simpler operations */
                checksum += data[outer % ARRAY_SIZE] * 0.5;
            }
        }
        
        /* Pattern 4: Mixed operations with barriers */
        {
            /* Group 1: FP operations */
            double f1 = data[outer % ARRAY_SIZE];
            double f2 = f1 * 2.0 + 1.0;
            double f3 = sin(f2) * cos(f2);
            
            /* Barrier between groups */
            asm volatile ("" ::: "memory");
            
            /* Group 2: Integer operations */
            int i1 = (int)f3;
            int i2 = i1 * 3 + 7;
            int i3 = i2 % 13;
            int i4 = i3 | (i3 << 2);
            
            /* Barrier */
            asm volatile ("" ::: "memory");
            
            /* Group 3: Memory operations */
            data[(outer + 1) % ARRAY_SIZE] = f3;
            double loaded = data[(outer + 2) % ARRAY_SIZE];
            
            /* Group 4: More FP */
            double f4 = loaded * f3 + sqrt(fabs(loaded));
            checksum += f4 + i4;
        }
        
        /* Call process_block helper */
        int block_start = outer % (ARRAY_SIZE - 100);
        double block_result = process_block(data, block_start, block_start + 50);
        checksum += block_result;
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += data[i] * 0.01;
    }
    checksum += final_result;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
