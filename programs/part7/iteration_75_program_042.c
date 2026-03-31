/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define LINKED_LIST_SIZE 256

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 1.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Helper functions with different computation patterns */
static int compute_chain_a(int a, int b) {
    int t1 = a * 1103515245 + 12345;
    int t2 = b ^ t1;
    int t3 = t2 * 1664525 + 1013904223;
    return t3 & 0x7FFFFFFF;
}

static int compute_chain_b(int a, int b) {
    float f1 = (float)a * 3.14159f;
    float f2 = (float)b * 2.71828f;
    int t1 = (int)(f1 * f2);
    int t2 = t1 >> 4;
    int t3 = t2 * 16807 % 2147483647;
    return t3;
}

static double compute_chain_c(double a, double b) {
    double d1 = a * b + 1.0;
    double d2 = sin(d1) * cos(d1);
    double d3 = d2 * d2 + a - b;
    return d3;
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) static int complex_transform(int x, int y) {
    struct MixedData md;
    md.c = (char)(x & 0xFF);
    md.i = x * y + 123;
    md.d = (double)x / (y + 1.0);
    md.s = (short)(y & 0xFFFF);
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    int result = md.i + (int)md.d + md.s + md.c;
    g_volatile_counter++;
    return result;
}

/* Another non-inlineable function with switch statement */
__attribute__((noinline)) static int switch_computation(int val, int mode) {
    int result = val;
    
    switch (mode % 10) {
        case 0:
            result = result * 3 + 1;
            /* Fall through */
        case 1:
            result ^= 0xAAAAAAAA;
            result = (result << 3) | (result >> 29);
            break;
        case 2:
            result = result * 16807 % 2147483647;
            break;
        case 3:
            result = ~result;
            result += g_volatile_counter;
            break;
        case 4:
            result = (result & 0x55555555) << 1 | (result & 0xAAAAAAAA) >> 1;
            break;
        case 5:
            result = result * result;
            result = result % 1000000;
            break;
        case 6:
            result = result + (result >> 16) + (result >> 8);
            break;
        case 7:
            result = result | 1;
            result = result * 1103515245 + 12345;
            break;
        case 8:
            result = (result << 5) - result;  /* result * 31 */
            break;
        case 9:
            result = result & (result - 1);  /* clear lowest set bit */
            result = result ^ 0x12345678;
            break;
    }
    
    /* Another memory barrier */
    asm volatile("" ::: "memory");
    return result;
}

/* Linked list node for pointer chasing */
struct ListNode {
    int data;
    int index;
    struct ListNode* next;
};

