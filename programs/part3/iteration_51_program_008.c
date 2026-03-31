#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline)) 
void vla_helper(int size, int iter) {
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (iter * i) % 256;
    }
    /* Use the VLA to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += vla[i];
    }
}

/* Another helper with alloca */
__attribute__((noinline))
void alloca_helper(int size) {
    int* dyn = (int*)alloca(size * sizeof(int));
    for (int i = 0; i < size; ++i) {
        dyn[i] = i * i;
    }
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

int main() {
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
        
        /* Long chain of dependent FP operations */
        double t1 = a + b;
        double t2 = t1 * c;
        double t3 = t2 / (d + 1.0);
        double t4 = sqrt(fabs(t3));
        double t5 = sin(t4) * cos(t3);
        double t6 = t5 * t5 + t4 * t4;
        double t7 = exp(t6 * 0.01);
        double t8 = log(fabs(t7) + 1.0);
        double t9 = t8 * t3 + t2 / t1;
        double t10 = t9 - t5 * t4;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More dependent integer operations */
        int i1 = (int)t10;
        int i2 = i1 * 13;
        int i3 = i2 % 17;
        int i4 = i3 + i1;
        int i5 = i4 ^ (i2 << 3);
        int i6 = i5 | (i3 & 0xFF);
        
        checksum += t10 + i6;
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 20) + 10, outer);
        
        /* Pattern 2: Nested loops with data-dependent inner bound */
        int inner_limit = rand() % INNER_BASE + 10; /* Data-dependent */
        for (int i = 0; i < 5; ++i) { /* Fixed outer */
            for (int j = 0; j < inner_limit; ++j) { /* Variable inner */
                /* Mixed operations with memory accesses */
                int idx = (i * inner_limit + j) % ARRAY_SIZE;
                array[idx] = array[idx] * 1.01 + sin(array[(idx + 1) % ARRAY_SIZE]);
                
                /* Integer operations */
                int mod = (i * j) % 7;
                array[idx] += mod;
                
                /* Another barrier */
                if (j % 8 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            
            /* Function call as scheduling barrier */
            double rnd = (double)rand() / RAND_MAX;
            array[i % ARRAY_SIZE] *= (1.0 + rnd);
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42); /* Rarely true */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; ++k) {
                cold_sum += sqrt(array[k % ARRAY_SIZE] + k);
                cold_sum *= 1.0001;
                
                /* More barriers in cold path */
                asm volatile("" ::: "memory");
                
                /* Memory intensive */
                for (int m = 0; m < 10; ++m) {
                    int idx2 = (k + m) % ARRAY_SIZE;
                    array[idx2] = array[idx2] * 0.99 + cold_sum * 0.01;
                }
            }
            checksum += cold_sum;
        } else {
            /* Hot path - simpler but still complex */
            for (int k = 0; k < 20; ++k) {
                checksum += array[(outer + k) % ARRAY_SIZE];
            }
        }
        
        /* Pattern 4: Mixed type operations in unrolled loop */
        float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
        int iacc = 0;
        double dacc = 0.0;
        
        /* Partially unrolled manually */
        for (int u = 0; u < 8; u += 2) {
            /* Integer ops */
            iacc += (outer + u) * 3;
            iacc -= (outer - u) / 2;
            
            /* Float ops */
            f1 = f1 * f2 + f3;
            f2 = f2 - f1 * 0.5f;
            f3 = f3 / (f1 + 1.0f);
            
            /* Double ops with dependency */
            dacc = dacc + array[(outer + u) % ARRAY_SIZE];
            dacc = dacc * 0.999 - sin(dacc * 0.001);
            
            /* Memory store with addressing mode variation */
            array[(outer + u + 1) % ARRAY_SIZE] = dacc + iacc + f1;
        }
        
        checksum += dacc + iacc + f1 + f2 + f3;
        
        /* Use alloca helper */
        alloca_helper((outer % 15) + 5);
        
        /* Pattern 5: Pointer chasing with mixed operations */
        double* ptr = &array[outer % ARRAY_SIZE];
        for (int p = 0; p < 10; ++p) {
            *ptr = *ptr * 1.5 + p;
            ptr = &array[((int)(*ptr) + p) % ARRAY_SIZE];
            
            /* Barrier every few iterations */
            if (p % 3 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Final computation to prevent elimination */
    double final_result = 0.0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        final_result += array[i] * (i % 3);
        final_result = sqrt(fabs(final_result)) + 1.0;
    }
    final_result += checksum;
    
    printf("Result: %f\n", final_result);
    return 0;
}
