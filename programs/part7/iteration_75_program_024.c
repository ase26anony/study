#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile void* g_volatile_ptr = NULL;

/* Memory barrier */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Non-inlineable helper functions */
static __attribute__((noinline)) 
int helper_func1(int a, int b) {
    COMPILER_BARRIER();
    int result = (a * b) ^ (a + b);
    g_volatile_counter++;
    return result;
}

static __attribute__((noinline))
float helper_func2(float* arr, int idx) {
    COMPILER_BARRIER();
    float val = arr[idx] * 1.5f;
    arr[idx] = val + (float)g_volatile_counter;
    return val;
}

/* Inlineable helper */
static inline int fast_mul(int x, int y) {
    return x * y + (x ^ y);
}

/* Complex computation kernel */
static int compute_kernel(int* int_arr, float* float_arr, double* double_arr,
                         struct PackedData* packed_arr, int size, int iter) {
    int result = 0;
    int i, j;
    
    /* Pointer chasing simulation */
    int* chase_ptr = &int_arr[iter % size];
    for (i = 0; i < 50; i++) {
        int next_idx = (*chase_ptr) % size;
        chase_ptr = &int_arr[next_idx];
        result ^= *chase_ptr;
        COMPILER_BARRIER();
    }
    
    /* Nested loops with loop-carried dependencies */
    double sum = 0.0;
    for (i = 1; i < 100; i++) {
        float temp = float_arr[i];
        for (j = 1; j < 10; j++) {
            temp = temp * 0.99f + float_arr[i-1] * 0.01f;
            sum += (double)temp * double_arr[j % size];
        }
        float_arr[i] = temp;
    }
    
    /* Mixed integer/float operations */
    int int_sum = 0;
    float float_sum = 0.0f;
    for (i = 0; i < size; i++) {
        int_sum += int_arr[i] * (i + 1);
        float_sum += float_arr[i] / (i + 1.0f);
        double_arr[i] = (double)int_sum * (double)float_sum;
    }
    
    /* Deep conditional chain */
    int branch_var = result % 8;
    if (branch_var == 0) {
        result = helper_func1(result, int_sum);
    } else if (branch_var == 1) {
        result = fast_mul(result, int_sum);
        float dummy = helper_func2(float_arr, result % size);
        result += (int)dummy;
    } else if (branch_var == 2) {
        for (i = 0; i < size; i += 2) {
            int_arr[i] = (int_arr[i] << 3) | (int_arr[i] >> 5);
        }
    } else if (branch_var == 3) {
        result = ~result;
    } else if (branch_var == 4) {
        double* dp = double_arr;
        for (i = 0; i < size; i++) {
            *dp = sqrt(fabs(*dp)) + 1.0;
            dp++;
        }
    } else if (branch_var == 5) {
        struct PackedData* p = &packed_arr[result % size];
        p->i = result;
        p->d = (double)result * 0.5;
        result = p->i ^ (int)p->d;
    } else if (branch_var == 6) {
        /* Function pointer computation */
        ComputeFunc funcs[] = {helper_func1, fast_mul};
        int idx = (result >> 4) & 1;
        result = funcs[idx](result, int_sum);
    } else {
        /* Default: complex arithmetic chain */
        int a = result;
        int b = int_sum;
        int c = a * b + (a ^ b);
        int d = c << (a % 16);
        int e = d / (b ? b : 1);
        int f = e ^ d ^ c;
        result = f + (a % 256);
    }
    
    return result;
}

/* Switch-based dispatcher */
static int switch_dispatcher(int* arr, int idx, int mode) {
    int result = arr[idx];
    
    switch (mode % 10) {
        case 0:
            result = result * 3 + 1;
            result = (result << 1) | (result >> 31);
            break;
        case 1:
            result = result ^ 0xAAAAAAAA;
            result = result * 13;
            break;
        case 2:
            result = result + arr[(idx + 1) % 1024];
            result = result - arr[(idx + 2) % 1024];
            break;
        case 3:
            result = (result & 0xFF) << 24 |
                    (result & 0xFF00) << 8 |
                    (result & 0xFF0000) >> 8 |
                    (result & 0xFF000000) >> 24;
            break;
        case 4:
            result = result * result;
            result = result % 10007;
            break;
        case 5:
            result = ~result;
            result = result * 7;
            break;
        case 6:
            result = sqrt(fabs(result));
            break;
        case 7:
            result = (result << 3) + (result >> 2);
            result = result ^ idx;
            break;
        case 8:
            result = result * 2 - result / 3;
            break;
        case 9:
            result = (result & 0x55555555) << 1 |
                    (result & 0xAAAAAAAA) >> 1;
            break;
    }
    
    COMPILER_BARRIER();
    return result;
}

