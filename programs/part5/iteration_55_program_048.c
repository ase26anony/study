/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force register pressure by using many variables */
#define FORCE_REGISTER_PRESSURE \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9; \
    volatile int x0, x1, x2, x3, x4, x5, x6, x7, x8, x9; \
    volatile int y0, y1, y2, y3, y4, y5, y6, y7, y8, y9; \
    volatile int z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;

/* Complex array structure to force address computations */
struct nested_array {
    int data[3][4][5];
    int *ptr_array[8];
    struct nested_array *next;
};

/* Test function 1: Complex array addressing with multiple index computations */
__attribute__((noinline))
static int test_complex_addressing(struct nested_array *arr, int idx1, int idx2, int idx3) {
    FORCE_REGISTER_PRESSURE
    
    /* Force RELOAD_FOR_INPUT: values used multiple times after clobbering */
    int base = idx1 * 7 + idx2 * 3;
    int offset = idx3 * 11 + 5;
    
    /* Complex addressing requiring RELOAD_FOR_INPUT_ADDRESS */
    int val1 = arr->data[idx1][idx2][idx3];
    int val2 = arr->data[idx2][idx3][idx1];
    
    /* More complex: address computation with multiple components */
    int *ptr1 = &arr->data[idx1][idx2][0];
    int *ptr2 = &arr->data[idx3][idx1][idx2];
    
    /* Nested addressing requiring RELOAD_FOR_INPADDR_ADDRESS */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Manual unrolling to increase pressure */
        sum += arr->data[i][idx1][idx2];
        sum += arr->data[idx2][i][idx3];
        sum += arr->data[idx3][idx2][i];
    }
    
    /* Force spills with many intermediate computations */
    v0 = base + offset;
    v1 = base * offset;
    v2 = base - offset;
    v3 = base & offset;
    v4 = base | offset;
    v5 = base ^ offset;
    v6 = base << (offset & 3);
    v7 = base >> (offset & 3);
    
    /* Use all volatile variables to prevent optimization */
    w0 = val1 + val2;
    w1 = val1 - val2;
    w2 = val1 * val2;
    w3 = *ptr1 + *ptr2;
    w4 = ptr1 - ptr2;
    
    return sum + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + w0 + w1 + w2 + w3 + w4;
}

/* Test function 2: Inline assembly with multiple outputs and clobbers */
__attribute__((noinline))
static int test_asm_reloads(int a, int b, int c, int d, int e, int f) {
    FORCE_REGISTER_PRESSURE
    
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3;
    
    /* Inline asm with memory output - triggers RELOAD_FOR_OUTPUT_ADDRESS */
    __asm__ volatile (
        "movl %[input1], %[output1]\n\t"
        "addl %[input2], %[output1]\n\t"
        "movl %[output1], %[memout1]\n\t"
        : [output1] "=r" (out1),
          [memout1] "=m" (mem1)
        : [input1] "r" (a),
          [input2] "r" (b)
        : "cc"
    );
    
    /* More complex asm with multiple memory outputs */
    __asm__ volatile (
        "imull %[in1], %[in2]\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%edx, %[out2]\n\t"
        "leal (%[in3], %[in4], 4), %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=m" (mem2),
          [out2] "=m" (mem3),
          [out3] "=r" (out2)
        : [in1] "r" (c),
          [in2] "r" (d),
          [in3] "r" (e),
          [in4] "r" (f)
        : "eax", "edx", "ecx", "cc"
    );
    
    /* Asm with complex addressing mode - triggers RELOAD_FOR_OUTADDR_ADDRESS */
    int *addr = &mem1;
    __asm__ volatile (
        "movl $0x12345678, (%[addr], %[index], 4)\n\t"
        : 
        : [addr] "r" (addr),
          [index] "r" (out1)
        : "memory"
    );
    
    return out1 + out2 + mem1 + mem2 + mem3;
}

