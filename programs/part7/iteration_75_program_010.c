/* haifa_sched_trigger.c - Program to trigger HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256

/* Volatile variables to create scheduling hazards */
volatile int vol_counter = 0;
volatile double vol_double = 1.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    float f;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different computation patterns */
static int helper1(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return (a * b) + (a >> 3) - (b << 2);
}

static int helper2(int a, int b) {
    volatile int temp = a;
    return (temp % 7) * b + (a ^ b);
}

static float helper3(float a, float b) {
    float result = a * b;
    asm volatile("" ::: "memory");
    result = result / (b + 1.0f);
    return result;
}

static double helper4(double a, double b) {
    volatile double v = a;
    return v * b - sin(v) + cos(b);
}

/* Non-inlineable function to create scheduling boundary */
__attribute__((noinline)) 
int complex_chain(int start, int *array, int size) {
    int result = start;
    for (int i = 0; i < size; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        array[i] = result % 100;
        
        /* Create loop-carried dependency */
        if (i > 0) {
            array[i] += array[i-1] * 3;
        }
        
        /* Mixed integer/floating point operations */
        float f = (float)array[i] * 0.5f;
        double d = (double)f * 1.234;
        array[i] = (int)(d * 100.0);
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return result;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int value, int *data) {
    int result = value;
    
    switch (value % 10) {
        case 0:
            result = data[0] * data[1] + data[2];
            result >>= 2;
            break;
        case 1:
            result = (data[0] ^ data[1]) | data[2];
            result = result * 3 - 7;
            break;
        case 2:
            result = data[0] + data[1] * 2;
            result = result / (data[2] + 1);
            break;
        case 3:
            result = data[0] - data[1] + data[2];
            result = result * result;
            break;
        case 4:
            result = (data[0] << 3) | (data[1] >> 2);
            result = result & data[2];
            break;
        case 5:
            result = data[0] * 7 + data[1] * 3;
            result = result - data[2] * 5;
            break;
        case 6:
            result = data[0] ^ data[1] ^ data[2];
            result = ~result;
            break;
        case 7:
            result = data[0] % 17 + data[1] % 13;
            result = result * data[2];
            break;
        case 8:
            result = data[0] + data[1] + data[2];
            result = result * 2 - 1;
            break;
        case 9:
            result = (data[0] * data[1]) / (data[2] + 1);
            result = result + 255;
            break;
    }
    
    return result;
}

/* Main computation with dense instruction mix */
int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct mixed_data *mixed = (struct mixed_data*)malloc(LINKED_LIST_SIZE * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7fff;
        float_array[i] = (float)(i * 0.12345);
        double_array[i] = (double)(i * 0.6789);
    }
    
    /* Initialize linked-list like structure */
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        mixed[i].c = (char)(i & 0xff);
        mixed[i].i = i * 3;
        mixed[i].d = sin((double)i * 0.1);
        mixed[i].f = cos((float)i * 0.2f);
        mixed[i].s = (short)(i * 5);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2};
    
    int final_result = 0;
    long long checksum = 0;
    
    /* Primary computation loop */
    for (int iter = 0; iter < N; iter++) {
        int local_sum = 0;
        
        /* Pointer chasing through mixed struct array */
        struct mixed_data *current = &mixed[iter % LINKED_LIST_SIZE];
        for (int chase = 0; chase < 32; chase++) {
            int idx = (current->i + chase) % LINKED_LIST_SIZE;
            current = &mixed[idx];
            
            /* Access all fields to create memory dependencies */
            local_sum += current->c;
            local_sum ^= current->i;
            local_sum += (int)(current->d * 100.0);
            local_sum += (int)(current->f * 10.0f);
            local_sum += current->s;
            
            /* Compiler barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Chain of dependent arithmetic operations */
        double a = double_array[iter % ARRAY_SIZE];
        float b = float_array[(iter + 1) % ARRAY_SIZE];
        int c = int_array[(iter + 2) % ARRAY_SIZE];
        
        a = a * 1.2345 + sin(b * 0.01);
        b = b * 2.0f + cos((float)a * 0.1f);
        c = c * 3 + (int)(a * 100.0) + (int)(b * 10.0f);
        
        /* Store results back */
        double_array[iter % ARRAY_SIZE] = a;
        float_array[(iter + 1) % ARRAY_SIZE] = b;
        int_array[(iter + 2) % ARRAY_SIZE] = c;
        
        /* Switch statement with multiple computation paths */
        int switch_val = switch_computation(iter, &int_array[iter % (ARRAY_SIZE - 3)]);
        
        /* Conditional with function call */
        if (iter & 1) {
            int func_idx = iter & 1;
            int helper_result = funcs[func_idx](iter, switch_val);
            local_sum += helper_result;
            
            /* Additional floating point helper */
            float f_result = helper3((float)iter, (float)switch_val);
            local_sum += (int)(f_result * 100.0f);
        } else {
            double d_result = helper4((double)iter, (double)switch_val);
            local_sum += (int)(d_result * 50.0);
        }
        
        /* Nested loop with data dependencies */
        for (int j = 0; j < 16; j++) {
            int idx = (iter + j) % ARRAY_SIZE;
            int_array[idx] = int_array[idx] * 3 - int_array[(idx + 1) % ARRAY_SIZE];
            
            /* Every 4 iterations, create a longer dependency chain */
            if (j % 4 == 0) {
                for (int k = 0; k < 4; k++) {
                    float_array[(idx + k) % ARRAY_SIZE] = 
                        float_array[(idx + k) % ARRAY_SIZE] * 1.1f + 
                        float_array[(idx + k + 1) % ARRAY_SIZE] * 0.9f;
                }
            }
        }
        
        /* Update volatile variables */
        vol_counter++;
        vol_double *= 1.0001;
        
        /* Complex chain computation */
        int chain_result = complex_chain(iter, &int_array[(iter * 7) % (ARRAY_SIZE - 64)], 64);
        local_sum += chain_result;
        
        final_result ^= local_sum;
        checksum += local_sum;
    }
    
    /* Reduction across all arrays */
    long long array_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_sum += int_array[i];
        array_sum += (long long)(float_array[i] * 1000.0f);
        array_sum += (long long)(double_array[i] * 1000.0);
    }
    
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        array_sum += mixed[i].c + mixed[i].i + mixed[i].s;
        array_sum += (long long)(mixed[i].d * 1000.0);
        array_sum += (long long)(mixed[i].f * 1000.0f);
    }
    
    /* Final output to prevent elimination */
    printf("Final result: %d\n", final_result);
    printf("Checksum: %lld\n", checksum);
    printf("Array sum: %lld\n", array_sum);
    printf("Volatile counter: %d, double: %f\n", vol_counter, vol_double);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed);
    
    return 0;
}
