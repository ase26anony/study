#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function with VLA - forces stack adjustments */
__attribute__((noinline))
static double use_vla(int size) {
    double vla[size];
    double sum = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1);
        sum += vla[i];
    }
    
    /* Memory barrier to prevent optimization */
    asm volatile("" ::: "memory");
    return sum;
}

/* Another helper with alloca */
__attribute__((noinline))
static int use_alloca(int n) {
    int* data = (int*)alloca(n * sizeof(int));
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        data[i] = i * (rand() % 10);
        sum += data[i];
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
    d = sqrt(fabs(c)) + sin(c);
    e = d * d - c * c;
    f = exp(e * 0.01);
    g = f + log(fabs(d) + 1.0);
    h = g * g - f * f;
    i = sin(h) + cos(h);
    j = i * i - h * h;
    k = tan(j * 0.1);
    l = k + atan(k);
    m = l * l - k * k;
    n = m / (fabs(m) + 1.0);
    o = n + sin(n) + cos(n);
    p = o * o - n * n;
    q = sqrt(fabs(p));
    r = q + log(fabs(q) + 1.0);
    s = r * r - q * q;
    t = s / (fabs(s) + 1.0);
    
    /* Inline assembly as scheduling barrier */
    asm volatile("" ::: "memory");
    
    /* More dependent integer operations */
    int x = (int)t;
    int y = x * x - (int)s;
    int z = y % 137 + (int)r;
    int w = z * z - y * y;
    
    /* Another scheduling barrier */
    asm volatile("" ::: "memory");
    
    /* Mixed operations */
    double result = t + (double)w / 1000.0;
    result += sin(result) * cos(result);
    
    return result;
}

/* Function with __builtin_expect cold path */
__attribute__((noinline))
static double unlikely_path_operation(double* arr, int idx, int threshold) {
    double result = arr[idx];
    
    /* Hot path - usually taken */
    if (__builtin_expect(idx < threshold, 1)) {
        for (int i = 0; i < 10; ++i) {
            result += arr[idx + i] * 0.5;
        }
    } 
    /* Cold path - contains complex operations */
    else {
        /* Complex sequence in cold path */
        double temp = 0.0;
        for (int i = 0; i < 20; ++i) {
            double x = arr[idx + i % 10];
            temp += sqrt(fabs(x)) * sin(x * 0.1);
            
            /* Dependency chain */
            for (int j = 0; j < 5; ++j) {
                temp = temp * 1.1 - x * 0.05;
                x = sin(temp) + cos(temp);
            }
        }
        result = temp;
        
        /* VLA in cold path */
        double vla[15];
        for (int i = 0; i < 15; ++i) {
            vla[i] = sin(temp * i * 0.01);
            result += vla[i];
        }
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    double data[ARRAY_SIZE];
    int int_data[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = (double)(rand() % 1000) / 100.0;
        int_data[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_sum = 0;
    
    /* Outer driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        
        /* Pattern 1: Large dependency chain basic block */
        for (int i = 0; i < ARRAY_SIZE - 20; i += 10) {
            double chain_result = complex_dependency_chain(data, i);
            total_sum += chain_result;
            
            /* Interleave integer operations */
            int_sum += int_data[i] * int_data[i + 1];
            int_sum -= int_data[i + 2] % 97;
            int_sum *= (int_data[i + 3] % 17 + 1);
        }
        
        /* Use VLA helper between patterns */
        double vla_result = use_vla((rand() % 20) + 10);
        total_sum += vla_result;
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        for (int i = 0; i < 20; ++i) {
            /* Data-dependent inner loop bound */
            int inner_bound = (rand() % INNER_BASE) + 10;
            
            for (int j = 0; j < inner_bound; ++j) {
                /* Mixed operations with memory accesses */
                int idx = (i * 17 + j * 13) % ARRAY_SIZE;
                
                /* FP operations */
                data[idx] = data[idx] * 1.01 + sin(data[idx]);
                total_sum += data[idx];
                
                /* Integer operations */
                int_data[idx] = (int_data[idx] + j) * 3 - i;
                int_sum += int_data[idx] % 101;
                
                /* Memory barrier every 5 iterations */
                if (j % 5 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            
            /* Function call as scheduling barrier */
            double x = sqrt(fabs(data[i % ARRAY_SIZE]));
            total_sum += x;
        }
        
        /* Use alloca helper */
        int alloca_result = use_alloca((rand() % 15) + 5);
        int_sum += alloca_result;
        
        /* Pattern 3: __builtin_expect with cold path */
        for (int i = 0; i < ARRAY_SIZE - 10; i += 5) {
            /* Make cold path taken occasionally */
            int threshold = (outer == OUTER_LOOPS - 1) ? -1 : 500;
            
            double unlikely_result = unlikely_path_operation(data, i, threshold);
            total_sum += unlikely_result;
            
            /* More integer math */
            int_sum = (int_sum * 31 + int_data[i]) % 1000000;
        }
        
        /* Pattern 4: Complex loop with multiple barriers */
        for (int i = 0; i < 50; ++i) {
            double accum = 0.0;
            
            /* First segment */
            for (int j = 0; j < 8; ++j) {
                accum += data[(i + j) % ARRAY_SIZE] * j;
            }
            
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Second segment */
            accum = sqrt(fabs(accum));
            for (int j = 0; j < 6; ++j) {
                accum *= 1.1 - sin(accum * 0.01);
            }
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Third segment with function call */
            accum += cos(accum) * tan(accum * 0.1);
            
            total_sum += accum;
        }
        
        /* Prevent loop optimization */
        asm volatile("" ::: "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = total_sum / (ARRAY_SIZE * OUTER_LOOPS);
    final_result += (double)int_sum / 1000000.0;
    
    /* Use result to affect control flow */
    if (final_result > 1000.0) {
        printf("Large result: %f\n", final_result);
    } else {
        printf("Result: %f\n", final_result);
    }
    
    return 0;
}
