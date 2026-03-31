/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex data-dependent computation with mixed operations */
static inline int64_t compute_kernel(int32_t a, int32_t b, int32_t c, float* fp_temp) {
    /* Mixed integer operations */
    int64_t wide = (int64_t)a * (int64_t)b;
    int32_t narrow = (int32_t)(wide >> 16);
    
    /* Floating point operations */
    float f1 = (float)a * 1.5f;
    float f2 = (float)b * 0.75f;
    float f3 = f1 / (f2 + 0.001f);  /* Avoid division by zero */
    
    /* Conditional move via ternary */
    int32_t cond = (a > b) ? (a - b) : (b - a);
    
    /* Mixed-width arithmetic */
    int64_t result = wide + (int64_t)cond * (int64_t)narrow;
    
    /* Store floating point result */
    *fp_temp = f3;
    
    /* Inline assembly to create fixed RTL pattern */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result;
}

/* Matrix-vector multiplication kernel */
static void matvec_multiply(int32_t* restrict mat, int32_t* restrict vec, 
                           int32_t* restrict result, int n, int m) {
    for (int i = 0; i < n; i++) {
        int32_t sum = 0;
        #pragma GCC unroll 4
        for (int j = 0; j < m; j++) {
            /* Complex addressing with stride */
            int idx = i * m + j;
            
            /* Data-dependent computation with carried dependency */
            if (j > 0) {
                sum += mat[idx] * vec[j] + (mat[idx - 1] & 0xFF);
            } else {
                sum += mat[idx] * vec[j];
            }
            
            /* Conditional branch with computation in both paths */
            if (vec[j] > 100) {
                /* Division with non-constant divisor */
                sum /= (vec[j] % 7 + 1);
                
                /* Floating point in integer loop */
                float temp;
                compute_kernel(mat[idx], vec[j], sum, &temp);
                sum += (int32_t)temp;
            } else {
                /* Different computation path */
                sum ^= (mat[idx] << 3);
                
                /* Pointer chasing pattern */
                int32_t* ptr = &mat[idx];
                for (int k = 0; k < 2; k++) {
                    if (k == 0) {
                        sum += *ptr;
                        ptr = &vec[j];
                    } else {
                        sum += *ptr * 2;
                    }
                }
            }
            
            /* Another inline assembly barrier */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        result[i] = sum;
    }
}

/* Main computation with nested loops and complex dependencies */
static int64_t run_computation(int32_t* data, int size) {
    int64_t total = 0;
    volatile int volatile_counter = 0;
    
    /* Use volatile in loop bound */
    int bound = volatile_bound;
    
    /* Outer loop with data-dependent trip count */
    for (int i = 1; i < bound; i++) {
        /* Volatile check to prevent optimization */
        if (volatile_counter++ > 1000000) break;
        
        int inner_bound = (rand() % 50) + 10;  /* Variable inner loop bound */
        
        /* Middle loop */
        for (int j = 0; j < inner_bound; j++) {
            float fp_temp[2];
            int32_t idx1 = (i * 17 + j * 13) % size;
            int32_t idx2 = (i * 23 + j * 19) % size;
            
            /* Complex addressing with modulo */
            int32_t* ptr1 = &data[idx1];
            int32_t* ptr2 = &data[idx2];
            
            /* Switch statement with multiple cases */
            switch ((i + j) % 4) {
                case 0:
                    /* Case with heavy computation */
                    total += compute_kernel(*ptr1, *ptr2, i, &fp_temp[0]);
                    total ^= (int64_t)(*ptr1) << 32;
                    break;
                case 1:
                    /* Different computation path */
                    total -= compute_kernel(*ptr2, *ptr1, j, &fp_temp[1]);
                    total |= (int64_t)(*ptr2) << 16;
                    break;
                case 2:
                    /* Mixed operations */
                    total *= (compute_kernel(i, j, *ptr1, &fp_temp[0]) & 0xFFFF);
                    total += (*ptr1) * (*ptr2);
                    break;
                default:
                    /* Division operation */
                    if (*ptr2 != 0) {
                        total /= (*ptr2 % 8 + 1);
                    }
                    total += compute_kernel(j, i, *ptr2, &fp_temp[1]);
                    break;
            }
            
            /* Data dependency across iterations */
            if (j > 0) {
                data[idx1] = (int32_t)(total & 0xFFFFFFFF) + data[(idx1 - 1) % size];
            }
            
            /* Another memory barrier */
            asm volatile ("" : : "r"(total), "r"(data[idx1]) : "memory");
        }
        
        /* Periodic reduction */
        if (i % 7 == 0) {
            total = (total >> 1) | (total << 63);  /* Rotate */
        }
    }
    
    return total;
}

int main(void) {
    const int size = 1024;
    const int mat_size = 64;
    
    /* Initialize with pseudo-random data */
    int32_t* data = (int32_t*)malloc(size * sizeof(int32_t));
    int32_t* matrix = (int32_t*)malloc(mat_size * mat_size * sizeof(int32_t));
    int32_t* vector = (int32_t*)malloc(mat_size * sizeof(int32_t));
    int32_t* result = (int32_t*)malloc(mat_size * sizeof(int32_t));
    
    /* Simple PRNG for initialization */
    uint32_t seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int32_t)(seed >> 16);
    }
    
    for (int i = 0; i < mat_size * mat_size; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (int32_t)(seed >> 16) % 100;
    }
    
    for (int i = 0; i < mat_size; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (int32_t)(seed >> 16) % 100;
    }
    
    /* Run first computation kernel */
    int64_t total1 = run_computation(data, size);
    
    /* Run second computation kernel (matrix-vector) */
    matvec_multiply(matrix, vector, result, mat_size, mat_size);
    
    /* Final reduction across both computations */
    int64_t total2 = 0;
    for (int i = 0; i < mat_size; i++) {
        total2 += result[i];
        /* Mix with first result */
        total2 ^= total1 >> (i % 64);
    }
    
    /* Final result with volatile store */
    volatile int64_t final_result = total1 ^ total2;
    
    /* Print to prevent optimization */
    printf("Result: %ld\n", (long)final_result);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
