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
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" : : : "memory");
}

/* Another helper with alloca */
__attribute__((noinline))
static int dynamic_alloca_ops(int n) {
    int* ptr = (int*)alloca(n * sizeof(int));
    int sum = 0;
    for (int i = 0; i < n; i++) {
        ptr[i] = i * 3;
        sum += ptr[i];
    }
    return sum;
}

int main() {
    double array[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Complex FP dependency chain */
        double t1 = a + b;
        double t2 = t1 * c;
        double t3 = t2 / (d + 1.0);
        double t4 = sqrt(fabs(t3));
        double t5 = sin(t4) * cos(t3);
        double t6 = t5 * t5 + t4 * t4;
        
        /* Integer dependency chain mixed in */
        int i1 = (int)t6;
        int i2 = i1 * 37;
        int i3 = i2 % 17;
        int i4 = i3 + i1 - i2;
        
        /* Memory store with addressing computation */
        array[(outer * 7) % ARRAY_SIZE] = t6 + i4;
        
        /* Inline assembly barrier - creates scheduling region boundary */
        asm volatile ("" : : : "memory");
        
        /* More mixed operations after barrier */
        double t7 = array[(i4 + outer) % ARRAY_SIZE];
        t7 = t7 * 2.5 - 1.8;
        int i5 = (int)(t7 * 100) % 256;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = (rand() % INNER_BASE) + 10; /* Data-dependent */
        for (int j = 0; j < inner_bound; j++) {
            /* Complex addressing with mixed operations */
            int idx = (outer * j + i5) % ARRAY_SIZE;
            double val = array[idx];
            
            /* FP operations with dependencies */
            val = val * 1.01 + 0.5;
            val = sin(val * 0.1) * 100.0;
            
            /* Integer operations on FP results */
            int ival = (int)val;
            ival = (ival * 13) % 97;
            
            /* Store with pointer arithmetic */
            double* ptr = &array[idx];
            *ptr = val * ival;
            
            /* Periodic inline assembly barrier */
            if (j % 5 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000) == 0; /* Rare condition */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_sum += sqrt(array[(outer + k) % ARRAY_SIZE]);
                cold_sum *= 1.0001;
            }
            
            /* More complex cold path operations */
            for (int k = 0; k < 20; k++) {
                double temp = array[k] * k;
                temp = sin(temp) + cos(temp);
                array[k] = temp;
            }
            
            checksum += cold_sum;
        } else {
            /* Hot path - simpler but still complex */
            double hot_sum = 0.0;
            for (int k = 0; k < 10; k++) {
                hot_sum += array[(outer + k * 7) % ARRAY_SIZE];
            }
            checksum += hot_sum * 0.01;
        }
        
        /* Pattern 4: Function calls as scheduling barriers */
        double r1 = (double)rand() / RAND_MAX;
        double r2 = (double)rand() / RAND_MAX;
        double r3 = sqrt(r1 * r1 + r2 * r2);
        r3 = sin(r3) * cos(r3);
        
        /* Call helper with VLA between patterns */
        use_vla((outer % 20) + 5);
        
        /* More complex operations after VLA */
        for (int m = 0; m < 15; m++) {
            int idx2 = (outer + m * 11) % ARRAY_SIZE;
            array[idx2] = array[idx2] * 0.99 + r3;
        }
        
        /* Use alloca helper */
        int alloca_result = dynamic_alloca_ops((outer % 15) + 3);
        checksum += alloca_result * 0.001;
        
        /* Final complex block with mixed operations */
        double final_val = array[outer % ARRAY_SIZE];
        final_val = final_val * final_val - sqrt(final_val);
        final_val = final_val > 0 ? final_val : -final_val;
        
        int int_part = (int)final_val;
        double frac_part = final_val - int_part;
        
        /* Complex integer arithmetic */
        int_part = (int_part * 31) % 127;
        int_part = int_part ^ (int_part >> 3);
        int_part = int_part * 7 + 13;
        
        /* Final store with addressing mode */
        array[(outer * 19) % ARRAY_SIZE] = int_part + frac_part;
        
        /* Another scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute final checksum to prevent elimination */
    double final_checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array[i] * (i % 7 + 1);
    }
    
    printf("Final checksum: %f\n", final_checksum + checksum);
    return 0;
}
