#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper functions to prevent inlining */
__attribute__((noinline)) 
static void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" : : : "memory");
}

__attribute__((noinline))
static double complex_fp_chain(double a, double b, double c, double d) {
    double t1 = a + b;
    double t2 = t1 * c;
    double t3 = t2 / d;
    double t4 = sqrt(fabs(t3));
    double t5 = sin(t4) + cos(t3);
    return t5 * t1 - t2 / t4;
}

/* Main computation with scheduling-intensive patterns */
static double compute_checksum(double* data, int size) {
    double checksum = 0.0;
    volatile double sink = 0.0; /* Prevent optimization */
    
    /* Pattern 1: Large basic block with dependency chains */
    for (int i = 0; i < size - 10; i++) {
        double a = data[i];
        double b = data[i+1];
        double c = data[i+2];
        
        /* Integer dependency chain */
        int x = (int)a % 100;
        int y = x * (int)b + 7;
        int z = y / ((int)c % 50 + 1);
        int w = z - (x ^ y) * (z & 0xFF);
        
        /* Floating-point dependency chain */
        double fp1 = a * b + c;
        double fp2 = fp1 / (b + 1.0);
        double fp3 = sqrt(fabs(fp2)) * 2.5;
        double fp4 = fp3 - sin(fp1) * cos(fp2);
        
        /* Memory access pattern */
        data[i+3] = fp4 * w;
        checksum += fp4 + w;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Pattern 2: Nested loops with data-dependent bounds */
    for (int outer = 0; outer < 20; outer++) {
        int inner_bound = (rand() % INNER_BASE) + 10;
        
        for (int inner = 0; inner < inner_bound; inner++) {
            int idx = (outer * 31 + inner * 17) % size;
            
            /* Mixed operations */
            double val = data[idx];
            data[idx] = val * 1.01 + sin(val * 0.1);
            
            /* Integer math with dependencies */
            int ival = (int)val;
            ival = (ival * 3 + 7) % 1000;
            ival = ival ^ (ival >> 3);
            ival = ival * 11 - 5;
            
            checksum += ival * 0.001;
            
            /* Another scheduling barrier */
            if (inner % 5 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Use VLA between loop iterations */
        use_vla((outer % 10) + 5);
    }
    
    /* Pattern 3: __builtin_expect with cold path */
    int rare_condition = rand() % 10000;
    
    if (__builtin_expect(rare_condition < 5, 0)) {
        /* Cold path - complex operations */
        double cold_sum = 0.0;
        for (int i = 0; i < size; i += 4) {
            double a = data[i];
            double b = data[i+1];
            double c = data[i+2];
            double d = data[i+3];
            
            /* Long dependency chain */
            double t1 = a + b;
            double t2 = t1 * c;
            double t3 = t2 / (d + 1.0);
            double t4 = sqrt(t3 + 1.0);
            double t5 = t4 * sin(t2) - cos(t3);
            double t6 = t5 * 2.5 + t1 / t4;
            
            cold_sum += t6;
            data[i] = t6 * 0.5;
            
            /* Multiple barriers in cold path */
            asm volatile ("" : : : "memory");
            asm volatile ("" : : : "memory");
        }
        checksum += cold_sum * 0.01;
    } else {
        /* Hot path - simpler operations */
        for (int i = 0; i < size; i += 8) {
            checksum += data[i] * 0.5;
        }
    }
    
    /* Pattern 4: Alloca-based computation */
    for (int block = 0; block < 5; block++) {
        int alloc_size = (rand() % 20) + 10;
        double* block_data = (double*)alloca(alloc_size * sizeof(double));
        
        for (int i = 0; i < alloc_size; i++) {
            block_data[i] = sin(checksum + i) * 1.5;
            
            /* Dependent operations */
            if (i > 0) {
                block_data[i] = block_data[i] * block_data[i-1] + i;
            }
            
            checksum += block_data[i] * 0.01;
        }
        
        /* Force spill/reload around alloca */
        asm volatile ("" : : : "memory");
    }
    
    /* Pattern 5: Function call barriers */
    for (int i = 0; i < size; i += 16) {
        double a = data[i];
        double b = data[(i + 5) % size];
        double c = data[(i + 11) % size];
        
        /* Complex FP chain via function call */
        double result = complex_fp_chain(a, b, c, checksum + 1.0);
        checksum += result * 0.001;
        
        /* Memory store with addressing mode variation */
        data[(i * 3 + 7) % size] = result;
        
        /* VLA usage between computations */
        if (i % 32 == 0) {
            use_vla(8);
        }
    }
    
    sink = checksum;
    return checksum;
}

int main() {
    srand(time(NULL));
    
    /* Initialize with random data */
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    double total_checksum = 0.0;
    
    /* Outer driver loop */
    for (int outer_iter = 0; outer_iter < OUTER_LOOPS; outer_iter++) {
        /* Vary input slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] += sin(outer_iter * 0.1) * 0.01;
        }
        
        /* Perform main computation */
        double iter_checksum = compute_checksum(data, ARRAY_SIZE);
        total_checksum += iter_checksum;
        
        /* Occasionally reinitialize to change patterns */
        if (outer_iter % 30 == 29) {
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
            }
        }
        
        /* Use VLA between outer iterations */
        use_vla((outer_iter % 15) + 3);
    }
    
    /* Final result depends on all computations */
    printf("Final checksum: %.10f\n", total_checksum);
    
    free(data);
    return 0;
}
