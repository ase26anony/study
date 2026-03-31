/* test_early_remat.c - Test program to trigger GCC's early rematerialization pass */
/* Compile with: gcc -O2 -c test_early_remat.c -fdump-rtl-early_remat -da */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_compute(int x, int y) {
    return (x * 3 + y * 7) ^ 0x1234;
}

/* Noinline to prevent inlining from simplifying control flow */
static int __attribute__((noinline)) compute_offset(int i, int j) {
    volatile int dummy = 0; /* Prevent optimization */
    return (i * 17 + j * 23) & 0xFF;
}

/* Complex structure to create varied addressing modes */
struct MixedData {
    int id;
    double value;
    float factor;
    char tag;
    int32_t data[4];
};

/* Hot function 1: Creates high register pressure with mixed types */
static long __attribute__((noinline)) 
hot_function1(struct MixedData *arr, int size, int iterations) {
    long total = 0;
    double acc_double = 0.0;
    float acc_float = 0.0f;
    int *temp_ptrs[8];
    
    /* Initialize some pointers to create register pressure */
    for (int i = 0; i < 8; i++) {
        temp_ptrs[i] = &arr[i % size].id;
    }
    
    /* Outer loop with high register pressure */
    for (int iter = 0; iter < iterations; iter++) {
        /* Many live variables of different types */
        int base = pure_compute(iter, size);
        double scale = sin(iter * 0.01) + 1.0;
        float factor = cos(iter * 0.005) * 2.0f;
        
        /* Inner loop with complex expressions */
        #pragma GCC optimize ("O2")
        for (int i = 0; i < size; i++) {
            /* Multiple expressions that could be rematerialized */
            int offset1 = compute_offset(i, iter);
            int offset2 = pure_compute(i, offset1);
            double val1 = arr[i].value * scale;
            float val2 = arr[i].factor * factor;
            
            /* Force register pressure with many intermediate values */
            int t1 = offset1 * 3;  /* Candidate for remat */
            int t2 = offset2 * 7;  /* Candidate for remat */
            int t3 = t1 + t2;
            int t4 = t3 ^ base;
            
            /* Array accesses with different addressing modes */
            int idx1 = (t4 + i) % 4;
            int idx2 = (t4 - i) & 3;
            
            /* Mixed type computations */
            double dtemp = val1 * t1 + val2 * t2;
            float ftemp = (float)dtemp * arr[i].factor;
            
            /* Pointer arithmetic creating register pressure */
            int *ptr1 = &arr[(i + idx1) % size].data[idx2];
            int *ptr2 = &arr[(i + idx2) % size].data[idx1];
            
            /* Use inline assembly to force register usage */
            asm volatile ("# Force register usage %0 %1" 
                         : "+r" (ptr1), "+r" (ptr2));
            
            /* Complex expression using t1, t2 multiple times */
            total += *ptr1 * t1 + *ptr2 * t2;
            acc_double += dtemp;
            acc_float += ftemp;
            
            /* Conditional to create control flow complexity */
            if (t3 & 1) {
                /* Another use of t1, t2 - potential remat candidates */
                total -= (t1 - t2) * (i % 16);
                goto skip_point; /* Non-trivial control flow */
            }
            
            total += t4 * (i % 8);
            
        skip_point:
            /* Use computed values again */
            if (t4 & 2) {
                total += arr[i].id * t3;
            }
        }
        
        /* Use values across loop iterations */
        total += (long)(acc_double * 100) + (long)(acc_float * 50);
    }
    
    return total;
}

/* Hot function 2: Nested loops with register pressure */
static int __attribute__((noinline))
hot_function2(int *data, int width, int height) {
    int sum = 0;
    register int r1, r2, r3, r4, r5; /* Hint for register allocation */
    
    /* Create many live variables */
    int stride = width * 2;
    int half_width = width / 2;
    int quarter_height = height / 4;
    
    /* Complex loop structure */
    for (int y = 0; y < height; y++) {
        int y_offset = y * width;
        int y_factor = pure_compute(y, height);
        
        for (int x = 0; x < width; x++) {
            /* Multiple expressions that are cheap to recompute */
            int pos = y_offset + x;
            int x_factor = pure_compute(x, width);
            
            /* Many intermediate values */
            r1 = data[pos] * 3;      /* Remat candidate */
            r2 = x_factor * 5;       /* Remat candidate */
            r3 = y_factor * 7;       /* Remat candidate */
            r4 = (r1 + r2) ^ r3;
            r5 = (r1 - r2) | r3;
            
            /* Use values multiple times in complex expressions */
            sum += r4 * (x % 16);
            sum += r5 * (y % 16);
            sum += (r1 * r2) >> 4;
            sum += (r3 * r4) & 0xFF;
            
            /* Array access with stride */
            if (x < half_width && y < quarter_height) {
                int idx = (y * stride + x * 2) % (width * height);
                sum += data[idx] * r1;  /* Another use of r1 */
            }
            
            /* Switch statement for control flow complexity */
            switch (x % 4) {
                case 0:
                    sum += r2 * 2;  /* Use r2 again */
                    break;
                case 1:
                    sum += r3 * 3;  /* Use r3 again */
                    break;
                case 2:
                    sum += r4 * 4;  /* Use r4 again */
                    break;
                default:
                    sum += r5 * 5;  /* Use r5 again */
                    /* Fall through to create edge case */
            }
        }
        
        /* Cross-iteration dependencies */
        if (y > 0) {
            int prev_pos = (y - 1) * width + (y % width);
            sum += data[prev_pos] * y_factor;  /* Use y_factor again */
        }
    }
    
    return sum;
}

