/* sel-sched-trigger.c
 * Designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling on this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_function_1(int* arr_a, int* arr_b, float* arr_c, int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches */
    for (int j = 0; j < 4; j++) {  /* Outer loop */
        int base = (j * 17) & 0xFF;  /* Outer loop carried state */
        
        for (int i = 0; i < vol_size; i++) {  /* Inner loop */
            /* Multiple dependency types and mixed operations */
            int temp = arr_a[i] * base;
            
            /* Flow dependency */
            sum = sum + temp;
            
            /* Anti dependency on arr_b */
            int old_b = arr_b[i];
            arr_b[i] = temp ^ old_b;
            
            /* Control dependency with branch */
            if (sum > threshold) {
                /* Output dependency on arr_c */
                arr_c[i] = fsum + (float)sum * 0.5f;
                sum = sum / 2;  /* Reset-like operation */
            }
            
            /* Floating point operations create different RTL */
            fsum = fsum + arr_c[i] * 0.75f;
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < vol_size) {
                int temp2 = arr_a[i + 1] * (base + 1);
                sum = sum - temp2;  /* Different operation */
                arr_b[i + 1] = arr_b[i + 1] ^ temp2;
                
                /* Inline assembly barrier creates scheduling complexity */
                asm volatile("" ::: "memory");
                
                if (fsum > 1000.0f) {
                    arr_c[i + 1] = fsum * 0.25f;
                    fsum = fsum * 0.5f;
                }
                i++;  /* Skip next iteration */
            }
            
            /* Data-dependent branch with bitwise operation */
            if (arr_a[i] & 1) {
                /* Complex expression with multiple operations */
                arr_b[i] = (arr_b[i] << 2) | (arr_b[i] >> 30);
                fsum = fsum + (float)(arr_b[i] % 256);
            }
        }
        
        /* Outer loop modification of inner loop data */
        for (int k = 0; k < 8 && k < vol_size; k++) {
            arr_a[k] = arr_a[k] + base;
        }
    }
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_function_2(double* arr_d, int* arr_e, int size) {
    volatile int vol_counter = size;
    double dsum = 0.0;
    int isum = 0;
    
    /* Loop with multiple volatile variables */
    for (volatile int v = 0; v < vol_counter; v++) {
        /* Mixed double and int operations */
        dsum = dsum + arr_d[v] * 1.5;
        isum = isum + arr_e[v];
        
        /* Resource conflict simulation */
        double temp_d = arr_d[v] * dsum;
        int temp_i = arr_e[v] * isum;
        
        /* Conditional with side effects */
        if ((temp_i & 0xF) > 8) {
            arr_d[v] = temp_d * 0.8;
            /* Memory barrier affects scheduling */
            asm volatile("" ::: "memory");
            arr_e[v] = temp_i ^ 0xAA55;
        }
        
        /* Nested condition with floating compare */
        if (dsum > 10000.0) {
            dsum = dsum * 0.9;
            /* Another barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Manual unrolling - 3 iterations */
        if (v + 2 < vol_counter) {
            /* Different operations for each unrolled iteration */
            arr_d[v + 1] = arr_d[v + 1] + dsum;
            arr_e[v + 1] = arr_e[v + 1] - isum;
            
            asm volatile("" ::: "memory");
            
            arr_d[v + 2] = arr_d[v + 2] * 1.1;
            arr_e[v + 2] = arr_e[v + 2] | 0xFF;
            
            v += 2;  /* Skip two iterations */
        }
    }
}

/* Outer loop carried state pattern */
void test_function_3(int* arr_f, int* arr_g, int outer, int inner) {
    int state = 0;
    
    for (int j = 0; j < outer; j++) {
        /* Outer loop modifies state used in inner loop */
        int base = (state * 13 + j * 7) & 0xFF;
        state = base;
        
        /* Volatile prevents optimization */
        volatile int vol_inner = inner;
        
        for (int i = 0; i < vol_inner; i++) {
            /* Loop-carried dependency spanning nesting levels */
            int val = arr_f[i] + base;
            
            /* Complex conditional with multiple operations */
            if (val > 1000) {
                arr_g[i] = (arr_g[i] * val) >> 3;
                /* Memory operation affects scheduling */
                asm volatile("" ::: "memory");
                arr_f[i] = arr_f[i] - (base >> 1);
            } else {
                arr_g[i] = arr_g[i] + val * 2;
            }
            
            /* Data-dependent array access pattern */
            int idx = (i + base) % vol_inner;
            arr_f[idx] = arr_f[idx] ^ arr_g[i];
            
            /* Additional floating point for RTL variety */
            float ftemp = (float)arr_f[i] * 0.003f;
            if (ftemp > 1.0f) {
                arr_g[i] = arr_g[i] + (int)(ftemp * 10.0f);
            }
        }
        
        /* Modify state based on inner loop results */
        if (arr_f[0] > arr_g[0]) {
            state = state + 1;
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_e[SIZE];
    int array_f[SIZE];
    int array_g[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
        array_e[i] = (int)(lcg_rand() % 1000);
        array_f[i] = (int)(lcg_rand() % 1000);
        array_g[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Volatile flag for runtime variability */
    volatile int flag = 0;
    int checksum = 0;
    
    /* Call test functions multiple times with volatile control */
    for (int iter = 0; iter < 5; iter++) {
        flag = array_a[iter] & 1;  /* Data-dependent flag */
        
        if (flag) {
            for (int rep = 0; rep < 3; rep++) {
                test_function_1(array_a, array_b, array_c, SIZE, 5000);
            }
        }
        
        test_function_2(array_d, array_e, SIZE);
        
        if (!flag || (iter % 2 == 0)) {
            test_function_3(array_f, array_g, 8, SIZE / 8);
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum = (checksum * 31 + array_a[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + array_b[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + (int)array_c[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + (int)array_d[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + array_e[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + array_f[i]) & 0x7FFFFFFF;
        checksum = (checksum * 31 + array_g[i]) & 0x7FFFFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
