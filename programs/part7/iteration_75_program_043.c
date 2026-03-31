/* haifa_sched_trigger.c
 * Designed to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_sched_trigger.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256

/* Mixed data types with different alignments */
typedef struct __attribute__((packed)) {
    uint8_t  byte_val;
    uint32_t int_val;
    double   dbl_val;
    uint16_t short_val;
} MixedData;

/* Volatile variables to create scheduling barriers */
static volatile int vol_barrier = 0;

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper_mul_chain(int a, int b) {
    int t1 = a * 3;
    int t2 = b * 7;
    int t3 = t1 * t2;
    int t4 = t3 * 11;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return t4 * 13;
}

static int helper_add_chain(int a, int b) {
    int t1 = a + b;
    int t2 = t1 + (a >> 2);
    int t3 = t2 + (b << 1);
    asm volatile("" ::: "memory");
    return t3 + (t1 * 2);
}

static float helper_float_ops(float a, float b) {
    float t1 = a * 1.5f;
    float t2 = b * 2.5f;
    float t3 = t1 + t2;
    float t4 = t3 / 1.7f;
    vol_barrier = (int)t4;  /* Volatile store creates scheduling hazard */
    return t4 * 0.9f;
}

/* Linked list node for pointer chasing */
typedef struct ListNode {
    int value;
    struct ListNode* next;
    struct ListNode* prev;
    float fvalue;
} ListNode;

/* Initialize a linked list with pseudo-random connections */
static void init_linked_list(ListNode* nodes, int size) {
    for (int i = 0; i < size; i++) {
        nodes[i].value = i * 1103515245;
        nodes[i].fvalue = (float)(i * 1103515245) * 0.001f;
        nodes[i].next = &nodes[(i * 13 + 7) % size];
        nodes[i].prev = &nodes[(i * 17 + 3) % size];
    }
}

/* Complex computation with many dependencies */
static int complex_computation(int* int_array, float* float_array, 
                               double* double_array, MixedData* mixed_array,
                               int index, int iter) {
    int result = 0;
    
    /* Pointer chasing through multiple arrays */
    int* ptr1 = &int_array[index];
    float* ptr2 = &float_array[*ptr1 % ARRAY_SIZE];
    double* ptr3 = &double_array[(int)(*ptr2) % ARRAY_SIZE];
    
    /* Chain of dependent operations */
    int a = *ptr1 + iter;
    int b = (int)(*ptr2 * 100.0f) ^ a;
    double c = *ptr3 * 0.5 + (double)b;
    int d = (int)c * 7;
    
    /* Mixed data type access with different alignments */
    mixed_array[index].byte_val = (uint8_t)(d & 0xFF);
    mixed_array[index].int_val = d * 3;
    mixed_array[index].dbl_val = c * 1.7;
    mixed_array[index].short_val = (uint16_t)(d >> 8);
    
    /* Volatile read creates scheduling hazard */
    int barrier = vol_barrier;
    result = d + barrier + mixed_array[index].byte_val;
    
    return result;
}

/* Switch-based computation with different kernels */
static int switch_computation(int i, int val) {
    int result = val;
    
    switch (i % 10) {
        case 0:
            result = result * 3 + 7;
            result = result >> 1;
            result = result ^ 0x55AA55AA;
            break;
        case 1:
            result = result + (result << 2);
            result = result * 11;
            result = result - (result >> 3);
            break;
        case 2:
            result = helper_mul_chain(result, i);
            break;
        case 3:
            result = result * result;
            result = result % 10007;
            break;
        case 4:
            result = (result & 0xFFFF) * (result >> 16);
            break;
        case 5:
            result = helper_add_chain(result, i);
            break;
        case 6:
            result = result + (i * 3);
            result = result * 7;
            result = result - 13;
            break;
        case 7:
            result = result ^ (result << 13);
            result = result ^ (result >> 17);
            result = result ^ (result << 5);
            break;
        case 8:
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
            break;
        case 9:
            result = result | (1 << (i % 32));
            result = result & ~(1 << ((i + 7) % 32));
            break;
    }
    
    return result;
}

