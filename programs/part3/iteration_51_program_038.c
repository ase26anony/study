#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define OUTER_LOOPS 100
#define INNER_BASE 50

/* Helper function using VLA - marked noinline to prevent optimization */
__attribute__((noinline)) 
void use_vla(int size) {
    int vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2 + (i % 3);
    }
    /* Use the VLA to prevent elimination */
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += vla[i];
    }
}

/* Another helper with alloca */
__attribute__((noinline))
void use_alloca(int size) {
    int* dyn = (int*)alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        dyn[i] = (i * 3) % 17;
    }
    volatile int check = 0;
    for (int i = 0; i < size; i++) {
        check ^= dyn[i];
    }
}

int main() {
    double arr[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    int int_arr[ARRAY_SIZE];
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = (double)rand() / RAND_MAX * 100.0;
        arr2[i] = (double)rand() / RAND_MAX * 50.0;
        int_arr[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_sum = 0;
    
    /* Outer loop - driver */
    for (int outer = 0; outer < OUTER_LOOPS; outer++) {
        /* Pattern 1: Large dependency chain basic block */
        double a = arr[outer % ARRAY_SIZE];
        double b = arr2[outer % ARRAY_SIZE];
        double c = sin(a) * cos(b);
        double d = sqrt(fabs(c)) + tan(a/b);
        double e = d * 3.14159 / (a + 1.0);
        double f = exp(log(e + 1.0)) * pow(2.0, c);
        double g = fmod(f, 7.0) + remainder(d, 3.0);
        double h = g * g - 2.0 * g + 1.0;
        
        /* Chain continues with integer ops */
        int x = int_arr[outer % ARRAY_SIZE];
        int y = x * 3 + 7;
        int z = y % 13 + (y / 5);
        int w = z * z - z * 2 + 1;
        
        /* Memory access pattern */
        arr[(x + outer) % ARRAY_SIZE] = h;
        int_arr[(y + outer) % ARRAY_SIZE] = w;
        
        total_sum += h;
        int_sum += w;
        
        /* Inline assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 2: Nested loops with data-dependent bounds */
        int inner_limit = rand() % INNER_BASE + 10;
        for (int i = 0; i < inner_limit; i++) {
            /* Complex inner loop with mixed operations */
            for (int j = 0; j < (rand() % 5 + 2); j++) {
                double* ptr = &arr[(i * j + outer) % ARRAY_SIZE];
                double temp = *ptr;
                *ptr = temp * 1.01 + sin(temp) * 0.1;
                
                int* iptr = &int_arr[(i + j + outer) % ARRAY_SIZE];
                *iptr = (*iptr * 3 + 7) % 97;
                
                /* More FP operations */
                double t1 = arr2[(i * 3) % ARRAY_SIZE];
                double t2 = arr2[(j * 7) % ARRAY_SIZE];
                arr2[(i + j) % ARRAY_SIZE] = t1 * t2 - t1 / (t2 + 0.001);
            }
            
            /* Call function with VLA between inner iterations */
            if (i % 3 == 0) {
                use_vla((i % 10) + 5);
            }
        }
        
        /* Another assembly barrier */
        asm volatile ("" ::: "memory");
        
        /* Pattern 3: __builtin_expect with cold path */
        int rare_condition = (rand() % 10000 == 0); /* Very rare */
        if (__builtin_expect(rare_condition, 0)) {
            /* Cold path - complex operations */
            double cold_sum = 0.0;
            for (int k = 0; k < 100; k++) {
                cold_sum += sqrt(arr[k] * arr2[k]) * sin(k * 0.01);
                cold_sum = fmod(cold_sum, 1000.0);
                
                /* Use alloca in cold path */
                use_alloca((k % 20) + 1);
            }
            total_sum += cold_sum * 0.001;
        }
        
        /* Pattern 4: More complex dependency chains */
        double chain_start = arr[outer % ARRAY_SIZE];
        for (int step = 0; step < 20; step++) {
            chain_start = chain_start * 1.5 - sin(chain_start) * 0.3;
            chain_start = chain_start + cos(chain_start * 0.5) * 0.2;
            chain_start = fabs(chain_start) + 0.1;
            
            /* Integer operations intertwined */
            int idx = (step * 7 + outer) % ARRAY_SIZE;
            int_arr[idx] = (int_arr[idx] + (int)(chain_start * 100)) % 1000;
            
            /* Memory store with addressing mode variation */
            arr[(idx * 3) % ARRAY_SIZE] = chain_start;
            arr2[(idx * 5 + 1) % ARRAY_SIZE] = chain_start * 0.5;
        }
        
        total_sum += chain_start;
        
        /* Pattern 5: Function calls as scheduling barriers */
        for (int fn = 0; fn < 3; fn++) {
            /* Call rand() which acts as a barrier */
            int r = rand() % 100;
            double s = sqrt((double)r);
            double t = sin(s * 3.14159 / 180.0);
            
            /* Dependent operations after call */
            arr[(outer + fn) % ARRAY_SIZE] = arr[(outer + fn) % ARRAY_SIZE] * t + s;
            int_arr[(outer + fn * 2) % ARRAY_SIZE] = (int)(s * 1000) % 500;
            
            /* Another assembly barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Use VLA between outer iterations */
        use_vla((outer % 15) + 3);
    }
    
    /* Final computation to prevent elimination */
    double final_result = total_sum / OUTER_LOOPS + int_sum * 0.001;
    
    /* Additional complex block to ensure scheduler context usage */
    {
        double a = final_result;
        double b = sin(a);
        double c = cos(a);
        double d = a * b + c * a - b / (c + 0.001);
        double e = sqrt(fabs(d)) * tan(d * 0.01);
        double f = exp(e) - log(fabs(e) + 1.0);
        final_result = fmod(f, 1000.0);
        
        /* Integer operations */
        int ia = (int)(final_result * 1000);
        int ib = ia * 3 + ia / 2 - ia % 7;
        int ic = ib ^ (ib >> 3) ^ (ib << 5);
        int id = ic % 1000 + ic / 1000;
        
        final_result += id * 0.001;
    }
    
    printf("Result: %f\n", final_result);
    
    /* Force exit with result to prevent tail optimization */
    if (final_result > 1000000.0) {
        /* This should never happen, but prevents optimization */
        abort();
    }
    
    return (int)(final_result * 1000) % 255;
}
