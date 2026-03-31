/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register usage patterns */
#define FORCE_REGISTER_PRESSURE __attribute__((noinline, optimize("O0")))

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int flags;
    double coords[3];
};

/* Global arrays to prevent optimization */
volatile int global_counter = 0;
int global_array[256];
struct NestedData global_structs[16];

/* Test 1: Complex array addressing with multiple index computations */
FORCE_REGISTER_PRESSURE
static int test_array_addressing(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = a + c;
    volatile int v5 = b * d;
    volatile int v6 = e - f;
    volatile int v7 = a ^ c;
    volatile int v8 = b + e;
    volatile int v9 = d * f;
    volatile int v10 = a - d;
    
    /* Multi-dimensional array access with complex addressing */
    int arr3d[4][4][4];
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    /* Complex address computation that needs temporary registers */
    for (int i = v1 & 3; i < 4; i++) {
        for (int j = v2 & 3; j < 4; j++) {
            for (int k = v3 & 3; k < 4; k++) {
                /* Nested addressing requiring multiple registers */
                arr3d[i][j][k] = 
                    global_array[(i * 16 + j * 4 + k) & 255] +
                    global_array[((i + v4) * 8 + (j + v5) * 2 + (k + v6)) & 255];
            }
        }
    }
    
    /* More complex expressions to use results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr3d[i][i & 3][(i + v7) & 3] * 
               arr3d[(i + v8) & 3][(i + v9) & 3][(i + v10) & 3];
    }
    
    return sum;
}

/* Test 2: Structure member access with pointer chasing */
FORCE_REGISTER_PRESSURE  
static int test_structure_access(int seed) {
    /* Many pointer variables */
    struct NestedData *ptr1 = &global_structs[seed & 7];
    struct NestedData *ptr2 = &global_structs[(seed + 1) & 7];
    struct NestedData *ptr3 = &global_structs[(seed + 2) & 7];
    struct NestedData *ptr4 = &global_structs[(seed + 3) & 7];
    
    volatile int offset1 = seed * 2;
    volatile int offset2 = seed * 3;
    volatile int offset3 = seed * 5;
    volatile int offset4 = seed * 7;
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    /* Complex structure member accesses */
    int result = 0;
    
    /* Pointer arithmetic with multiple bases */
    result += ptr1->values[offset1 & 7];
    result += ptr2->values[offset2 & 7];
    result += ptr3->values[offset3 & 7];
    result += ptr4->values[offset4 & 7];
    
    /* Nested pointer access - address of pointer needs reload */
    ptr1->next = ptr2;
    ptr2->next = ptr3;
    ptr3->next = ptr4;
    
    /* Chain dereference requiring address reloads */
    if (ptr1->next->next->next) {
        result += ptr1->next->next->next->values[0];
    }
    
    /* Volatile accesses force memory operations */
    ptr1->flags = result;
    ptr2->flags = result * 2;
    ptr3->flags = result * 3;
    ptr4->flags = result * 4;
    
    return result;
}

/* Test 3: Inline assembly with multiple outputs and constraints */
FORCE_REGISTER_PRESSURE
static int test_inline_asm(int x, int y, int z) {
    int out1, out2, out3, out4, out5, out6;
    volatile int mem1, mem2, mem3;
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    /* Inline asm with memory outputs at complex addresses */
    
    /* Memory output with complex address */
    asm volatile (
        "movl %[x], %[mem1]\n\t"
        "addl %[y], %[mem1]\n\t"
        : [mem1] "=m" (global_array[x + y])  /* Complex address computation */
        : [x] "r" (x), [y] "r" (y)
        : "memory"
    );
    
    /* Multiple output operands clobbering many registers */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1\n\t"
        "imull %4, %0\n\t"
        "addl %0, %1\n\t"
        : "=&r" (out1), "=&r" (out2)
        : "r" (x), "r" (y), "r" (z)
        : "cc"
    );
    
    /* More outputs to increase pressure */
    asm volatile (
        "leal (%1, %2, 4), %0\n\t"
        : "=r" (out3)
        : "r" (x), "r" (y)
    );
    
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        : "=r" (out4)
        : "r" (y), "r" (z)
    );
    
    /* Force RELOAD_FOR_OTHER_ADDRESS with obscure constraints */
    int *ptr = &global_array[z & 255];
    asm volatile (
        "movl (%1), %0\n\t"
        "addl $1, %0\n\t"
        "movl %0, (%1)\n\t"
        : "=&r" (out5)
        : "r" (ptr)  /* Pointer operand needing reload */
        : "memory"
    );
    
    return out1 + out2 + out3 + out4 + out5;
}

