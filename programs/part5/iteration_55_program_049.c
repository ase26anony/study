/* reload_coverage.c - Program to trigger various reload types in GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Structure to create complex addressing modes */
typedef struct {
    int data[8];
    struct {
        int x, y, z;
    } coord;
    volatile int* ptr;
} ComplexStruct;

/* Global arrays to force memory operands */
volatile int global_array[256];
volatile ComplexStruct global_structs[16];

/* Test 1: Complex array addressing with multiple index computations */
static inline __attribute__((always_inline)) 
int test_array_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = a | c;
    volatile int v5 = b & d;
    volatile int v6 = e << 2;
    volatile int v7 = f >> 1;
    volatile int v8 = v1 + v2;
    volatile int v9 = v3 * v4;
    volatile int v10 = v5 - v6;
    
    /* Multi-dimensional array-like addressing with complex index */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
    int idx1 = (v1 + v2 * 3 - v3) & 0xFF;
    int idx2 = (v4 * 2 + v5 / 4 + v6) & 0xFF;
    int idx3 = (v7 ^ v8 | v9) & 0xFF;
    
    /* Nested addressing - address computation needs reload */
    int result = global_array[idx1 + idx2 * 8 + idx3];
    
    /* More computations to keep values live */
    result += global_array[(idx1 * idx2 + v10) & 0xFF];
    result += global_array[(idx3 * 4 + v1) & 0xFF];
    
    /* Force address reloads with pointer arithmetic */
    volatile int* ptr1 = &global_array[idx1];
    volatile int* ptr2 = ptr1 + idx2;
    volatile int* ptr3 = ptr2 - idx3;
    
    result += *ptr1 + *ptr2 + *ptr3;
    
    return result;
}

/* Test 2: Structure member accesses with inline assembly */
static __attribute__((noinline))
int test_struct_reloads(int seed) {
    /* Local structure with many members */
    ComplexStruct local1, local2, local3;
    volatile int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Initialize with complex expressions */
    for (int i = 0; i < 8; i++) {
        local1.data[i] = seed + i * 3;
        local2.data[i] = seed - i * 2;
        local3.data[i] = seed ^ i;
    }
    
    local1.coord.x = seed * 2;
    local1.coord.y = seed + 5;
    local1.coord.z = seed - 3;
    
    local2.coord.x = seed / 2;
    local2.coord.y = seed % 7;
    local2.coord.z = seed | 0xFF;
    
    /* Complex structure member addressing - should trigger RELOAD_FOR_INPADDR_ADDRESS */
    temp1 = local1.data[local1.coord.x & 7];
    temp2 = local2.data[local2.coord.y & 7];
    temp3 = local3.data[local3.coord.z & 7];
    
    /* Inline assembly with multiple outputs to clobber registers */
    /* This should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int out1, out2, out3;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=m" (out1), [out2] "=m" (out2), [out3] "=m" (out3)
        : [in1] "mr" (temp1), [in2] "mr" (temp2), [in3] "mr" (temp3)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* More complex addressing with structure pointers */
    ComplexStruct* ptrs[3] = {&local1, &local2, &local3};
    int idx = (seed * 3) % 3;
    
    /* This should trigger RELOAD_FOR_OPERAND_ADDRESS */
    temp4 = ptrs[idx]->data[(seed + idx) & 7];
    temp5 = ptrs[(idx + 1) % 3]->coord.x;
    temp6 = ptrs[(idx + 2) % 3]->coord.y;
    
    return out1 + out2 + out3 + temp4 + temp5 + temp6;
}

