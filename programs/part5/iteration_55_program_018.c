/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force specific compilation for 32-bit target */
#ifdef __x86_64__
#error "Compile with -m32 flag for 32-bit target"
#endif

/* Volatile variables to prevent optimization */
static volatile int global_seed = 42;

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData* next;
    int matrix[3][3];
};

/* Large array to force memory addressing */
static int global_array[256][16];

/* Test function 1: Complex array addressing with multiple index computations */
__attribute__((noinline))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a + global_seed;
    volatile int v2 = b * global_seed;
    volatile int v3 = c ^ global_seed;
    volatile int v4 = d | global_seed;
    volatile int v5 = e & global_seed;
    volatile int v6 = f + global_seed;
    
    /* Multi-level array indexing - forces RELOAD_FOR_INPUT_ADDRESS */
    int* ptr1 = &global_array[a][b];
    int* ptr2 = &global_array[c][d];
    int* ptr3 = &global_array[e][f];
    
    /* Complex address computations */
    int result = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different addressing modes */
    
    /* RELOAD_FOR_INPUT: v1 used multiple times after clobber */
    result += *(ptr1 + v1) + v1;
    
    /* Nested addressing requiring RELOAD_FOR_INPADDR_ADDRESS */
    result += *(ptr2 + (v2 * 4 + v3)) + v2;
    
    /* More complex expressions */
    result += *(ptr3 + (v4 / 2 + v5 % 3)) + v3;
    
    /* Force spills with many intermediate values */
    int t1 = v1 * v2;
    int t2 = v3 * v4;
    int t3 = v5 * v6;
    int t4 = t1 + t2;
    int t5 = t3 + t4;
    int t6 = t5 * 2;
    int t7 = t6 - v1;
    int t8 = t7 + v2;
    int t9 = t8 - v3;
    int t10 = t9 + v4;
    
    result += t10;
    
    return result;
}

/* Test function 2: Inline assembly with multiple outputs and constraints */
__attribute__((noinline))
static int test_asm_reloads(int x, int y, int z) {
    int out1, out2, out3, out4;
    volatile int mem1, mem2, mem3;
    
    /* Inline asm with memory outputs - forces RELOAD_FOR_OUTPUT_ADDRESS */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "movl %0, %3\n\t"
        : "=r"(out1), "=m"(mem1)
        : "r"(x), "m"(mem1)
        : "eax", "ebx", "ecx", "edx"
    );
    
    /* Another asm with different constraints */
    __asm__ volatile (
        "imull %1, %0\n\t"
        "movl %0, %2\n\t"
        : "=r"(out2), "=r"(out3)
        : "r"(y), "1"(z), "m"(mem2)
        : "cc"
    );
    
    /* Complex addressing in asm output */
    int* addr = &mem3;
    __asm__ volatile (
        "leal (%1, %2, 4), %0\n\t"
        "movl %0, (%3)\n\t"
        : "=r"(out4)
        : "r"(x), "r"(y), "r"(addr)
        : "memory"
    );
    
    return out1 + out2 + out3 + out4 + mem1 + mem2 + mem3;
}

/* Test function 3: Structure access with pointer chasing */
__attribute__((noinline))
static int test_structure_reloads(struct NestedData* data, int count) {
    int sum = 0;
    struct NestedData* current = data;
    
    /* Manual unrolling for register pressure */
    #pragma GCC unroll 4
    for (int i = 0; i < count && i < 4; i++) {
        if (current) {
            /* Structure member access with offset - forces various reloads */
            sum += current->values[0];
            sum += current->values[1];
            sum += current->values[2];
            sum += current->values[3];
            
            /* Matrix access - forces RELOAD_FOR_OPERAND_ADDRESS */
            sum += current->matrix[0][0];
            sum += current->matrix[1][1];
            sum += current->matrix[2][2];
            
            /* Pointer chasing - forces RELOAD_FOR_OPADDR_ADDR */
            current = current->next;
            
            /* More computations to use registers */
            int t1 = sum * i;
            int t2 = t1 + global_seed;
            int t3 = t2 ^ 0x55AA55AA;
            sum = t3;
        }
    }
    
    return sum;
}

