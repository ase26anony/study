#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100

/* Helper functions to prevent inlining */
__attribute__((noinline)) void use_vla(int size) {
    volatile int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    asm volatile ("" : : : "memory");
}

__attribute__((noinline)) double complex_fp_chain(double a, double b, double c, double d) {
    double t1 = a + b * c;
    double t2 = sin(t1) * cos(d);
    double t3 = sqrt(fabs(t2)) + log(fabs(a) + 1.0);
    return t3 * t2 / (t1 + 1.0);
}

__attribute__((noinline)) int complex_int_chain(int a, int b, int c, int d) {
    int t1 = a * b + c;
    int t2 = (t1 % 37) * d;
    int t3 = t2 ^ (t1 << 3);
    int t4 = t3 / (abs(b) + 1);
    return t4 * t1 - t2;
}

int main() {
    srand(time(NULL));
    
    /* Initialize data arrays */
    double fp_data[ARRAY_SIZE];
    int int_data[ARRAY_SIZE];
    volatile double checksum = 0.0;
    volatile int int_checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fp_data[i] = (rand() % 1000) / 100.0 + 0.1;
        int_data[i] = rand() % 1000 + 1;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double fp_acc = fp_data[outer % ARRAY_SIZE];
        int int_acc = int_data[outer % ARRAY_SIZE];
        
        for (int i = 0; i < 50; i++) {
            /* Integer dependency chain */
            int_acc = int_acc * 3 + (int_acc % 17);
            int_acc = (int_acc << 2) | (int_acc >> 30);
            int_acc = int_acc ^ (int_data[i] * 7);
            
            /* Floating-point dependency chain */
            fp_acc = fp_acc + sin(fp_acc * 0.1);
            fp_acc = sqrt(fabs(fp_acc)) + cos(fp_acc * 0.2);
            fp_acc = fp_acc * 1.1 - tanh(fp_acc * 0.05);
            
            /* Memory access with addressing */
            int idx = (int_acc + i) % ARRAY_SIZE;
            fp_acc += fp_data[idx] * 0.01;
            int_acc += int_data[(idx * 7) % ARRAY_SIZE];
        }
        
        checksum += fp_acc;
        int_checksum += int_acc;
        
        /* Insert VLA helper */
        use_vla((outer % 20) + 10);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % 50 + 10;
        for (int j = 0; j < inner_loops; j++) {
            double local_fp = 1.0;
            int local_int = 1;
            
            /* Mixed operations in inner loop */
            for (int k = 0; k < 15; k++) {
                /* Integer operations */
                local_int = local_int * (int_data[j + k] % 31 + 2);
                local_int = local_int - (local_int / 3) * 2;
                local_int = local_int ^ (local_int << (k % 5));
                
                /* Floating-point operations */
                local_fp = local_fp * (1.0 + fp_data[j + k] * 0.01);
                local_fp = local_fp + sin(local_fp * 0.3) * 0.1;
                local_fp = fmod(local_fp, 10.0) + 0.5;
                
                /* Memory store with barrier */
                if (k % 3 == 0) {
                    fp_data[(j * k) % ARRAY_SIZE] = local_fp;
                    asm volatile ("" : : : "memory");
                }
            }
            
            checksum += local_fp;
            int_checksum ^= local_int;
        }
        
        /* Pattern 3: Inline assembly barriers between dependent ops */
        double barrier_fp = fp_data[outer % ARRAY_SIZE];
        int barrier_int = int_data[outer % ARRAY_SIZE];
        
        barrier_fp = barrier_fp * 2.5 + 1.0;
        barrier_int = barrier_int * 3 + 7;
        asm volatile ("" : : : "memory");
        
        barrier_fp = sin(barrier_fp) * cos(barrier_fp);
        barrier_int = barrier_int % 97 + barrier_int / 13;
        asm volatile ("" : : : "memory");
        
        barrier_fp = sqrt(fabs(barrier_fp)) + barrier_fp * 0.1;
        barrier_int = (barrier_int << 3) | (barrier_int >> 29);
        asm volatile ("" : : : "memory");
        
        checksum += barrier_fp;
        int_checksum += barrier_int;
        
        /* Pattern 4: __builtin_expect with cold path */
        int rare_condition = (outer == 37 || outer == 73); /* Rare cases */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_fp = 0.0;
            int cold_int = 0;
            
            for (int i = 0; i < 100; i++) {
                cold_fp += complex_fp_chain(fp_data[i], fp_data[i+1], 
                                           fp_data[i+2], fp_data[i+3]);
                cold_int ^= complex_int_chain(int_data[i], int_data[i+1],
                                             int_data[i+2], int_data[i+3]);
                
                /* Use alloca in cold path */
                int* dyn_arr = (int*)alloca(sizeof(int) * 10);
                for (int j = 0; j < 10; j++) {
                    dyn_arr[j] = cold_int * j;
                }
            }
            
            checksum += cold_fp * 0.01;
            int_checksum += cold_int;
        } else {
            /* Hot path - simpler operations */
            checksum += fp_data[outer % ARRAY_SIZE] * 0.001;
            int_checksum += int_data[outer % ARRAY_SIZE] % 100;
        }
        
        /* Another VLA usage */
        use_vla((outer % 15) + 5);
        
        /* Pattern 5: Mixed loads/stores with varying addressing */
        for (int i = 0; i < 20; i++) {
            /* Different addressing modes */
            double* ptr1 = &fp_data[(i * 13) % ARRAY_SIZE];
            double* ptr2 = &fp_data[(i * 17 + outer) % ARRAY_SIZE];
            int* iptr1 = &int_data[(i * 11) % ARRAY_SIZE];
            int* iptr2 = &int_data[(i * 19 + outer) % ARRAY_SIZE];
            
            *ptr1 = *ptr1 * *ptr2 + 0.5;
            *iptr1 = (*iptr1 + *iptr2) * (i % 7 + 1);
            
            /* Array indexing */
            fp_data[i] = fp_data[i] + fp_data[ARRAY_SIZE - i - 1] * 0.1;
            int_data[i] = int_data[i] ^ int_data[ARRAY_SIZE - i - 1];
        }
    }
    
    /* Final computation to prevent elimination */
    double final_fp = 0.0;
    int final_int = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_fp += fp_data[i] * (i % 10 + 1);
        final_int += int_data[i] * ((i * 3) % 7 + 1);
    }
    
    checksum += final_fp;
    int_checksum += final_int;
    
    printf("Final checksums: fp=%f, int=%d\n", checksum, int_checksum);
    return 0;
}
