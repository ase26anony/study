/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex nested structure to force address computations */
struct inner {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct outer {
    struct inner inner_struct[4];
    int base_array[16];
    volatile int index;
};

/* Global variables to increase register pressure */
volatile int g_index1, g_index2, g_index3;
volatile int* g_ptr1, *g_ptr2;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(struct outer* data, int idx1, int idx2) {
    /* Complex addressing: array[(index << 2) + struct.member] */
    int val;
    
    /* Force input address reload with multiple register components */
    asm volatile (
        "movl %[result], %0\n\t"
        : "=r" (val)
        : [result] "m" (data->inner_struct[idx1].member_array[(idx2 << 2) + data->index]),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Another complex input address */
    asm volatile (
        ""
        :: "m" (data->base_array[g_index1 + (idx1 * idx2)]),
           "r" (g_index1), "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    g_ptr1 = &data->inner_struct[0].member_array[val];
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(struct outer* data, int idx1, int idx2, int idx3) {
    /* Complex output addressing */
    asm volatile (
        "movl %1, %0\n\t"
        : "=m" (data->inner_struct[idx1].member_array[(idx2 << 1) + idx3])
        : "r" (idx1 + idx2 + idx3),
          "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    /* Mixed input/output with different addressing */
    int temp = data->index;
    asm volatile (
        "addl %2, %1\n\t"
        "movl %1, %0\n\t"
        : "=m" (data->base_array[temp + g_index2]),
          "+r" (temp)
        : "r" (g_index2),
          "m" (data->inner_struct[0].member_array[idx1])
        : "memory"
    );
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
NOOPT void helper_func(int* addr1, int* addr2, int* addr3) {
    /* Force address computations before call */
    asm volatile (
        ""
        :: "r" (addr1), "r" (addr2), "r" (addr3)
        : "memory"
    );
}

NOOPT void test_operand_address(struct outer* data, int idx1, int idx2) {
    /* Complex address expressions as function arguments */
    helper_func(
        &data->inner_struct[idx1].member_array[idx2],
        &data->base_array[(idx1 << 3) + idx2],
        &data->inner_struct[g_index1].member_array[g_index2]
    );
    
    /* Multiple address computations in one call */
    helper_func(
        &data->inner_struct[0].member_array[data->index],
        &data->base_array[idx1 * idx2],
        &data->inner_struct[idx2].member_array[idx1]
    );
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
NOOPT void test_inpaddr_outaddr(struct outer* data, int idx1, int idx2, int idx3) {
    volatile int* ptr1, *ptr2;
    
    /* Input address of an input address */
    asm volatile (
        "leal (%[base], %[idx1], 4), %[ptr1]\n\t"
        "movl (%[ptr1]), %[temp]\n\t"
        : [ptr1] "=r" (ptr1), [temp] "=r" (g_index3)
        : [base] "r" (&data->inner_struct[0].member_array[0]),
          [idx1] "r" (idx1),
          "m" (data->inner_struct[0].member_array[idx1])
        : "memory"
    );
    
    /* Output address of an output address */
    asm volatile (
        "leal (%[base], %[idx2], 8), %[ptr2]\n\t"
        "movl %[val], (%[ptr2])\n\t"
        : "=m" (*(int**)&data->inner_struct[idx3].volatile_ptr),
          [ptr2] "=&r" (ptr2)
        : [base] "r" (&data->base_array[0]),
          [idx2] "r" (idx2),
          [val] "r" (&g_index1)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
NOOPT void test_other_address(struct outer* data, int idx1, int idx2) {
    /* Multiple complex memory operations in sequence */
    int temp1, temp2, temp3;
    
    /* First operation - creates various reload needs */
    asm volatile (
        "movl (%[addr1]), %[t1]\n\t"
        "addl (%[addr2]), %[t1]\n\t"
        "movl %[t1], (%[addr3])\n\t"
        : [t1] "=&r" (temp1)
        : [addr1] "r" (&data->inner_struct[idx1].member_array[idx2]),
          [addr2] "r" (&data->base_array[idx1 + idx2]),
          [addr3] "r" (&data->inner_struct[idx2].member_array[idx1])
        : "memory"
    );
    
    /* Second operation with different addressing */
    asm volatile (
        "imull %[idx1], %[idx2], %[t2]\n\t"
        "movl %[t2], (%[addr])\n\t"
        : [t2] "=&r" (temp2)
        : [idx1] "r" (idx1),
          [idx2] "r" (idx2),
          [addr] "r" (&data->base_array[temp1 & 0xF])
        : "memory"
    );
    
    /* Third operation forcing OTHER reloads */
    asm volatile (
        ""
        : "=m" (data->inner_struct[0].member_array[0]),
          "=m" (data->inner_struct[1].member_array[0]),
          "=m" (data->inner_struct[2].member_array[0])
        : "r" (idx1), "r" (idx2), "r" (temp1), "r" (temp2)
        : "memory"
    );
}

/* Mixed test combining all patterns */
NOOPT int test_mixed(struct outer* data, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int idx1 = (i * 3) & 0x7;
        int idx2 = (i * 5) & 0x7;
        int idx3 = (i * 7) & 0x7;
        
        /* Update volatile globals to force spills */
        g_index1 = idx1;
        g_index2 = idx2;
        g_index3 = idx3;
        
        /* Call various test functions to create different reload patterns */
        test_input_address(data, idx1, idx2);
        test_output_address(data, idx1, idx2, idx3);
        test_operand_address(data, idx1, idx2);
        test_inpaddr_outaddr(data, idx1, idx2, idx3);
        test_other_address(data, idx1, idx2);
        
        /* Accumulate results to prevent optimization */
        sum += data->inner_struct[idx1].member_array[idx2];
        sum += data->base_array[idx3];
    }
    
    return sum;
}

/* Main driver */
int main() {
    /* Allocate and initialize test data */
    struct outer* data = (struct outer*)malloc(sizeof(struct outer));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            data->inner_struct[i].member_array[j] = i * 100 + j;
        }
        data->inner_struct[i].volatile_ptr = NULL;
    }
    
    for (int i = 0; i < 16; i++) {
        data->base_array[i] = i * 10;
    }
    
    data->index = 2;
    
    /* Initialize global pointers */
    g_ptr1 = &data->base_array[0];
    g_ptr2 = &data->inner_struct[0].member_array[0];
    
    /* Run tests */
    int result = test_mixed(data, 8);
    
    /* Additional targeted tests */
    test_input_address(data, 1, 2);
    test_output_address(data, 2, 3, 1);
    test_operand_address(data, 3, 1);
    test_inpaddr_outaddr(data, 1, 3, 2);
    test_other_address(data, 2, 1);
    
    printf("Test result: %d\n", result);
    printf("Final check: %p %p\n", (void*)g_ptr1, (void*)g_ptr2);
    
    free(data);
    return 0;
}