/* Test 3: Pointer chasing and complex address computations */
static __attribute__((noinline))
int test_pointer_chasing(int iterations) {
    volatile int* ptr_array[16];
    volatile int values[32];
    int sum = 0;
    
    /* Initialize pointer array with complex offsets */
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = &values[(i * 3) % 32];
        values[i] = i * i + iterations;
        values[i + 16] = (i * iterations) & 0xFF;
    }
    
    /* Unrolled loop to increase register pressure */
    /* #pragma GCC unroll 4 */
    for (int i = 0; i < iterations && i < 8; i++) {
        /* Multiple pointer dereferences with address computations */
        volatile int* p1 = ptr_array[i] + (i & 3);
        volatile int* p2 = ptr_array[i + 1] - ((i + 1) & 3);
        volatile int* p3 = ptr_array[i + 2] + ((i * 2) & 7);
        
        /* Complex address computations - should trigger RELOAD_FOR_OPADDR_ADDR */
        int val1 = *(p1 + (i % 2));
        int val2 = *(p2 - ((i + 1) % 2));
        int val3 = *(p3 + ((i * 3) % 4));
        
        /* More computations to use many temporaries */
        int t1 = val1 * val2;
        int t2 = val2 + val3;
        int t3 = val1 ^ val3;
        int t4 = t1 - t2;
        int t5 = t2 * t3;
        int t6 = t3 | t4;
        int t7 = t4 & t5;
        int t8 = t5 ^ t6;
        int t9 = t6 + t7;
        int t10 = t7 * t8;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Update pointers with complex arithmetic */
        ptr_array[i] = &values[((i * 5 + t1) & 31)];
        ptr_array[i + 1] = &values[((i * 7 + t2) & 31)];
        ptr_array[i + 2] = &values[((i * 11 + t3) & 31)];
    }
    
    return sum;
}

/* Test 4: Mixed operations with inline assembly constraints */
static __attribute__((noinline))
int test_mixed_reloads(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many input parameters to use argument registers */
    volatile int v1 = a + b;
    volatile int v2 = c * d;
    volatile int v3 = e ^ f;
    volatile int v4 = g | h;
    volatile int v5 = a - c;
    volatile int v6 = b + d;
    volatile int v7 = e & f;
    volatile int v8 = g ^ h;
    
    /* Complex expression with many intermediate values */
    int t1 = v1 * v2 + v3;
    int t2 = v4 - v5 * v6;
    int t3 = v7 | v8 ^ v1;
    int t4 = v2 & v3 + v4;
    int t5 = v5 * v6 - v7;
    int t6 = v8 | v1 & v2;
    int t7 = v3 ^ v4 + v5;
    int t8 = v6 * v7 - v8;
    
    /* Inline assembly with multiple alternative constraints */
    /* This should trigger RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
    int result1, result2;
    asm volatile (
        "addl %[x1], %[x2]\n\t"
        "subl %[x3], %[x4]\n\t"
        "movl %[x2], %[r1]\n\t"
        "movl %[x4], %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [x1] "0" (t1), [x2] "r" (t2), [x3] "r" (t3), [x4] "1" (t4)
        : "cc"
    );
    
    /* More complex memory operations */
    volatile int* mem_ptr = (volatile int*)&global_structs[0].data[0];
    
    /* Strided accesses with complex index computations */
    for (int i = 0; i < 4; i++) {
        int idx = (t5 + i * t6) & 7;
        mem_ptr[idx] = result1 + i;
        result2 += mem_ptr[(idx + t7) & 7];
        mem_ptr[(idx + t8) & 7] = result2 * i;
    }
    
    return result1 + result2 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Main driver function */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].data[j] = i * 8 + j;
        }
        global_structs[i].coord.x = i * 2;
        global_structs[i].coord.y = i * 3;
        global_structs[i].coord.z = i * 5;
    }
    
    /* Call test functions with different patterns */
    
    /* Test 1: Array addressing - should trigger input address reloads */
    result += test_array_addressing(1, 2, 3, 4, 5, 6);
    result += test_array_addressing(7, 8, 9, 10, 11, 12);
    result += test_array_addressing(13, 14, 15, 16, 17, 18);
    
    /* Test 2: Structure accesses - should trigger various address reloads */
    result += test_struct_reloads(100);
    result += test_struct_reloads(200);
    result += test_struct_reloads(300);
    
    /* Test 3: Pointer chasing - should trigger operand address reloads */
    result += test_pointer_chasing(4);
    result += test_pointer_chasing(6);
    result += test_pointer_chasing(8);
    
    /* Test 4: Mixed operations - should trigger other reload types */
    result += test_mixed_reloads(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_mixed_reloads(9, 10, 11, 12, 13, 14, 15, 16);
    result += test_mixed_reloads(17, 18, 19, 20, 21, 22, 23, 24);
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result > 0 ? 0 : 1;
}

#pragma GCC pop_options