/* Hot function 3: Pointer chasing with mixed types */
static double __attribute__((noinline))
hot_function3(double *array, int size, int *indices, int idx_count) {
    double result = 0.0;
    double *current = array;
    int step = 0;
    
    /* Unrolled loop to increase register pressure */
    for (int i = 0; i < idx_count; i += 4) {
        /* Multiple pointer computations */
        double *p1 = array + indices[i % size];
        double *p2 = array + indices[(i + 1) % size];
        double *p3 = array + indices[(i + 2) % size];
        double *p4 = array + indices[(i + 3) % size];
        
        /* Expressions that could be rematerialized */
        double scale1 = sin(step * 0.1) * 2.0;
        double scale2 = cos(step * 0.05) * 3.0;
        double scale3 = scale1 * scale2;  /* Remat candidate */
        double scale4 = scale1 + scale2;  /* Remat candidate */
        
        /* Complex addressing */
        double val1 = *p1 * scale1;
        double val2 = *p2 * scale2;
        double val3 = *p3 * scale3;  /* Use scale3 */
        double val4 = *p4 * scale4;  /* Use scale4 */
        
        /* Multiple uses of computed values */
        result += val1 * scale3;  /* Use scale3 again */
        result += val2 * scale4;  /* Use scale4 again */
        result += val3 * scale1;  /* Use scale1 again */
        result += val4 * scale2;  /* Use scale2 again */
        
        /* More computations with the same values */
        result += (val1 + val2) * (scale3 - scale4);
        result += (val3 - val4) * (scale1 + scale2);
        
        /* Update step in a way that prevents optimization */
        step = (step * 13 + i) % 100;
        
        /* Conditional with goto for complex CFG */
        if (step & 1) {
            goto update_pointers;
        }
        
        result *= 0.99;
        
    update_pointers:
        /* Pointer arithmetic that might need registers */
        current = array + (step % size);
        result += *current * (scale3 + scale4);  /* Use scale3, scale4 again */
    }
    
    return result;
}

/* Main function that calls all hot functions */
int main(void) {
    const int ARRAY_SIZE = 256;
    const int ITERATIONS = 1000;
    const int WIDTH = 64;
    const int HEIGHT = 64;
    
    /* Allocate and initialize data structures */
    struct MixedData *data_array = malloc(ARRAY_SIZE * sizeof(struct MixedData));
    int *int_data = malloc(WIDTH * HEIGHT * sizeof(int));
    double *double_array = malloc(ARRAY_SIZE * sizeof(double));
    int *indices = malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data_array || !int_data || !double_array || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i].id = i;
        data_array[i].value = sin(i * 0.1) * 100.0;
        data_array[i].factor = cos(i * 0.05) * 2.0f;
        data_array[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 4; j++) {
            data_array[i].data[j] = (i * 17 + j * 23) & 0xFF;
        }
        
        double_array[i] = data_array[i].value * 1.5;
        indices[i] = (i * 13) % ARRAY_SIZE;
    }
    
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        int_data[i] = (i * 29) % 256;
    }
    
    /* Call hot functions to trigger rematerialization */
    long result1 = hot_function1(data_array, ARRAY_SIZE, ITERATIONS / 10);
    int result2 = hot_function2(int_data, WIDTH, HEIGHT);
    double result3 = hot_function3(double_array, ARRAY_SIZE, indices, ITERATIONS);
    
    /* Combine results to prevent dead code elimination */
    long final_result = (long)(result1 + result2 + result3);
    
    /* Print result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    /* Cleanup */
    free(data_array);
    free(int_data);
    free(double_array);
    free(indices);
    
    return 0;
}
