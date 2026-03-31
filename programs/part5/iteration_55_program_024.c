/* reload_coverage.c - Test program to trigger various reload types in GCC reload1.cc */
#include <stdint.h>
#include <stdlib.h>

/* Force specific architecture for register scarcity */
#ifdef __x86_64__
#error "Compile with -m32 for 32-bit target to increase register pressure"
#endif

/* Prevent unwanted optimizations */
#pragma GCC optimize("O0")
#pragma GCC push_options

/* Volatile variables to force memory accesses */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = NULL;

/* Complex structure for address computations */
struct NestedData {
    int values[8];
    struct NestedData* next;
    int matrix[3][3];
    volatile int flags;
};

/* Array of structures for complex addressing */
static struct NestedData g_data_array[16];

/* ========== TEST FUNCTION 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static int __attribute__((noinline)) test_complex_addressing(int idx1, int idx2, int idx3) {
    /* Many local variables to consume registers */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    int b1, b2, b3, b4, b5, b6, b7, b8, b9, b10;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    volatile int v1, v2, v3, v4;
    
    /* Multi-dimensional array with complex indexing */
    int md_array[4][5][6];
    
    /* Initialize with volatile reads to prevent optimization */
    v1 = g_volatile_counter;
    v2 = *g_volatile_ptr;
    
    /* Complex address computations that need multiple registers */
    a1 = md_array[idx1][idx2][idx3];
    a2 = md_array[idx1+1][idx2*2][idx3/2];
    a3 = md_array[idx1*2][idx2+1][idx3-1];
    
    /* Nested addressing with multiple computations */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
    a4 = md_array[md_array[idx1][idx2][0]][md_array[0][idx2][idx3]][md_array[idx1][0][idx3]];
    
    /* Pointer arithmetic with multiple bases */
    int* ptr1 = &md_array[idx1][idx2][idx3];
    int* ptr2 = &md_array[idx2][idx3][idx1];
    int* ptr3 = &md_array[idx3][idx1][idx2];
    
    /* Complex expression with many intermediate values */
    /* Forces RELOAD_FOR_INPUT when registers spill */
    b1 = (*ptr1) * (*ptr2) + (*ptr3);
    b2 = (*ptr2) * (*ptr3) + (*ptr1);
    b3 = (*ptr3) * (*ptr1) + (*ptr2);
    
    /* Chain computations to increase register pressure */
    c1 = b1 + b2 + b3;
    c2 = b1 * b2 * b3;
    c3 = (b1 << 2) | (b2 << 1) | b3;
    
    /* Manual loop unrolling for more register pressure */
    /* Each iteration uses different addressing modes */
#pragma GCC unroll 4
    for (int i = 0; i < 4; i++) {
        /* Different addressing each iteration */
        a5 += md_array[i][idx1][idx2] * md_array[idx2][i][idx3];
        a6 += md_array[idx3][idx2][i] * md_array[i][idx3][idx1];
        
        /* Volatile access forces spill */
        v3 = g_volatile_counter + i;
        g_volatile_counter = v3;
    }
    
    /* Final complex computation */
    return a1 + a2 + a3 + a4 + b1 + b2 + b3 + c1 + c2 + c3 + a5 + a6 + v1 + v2 + v3;
}

/* ========== TEST FUNCTION 2: Structure and Inline Assembly ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
static int __attribute__((noinline)) test_asm_and_structs(int base_idx) {
    struct NestedData local_data[4];
    int results[8];
    volatile int temp;
    
    /* Initialize structure with complex patterns */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            local_data[i].values[j] = base_idx + i * 8 + j;
        }
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                local_data[i].matrix[j][k] = (base_idx + i + j + k) * 2;
            }
        }
        local_data[i].flags = i;
    }
    
    /* Inline assembly with multiple outputs and complex addressing */
    /* This should trigger various address reload types */
    for (int i = 0; i < 4; i++) {
        int* volatile ptr = &local_data[i].values[0];
        struct NestedData* volatile sptr = &local_data[i];
        
        /* Complex address computation before asm */
        int offset = (i * 7 + base_idx) % 8;
        
        /* Inline asm with memory outputs and clobbers */
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS */
        __asm__ volatile (
            "movl %[offset], %%eax\n\t"
            "leal (%[ptr], %%eax, 4), %%ebx\n\t"
            "movl (%%ebx), %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            "movl %%ecx, (%%ebx)\n\t"
            "movl %%ecx, %[result]\n\t"
            : [result] "=m" (results[i])  /* Memory output */
            : [ptr] "r" (ptr), [offset] "r" (offset)
            : "eax", "ebx", "ecx", "memory"
        );
        
        /* Another asm with structure member access */
        /* May trigger RELOAD_FOR_OPERAND_ADDRESS */
        int matrix_val;
        __asm__ volatile (
            "movl %[sptr], %%esi\n\t"
            "movl $12, %%eax\n\t"
            "addl %%eax, %%esi\n\t"
            "movl (%%esi), %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (matrix_val)
            : [sptr] "r" (sptr)
            : "esi", "eax", "memory"
        );
        
        results[i + 4] = matrix_val;
    }
    
    /* Sum results with volatile accesses */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        temp = results[i];
        sum += temp;
        g_volatile_counter = sum;
    }
    
    return sum;
}

