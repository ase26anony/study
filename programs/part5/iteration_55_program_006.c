/* reload_coverage.c - Program to trigger various reload types in GCC reload pass */

#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage and prevent optimizations */
#pragma GCC optimize ("O0")
#pragma GCC push_options

/* Test structure for complex addressing */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int flags;
};

/* Global arrays to force memory accesses */
volatile int global_array[256];
volatile struct NestedData global_structs[32];

/* ========== TEST 1: Complex array addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
__attribute__((noinline))
static int test_complex_addressing(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to consume registers */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = e ^ f;
    volatile int v4 = g & h;
    volatile int v5 = a + c + e + g;
    volatile int v6 = b + d + f + h;
    volatile int v7 = v1 * v2;
    volatile int v8 = v3 | v4;
    
    /* Multi-dimensional array with complex indexing */
    int arr3d[4][4][4];
    
    /* Complex addressing that requires address reloads */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                /* This creates RELOAD_FOR_INPUT_ADDRESS */
                arr3d[i][j][k] = global_array[
                    (i * v1 + j * v2 + k * v3 + v4) & 0xFF
                ];
                
                /* More complex expression for RELOAD_FOR_INPADDR_ADDRESS */
                global_array[
                    ((i << v5) + (j << v6) + (k << v7)) & 0xFF
                ] = arr3d[k][j][i] + v8;
            }
        }
    }
    
    /* Sum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                sum += arr3d[i][j][k];
            }
        }
    }
    return sum;
}

/* ========== TEST 2: Inline assembly with multiple outputs ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_OTHER */
__attribute__((noinline))
static int test_asm_reloads(int x, int y, int z) {
    volatile int results[8];
    volatile int *ptr_array[4];
    volatile int temp1, temp2, temp3, temp4;
    
    /* Setup pointer array with complex addresses */
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &results[i * 2] + (x * y * z * i);
    }
    
    /* Inline asm with multiple outputs and clobbers */
    /* This should trigger various reload types */
    for (int i = 0; i < 4; i++) {
        int idx = i + x + y + z;
        asm volatile (
            /* Complex output addressing - RELOAD_FOR_OUTPUT_ADDRESS */
            "movl %[val], (%[addr1], %[idx1], 4)\n\t"
            /* Another output with different addressing - RELOAD_FOR_OUTADDR_ADDRESS */
            "leal (%[base], %[idx2], 2), %%eax\n\t"
            "movl %%eax, (%[addr2])\n\t"
            /* Multiple clobbers to increase register pressure */
            :
            : [val] "r" (idx),
              [addr1] "r" (ptr_array[i]),
              [idx1] "r" (i * x * y),
              [base] "r" (&global_array[0]),
              [idx2] "r" (i * z),
              [addr2] "r" (&results[7 - i])
            : "eax", "ebx", "ecx", "edx", "memory", "cc"
        );
    }
    
    return results[0] + results[7];
}

/* ========== TEST 3: Structure access with pointer chasing ========== */
/* Targets: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR, RELOAD_FOR_OTHER_ADDRESS */
__attribute__((noinline))
static int test_structure_access(int seed) {
    /* Many local structure pointers */
    struct NestedData *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8;
    volatile int counter = seed;
    
    /* Initialize structure pointers with complex expressions */
    p1 = &global_structs[(seed * 1) & 31];
    p2 = &global_structs[(seed * 3) & 31];
    p3 = &global_structs[(seed * 5) & 31];
    p4 = &global_structs[(seed * 7) & 31];
    p5 = &global_structs[(seed * 11) & 31];
    p6 = &global_structs[(seed * 13) & 31];
    p7 = &global_structs[(seed * 17) & 31];
    p8 = &global_structs[(seed * 19) & 31];
    
    /* Chain structures */
    p1->next = p2;
    p2->next = p3;
    p3->next = p4;
    p4->next = p5;
    p5->next = p6;
    p6->next = p7;
    p7->next = p8;
    p8->next = p1;
    
    /* Pointer chasing with complex addressing */
    /* This should trigger RELOAD_FOR_OPERAND_ADDRESS */
    int sum = 0;
    struct NestedData *current = p1;
    for (int i = 0; i < 32; i++) {
        /* Complex array indexing within structure */
        int idx = (i * seed + counter++) & 7;
        
        /* Multiple structure accesses - operand addresses need reloading */
        sum += current->values[idx];
        sum += current->next->values[(idx + 1) & 7];
        sum += current->next->next->values[(idx + 2) & 7];
        
        /* Volatile access forces memory operations */
        current->flags = sum;
        
        /* Complex pointer arithmetic */
        current = current->next + ((i * seed) & 1);
    }
    
    return sum;
}

/* ========== TEST 4: Mixed operations with unrolled loops ========== */
/* Targets all reload types through varied patterns */
__attribute__((noinline))
static int test_mixed_operations(void) {
    volatile int array1[16], array2[16], array3[16];
    volatile int *ptrs[8];
    int accumulators[8];  /* Many accumulators to use registers */
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
        array3[i] = i * 7;
    }
    
    for (int i = 0; i < 8; i++) {
        ptrs[i] = &array1[i * 2];
        accumulators[i] = 0;
    }
    
    /* Manually unrolled loop with complex expressions */
    /* Each iteration uses different addressing modes */
    
    /* Iteration 1: Direct array access */
    accumulators[0] += array1[array2[array3[0] & 15] & 15];
    
    /* Iteration 2: Pointer arithmetic */
    accumulators[1] += *(ptrs[1] + (array2[1] & 7));
    
    /* Iteration 3: Nested pointer dereference */
    accumulators[2] += **(volatile int **)((void *)&ptrs[2]);
    
    /* Iteration 4: Complex inline asm */
    asm volatile (
        "movl %[src], %%eax\n\t"
        "addl %%eax, %[dst]\n\t"
        : [dst] "+m" (accumulators[3])
        : [src] "mr" (array3[3])
        : "eax", "cc"
    );
    
    /* Iteration 5: More complex addressing */
    accumulators[4] += global_array[
        (accumulators[0] + accumulators[1] + accumulators[2]) & 0xFF
    ];
    
    /* Iteration 6: Structure member access */
    accumulators[5] += global_structs[0].values[accumulators[3] & 7];
    
    /* Iteration 7: Volatile with complex index */
    int idx = accumulators[4] ^ accumulators[5];
    accumulators[6] += *(volatile int *)((char *)&array1[0] + (idx & 63));
    
    /* Iteration 8: Final mix */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "imull %[b], %%eax\n\t"
        "addl %%eax, %[c]\n\t"
        : [c] "+m" (accumulators[7])
        : [a] "r" (accumulators[6]),
          [b] "r" (idx)
        : "eax", "cc"
    );
    
    /* Sum all accumulators */
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += accumulators[i];
    }
    
    return total;
}

/* ========== MAIN DRIVER ========== */
int main(void) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].values[j] = i * 8 + j;
        }
        global_structs[i].next = NULL;
        global_structs[i].flags = 0;
    }
    
    /* Call all test functions with different arguments */
    result += test_complex_addressing(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_complex_addressing(8, 7, 6, 5, 4, 3, 2, 1);
    
    result += test_asm_reloads(42, 17, 99);
    result += test_asm_reloads(13, 27, 53);
    
    result += test_structure_access(12345);
    result += test_structure_access(54321);
    
    result += test_mixed_operations();
    result += test_mixed_operations();
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result & 0xFF;
}

#pragma GCC pop_options