int main(int argc, char** argv) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    const int ARRAY_SIZE = 1024;
    
    /* Allocate arrays with different types and alignments */
    int* int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct PackedData* packed_arr = 
        (struct PackedData*)malloc(ARRAY_SIZE * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_arr[i] = (float)(int_arr[i] % 1000) * 0.001f;
        double_arr[i] = (double)(int_arr[i] % 2000) * 0.0005;
        packed_arr[i].c = (char)(i & 0xFF);
        packed_arr[i].i = int_arr[i];
        packed_arr[i].d = double_arr[i];
        packed_arr[i].s = (short)(i & 0xFFFF);
    }
    
    g_volatile_ptr = int_arr;
    
    int final_result = 0;
    
    /* Main computation loop */
    for (int iter = 0; iter < N; iter++) {
        COMPILER_BARRIER();
        
        /* Complex kernel */
        int kernel_result = compute_kernel(int_arr, float_arr, double_arr,
                                         packed_arr, ARRAY_SIZE, iter);
        
        /* Switch-based computation */
        int switch_result = switch_dispatcher(int_arr, iter % ARRAY_SIZE, iter);
        
        /* Conditional with function call */
        int conditional_result;
        if (iter & 1) {
            conditional_result = helper_func1(kernel_result, switch_result);
        } else {
            conditional_result = fast_mul(kernel_result, switch_result);
        }
        
        /* Large sequential basic block - fills instruction queue */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Independent operations to fill scheduler queues */
            float_arr[i] = float_arr[i] * 1.01f + (float)(i % 100);
            double_arr[i] = double_arr[i] * 0.99 + sin((double)i * 0.01);
            
            /* Occasional memory barrier */
            if ((i & 31) == 0) {
                COMPILER_BARRIER();
            }
        }
        
        /* Non-linear control flow using goto */
        int label_var = conditional_result % 4;
        
        if (label_var == 0) goto label0;
        else if (label_var == 1) goto label1;
        else if (label_var == 2) goto label2;
        else goto label3;
        
    label0:
        int_arr[iter % ARRAY_SIZE] ^= 0x12345678;
        goto join_point;
    label1:
        int_arr[iter % ARRAY_SIZE] += 0x87654321;
        goto join_point;
    label2:
        int_arr[iter % ARRAY_SIZE] *= 3;
        goto join_point;
    label3:
        int_arr[iter % ARRAY_SIZE] = ~int_arr[iter % ARRAY_SIZE];
        goto join_point;
        
    join_point:
        /* Reduction across arrays */
        int local_sum = 0;
        for (int i = 0; i < 64; i++) {
            int idx = (iter + i) % ARRAY_SIZE;
            local_sum ^= int_arr[idx];
            local_sum += (int)float_arr[idx];
            local_sum ^= (int)double_arr[idx];
        }
        
        final_result ^= conditional_result + local_sum;
        final_result = (final_result << 1) | (final_result >> 31);
        
        /* Update volatile */
        g_volatile_counter = iter;
    }
    
    /* Final reduction to prevent elimination */
    int total_xor = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total_xor ^= int_arr[i];
        total_xor ^= *(int*)&float_arr[i];
        total_xor ^= ((int*)&double_arr[i])[0];
        total_xor ^= ((int*)&double_arr[i])[1];
    }
    
    final_result ^= total_xor;
    
    printf("Result: %d (volatile counter: %d)\n", 
           final_result, g_volatile_counter);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(packed_arr);
    
    return final_result & 0xFF;
}