/* ========== TEST FUNCTION 3: Pointer Chasing and Multiple Reload Types ========== */
/* Targets: RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
static int __attribute__((noinline)) test_pointer_chasing(int iterations) {
    /* Create a linked structure in local array */
    struct NestedData nodes[8];
    for (int i = 0; i < 7; i++) {
        nodes[i].next = &nodes[i + 1];
        for (int j = 0; j < 8; j++) {
            nodes[i].values[j] = (i * 8 + j) * 3;
        }
    }
    nodes[7].next = &nodes[0];  /* Circular */
    
    /* Many pointer variables to increase register pressure */
    struct NestedData* p1, *p2, *p3, *p4, *p5;
    int* ip1, *ip2, *ip3, *ip4;
    volatile int v[8];
    
    /* Start with different pointers */
    p1 = &nodes[0];
    p2 = &nodes[2];
    p3 = &nodes[4];
    p4 = &nodes[6];
    
    int total = 0;
    
    /* Pointer chasing loop with complex expressions */
    for (int i = 0; i < iterations; i++) {
        /* Multiple levels of indirection */
        ip1 = &p1->values[i % 8];
        ip2 = &p2->values[(i + 1) % 8];
        ip3 = &p3->values[(i + 2) % 8];
        ip4 = &p4->values[(i + 3) % 8];
        
        /* Complex address computations */
        /* May trigger RELOAD_FOR_OPADDR_ADDR */
        int val1 = *(ip1 + (i % 4));
        int val2 = *(ip2 + ((i + 1) % 4));
        int val3 = *(ip3 + ((i + 2) % 4));
        int val4 = *(ip4 + ((i + 3) % 4));
        
        /* Use __builtin_expect to create data dependencies */
        if (__builtin_expect((val1 & 1) != 0, 0)) {
            val2 += val1;
        }
        if (__builtin_expect((val2 & 2) != 0, 1)) {
            val3 += val2;
        }
        
        /* Volatile writes force spills */
        v[i % 8] = val1 + val2 + val3 + val4;
        total += v[i % 8];
        
        /* Update pointers with complex addressing */
        p1 = p1->next->next;
        p2 = p2->next;
        p3 = p3->next->next->next;
        p4 = (struct NestedData*)((char*)p4->next + 1);
        
        /* Access structure through volatile pointer */
        /* May trigger RELOAD_FOR_OTHER_ADDRESS */
        volatile struct NestedData* vp = (volatile struct NestedData*)p1;
        total += vp->flags;
    }
    
    /* Final computation with many temporaries */
    int t1 = total * 3;
    int t2 = total / 2;
    int t3 = total << 1;
    int t4 = total >> 1;
    int t5 = t1 + t2;
    int t6 = t3 + t4;
    int t7 = t5 * t6;
    int t8 = t7 - total;
    
    return t8;
}

/* ========== TEST FUNCTION 4: Mixed Patterns for Complete Coverage ========== */
static int __attribute__((noinline)) test_mixed_patterns(int seed) {
    /* Combine all patterns in one function */
    int array3d[3][4][5];
    struct NestedData structs[3];
    int* ptr_array[8];
    volatile int vol_vars[4];
    
    /* Initialize everything */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                array3d[i][j][k] = seed + i * 100 + j * 10 + k;
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 8; j++) {
            structs[i].values[j] = seed * (i + 1) * (j + 1);
        }
        structs[i].next = &structs[(i + 1) % 3];
    }
    
    /* Create complex pointer expressions */
    ptr_array[0] = &array3d[0][0][0];
    ptr_array[1] = &array3d[1][1][1];
    ptr_array[2] = &array3d[2][2][2];
    ptr_array[3] = structs[0].values;
    ptr_array[4] = structs[1].values;
    ptr_array[5] = structs[2].values;
    ptr_array[6] = (int*)&structs[0];
    ptr_array[7] = (int*)&structs[1];
    
    /* Complex computation mixing all patterns */
    int result = 0;
    
    /* Manual unrolling */
    result += *(ptr_array[0] + seed % 5);
    result += *(ptr_array[1] + (seed + 1) % 5);
    result += *(ptr_array[2] + (seed + 2) % 5);
    result += *(ptr_array[3] + (seed + 3) % 8);
    result += *(ptr_array[4] + (seed + 4) % 8);
    result += *(ptr_array[5] + (seed + 5) % 8);
    
    /* Inline asm with multiple constraints */
    int asm_result;
    __asm__ volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl %%ebx, %[sum]\n\t"
        "movl %[sum], %[out]\n\t"
        : [out] "=r" (asm_result), [sum] "+r" (result)
        : [addr] "r" (ptr_array[6])
        : "eax", "ebx", "memory"
    );
    
    /* Force volatile accesses */
    for (int i = 0; i < 4; i++) {
        vol_vars[i] = result + i;
        result += vol_vars[i];
        g_volatile_counter = result;
    }
    
    return result + asm_result;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char** argv) {
    /* Initialize global volatile pointer */
    int dummy = 42;
    g_volatile_ptr = &dummy;
    
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            g_data_array[i].values[j] = i * 8 + j;
        }
        g_data_array[i].flags = i;
    }
    
    int total_result = 0;
    
    /* Call all test functions with different arguments */
    total_result += test_complex_addressing(1, 2, 3);
    total_result += test_complex_addressing(0, 3, 1);
    total_result += test_complex_addressing(2, 1, 0);
    
    total_result += test_asm_and_structs(10);
    total_result += test_asm_and_structs(20);
    
    total_result += test_pointer_chasing(8);
    total_result += test_pointer_chasing(12);
    
    total_result += test_mixed_patterns(5);
    total_result += test_mixed_patterns(15);
    total_result += test_mixed_patterns(25);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000) {
        return total_result % 256;
    }
    
    return 0;
}

#pragma GCC pop_options
