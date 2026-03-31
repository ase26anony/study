/* haifa-sched-trigger.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-trigger.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256

/* Mixed data types with different alignments */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    float f;
    char arr[3];
};

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile struct MixedData g_volatile_data;

/* Function pointer for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions with different computation patterns */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (a ^ b);
}

static int helper2(int a, int b) {
    int temp = a;
    for (int i = 0; i < 3; i++) {
        temp = (temp << 2) | (b & 3);
        b >>= 2;
    }
    return temp;
}

static float helper3(float a, float b) {
    float result = a;
    for (int i = 0; i < 4; i++) {
        result = result * b + i;
    }
    return result;
}

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
static double complex_math(double a, double b, int iterations) {
    double result = a;
    for (int i = 0; i < iterations; i++) {
        result = result * b - (i * 0.1);
        result = result / (b + 1.0);
        result = result + (a * 0.5);
    }
    return result;
}

/* Pointer chasing through linked list */
struct ListNode {
    int value;
    struct ListNode* next;
    struct ListNode* prev;
};

static void build_linked_list(struct ListNode* nodes, int size) {
    for (int i = 0; i < size; i++) {
        nodes[i].value = i * 1103515245;
        nodes[i].next = &nodes[(i + 1) % size];
        nodes[i].prev = &nodes[(i - 1 + size) % size];
    }
}

/* Main computation with complex control flow */
static uint64_t compute_kernel(int iterations, int* int_array, 
                               float* float_array, double* double_array,
                               struct MixedData* mixed_array) {
    uint64_t accumulator = 0;
    struct ListNode linked_list[LINKED_LIST_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_array[i] = (float)(i * 0.12345);
        double_array[i] = (double)(i * 0.6789);
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * 3;
        mixed_array[i].d = (double)i * 2.5;
        mixed_array[i].f = (float)i * 1.5f;
    }
    
    build_linked_list(linked_list, LINKED_LIST_SIZE);
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2};
    
    /* Main computation loop with complex dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        struct ListNode* current = &linked_list[iter % LINKED_LIST_SIZE];
        int temp_int = int_array[iter % ARRAY_SIZE];
        float temp_float = float_array[iter % ARRAY_SIZE];
        double temp_double = double_array[iter % ARRAY_SIZE];
        
        /* Pointer chasing with data dependencies */
        for (int chase = 0; chase < 8; chase++) {
            temp_int = temp_int ^ current->value;
            current = current->next;
            temp_int += current->value;
            current = current->prev->next;
        }
        
        /* Chain of dependent arithmetic operations */
        double a = temp_double * 1.1;
        double b = a + temp_float;
        double c = b / (temp_int + 1);
        double d = c * 2.0 - a;
        temp_double = d + complex_math(a, b, 4);
        
        /* Large switch statement for control flow complexity */
        switch (iter % 10) {
            case 0:
                temp_int = helper1(temp_int, iter);
                temp_float = helper3(temp_float, iter * 0.1f);
                break;
            case 1:
                temp_int = temp_int * 3 + (iter & 0xFF);
                temp_float = temp_float * 2.0f - 1.0f;
                break;
            case 2:
                for (int i = 0; i < 16; i++) {
                    temp_int = (temp_int << 1) | ((temp_int >> 31) & 1);
                }
                break;
            case 3:
                temp_int = funcs[iter & 1](temp_int, iter);
                break;
            case 4:
                /* Memory barrier to force scheduling constraints */
                asm volatile("" ::: "memory");
                temp_int = mixed_array[iter % ARRAY_SIZE].i;
                g_volatile_counter = temp_int;
                break;
            case 5:
                temp_double = complex_math(temp_double, iter * 0.01, 3);
                break;
            case 6:
                temp_int = temp_int ^ int_array[(iter + 1) % ARRAY_SIZE];
                temp_int = temp_int * 7 - 13;
                break;
            case 7:
                /* Volatile access */
                g_volatile_data.i = temp_int;
                temp_int = g_volatile_data.i + g_volatile_counter;
                break;
            case 8:
                temp_float = temp_float * temp_float + 1.0f;
                temp_float = temp_float / (iter + 1.0f);
                break;
            case 9:
                temp_int = (temp_int << 4) | (temp_int >> 28);
                temp_int = temp_int ^ 0xAAAAAAAA;
                break;
        }
        
        /* Deeply nested conditionals */
        if (iter & 1) {
            if (temp_int > 1000) {
                if (temp_float < 500.0f) {
                    temp_int = helper2(temp_int, iter);
                } else {
                    temp_int = temp_int / 2;
                }
            } else if (temp_int < -1000) {
                temp_int = -temp_int;
            } else {
                temp_int = temp_int * 3;
            }
        }
        
        /* Update arrays with data dependencies */
        int idx = iter % ARRAY_SIZE;
        int_array[idx] = temp_int;
        float_array[idx] = temp_float;
        double_array[idx] = temp_double;
        
        /* Mixed data type operations */
        mixed_array[idx].i = temp_int;
        mixed_array[idx].f = temp_float;
        mixed_array[idx].d = temp_double;
        
        /* Accumulate results */
        accumulator ^= (uint64_t)temp_int;
        accumulator += (uint64_t)(temp_float * 1000);
        accumulator ^= (uint64_t)(temp_double * 10000);
    }
    
    return accumulator;
}

/* Large basic block generator */
static void large_sequential_block(int* dest, int* src1, int* src2, int size) {
    /* Many independent instructions to fill instruction queue */
    for (int i = 0; i < size; i++) {
        dest[i] = src1[i] + src2[i];
        dest[i] = dest[i] * 2 - src1[i];
        dest[i] = dest[i] ^ src2[i];
        dest[i] = dest[i] << 1;
        dest[i] = dest[i] >> 2;
        dest[i] = dest[i] | 0x1;
        dest[i] = dest[i] & 0x7FFFFFFF;
        dest[i] = dest[i] + i;
        dest[i] = dest[i] - src2[i];
        dest[i] = dest[i] * 3;
    }
}

int main(int argc, char** argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate arrays with different data types */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct MixedData* mixed_array = 
        (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    /* Additional arrays for large basic blocks */
    int* src1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* src2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* dest = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize source arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        src1[i] = i * 3;
        src2[i] = i * 5 + 1;
    }
    
    /* Execute large sequential block to fill instruction queues */
    large_sequential_block(dest, src1, src2, ARRAY_SIZE);
    
    /* Main computation */
    uint64_t result = compute_kernel(iterations, int_array, float_array, 
                                    double_array, mixed_array);
    
    /* Final reduction across all data */
    uint64_t final_accumulator = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_accumulator ^= (uint64_t)int_array[i];
        final_accumulator += (uint64_t)(float_array[i] * 100);
        final_accumulator ^= (uint64_t)(double_array[i] * 1000);
        final_accumulator += mixed_array[i].i;
        final_accumulator ^= dest[i];
    }
    
    final_accumulator ^= result;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)final_accumulator);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    free(src1);
    free(src2);
    free(dest);
    
    return 0;
}
