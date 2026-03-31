/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Complex data-dependent computation with mixed operations */
static inline int64_t complex_op(int64_t a, int64_t b, float c, double d) {
    /* Mixed-width operations and conditional moves */
    int32_t a32 = (int32_t)a;
    int64_t result = (a > b) ? (a * b) : (a + b * 2);
    
    /* Floating point operations */
    float f_result = (float)result * c;
    double d_result = (double)f_result / (d + 1e-10);
    
    /* Non-constant division creates complex RTL */
    result = (int64_t)d_result + (a32 / (b & 0xFF + 1));
    
    /* Pointer chasing simulation */
    int64_t* ptr = &result;
    *ptr += (*ptr & 0xF);
    
    return result;
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(int n, float* restrict mat, float* restrict vec, 
                     float* restrict result) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; ++i) {
        float sum = 0.0f;
        volatile int j_bound = n;  /* Volatile to prevent optimization */
        
        for (int j = 0; j < j_bound; ++j) {
            /* Data-dependent computation with stride access */
            float val = mat[i * n + j];
            
            /* Complex addressing mode */
            float vec_val = vec[(j + i) % n];
            
            /* Mixed operations */
            sum += val * vec_val;
            
            /* Conditional operation */
            if (val > vec_val) {
                sum -= 0.5f * (val - vec_val);
            } else {
                sum += 0.3f * (vec_val - val);
            }
            
            /* Inline assembly to create fixed RTL instructions */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Non-trivial reduction */
        result[i] = sum / (float)(n + 1);
    }
}

/* Main computation with nested loops and complex dependencies */
uint64_t compute_kernel(int size, int64_t* data) {
    uint64_t total = 0;
    int outer_bound = g_volatile_bound % size;
    
    /* Outer loop with volatile control */
    for (int i = 1; i < outer_bound; i += 1 + (rand() % 3)) {
        int64_t running_sum = data[i];
        float fp_acc = (float)running_sum;
        double dp_acc = 0.0;
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < size - 1; j++) {
            /* Data-dependent computation across iterations */
            int64_t prev = (j > 0) ? data[j - 1] : data[size - 1];
            int64_t curr = data[j];
            int64_t next = data[(j + 1) % size];
            
            /* Complex computation with mixed operations */
            int64_t temp = complex_op(curr, prev, fp_acc, dp_acc);
            
            /* Running product with dependency */
            running_sum += temp * (next % 256 + 1);
            
            /* Floating point accumulation */
            fp_acc += (float)temp * 0.01f;
            dp_acc = dp_acc * 0.99 + (double)temp * 0.001;
            
            /* Switch statement creating multiple basic blocks */
            switch (temp & 0x7) {
                case 0:
                    running_sum >>= 1;
                    break;
                case 1:
                    running_sum *= 3;
                    break;
                case 2:
                    running_sum ^= 0xAAAAAAAA;
                    break;
                case 3:
                    running_sum = (running_sum << 4) | (running_sum >> 60);
                    break;
                default:
                    running_sum += (temp & 0xFFF);
                    break;
            }
            
            /* Memory barrier via inline assembly */
            asm volatile ("" : : "r"(running_sum), "r"(fp_acc) : "memory");
        }
        
        /* Final reduction with volatile */
        volatile int64_t vol_sum = running_sum;
        total ^= (uint64_t)vol_sum;
        total += (uint64_t)(fp_acc * 100.0f);
    }
    
    return total;
}

/* Second computation kernel with different pattern */
double compute_kernel2(int size, double* data) {
    double result = 0.0;
    int iter = g_volatile_seed % 100 + 50;
    
    for (int k = 0; k < iter; k++) {
        double acc = 1.0;
        
        #pragma GCC unroll 2
        for (int i = 0; i < size; i++) {
            /* Pointer chasing with stride */
            double* ptr = data + i;
            double val = *ptr;
            
            /* Complex floating point operations */
            if (val != 0.0) {
                acc *= val + (double)(i % 8);
                acc /= (val > 0.0) ? (val + 0.5) : (1.0 - val);
            }
            
            /* Trigonometric approximation */
            double angle = acc * 3.14159 / 180.0;
            double sin_approx = angle - (angle*angle*angle)/6.0;
            acc += sin_approx * 0.1;
            
            /* Conditional store */
            data[i] = (i % 3 == 0) ? acc : val;
        }
        
        result += acc / (double)(k + 1);
        
        /* External function call prevents optimization */
        if (k % 10 == 0) {
            g_volatile_seed = rand();
        }
    }
    
    return result;
}

int main(void) {
    const int SIZE = 512;
    const int MAT_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    int64_t* data = (int64_t*)malloc(SIZE * sizeof(int64_t));
    double* dbl_data = (double*)malloc(SIZE * sizeof(double));
    float* matrix = (float*)malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float* vector = (float*)malloc(MAT_SIZE * sizeof(float));
    float* result = (float*)malloc(MAT_SIZE * sizeof(float));
    
    if (!data || !dbl_data || !matrix || !vector || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varied data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (int64_t)rand() * rand();
        dbl_data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        matrix[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        vector[i] = (float)rand() / RAND_MAX;
    }
    
    /* Perform computations that should trigger selective scheduling */
    printf("Starting complex computations...\n");
    
    /* First kernel with integer-heavy operations */
    uint64_t total1 = compute_kernel(SIZE, data);
    
    /* Matrix-vector multiplication */
    matvec_multiply(MAT_SIZE, matrix, vector, result);
    
    /* Second kernel with floating-point operations */
    double total2 = compute_kernel2(SIZE, dbl_data);
    
    /* Final reduction to prevent dead code elimination */
    float final_result = 0.0f;
    for (int i = 0; i < MAT_SIZE; i++) {
        final_result += result[i];
    }
    
    /* Print results to ensure side effects */
    printf("Result 1: %llu\n", (unsigned long long)total1);
    printf("Result 2: %f\n", total2);
    printf("Matrix result sum: %f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(dbl_data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
