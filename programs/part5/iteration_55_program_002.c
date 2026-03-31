/* reload_coverage.c - Program to trigger various reload types in GCC reload1.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* ========== Test Function 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static int __attribute__((noinline)) 
test_array_addressing(int seed) {
    /* Many local variables to consume registers */
    volatile int a1 = seed + 1;
    volatile int a2 = seed + 2;
    volatile int a3 = seed + 3;
    volatile int a4 = seed + 4;
    volatile int a5 = seed + 5;
    volatile int a6 = seed + 6;
    volatile int a7 = seed + 7;
    volatile int a8 = seed + 8;
    volatile int a9 = seed + 9;
    volatile int a10 = seed + 10;
    
    /* Multi-dimensional arrays with complex indexing */
    int arr3d[3][4][5];
    int arr2d[8][8];
    
    /* Initialize arrays */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k + seed;
            }
        }
    }
    
    /* Complex address computations that need multiple registers */
    int sum = 0;
    
    /* Manual loop unrolling to increase register pressure */
    /* Each iteration uses different index computations */
    sum += arr3d[a1%3][a2%4][a3%5];          /* Direct 3D access */
    sum += arr3d[(a1+a2)%3][(a2+a3)%4][a4%5]; /* Complex indices */
    sum += arr3d[a5%3][(a6*a7)%4][a8%5];      /* More complex */
    sum += arr3d[(a9-a10)%3][a1%4][a2%5];     /* Mixed operations */
    
    /* Pointer arithmetic with multiple bases */
    int *ptr1 = &arr3d[0][0][0];
    int *ptr2 = &arr3d[1][0][0];
    int *ptr3 = &arr3d[2][0][0];
    
    /* Address computations that may need reloads */
    sum += *(ptr1 + a1*4 + a2);      /* RELOAD_FOR_INPUT_ADDRESS */
    sum += *(ptr2 + a3*3 + a4);      /* Another address computation */
    sum += *(ptr3 + a5*2 + a6);      /* More pressure */
    
    /* Nested addressing: address of address computation */
    int **ptr_to_ptr = &ptr1;
    sum += **ptr_to_ptr;             /* RELOAD_FOR_OPERAND_ADDRESS */
    
    return sum;
}

/* ========== Test Function 2: Structure and Inline Assembly ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
static int __attribute__((noinline))
test_struct_asm(int seed) {
    /* Complex structure with many members */
    struct BigStruct {
        int x[10];
        int y[10];
        int z[10];
        volatile int *ptr;
    };
    
    /* Multiple structure instances */
    struct BigStruct s1, s2, s3;
    volatile int temp;
    
    /* Initialize structures */
    for (int i = 0; i < 10; i++) {
        s1.x[i] = seed + i;
        s1.y[i] = seed * i;
        s1.z[i] = seed - i;
        
        s2.x[i] = seed + i * 2;
        s2.y[i] = seed * i * 2;
        s2.z[i] = seed - i * 2;
    }
    
    s1.ptr = &s1.x[0];
    s2.ptr = &s2.y[0];
    
    /* Complex structure member accesses with offsets */
    int result = 0;
    
    /* Multiple volatile reads to force memory accesses */
    temp = s1.x[0];
    result += temp;
    temp = s1.y[1];
    result += temp;
    temp = s1.z[2];
    result += temp;
    
    /* Inline assembly with multiple outputs and clobbers */
    /* This creates RELOAD_FOR_OUTPUT_ADDRESS */
    int out1, out2, out3;
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "subl %[in4], %[out2]\n\t"
        "movl %[in5], %[out3]\n\t"
        "imull %[in6], %[out3]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [in1] "r" (s1.x[0]), [in2] "r" (s1.y[1]), 
          [in3] "r" (s2.x[0]), [in4] "r" (s2.y[1]),
          [in5] "r" (seed), [in6] "r" (seed + 1)
        : "cc", "memory"
    );
    
    result += out1 + out2 + out3;
    
    /* More inline asm with memory output */
    /* RELOAD_FOR_OUTADDR_ADDRESS */
    int mem_output;
    asm volatile (
        "movl %%eax, %[mem]"
        : [mem] "=m" (mem_output)
        : "a" (result)
        : "memory"
    );
    
    return result + mem_output;
}

