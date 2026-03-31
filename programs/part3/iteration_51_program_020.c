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
    int vla[size];
    for (int i = 0; i < size; ++i) {
        vla[i] = (seed + i) * 3 - (seed % 7);
    }
    /* Use the VLA to prevent elimination */
    asm volatile ("" : : "r"(vla[size-1]) : "memory");
}

/* Another helper with different VLA pattern */
__attribute__((noinline))
void vla_complex(int iter) {
    int size = (iter % 20) + 10;
    double vla[size];
    double acc = 0.0;
    
    for (int i = 0; i < size; ++i) {
        vla[i] = sin(i * 0.1) * cos(iter * 0.05);
        acc += vla[i];
    }
    
    /* Force side effect */
    if (acc > 1000.0) {  /* Never true, but compiler doesn't know */
        printf("Impossible\n");
    }
}

/* Complex dependency chain in a single basic block */
double complex_block(double* arr, int idx, double init) {
    double a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Long dependency chain with mixed operations */
    a = arr[idx] + init;
    b = a * arr[idx + 1] - sin(a);
    c = b / (fabs(arr[idx + 2]) + 1.0);
    d = c * c * c - sqrt(fabs(c));
    
    /* Memory barrier */
    asm volatile ("" ::: "memory");
    
    e = d + arr[idx + 3] * arr[idx + 4];
    f = e / (cos(d) + 2.0);
    g = f * tan(e * 0.01);
    h = g + arr[idx + 5] - arr[idx + 6];
    
    /* Another barrier */
    asm volatile ("" ::: "memory");
    
    i = h * log(fabs(h) + 1.0);
    j = i + sin(h) * cos(i);
    k = j / (exp(fabs(j) * 0.01) + 1.0);
    l = k * arr[idx + 7] + arr[idx + 8] * arr[idx + 9];
    
    m = l - sqrt(fabs(l));
    n = m * tan(m * 0.001);
    o = n + cos(m) * sin(n);
    p = o / (fabs(o) + 1.0) * arr[idx + 10];
    
    return p;
}

int main() {
    double data[ARRAY_SIZE];
    double checksum = 0.0;
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    /* Main driver loop */
    for (int outer = 0; outer < OUTER_LOOPS; ++outer) {
        /* Pattern 1: Large dependency chain basic block */
        for (int i = 0; i < 10; ++i) {
            double result = complex_block(data, (outer * 17 + i * 13) % (ARRAY_SIZE - 20), 
                                         sin(outer * 0.1));
            checksum += result;
        }
        
        /* Call VLA helper between patterns */
        vla_helper((outer % 15) + 5, outer);
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_loops = rand() % 50 + 10;  /* Data-dependent */
        for (int j = 0; j < inner_loops; ++j) {
            double acc = 0.0;
            /* Inner loop with mixed operations */
            for (int k = 0; k < 25; ++k) {
                int idx = (j * 29 + k * 31 + outer) % ARRAY_SIZE;
                acc += data[idx] * data[(idx + 1) % ARRAY_SIZE];
                acc -= sin(data[(idx + 2) % ARRAY_SIZE] * 0.01);
                acc *= 0.99 + cos(j * 0.1) * 0.01;
                
                /* Memory operations with different addressing */
                double temp = data[(idx + k) % ARRAY_SIZE];
                data[(idx + k + 1) % ARRAY_SIZE] = temp * 0.9 + acc * 0.1;
            }
            checksum += acc;
        }
        
        vla_complex(outer);
        
        /* Pattern 3: Block with inline assembly barriers */
        {
            double x = checksum * 0.01;
            double y = 0.0;
            
            for (int i = 0; i < 8; ++i) {
                x = x * 1.1 + sin(x) * 0.1;
                y += x;
                
                /* Barrier every 2 iterations */
                if (i % 2 == 1) {
                    asm volatile ("" ::: "memory");
                }
            }
            
            /* Mixed integer/floating point */
            int int_part = (int)fabs(x);
            for (int i = 0; i < int_part % 10; ++i) {
                y += (i * y) / (i + 1.0);
            }
            
            checksum += y;
        }
        
        /* Pattern 4: __builtin_expect with cold path */
        int rare_condition = (outer == 37 || outer == 73);  /* Rare cases */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_acc = 0.0;
            for (int i = 0; i < 100; ++i) {
                cold_acc += sqrt(fabs(data[i])) * tan(data[i] * 0.001);
                cold_acc -= log(fabs(data[ARRAY_SIZE - 1 - i]) + 1.0);
                
                /* Use alloca in cold path */
                double* dyn = (double*)alloca(sizeof(double) * 8);
                for (int j = 0; j < 8; ++j) {
                    dyn[j] = cold_acc * j;
                    cold_acc += dyn[j] * 0.1;
                }
            }
            checksum += cold_acc * 0.01;
        } else {
            /* Hot path - simpler */
            checksum += cos(outer * 0.01) * 0.001;
        }
        
        /* More VLA usage */
        {
            int vla_size = (outer % 8) + 3;
            float vla_f[vla_size];
            for (int i = 0; i < vla_size; ++i) {
                vla_f[i] = checksum * i * 0.0001f;
                checksum += vla_f[i];
            }
        }
        
        /* Function calls as scheduling barriers */
        double rnd = (double)rand() / RAND_MAX;
        checksum += rnd * 0.0001;
        
        /* Another complex block with dependencies */
        double chain = checksum;
        for (int i = 0; i < 5; ++i) {
            chain = chain * 1.05 + sin(chain) * 0.05;
            chain = chain / (cos(chain * 0.1) + 1.5);
            chain = sqrt(fabs(chain)) + chain * 0.5;
        }
        checksum = chain;
    }
    
    printf("Final checksum: %.15f\n", checksum);
    return 0;
}
