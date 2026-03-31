#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_ITERATIONS 100

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Inline assembly as scheduling barrier */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        data[i] = i * i - i;
        sum += data[i] % 17;
    }
    
    asm volatile("" ::: "memory");
    return sum;
}

/* Complex dependency chain in a single basic block */
static double complex_dependency_chain(double* arr, int idx) {
    double a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Long chain of dependent FP operations */
    a = arr[idx] + arr[idx + 1];
    b = a * arr[idx + 2];
    c = b / (arr[idx + 3] + 1.0);
    d = sqrt(fabs(c));
    e = sin(d) + cos(d);
    f = e * e - 2.0 * e + 1.0;
    g = f * arr[idx + 4] / arr[idx + 5];
    h = exp(g * 0.1);
    i = log(h + 1.0);
    j = i * i + i * 2.0 + 1.0;
    k = j - floor(j);
    l = k * 100.0;
    m = l / (sin(l) + 2.0);
    n = m * m - m;
    o = n + arr[idx + 6] - arr[idx + 7];
    p = o * o / (o + 1.0);
    q = sqrt(p * p + 1.0);
    r = q + sin(q) + cos(q);
    s = r * arr[idx + 8] / arr[idx + 9];
    t = s - floor(s);
    
    /* Inline assembly barrier between operation groups */
    asm volatile("" ::: "memory");
    
    /* More dependent integer operations */
    int ia = (int)(t * 1000);
    int ib = ia * ia - ia;
    int ic = ib % 17 + ib % 13;
    int id = ic * ic + ic;
    int ie = id / (abs(ic) + 1);
    int ig = ie * ie - ie * 2 + 1;
    
    asm volatile("" ::: "memory");
    
    return t + (ig % 100) * 0.01;
}

/* Function with __builtin_expect for cold path */
__attribute__((noinline))
static double unlikely_path_operation(double* arr, int idx, int trigger) {
    /* Hot path - simple operation */
    if (__builtin_expect(trigger > 1000000, 0)) {
        /* Cold path - complex operations that rarely execute */
        double sum = 0.0;
        for (int i = 0; i < 50; i++) {
            double a = arr[(idx + i) % ARRAY_SIZE];
            double b = arr[(idx + i + 25) % ARRAY_SIZE];
            sum += sqrt(a * a + b * b) * sin(i * 0.1);
        }
        
        /* Memory operations with varying addressing modes */
        double* ptr = arr + idx;
        for (int i = 0; i < 10; i++) {
            ptr[i] = ptr[i * 2] * 0.5 + ptr[i * 3] * 0.3;
        }
        
        asm volatile("" ::: "memory");
        return sum;
    }
    
    return arr[idx];
}

int main() {
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX * 100.0;
        int_data[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_sum = 0;
    int cold_path_trigger = 0;
    
    /* Outer loop - driver */
    for (int outer = 0; outer < OUTER_ITERATIONS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        for (int i = 0; i < ARRAY_SIZE - 10; i += 10) {
            double result = complex_dependency_chain(data, i);
            total_sum += result;
            
            /* Mix integer operations */
            int_sum += int_data[i] * int_data[i + 1] - int_data[i + 2];
            int_sum %= 1000000;
        }
        
        /* Use VLA helper between patterns */
        double vla_result = use_vla(outer % 50 + 10);
        total_sum += vla_result * 0.01;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 20; i++) {
            /* Data-dependent inner loop bound */
            int inner_bound = rand() % 50 + 10;
            
            for (int j = 0; j < inner_bound; j++) {
                /* Mixed operations within nested loop */
                int idx = (i * 31 + j * 17) % ARRAY_SIZE;
                
                /* Integer arithmetic chain */
                int val = int_data[idx];
                val = val * 3 + val / 2 - val % 7;
                val = val * val - val;
                int_data[idx] = val % 1000;
                
                /* Floating point operations */
                double fp_val = data[idx];
                fp_val = fp_val * 1.1 + sin(fp_val) * 0.1;
                fp_val = fp_val / (cos(fp_val) + 2.0);
                data[idx] = fp_val;
                
                /* Memory store with complex addressing */
                if (j % 3 == 0) {
                    data[(idx + j) % ARRAY_SIZE] = fp_val * 0.5;
                }
            }
            
            /* Inline assembly barrier in loop */
            asm volatile("" ::: "memory");
        }
        
        /* Use alloca helper */
        int alloca_result = use_alloca(outer % 30 + 5);
        int_sum += alloca_result;
        
        /* Pattern 3: __builtin_expect with cold path */
        /* Rarely trigger cold path */
        if (outer == OUTER_ITERATIONS - 1) {
            cold_path_trigger = 1000001; /* Force cold path on last iteration */
        }
        
        for (int i = 0; i < ARRAY_SIZE; i += 50) {
            double result = unlikely_path_operation(data, i, cold_path_trigger);
            total_sum += result;
        }
        
        /* Reset trigger */
        cold_path_trigger = 0;
        
        /* Pattern 4: Mixed operations with function calls as barriers */
        for (int i = 0; i < ARRAY_SIZE / 2; i++) {
            /* Function call acts as scheduling barrier */
            double rnd = (double)rand() / RAND_MAX;
            
            /* Operations after barrier */
            data[i] = data[i] * rnd + sqrt(data[ARRAY_SIZE - i - 1]);
            
            /* More integer math */
            int_data[i] = (int_data[i] + int_data[ARRAY_SIZE - i - 1]) % 1000;
            int_data[i] = int_data[i] * int_data[i] - int_data[i];
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Continue with dependent operations */
            if (i % 7 == 0) {
                double temp = data[i] * 2.0;
                data[i] = sin(temp) + cos(temp);
                int_sum += (int)(data[i] * 100);
            }
        }
        
        /* Final VLA usage in outer loop */
        double final_vla = use_vla(10 + outer % 20);
        total_sum += final_vla;
    }
    
    /* Ensure result is used to prevent optimization */
    printf("Total sum: %f\n", total_sum);
    printf("Integer sum: %d\n", int_sum % 10000);
    
    free(data);
    free(int_data);
    
    return 0;
}
