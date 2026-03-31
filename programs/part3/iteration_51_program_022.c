#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper function with VLA to influence scheduling */
__attribute__((noinline))
static void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    /* Use the VLA to prevent optimization */
    asm volatile ("" : : "r"(vla) : "memory");
}

/* Another noinline helper with alloca */
__attribute__((noinline))
static int dynamic_alloca_operation(int base) {
    int* ptr = (int*)alloca(sizeof(int) * (base % 32 + 8));
    int sum = 0;
    for (int i = 0; i < (base % 32 + 8); i++) {
        ptr[i] = i * base + (i % 7);
        sum += ptr[i];
    }
    return sum;
}

int main() {
    double array[ARRAY_SIZE];
    int int_array[ARRAY_SIZE];
    double checksum = 0.0;
    int int_checksum = 0;
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)rand() / RAND_MAX * 100.0;
        int_array[i] = rand() % 1000;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = array[outer % ARRAY_SIZE];
        double b = array[(outer + 1) % ARRAY_SIZE];
        double c = array[(outer + 2) % ARRAY_SIZE];
        double d = array[(outer + 3) % ARRAY_SIZE];
        
        /* Complex FP dependency chain */
        double t1 = a + b * c;
        double t2 = sin(t1) * d;
        double t3 = sqrt(fabs(t2)) + c;
        double t4 = t3 / (a + 1.0);
        double t5 = t4 * t2 - t1;
        double t6 = cos(t5) + sin(t4);
        double t7 = t6 * t3 / t2;
        double t8 = exp(t7 * 0.01);
        
        /* Integer dependency chain mixed in */
        int x = int_array[outer % ARRAY_SIZE];
        int y = int_array[(outer + 1) % ARRAY_SIZE];
        int z = int_array[(outer + 2) % ARRAY_SIZE];
        
        int r1 = x + y * z;
        int r2 = r1 % (y + 1);
        int r3 = r2 * x - y;
        int r4 = r3 / (z + 1) + r1;
        int r5 = r4 % 97 + r2;
        
        /* Memory store to force scheduling considerations */
        array[outer % ARRAY_SIZE] = t8 + r5;
        int_array[outer % ARRAY_SIZE] = r5;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_bound = rand() % 50 + 10;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < inner_bound; j++) {
                /* Mixed operations within nested loop */
                double* ptr = &array[(i * j) % ARRAY_SIZE];
                *ptr = *ptr * 1.01 + sin((double)j * 0.1);
                
                int* iptr = &int_array[(i * j + outer) % ARRAY_SIZE];
                *iptr = (*iptr + j * 3) % 1000;
                
                /* Function call as scheduling barrier */
                double temp = sqrt(fabs(*ptr));
                *ptr += temp * 0.5;
            }
            /* Change inner bound occasionally */
            if (i % 2 == 0) {
                inner_bound = rand() % 30 + 5;
            }
        }
        
        /* Call helper with VLA */
        use_vla((outer % 20) + 10);
        
        /* Pattern 3: __builtin_expect with cold path */
        int condition = rand() % 10000;
        if (__builtin_expect(condition < 100, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 50; k++) {
                cold_sum += array[(outer + k) % ARRAY_SIZE] * 
                           sin((double)k * 0.2) * 
                           cos((double)k * 0.1);
            }
            
            /* Another dependency chain in cold path */
            double chain = cold_sum;
            for (int k = 0; k < 10; k++) {
                chain = sin(chain) * 1.1 + cos(chain) * 0.9;
                chain = sqrt(fabs(chain)) + k * 0.01;
            }
            
            array[outer % ARRAY_SIZE] += chain * 0.01;
            
            /* Multiple assembly barriers in cold path */
            asm volatile ("" ::: "memory");
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 4: Mixed operations with alloca */
        int alloca_result = dynamic_alloca_operation(outer);
        int_checksum += alloca_result;
        
        /* More complex FP math with dependencies */
        double acc = 0.0;
        for (int m = 0; m < 20; m++) {
            double idx = (double)((outer + m) % ARRAY_SIZE);
            acc += array[(int)idx] * sin(idx * 0.01) * 
                   cos((double)m * 0.05);
            
            /* Interleave integer operations */
            int_checksum += int_array[(outer + m) % ARRAY_SIZE] * 
                           ((m % 3) + 1);
        }
        
        checksum += acc;
        
        /* Final assembly barrier in outer loop */
        asm volatile ("" ::: "memory");
    }
    
    /* Compute final results to prevent dead code elimination */
    double final_result = checksum / (OUTER_LOOPS * 20.0);
    int_checksum = int_checksum % 1000000;
    
    printf("Final FP result: %f\n", final_result);
    printf("Final int checksum: %d\n", int_checksum);
    
    return 0;
}
