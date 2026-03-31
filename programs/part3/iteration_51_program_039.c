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
    for (int i = 0; i < size && i < 10; i++) {
        vla[i] = i * 2;
    }
    asm volatile ("" ::: "memory");
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

__attribute__((noinline))
static int complex_int_chain(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 % (d + 1);
    int t4 = t3 - b;
    int t5 = t4 * a;
    int t6 = t5 / (c + 1);
    return t6 + t3 - t4;
}

/* Main computation with scheduling-intensive patterns */
int main(void) {
    srand(time(NULL));
    
    /* Initialize data arrays */
    double fp_data[ARRAY_SIZE];
    int int_data[ARRAY_SIZE];
    volatile double checksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fp_data[i] = (double)rand() / RAND_MAX * 100.0;
        int_data[i] = rand() % 1000;
    }
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large basic block with dependency chains */
        double fp_acc = fp_data[outer % ARRAY_SIZE];
        int int_acc = int_data[outer % ARRAY_SIZE];
        
        /* Long dependency chain with mixed operations */
        fp_acc = fp_acc + fp_data[(outer + 1) % ARRAY_SIZE];
        fp_acc = fp_acc * 1.5;
        fp_acc = sqrt(fabs(fp_acc));
        int_acc = int_acc * 3;
        int_acc = int_acc + (int)fp_acc;
        fp_acc = fp_acc / (fp_data[(outer + 2) % ARRAY_SIZE] + 1.0);
        int_acc = int_acc % 97;
        fp_acc = sin(fp_acc) * cos(fp_acc);
        int_acc = int_acc - (int)(fp_acc * 10);
        
        /* Memory access pattern */
        for (int i = 0; i < 20; i++) {
            int idx = (outer + i) % ARRAY_SIZE;
            fp_data[idx] = fp_data[idx] * 0.99 + 0.01;
            int_data[idx] = (int_data[idx] + i) % 1000;
        }
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % INNER_BASE + 10;
        for (int j = 0; j < inner_loops; j++) {
            double local_fp = 1.0;
            int local_int = 1;
            
            /* Inner computation with dependencies */
            for (int k = 0; k < 5; k++) {
                local_fp = local_fp * fp_data[(j + k) % ARRAY_SIZE];
                local_int = local_int + int_data[(j + k) % ARRAY_SIZE];
                local_fp = local_fp / (k + 2.0);
                local_int = local_int % (k + 10);
            }
            
            /* Call complex functions */
            fp_acc += complex_fp_chain(local_fp, fp_data[j % ARRAY_SIZE], 
                                      fp_acc, local_fp + 1.0);
            int_acc += complex_int_chain(local_int, int_data[j % ARRAY_SIZE],
                                        int_acc, local_int);
            
            /* Another assembly barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (outer == 42 || outer == 77);
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_fp = 0.0;
            for (int i = 0; i < 30; i++) {
                cold_fp += sqrt(fp_data[i] * i);
                cold_fp = sin(cold_fp) * cos(cold_fp);
                asm volatile ("" ::: "memory");
            }
            fp_acc += cold_fp;
            
            /* Use alloca in cold path */
            int* cold_array = (int*)alloca(sizeof(int) * 20);
            for (int i = 0; i < 20; i++) {
                cold_array[i] = int_data[i] * i;
                int_acc += cold_array[i] % 17;
            }
        }
        
        /* Pattern 4: VLA usage between patterns */
        use_vla((outer % 20) + 5);
        
        /* Update checksum */
        checksum += fp_acc + int_acc;
        
        /* More mixed operations */
        for (int i = 0; i < 10; i++) {
            int idx = (outer * i) % ARRAY_SIZE;
            double temp = fp_data[idx];
            fp_data[idx] = temp * temp - sqrt(temp);
            int_data[idx] = (int_data[idx] * 31 + 17) % 1000;
        }
        
        /* Final assembly barrier in iteration */
        asm volatile ("" ::: "memory");
    }
    
    /* Additional stress test with varying patterns */
    volatile double final_result = 0.0;
    for (int phase = 0; phase < 3; phase++) {
        /* Different computation pattern each phase */
        switch (phase) {
            case 0: {
                /* Heavy FP math */
                for (int i = 0; i < ARRAY_SIZE; i += 4) {
                    double a = fp_data[i];
                    double b = fp_data[i+1];
                    double c = fp_data[i+2];
                    double d = fp_data[i+3];
                    
                    for (int j = 0; j < 3; j++) {
                        a = a * b + c;
                        b = b / (d + 1.0);
                        c = sqrt(a * a + b * b);
                        d = sin(a) * cos(b);
                    }
                    
                    fp_data[i] = a;
                    fp_data[i+1] = b;
                    final_result += a + b + c + d;
                }
                break;
            }
            case 1: {
                /* Integer-heavy with memory */
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    int val = int_data[i];
                    for (int j = 0; j < 5; j++) {
                        val = (val * 1103515245 + 12345) % 1000;
                        val = val ^ (val >> 3);
                        val = val + int_data[(i + j) % ARRAY_SIZE];
                    }
                    int_data[i] = val;
                    final_result += val;
                }
                break;
            }
            case 2: {
                /* Mixed operations with barriers */
                for (int i = 0; i < ARRAY_SIZE / 2; i++) {
                    double fp_val = fp_data[i];
                    int int_val = int_data[i];
                    
                    fp_val = fp_val * 2.0 - 1.0;
                    asm volatile ("" ::: "memory");
                    
                    int_val = (int_val * 3) % 100;
                    fp_val = sqrt(fabs(fp_val));
                    
                    asm volatile ("" ::: "memory");
                    
                    for (int k = 0; k < 2; k++) {
                        fp_val = sin(fp_val) * 0.5 + 0.5;
                        int_val = int_val + (int)(fp_val * 100);
                    }
                    
                    final_result += fp_val + int_val;
                }
                break;
            }
        }
        
        /* VLA between phases */
        use_vla(15 + phase * 5);
    }
    
    printf("Final checksum: %f\n", checksum + final_result);
    return 0;
}
