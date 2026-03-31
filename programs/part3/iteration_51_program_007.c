#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline)) 
void vla_helper(int size, int seed) {
    volatile int vla_size = size + (seed % 16);
    float vla[vla_size];
    
    for (int i = 0; i < vla_size; i++) {
        vla[i] = (i * seed) / (vla_size + 1.0f);
    }
    
    /* Use the VLA to prevent elimination */
    volatile float sum = 0.0f;
    for (int i = 0; i < vla_size; i++) {
        sum += vla[i];
    }
}

/* Another noinline helper with alloca */
__attribute__((noinline))
void alloca_helper(int iterations) {
    for (int i = 0; i < iterations; i++) {
        int* block = (int*)alloca(sizeof(int) * (i % 8 + 1));
        for (int j = 0; j < (i % 8 + 1); j++) {
            block[j] = i * j;
        }
        
        /* Use the allocated block */
        volatile int check = 0;
        for (int j = 0; j < (i % 8 + 1); j++) {
            check += block[j];
        }
    }
}

int main() {
    /* Initialize with random seed */
    srand(time(NULL));
    
    /* Large arrays for memory operations */
    float f_array[ARRAY_SIZE];
    int i_array[ARRAY_SIZE];
    double d_array[ARRAY_SIZE];
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        f_array[i] = (float)rand() / RAND_MAX * 100.0f;
        i_array[i] = rand() % 1000;
        d_array[i] = (double)rand() / RAND_MAX * 200.0;
    }
    
    /* Accumulator for final result */
    double total_result = 0.0;
    
    /* Outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        volatile int outer_mod = outer % ARRAY_SIZE;
        
        /* PATTERN 1: Large basic block with dependency chains */
        {
            float a = f_array[outer_mod];
            float b = f_array[(outer_mod + 1) % ARRAY_SIZE];
            float c = f_array[(outer_mod + 2) % ARRAY_SIZE];
            float d = f_array[(outer_mod + 3) % ARRAY_SIZE];
            float e = f_array[(outer_mod + 4) % ARRAY_SIZE];
            
            /* Long dependency chain */
            float t1 = a + b;
            float t2 = t1 * c;
            float t3 = t2 / (d + 1.0f);
            float t4 = sqrtf(fabsf(t3));
            float t5 = t4 * sinf(e);
            float t6 = t5 + cosf(a);
            float t7 = t6 * tanf(b * 0.01f);
            float t8 = t7 / (logf(fabsf(c) + 1.0f));
            float t9 = t8 + expf(d * 0.001f);
            float t10 = t9 * atanf(e);
            
            /* More integer operations mixed in */
            int i1 = i_array[outer_mod];
            int i2 = i_array[(outer_mod + 1) % ARRAY_SIZE];
            int i3 = i1 * i2;
            int i4 = i3 % (abs(i2) + 1);
            int i5 = i4 + (i1 << 2);
            int i6 = i5 ^ (i2 >> 1);
            
            /* Memory store to force scheduling considerations */
            f_array[outer_mod] = t10 + i6;
            
            total_result += t10 + i6;
        }
        
        /* Call VLA helper between patterns */
        vla_helper(outer % 32, rand());
        
        /* PATTERN 2: Nested loops with data-dependent bounds */
        {
            int inner_bound = (rand() % INNER_BASE) + 10;  /* Data-dependent */
            volatile int loop_counter = 0;
            
            for (int i = 0; i < 5; i++) {
                /* Inner loop with varying trip count */
                for (int j = 0; j < inner_bound; j++) {
                    /* Mixed operations within nested loop */
                    double val1 = d_array[(i + j) % ARRAY_SIZE];
                    double val2 = d_array[(i * j) % ARRAY_SIZE];
                    
                    double temp = val1 * val2;
                    temp = temp + sqrt(fabs(val1));
                    temp = temp / (fabs(val2) + 1.0);
                    
                    /* Store result back with complex addressing */
                    int idx = (i * 17 + j * 13) % ARRAY_SIZE;
                    d_array[idx] = temp * 0.99;
                    
                    loop_counter++;
                }
                
                /* Inline assembly barrier - forces scheduler boundary */
                asm volatile ("" ::: "memory");
                
                /* More operations after barrier */
                int idx2 = (i * 19) % ARRAY_SIZE;
                f_array[idx2] = sinf(f_array[idx2]) + 1.0f;
            }
            
            total_result += loop_counter;
        }
        
        /* PATTERN 3: Conditional with __builtin_expect */
        {
            int rare_condition = (rand() % 10000) == 0;  /* Rarely true */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                printf("Taking rare path in iteration %d\n", outer);
                
                /* Another inline assembly barrier */
                asm volatile ("" ::: "memory");
                
                /* Complex sequence in cold path */
                float acc = 0.0f;
                for (int k = 0; k < 20; k++) {
                    acc += sinf(f_array[(outer + k) % ARRAY_SIZE] * 0.1f);
                    acc *= cosf(d_array[k] * 0.01f);
                }
                
                /* Function call as scheduling barrier */
                double sqrt_val = sqrt(fabs(acc));
                
                /* More dependency chains */
                for (int k = 0; k < 10; k++) {
                    sqrt_val = sqrt_val * 1.1;
                    sqrt_val = sqrt_val + k * 0.01;
                }
                
                total_result += sqrt_val;
                
                /* Another assembly barrier */
                asm volatile ("" ::: "memory");
            } else {
                /* Hot path - simpler but still complex */
                float hot_acc = f_array[outer % ARRAY_SIZE];
                for (int k = 0; k < 5; k++) {
                    hot_acc = hot_acc * 1.05f + k;
                }
                total_result += hot_acc;
            }
        }
        
        /* Call alloca helper */
        alloca_helper((outer % 4) + 1);
        
        /* PATTERN 4: Mixed operations with multiple barriers */
        {
            /* Group 1 with barrier */
            int x = i_array[outer % ARRAY_SIZE];
            int y = i_array[(outer + 10) % ARRAY_SIZE];
            int z = x * y + (x % (abs(y) + 1));
            
            asm volatile ("" ::: "memory");
            
            /* Group 2 with barrier */
            float fx = (float)z * 0.01f;
            float fy = sinf(fx) * cosf(fx * 0.5f);
            float fz = sqrtf(fabsf(fy)) + 1.0f;
            
            asm volatile ("" ::: "memory");
            
            /* Group 3 with barrier */
            double dx = (double)fz;
            for (int i = 0; i < 3; i++) {
                dx = dx * 1.5 + i * 0.1;
            }
            
            asm volatile ("" ::: "memory");
            
            /* Final store */
            int store_idx = (outer * 7) % ARRAY_SIZE;
            d_array[store_idx] = dx;
            
            total_result += dx;
        }
        
        /* Occasionally call rand() as scheduling barrier */
        if (outer % 23 == 0) {
            volatile int r = rand();
            total_result += r % 100;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += f_array[i] + d_array[i] + i_array[i];
    }
    
    final_checksum += total_result;
    
    printf("Final result: %f\n", final_checksum);
    printf("Array[0]: %f, Array[%d]: %f\n", 
           f_array[0], ARRAY_SIZE-1, f_array[ARRAY_SIZE-1]);
    
    return 0;
}