/* Main computation with nested loops and complex control flow */
static uint64_t run_computation(int N, ListNode* list, int* int_array,
                               float* float_array, double* double_array,
                               MixedData* mixed_array) {
    uint64_t total = 0;
    ListNode* current = &list[0];
    
    for (int outer = 0; outer < N; outer++) {
        int iter_result = 0;
        
        /* Pointer chasing through linked list */
        for (int i = 0; i < 8; i++) {
            current = current->next;
            iter_result ^= current->value;
            current = current->prev->next;
            iter_result += (int)current->fvalue;
        }
        
        /* Large basic block with many independent-ish operations */
        for (int i = 0; i < 32; i++) {
            int idx = (iter_result + i) % ARRAY_SIZE;
            
            /* Complex computation with dependencies */
            int comp_result = complex_computation(int_array, float_array,
                                                 double_array, mixed_array,
                                                 idx, outer);
            
            /* Switch statement creates control flow complexity */
            comp_result = switch_computation(iter_result + i, comp_result);
            
            /* Conditional with function call */
            if (iter_result & (1 << (i & 7))) {
                comp_result = helper_mul_chain(comp_result, i);
                float f = helper_float_ops((float)comp_result, current->fvalue);
                comp_result += (int)f;
            } else {
                comp_result = helper_add_chain(comp_result, i);
            }
            
            /* Another level of conditional */
            if (outer & 1) {
                /* Deep conditional chain */
                if (comp_result & 0x100) {
                    comp_result = comp_result * 3;
                } else if (comp_result & 0x200) {
                    comp_result = comp_result / 2;
                } else if (comp_result & 0x400) {
                    comp_result = comp_result + 777;
                } else {
                    comp_result = comp_result - 333;
                }
                
                /* Use computed jump via function pointer */
                compute_func_t funcs[2] = {helper_mul_chain, helper_add_chain};
                int choice = (comp_result >> 4) & 1;
                comp_result = funcs[choice](comp_result, outer);
            }
            
            iter_result ^= comp_result;
            
            /* Memory store with potential aliasing */
            int_array[idx] = comp_result;
            float_array[idx] = (float)comp_result * 0.001f;
        }
        
        total += iter_result;
        
        /* Occasionally update volatile for scheduling hazards */
        if (outer % 37 == 0) {
            vol_barrier = iter_result & 0xFF;
        }
    }
    
    return total;
}

int main(int argc, char** argv) {
    int N = 1000;  /* Default iteration count */
    
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate and initialize arrays with different types */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    MixedData* mixed_array = (MixedData*)malloc(ARRAY_SIZE * sizeof(MixedData));
    ListNode* list_nodes = (ListNode*)malloc(LINKED_LIST_SIZE * sizeof(ListNode));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 1103515245;
        float_array[i] = (float)(i * 1103515245) * 0.001f;
        double_array[i] = (double)(i * 1103515245) * 0.00001;
        mixed_array[i].byte_val = (uint8_t)(i & 0xFF);
        mixed_array[i].int_val = i * 3;
        mixed_array[i].dbl_val = (double)i * 1.7;
        mixed_array[i].short_val = (uint16_t)(i & 0xFFFF);
    }
    
    init_linked_list(list_nodes, LINKED_LIST_SIZE);
    
    /* Run the main computation */
    uint64_t result = run_computation(N, list_nodes, int_array, float_array,
                                     double_array, mixed_array);
    
    /* Final reduction across all arrays to prevent dead code elimination */
    uint64_t final_check = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= int_array[i];
        final_check += (uint64_t)(float_array[i] * 1000.0f);
        final_check ^= *(uint64_t*)&double_array[i];
        final_check += mixed_array[i].int_val;
    }
    
    /* Mix in the linked list values */
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        final_check ^= list_nodes[i].value;
        final_check += (uint64_t)(list_nodes[i].fvalue * 100.0f);
    }
    
    /* Print results to prevent optimization */
    printf("Result: %lu, Final check: %lu\n", 
           (unsigned long)result, 
           (unsigned long)final_check);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    free(list_nodes);
    
    return 0;
}
