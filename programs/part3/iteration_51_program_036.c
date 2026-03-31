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
        vla[i] = i * 2 + (i % 3);
    }
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
}

/* Another helper with complex operations */
__attribute__((noinline))
static double complex_math_helper(double a, double b, double c) {
    double result = a;
    for (int i = 0; i < 5; i++) {
        result = sin(result) * b + cos(c) / (result + 1.0);
        result = sqrt(fabs(result)) + (result * result * 0.5);
    }
    return result;
}

int main(void) {
    /* Initialize with fixed seed for reproducibility */
    srand(42);
    
    /* Large arrays for memory operations */
    int int_array[ARRAY_SIZE];
    double fp_array[ARRAY_SIZE];
    volatile int checksum_int = 0;
    volatile double checksum_fp = 0.0;
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        fp_array[i] = (double)(rand() % 1000) / 3.14;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large basic block with dependency chains */
        {
            double a = fp_array[outer % ARRAY_SIZE];
            double b = fp_array[(outer + 1) % ARRAY_SIZE];
            double c = fp_array[(outer + 2) % ARRAY_SIZE];
            int x = int_array[outer % ARRAY_SIZE];
            int y = int_array[(outer + 1) % ARRAY_SIZE];
            int z = int_array[(outer + 2) % ARRAY_SIZE];
            
            /* Long dependency chain with mixed operations */
            a = b + c * 2.5;
            b = a / (c + 1.0) - sin(a);
            c = sqrt(fabs(b)) * cos(a);
            a = b * c + a / (c + 0.001);
            
            x = y + z * 3;
            y = x % (z + 1) + (x / 2);
            z = (x * y) - (z % 7);
            x = (y << 2) | (z & 0xFF);
            
            /* More FP operations with dependencies */
            double d = a * b + c;
            double e = d / (a + 0.5) - sin(b);
            double f = sqrt(e * e + d * d);
            double g = f * cos(d) + e * sin(f);
            
            /* Memory operations with addressing modes */
            int_array[(outer + 3) % ARRAY_SIZE] = x + y;
            fp_array[(outer + 3) % ARRAY_SIZE] = g;
            
            checksum_int += x + y + z;
            checksum_fp += a + b + c + d + e + f + g;
        }
        
        /* Use VLA helper between patterns */
        use_vla((outer % 20) + 10);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        {
            int inner_limit = (rand() % INNER_BASE) + 10; /* Data-dependent */
            volatile int temp_sum = 0;
            volatile double temp_fp = 0.0;
            
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < inner_limit; j++) {
                    /* Mixed operations in inner loop */
                    int idx = (i * j + outer) % ARRAY_SIZE;
                    double val = fp_array[idx];
                    
                    /* Integer operations */
                    temp_sum += int_array[idx] * j + (i % 3);
                    
                    /* Floating-point operations */
                    val = val * 1.1 + sin(val * 0.01);
                    val = val / (cos(val) + 2.0);
                    temp_fp += val;
                    
                    /* Memory store */
                    if (j % 3 == 0) {
                        fp_array[idx] = val;
                    }
                }
                
                /* Function call as scheduling barrier */
                temp_fp += complex_math_helper(temp_fp, (double)i, (double)outer);
            }
            
            checksum_int += temp_sum;
            checksum_fp += temp_fp;
        }
        
        /* Pattern 3: Inline assembly barriers between operations */
        {
            double a = checksum_fp * 0.01;
            int b = checksum_int % 100;
            
            /* Group 1 with barrier */
            a = sin(a) * 2.5 + cos(a * 0.5);
            b = (b * 3 + 7) % 13;
            asm volatile("" ::: "memory");
            
            /* Group 2 with barrier */
            a = sqrt(fabs(a)) + a * a * 0.1;
            b = (b << 2) | (b & 0xF);
            int_array[outer % ARRAY_SIZE] = b;
            asm volatile("" ::: "memory");
            
            /* Group 3 with barrier */
            a = complex_math_helper(a, (double)b, (double)outer);
            b = (int)(a * 100) % ARRAY_SIZE;
            fp_array[b] = a;
            asm volatile("" ::: "memory");
            
            checksum_fp += a;
            checksum_int += b;
        }
        
        /* Pattern 4: __builtin_expect with unlikely path */
        {
            int rare_condition = (outer == 73); /* Rare condition */
            
            if (__builtin_expect(rare_condition, 0)) {
                /* Cold path - complex operations */
                volatile double cold_result = 0.0;
                for (int i = 0; i < 20; i++) {
                    cold_result += sin((double)i) * cos((double)outer);
                    cold_result = sqrt(fabs(cold_result)) + cold_result * 0.5;
                    
                    /* Use alloca in cold path */
                    int* dyn_array = (int*)alloca(sizeof(int) * 10);
                    for (int j = 0; j < 10; j++) {
                        dyn_array[j] = i * j + outer;
                        cold_result += (double)dyn_array[j];
                    }
                }
                checksum_fp += cold_result;
                
                /* Another VLA in cold path */
                use_vla(25);
            } else {
                /* Hot path - simpler operations */
                checksum_int += outer * 2;
            }
        }
        
        /* Pattern 5: Mixed operations with memory aliasing */
        {
            int* ptr1 = &int_array[outer % ARRAY_SIZE];
            int* ptr2 = &int_array[(outer + 1) % ARRAY_SIZE];
            double* fptr1 = &fp_array[outer % ARRAY_SIZE];
            double* fptr2 = &fp_array[(outer + 2) % ARRAY_SIZE];
            
            /* Pointer chasing with computations */
            *ptr1 = *ptr1 + *ptr2 * 3;
            *fptr1 = sin(*fptr1) + cos(*fptr2);
            
            /* More complex addressing */
            for (int offset = 0; offset < 5; offset++) {
                int idx = (outer + offset * 7) % ARRAY_SIZE;
                fp_array[idx] = fp_array[idx] * 1.01 + (double)int_array[idx] * 0.001;
            }
            
            checksum_int += *ptr1;
            checksum_fp += *fptr1 + *fptr2;
        }
    }
    
    /* Final computation to prevent elimination */
    double final_result = (double)checksum_int + checksum_fp;
    
    /* Use result in non-eliminable way */
    printf("Final checksum: %f (int: %d, fp: %f)\n", 
           final_result, checksum_int, checksum_fp);
    
    return (final_result > 0.0) ? 0 : 1;
}