int main(int argc, char *argv[]) {
    int i, j, k;
    long long total_iterations = 1000;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        total_iterations = atoll(argv[1]);
        if (total_iterations < 100) total_iterations = 100;
        if (total_iterations > 1000000) total_iterations = 1000000;
    }
    
    /* Allocate and initialize arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct MixedData *mixed_array = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 1103515245 + 12345;
        float_array[i] = (float)i * 1.2345f;
        double_array[i] = (double)i * 3.1415926535;
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * i - i;
        mixed_array[i].d = sqrt((double)i + 1.0);
        mixed_array[i].s = (short)(i * 3);
    }
    
    /* Create linked list for pointer chasing */
    struct ListNode *nodes = (struct ListNode*)malloc(LINKED_LIST_SIZE * sizeof(struct ListNode));
    for (i = 0; i < LINKED_LIST_SIZE; i++) {
        nodes[i].data = int_array[i % ARRAY_SIZE];
        nodes[i].index = i;
        nodes[i].next = &nodes[(i * 37 + 1) % LINKED_LIST_SIZE];  /* pseudo-random next pointer */
    }
    
    /* Array of function pointers for computed jumps */
    ComputeFunc funcs[] = {compute_chain_a, compute_chain_b, compute_chain_a, compute_chain_b};
    
    /* Main computation loop - designed to create complex scheduling scenarios */
    long long accumulator = 0;
    struct ListNode *current_node = &nodes[0];
    
    for (i = 0; i < total_iterations; i++) {
        int temp1, temp2, temp3;
        float ftemp1, ftemp2;
        double dtemp1, dtemp2;
        
        /* ========== PART 1: Pointer chasing with data dependencies ========== */
        int chase_sum = 0;
        for (j = 0; j < 16; j++) {  /* Small unrollable but data-dependent loop */
            chase_sum += current_node->data;
            chase_sum ^= current_node->index;
            current_node = current_node->next;
            
            /* Create loop-carried dependency */
            chase_sum = (chase_sum << 1) | (chase_sum >> 31);
        }
        
        /* ========== PART 2: Complex arithmetic chain ========== */
        temp1 = int_array[i % ARRAY_SIZE];
        temp2 = chase_sum;
        
        /* Long dependency chain */
        temp3 = temp1 * temp2 + 12345;
        temp1 = temp3 ^ 0xDEADBEEF;
        temp2 = temp1 * 1664525 + 1013904223;
        temp3 = temp2 % 1000000007;
        temp1 = (temp3 << 5) - temp3;  /* temp3 * 31 */
        temp2 = temp1 & (temp1 - 1);   /* Clear lowest set bit */
        
        /* ========== PART 3: Mixed floating-point operations ========== */
        ftemp1 = float_array[i % ARRAY_SIZE];
        ftemp2 = (float)temp2 * 0.01f;
        
        ftemp1 = ftemp1 * ftemp2 + g_volatile_float;
        g_volatile_float = ftemp1 * 0.99f;  /* Volatile write creates scheduling hazard */
        
        dtemp1 = double_array[i % ARRAY_SIZE];
        dtemp2 = compute_chain_c(dtemp1, (double)temp2);
        dtemp1 = dtemp1 * 0.5 + dtemp2 * 0.5;
        
        /* ========== PART 4: Switch statement with different computation kernels ========== */
        int switch_val = switch_computation(temp2, i);
        
        /* ========== PART 5: Deeply nested conditionals ========== */
        if (i & 1) {
            /* Branch 1: Call helper function */
            temp3 = complex_transform(temp1, temp2);
            
            if (i & 2) {
                /* Nested branch with more computations */
                for (k = 0; k < 8; k++) {
                    mixed_array[(i + k) % ARRAY_SIZE].i += temp3;
                    mixed_array[(i + k) % ARRAY_SIZE].d *= 1.0001;
                }
            } else {
                /* Alternative nested branch */
                ftemp1 = sinf(ftemp1) * cosf(ftemp2);
                g_volatile_counter += (int)ftemp1;
            }
        } else if (i & 4) {
            /* Branch 2: Computed jump */
            ComputeFunc f = funcs[i % 4];
            temp3 = f(temp1, temp2);
            
            /* Memory operation with potential aliasing */
            int_array[(i * 7) % ARRAY_SIZE] = temp3;
            float_array[(i * 13) % ARRAY_SIZE] = (float)temp3;
        } else {
            /* Branch 3: Direct computation */
            temp3 = temp1 * 3 + temp2 * 7;
            temp3 = (temp3 << 3) | (temp3 >> 29);
        }
        
        /* ========== PART 6: Large basic block simulation ========== */
        /* Many independent instructions that can fill instruction queue */
        int local_sum = 0;
        for (j = 0; j < 32; j++) {
            /* Independent computations - scheduler can reorder these */
            int idx = (i * 19 + j * 23) % ARRAY_SIZE;
            local_sum += int_array[idx];
            local_sum ^= mixed_array[idx].i;
            
            /* Volatile read creates scheduling barrier */
            if (g_volatile_counter & 1) {
                local_sum += 1;
            }
        }
        
        /* ========== PART 7: Reduction and accumulation ========== */
        accumulator += temp3;
        accumulator += (long long)switch_val;
        accumulator += (long long)local_sum;
        accumulator += (long long)chase_sum;
        accumulator ^= (long long)(dtemp1 * 1000.0);
        
        /* Occasionally reset pointer chase */
        if ((i % 97) == 0) {
            current_node = &nodes[i % LINKED_LIST_SIZE];
        }
    }
    
    /* Final reduction across arrays */
    long long final_sum = accumulator;
    for (i = 0; i < ARRAY_SIZE; i += 8) {  /* Strided access pattern */
        final_sum += int_array[i];
        final_sum ^= (long long)(float_array[i] * 1000.0f);
        final_sum += (long long)(double_array[i] * 1000.0);
        final_sum += mixed_array[i].i;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %lld\n", final_sum % 1000000000);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    free(nodes);
    
    return 0;
}