/* Test function 4: Mixed addressing modes and volatile accesses */
__attribute__((noinline))
static int test_mixed_addressing(int base) {
    /* Many local arrays to force spills */
    volatile int arr1[8];
    volatile int arr2[8];
    volatile int arr3[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 8; i++) {
        arr1[i] = base + i;
        arr2[i] = base * i;
        arr3[i] = base ^ i;
    }
    
    int result = 0;
    
    /* Complex expressions with multiple memory accesses */
    /* Each line designed to trigger different reload types */
    
    /* RELOAD_FOR_INPUT */
    result = arr1[0] + arr1[1] + arr1[2];
    
    /* RELOAD_FOR_INPUT_ADDRESS with scaling */
    result += *(arr2 + (arr1[3] & 7));
    
    /* Nested addressing for RELOAD_FOR_INPADDR_ADDRESS */
    result += *(arr3 + (*(arr1 + 4) % 8));
    
    /* Multiple intermediate computations */
    int* ptr = (int*)&arr1[0];
    
    /* Pointer arithmetic for RELOAD_FOR_OTHER_ADDRESS */
    result += *(ptr + arr2[0]);
    result += *(ptr + arr2[1] + arr3[0]);
    result += *(ptr + arr2[2] * 2 + arr3[1]);
    
    /* Force many temporaries */
    int t1 = arr1[5];
    int t2 = arr2[5];
    int t3 = arr3[5];
    int t4 = t1 + t2;
    int t5 = t3 * t4;
    int t6 = t5 - arr1[6];
    int t7 = t6 + arr2[6];
    int t8 = t7 ^ arr3[6];
    int t9 = t8 | arr1[7];
    int t10 = t9 & arr2[7];
    
    result += t10;
    
    return result;
}

/* Test function 5: Extreme register pressure with all operand types */
__attribute__((noinline))
static int test_extreme_pressure(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Maximum number of local variables */
    int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e, v6 = f, v7 = g, v8 = h;
    int v9 = a + b, v10 = c + d, v11 = e + f, v12 = g + h;
    int v13 = a * b, v14 = c * d, v15 = e * f, v16 = g * h;
    int v17 = a ^ b, v18 = c ^ d, v19 = e ^ f, v20 = g ^ h;
    int v21 = a | b, v22 = c | d, v23 = e | f, v24 = g | h;
    int v25 = a & b, v26 = c & d, v27 = e & f, v28 = g & h;
    
    volatile int mem[32];
    
    /* Initialize memory */
    for (int i = 0; i < 32; i++) {
        mem[i] = i * global_seed;
    }
    
    /* Complex expression using all variables */
    /* This should trigger RELOAD_OTHER for spill reloads */
    int result = 0;
    
    /* Multiple addressing modes */
    result += mem[v1 & 31];
    result += mem[v2 & 31] + v3;
    result += mem[v4 & 31] * v5;
    result += *(mem + (v6 & 31)) ^ v7;
    result += *(mem + (v8 & 31) + (v9 & 15)) | v10;
    
    /* Chain computations to create dependencies */
    int* ptr1 = mem + (v11 & 15);
    int* ptr2 = mem + (v12 & 15) + 8;
    int* ptr3 = mem + (v13 & 15) + 16;
    
    result += *ptr1 + *ptr2 + *ptr3;
    
    /* More complex pointer expressions */
    result += *(ptr1 + v14) - *(ptr2 + v15);
    result += *(ptr3 + v16) ^ *(mem + v17);
    
    /* Use all remaining variables */
    result += v18 + v19 + v20 + v21 + v22 + v23 + v24;
    result += v25 * v26 * v27 * v28;
    
    /* Final complex expression */
    result = (result & 0xFFFF) + ((result >> 16) & 0xFFFF);
    
    return result;
}

/* Main driver function */
int main(void) {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            global_array[i][j] = i * 16 + j;
        }
    }
    
    /* Initialize structure for testing */
    struct NestedData data[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            data[i].values[j] = i * 8 + j;
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                data[i].matrix[j][k] = i * 9 + j * 3 + k;
            }
        }
        data[i].next = (i < 3) ? &data[i + 1] : NULL;
    }
    
    /* Call all test functions with different arguments */
    total += test_complex_addressing(1, 2, 3, 4, 5, 6);
    total += test_asm_reloads(10, 20, 30);
    total += test_structure_reloads(data, 4);
    total += test_mixed_addressing(100);
    total += test_extreme_pressure(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Call multiple times with different arguments */
    total += test_complex_addressing(7, 8, 9, 10, 11, 12);
    total += test_asm_reloads(40, 50, 60);
    total += test_mixed_addressing(200);
    total += test_extreme_pressure(9, 10, 11, 12, 13, 14, 15, 16);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return total % 256;
    }
    
    return 0;
}
