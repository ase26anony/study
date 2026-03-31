/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
struct Inner {
    int data[4];
    int *ptr_array[3];
};

struct Outer {
    struct Inner inner[5];
    volatile int index;
    int base;
};

/* Global variables to increase register pressure */
volatile int g_index1, g_index2, g_index3;
int g_array[100];
struct Outer g_outer;

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer *outer, int idx1, int idx2) {
    /* Complex addressing that requires input address reload */
    int val;
    
    /* Force address computation with multiple components */
    asm volatile (
        "movl %[input], %[output]\n\t"
        : [output] "=r" (val)
        : [input] "m" (outer->inner[idx1].data[idx2 * 2 + outer->base])
        : "memory"
    );
    
    /* Another complex input address */
    asm volatile (
        ""
        :
        : "m" (outer->inner[g_index1].ptr_array[idx1][idx2]),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    g_array[0] = val;
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer *outer, int idx1, int idx2, int value) {
    /* Complex output addressing */
    asm volatile (
        "movl %[val], %[addr]\n\t"
        : [addr] "=m" (outer->inner[idx1 + g_index2].data[idx2])
        : [val] "r" (value),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Output to computed address with shift */
    asm volatile (
        ""
        : "=m" (g_array[(idx1 << 2) + idx2 + outer->base])
        : "r" (value), "r" (idx1), "r" (idx2)
        : "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void helper_func(int *addr1, int *addr2) {
    /* Force address computations before call */
    *addr1 += *addr2;
}

void test_operand_address(struct Outer *outer, int idx1, int idx2) {
    /* Complex address expressions as function arguments */
    helper_func(
        &outer->inner[idx1].data[idx2 * 3],
        &g_array[(idx1 + idx2) * 2 + outer->base]
    );
    
    /* More complex nested addressing */
    helper_func(
        outer->inner[g_index1].ptr_array[idx1] + idx2,
        &outer->inner[idx2].data[outer->index]
    );
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct Outer *outer, int idx1, int idx2) {
    int temp;
    
    /* Mixed input/output with address-of operators */
    asm volatile (
        "lea (%[base], %[index], 4), %[temp]\n\t"
        "movl (%[temp]), %[temp]\n\t"
        : [temp] "=&r" (temp)
        : [base] "r" (&outer->inner[0].data[0]),
          [index] "r" (idx1 * 5 + idx2)
        : "memory"
    );
    
    /* Output to address that itself needs computation */
    int *dest_ptr = &outer->inner[idx1].data[idx2];
    asm volatile (
        "movl %[val], (%[dest])\n\t"
        : "=m" (*dest_ptr)
        : [val] "r" (temp),
          [dest] "r" (dest_ptr)
        : "memory"
    );
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct Outer *outer, int idx1, int idx2, int idx3) {
    /* Multiple complex addresses in one asm */
    asm volatile (
        "imull %[idx1], %[idx2]\n\t"
        "addl %[idx3], %[idx2]\n\t"
        "movl (%[base], %[idx2], 4), %[idx1]\n\t"
        : [idx1] "+r" (idx1), [idx2] "+r" (idx2)
        : [idx3] "r" (idx3),
          [base] "r" (&g_array[outer->base])
        : "memory", "cc"
    );
    
    /* Chain of dependent address computations */
    int offset = (idx1 << 3) | (idx2 << 1);
    asm volatile (
        ""
        : "=m" (outer->inner[offset % 5].data[offset % 4])
        : "r" (idx3), "r" (offset)
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(struct Outer *outer, int iterations) {
    for (int i = 0; i < iterations; i++) {
        int idx1 = (i * 3) % 5;
        int idx2 = (i * 7) % 4;
        int idx3 = (i * 11) % 3;
        
        /* Alternate between different patterns */
        switch (i % 4) {
            case 0:
                test_input_address(outer, idx1, idx2);
                break;
            case 1:
                test_output_address(outer, idx1, idx2, i);
                break;
            case 2:
                test_operand_address(outer, idx1, idx2);
                break;
            case 3:
                test_inpaddr_outaddr(outer, idx1, idx2);
                test_other_address(outer, idx1, idx2, idx3);
                break;
        }
        
        /* Force spill/reload between iterations */
        g_index1 = idx1;
        g_index2 = idx2;
        g_index3 = idx3;
    }
}

/* Initialize test data */
void init_test_data(void) {
    for (int i = 0; i < 100; i++) {
        g_array[i] = i * 2;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            g_outer.inner[i].data[j] = i * 10 + j;
        }
        for (int j = 0; j < 3; j++) {
            g_outer.inner[i].ptr_array[j] = &g_array[(i * 3 + j) * 2];
        }
    }
    
    g_outer.index = 2;
    g_outer.base = 16;
    
    g_index1 = 1;
    g_index2 = 2;
    g_index3 = 3;
}

/* Main driver */
int main(int argc, char **argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
        if (iterations > 100) iterations = 100;
    }
    
    init_test_data();
    
    printf("Testing reload coverage with %d iterations\n", iterations);
    
    /* Run all tests to trigger different reload types */
    test_mixed_reloads(&g_outer, iterations);
    
    /* Additional focused tests */
    test_input_address(&g_outer, 2, 1);
    test_output_address(&g_outer, 1, 3, 42);
    test_operand_address(&g_outer, 3, 2);
    test_inpaddr_outaddr(&g_outer, 0, 2);
    test_other_address(&g_outer, 2, 1, 0);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += g_array[i];
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            checksum += g_outer.inner[i].data[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    return 0;
}