/* Test 4: Mixed addressing modes in loops */
FORCE_REGISTER_PRESSURE
static int test_mixed_addressing(int iterations) {
    int arr1[32], arr2[32], arr3[32];
    int *ptr_arr[8];
    int sum = 0;
    
    /* Initialize arrays and pointers */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * iterations;
        arr2[i] = i + iterations;
        arr3[i] = i ^ iterations;
    }
    
    for (int i = 0; i < 8; i++) {
        ptr_arr[i] = &arr1[i * 4];
    }
    
    /* Unrolled loop with mixed addressing */
    #pragma GCC unroll 4
    for (int i = 0; i < iterations && i < 8; i++) {
        /* Multiple addressing modes in one expression */
        sum += arr1[i] + 
               *(ptr_arr[i] + (i * 2)) +  /* Base + index*scale */
               arr2[arr3[i] & 31] +       /* Index from array */
               global_array[(i * ptr_arr[i][0]) & 255]; /* Complex index */
        
        /* Force address reloads with volatile */
        volatile int *volatile vptr = ptr_arr[i];
        sum += *vptr;
        
        /* Pointer arithmetic requiring temporary */
        int *temp = ptr_arr[i] + arr1[i] % 4;
        sum += *temp;
    }
    
    return sum;
}

/* Test 5: Extreme register pressure with all operand types */
FORCE_REGISTER_PRESSURE
static int test_extreme_pressure(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Maximum local variables */
    int v1 = a + b, v2 = b + c, v3 = c + d, v4 = d + e;
    int v5 = e + f, v6 = f + g, v7 = g + h, v8 = h + a;
    int v9 = a * b, v10 = b * c, v11 = c * d, v12 = d * e;
    int v13 = e * f, v14 = f * g, v15 = g * h, v16 = h * a;
    volatile int v17 = v1 ^ v2, v18 = v3 ^ v4, v19 = v5 ^ v6, v20 = v7 ^ v8;
    
    /* Arrays for additional pressure */
    int arr1[8], arr2[8], arr3[8];
    int *ptrs[4];
    
    /* Complex expressions using all variables */
    for (int i = 0; i < 8; i++) {
        arr1[i] = v1 + i * v2;
        arr2[i] = v3 + i * v4;
        arr3[i] = v5 + i * v6;
    }
    
    ptrs[0] = &arr1[v7 & 7];
    ptrs[1] = &arr2[v8 & 7];
    ptrs[2] = &arr3[v9 & 7];
    ptrs[3] = &global_array[v10 & 255];
    
    /* Expression triggering multiple reload types */
    int result = 0;
    
    /* RELOAD_FOR_INPUT: values used multiple times after clobbering */
    result += (v1 * v2) + (v1 / v3) + (v1 % v4);
    
    /* RELOAD_FOR_INPUT_ADDRESS: complex array addressing */
    result += arr1[arr2[arr3[v11 & 7] & 7] & 7];
    
    /* RELOAD_FOR_OPERAND_ADDRESS: pointer dereference chain */
    result += *(ptrs[0] + (v12 & 3));
    result += *(ptrs[1] + (v13 & 3));
    result += *(ptrs[2] + (v14 & 3));
    
    /* Inline asm to clobber registers between uses */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (global_array[v15 & 255])
        : "r" (v15), "r" (v16)
        : "%eax", "%ebx", "memory"
    );
    
    /* Use values again after asm clobber - forces reloads */
    result += v1 + v2 + v3 + v4;
    
    return result;
}

/* Main driver */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].values[j] = i * 8 + j;
        }
        global_structs[i].next = NULL;
        global_structs[i].flags = 0;
    }
    
    /* Run all tests to trigger different reload patterns */
    result += test_array_addressing(1, 2, 3, 4, 5, 6);
    result += test_structure_access(argc);
    result += test_inline_asm(7, 8, 9);
    result += test_mixed_addressing(10);
    result += test_extreme_pressure(11, 12, 13, 14, 15, 16, 17, 18);
    
    /* Use result to prevent dead code elimination */
    global_counter = result;
    
    return result & 255;
}