/* ========== Test Function 3: Pointer Chasing and Mixed Types ========== */
/* Targets: RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
static int __attribute__((noinline))
test_pointer_chasing(int seed) {
    /* Many pointer variables */
    int *p1, *p2, *p3, *p4, *p5;
    int **pp1, **pp2, **pp3;
    int ***ppp1;
    
    /* Local arrays for pointer targets */
    int arr1[20], arr2[20], arr3[20];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        arr1[i] = seed + i * 3;
        arr2[i] = seed + i * 5;
        arr3[i] = seed + i * 7;
    }
    
    /* Complex pointer assignments */
    p1 = &arr1[0];
    p2 = &arr2[0];
    p3 = &arr3[0];
    
    pp1 = &p1;
    pp2 = &p2;
    pp3 = &p3;
    
    ppp1 = &pp1;
    
    /* Pointer chasing with arithmetic */
    int sum = 0;
    
    /* Multiple levels of indirection */
    sum += **pp1;           /* Double dereference */
    sum += ***ppp1;         /* Triple dereference */
    
    /* Pointer arithmetic in addressing */
    sum += *(p1 + (seed % 10));     /* Indexed access */
    sum += *(*(pp2) + (seed % 5));  /* Pointer + index */
    
    /* Complex address computation */
    p4 = p1 + (seed % 8);
    p5 = p2 + (seed % 6);
    
    sum += *p4 + *p5;
    
    /* Mix with volatile */
    volatile int *volatile_ptr = p3;
    sum += *volatile_ptr;
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(sum > 100, 0)) {
        sum += **pp3;
    } else {
        sum += *p1;
    }
    
    return sum;
}

/* ========== Test Function 4: Mixed Operations with Many Temporaries ========== */
/* Targets: All reload types through diverse patterns */
static int __attribute__((noinline))
test_mixed_operations(int seed) {
    /* Maximum register pressure with many variables */
    int v1 = seed, v2 = seed*2, v3 = seed*3, v4 = seed*4;
    int v5 = seed*5, v6 = seed*6, v7 = seed*7, v8 = seed*8;
    int v9 = seed*9, v10 = seed*10, v11 = seed*11, v12 = seed*12;
    int v13 = seed*13, v14 = seed*14, v15 = seed*15, v16 = seed*16;
    
    volatile int mem1, mem2, mem3, mem4;
    
    /* Complex expression with many intermediate values */
    int result = 0;
    
    /* Unrolled computation to use all variables */
    result += v1 * v2 + v3 - v4;
    result += v5 / (v6 + 1) * v7;
    result += (v8 << 2) | (v9 >> 1);
    result += v10 & v11 ^ v12;
    result += v13 % (v14 + 1) + v15 * v16;
    
    /* Memory operations with complex addresses */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = seed + i;
    }
    
    /* Multiple array accesses with different index computations */
    mem1 = array[v1 % 16];
    mem2 = array[v2 % 16];
    mem3 = array[v3 % 16];
    mem4 = array[v4 % 16];
    
    result += mem1 + mem2 + mem3 + mem4;
    
    /* More complex: array of pointers */
    int *ptr_array[8];
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = &array[i * 2];
    }
    
    /* Chain of pointer accesses */
    result += *ptr_array[0];
    result += **(&ptr_array[1]);
    result += *ptr_array[v5 % 8];
    
    return result;
}

/* ========== Main Driver ========== */
int main(int argc, char *argv[]) {
    int seed = 42;
    int total = 0;
    
    /* Call all test functions multiple times with different seeds */
    total += test_array_addressing(seed);
    total += test_struct_asm(seed + 1);
    total += test_pointer_chasing(seed + 2);
    total += test_mixed_operations(seed + 3);
    
    /* Call again with different parameters */
    total += test_array_addressing(seed + 10);
    total += test_struct_asm(seed + 11);
    total += test_pointer_chasing(seed + 12);
    total += test_mixed_operations(seed + 13);
    
    printf("Total result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        return 0;
    } else {
        return 1;
    }
}

#pragma GCC pop_options