/* Test function 3: Pointer chasing and operand address reloads */
__attribute__((noinline))
static int test_pointer_chasing(struct nested_array **arrays, int count) {
    FORCE_REGISTER_PRESSURE
    
    volatile int results[10];
    struct nested_array *current;
    int *ptr_array[10];
    int sum = 0;
    
    /* Setup pointer array - will need RELOAD_FOR_OPERAND_ADDRESS */
    for (int i = 0; i < 10 && i < count; i++) {
        ptr_array[i] = &arrays[i]->data[0][0][0];
    }
    
    /* Pointer chasing loop */
    current = arrays[0];
    for (int i = 0; i < 8 && current != NULL; i++) {
        /* Complex expression requiring multiple reloads */
        int idx = i & 3;
        int *data_ptr = current->data[idx];
        
        /* Access through multiple pointer levels - triggers RELOAD_FOR_OPADDR_ADDR */
        sum += data_ptr[idx];
        sum += *(ptr_array[idx] + i);
        
        /* More complex addressing */
        for (int j = 0; j < 3; j++) {
            sum += current->data[j][idx][i % 4];
        }
        
        current = current->next;
    }
    
    /* Force other address reloads with volatile accesses */
    volatile int *volatile_ptr = (volatile int*)&sum;
    for (int i = 0; i < 5; i++) {
        results[i] = *volatile_ptr + i;
        volatile_ptr = (volatile int*)((char*)volatile_ptr + 1);
    }
    
    return sum + results[0] + results[1] + results[2];
}

/* Test function 4: Mixed addressing modes and constraints */
__attribute__((noinline))
static int test_mixed_addressing(int *base, int index1, int index2, int index3) {
    FORCE_REGISTER_PRESSURE
    
    /* Multi-dimensional array simulation */
    int *arr2d = base + index1 * 16;
    int *arr3d = base + index1 * 16 + index2 * 4;
    
    /* Various addressing computations */
    int *addr1 = arr2d + index2;
    int *addr2 = arr3d + index3;
    int *addr3 = base + (index1 << 4) + (index2 << 2) + index3;
    
    /* Force different reload types through complex expressions */
    int val1 = *(addr1 + index3);
    int val2 = *(addr2 + index1);
    int val3 = *(addr3 + index2);
    
    /* Chain computations to increase register pressure */
    int t1 = val1 * val2;
    int t2 = val2 + val3;
    int t3 = val1 - val3;
    int t4 = t1 >> 2;
    int t5 = t2 << 1;
    int t6 = t3 & 0xFF;
    
    /* Use inline asm with alternative constraints */
    int result;
    __asm__ volatile (
        "addl %[v1], %[v2]\n\t"
        "subl %[v3], %[v2]\n\t"
        "movl %[v2], %[res]"
        : [res] "=r" (result)
        : [v1] "r" (t1),
          [v2] "r" (t2),
          [v3] "0" (t3)  /* Same as output */
        : "cc"
    );
    
    /* More complex asm with memory operand */
    __asm__ volatile (
        "movl %[in], %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (*addr1)
        : [in] "m" (*addr2)
        : "eax", "cc"
    );
    
    return result + t4 + t5 + t6 + *addr1 + *addr2;
}

/* Main driver that calls all test functions */
int main(void) {
    /* Initialize test data */
    struct nested_array arrays[5];
    struct nested_array *array_ptrs[5];
    
    for (int i = 0; i < 5; i++) {
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 4; y++) {
                for (int z = 0; z < 5; z++) {
                    arrays[i].data[x][y][z] = i * 1000 + x * 100 + y * 10 + z;
                }
            }
        }
        arrays[i].next = (i < 4) ? &arrays[i + 1] : NULL;
        array_ptrs[i] = &arrays[i];
    }
    
    int sum = 0;
    
    /* Call test functions multiple times with different arguments */
    for (int i = 0; i < 3; i++) {
        sum += test_complex_addressing(&arrays[0], i, i+1, i+2);
        sum += test_asm_reloads(i*10, i*20, i*30, i*40, i*50, i*60);
        sum += test_pointer_chasing(array_ptrs, 5);
        
        int base_array[100];
        for (int j = 0; j < 100; j++) {
            base_array[j] = j * 7;
        }
        sum += test_mixed_addressing(base_array, i, i+1, i+2);
    }
    
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
