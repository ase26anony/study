/* test_reload_coverage.c
 * 
 * This program creates complex addressing patterns to force GCC's reload pass
 * to generate various reload types, specifically targeting the switch cases
 * in chain_reload_insns() in reload1.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify addressing */
#define NOOPT __attribute__((optimize("O0")))

/* Complex nested structures to create addressing complexity */
struct Inner {
    int data[8];
    volatile int* volatile_ptr;
};

struct Middle {
    struct Inner inner[4];
    long offset;
    volatile int index;
};

struct Outer {
    struct Middle middle[3];
    int base;
    volatile int* dynamic_base;
};

/* Global variables to increase register pressure */
volatile int g_index1, g_index2, g_index3;
volatile int* g_ptr1, *g_ptr2;
struct Outer g_outer;

/* Function to create RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    /* Complex addressing: outer->middle[idx1].inner[idx2].data[idx3] */
    int val;
    
    /* Force address computation into registers */
    asm volatile (
        "mov %[addr], %[val]\n\t"
        : [val] "=r" (val)
        : [addr] "m" (outer->middle[idx1].inner[idx2].data[idx3])
        : "memory"
    );
    
    /* More complex addressing with shifting */
    asm volatile (
        ""
        :: "m" (outer->middle[(idx1 << 1) + idx2].inner[idx3].data[g_index1])
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(struct Outer* outer, int idx1, int idx2, int idx3, int value) {
    /* Complex output addressing */
    asm volatile (
        "mov %[val], %[addr]\n\t"
        : [addr] "=m" (outer->middle[idx1].inner[idx2].data[idx3])
        : [val] "r" (value)
        : "memory"
    );
    
    /* Output with computed index */
    asm volatile (
        ""
        : "=m" (outer->middle[g_index1].inner[g_index2].data[g_index3])
        : "r" (value), "r" (idx1), "r" (idx2)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
NOOPT void test_mixed_address(struct Outer* outer, int idx) {
    volatile int temp;
    
    /* Mixed input/output with address computations */
    asm volatile (
        "lea (%[base], %[idx], 4), %[addr1]\n\t"
        "mov (%[addr1]), %[temp]\n\t"
        "lea (%[base], %[idx], 8), %[addr2]\n\t"
        "mov %[temp], (%[addr2])\n\t"
        : [temp] "=r" (temp), [addr1] "=&r" (g_index1), [addr2] "=&r" (g_index2)
        : [base] "r" (&outer->base), [idx] "r" (idx)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OPERAND_ADDRESS */
NOOPT void test_operand_address(struct Outer* outer, int idx) {
    /* Force address computation before function call */
    void use_address(int* addr) {
        asm volatile ("" : : "r" (addr) : "memory");
    }
    
    /* Complex address expression as function argument */
    use_address(&outer->middle[idx].inner[idx % 4].data[(idx * 3) % 8]);
    
    /* Multiple complex addresses */
    use_address(&outer->middle[(idx + 1) % 3].inner[(idx * 2) % 4].data[g_index1]);
}

/* Function to create RELOAD_FOR_OPADDR_ADDR */
NOOPT void test_opaddr_addr(struct Outer* outer) {
    volatile int* addr1, *addr2;
    volatile int temp;
    
    /* Multiple address computations in sequence */
    asm volatile (
        "mov %[idx1], %%eax\n\t"
        "shl $4, %%eax\n\t"
        "add %[base], %%eax\n\t"
        "mov %%eax, %[addr1]\n\t"
        "mov %[idx2], %%ebx\n\t"
        "shl $3, %%ebx\n\t"
        "add %[base], %%ebx\n\t"
        "mov %%ebx, %[addr2]\n\t"
        : [addr1] "=m" (addr1), [addr2] "=m" (addr2)
        : [base] "r" (&outer->base), [idx1] "m" (g_index1), [idx2] "m" (g_index2)
        : "eax", "ebx", "memory"
    );
    
    /* Use computed addresses */
    asm volatile (
        "mov (%[addr1]), %[temp]\n\t"
        "add $1, %[temp]\n\t"
        "mov %[temp], (%[addr2])\n\t"
        : [temp] "=r" (temp)
        : [addr1] "r" (addr1), [addr2] "r" (addr2)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OTHER_ADDRESS */
NOOPT void test_other_address(struct Outer* outer) {
    volatile int temp;
    
    /* Complex addressing with multiple intermediate computations */
    asm volatile (
        "mov %[idx], %%eax\n\t"
        "imul $12, %%eax\n\t"
        "add %[base], %%eax\n\t"
        "add $8, %%eax\n\t"
        "mov (%%eax), %[temp]\n\t"
        : [temp] "=r" (temp)
        : [base] "r" (outer), [idx] "r" (g_index1)
        : "eax", "memory"
    );
    
    /* Another complex pattern */
    asm volatile (
        ""
        :: "m" (outer->middle[g_index1].inner[g_index2].data[g_index3]),
           "m" (outer->middle[g_index2].inner[g_index3].data[g_index1])
        : "memory"
    );
}

/* Function to create RELOAD_OTHER */
NOOPT void test_other_reload(struct Outer* outer) {
    volatile int temp1, temp2, temp3;
    
    /* Multiple memory operations with register pressure */
    asm volatile (
        "mov %[val1], %[temp1]\n\t"
        "mov %[val2], %[temp2]\n\t"
        "add %[temp1], %[temp2]\n\t"
        "mov %[temp2], %[val3]\n\t"
        : [temp1] "=&r" (temp1), [temp2] "=&r" (temp2), [temp3] "=m" (temp3)
        : [val1] "m" (outer->base), 
          [val2] "m" (outer->middle[0].offset),
          [val3] "m" (outer->middle[1].offset)
        : "memory"
    );
}

/* Main driver function */
int main() {
    struct Outer outer;
    int i, j, k;
    int checksum = 0;
    
    /* Initialize data */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 8; k++) {
                outer.middle[i].inner[j].data[k] = i * 100 + j * 10 + k;
            }
        }
        outer.middle[i].offset = i * 1000;
    }
    outer.base = 42;
    
    /* Set global indices */
    g_index1 = 1;
    g_index2 = 2;
    g_index3 = 3;
    
    /* Call test functions with various parameters to trigger different reload types */
    for (i = 0; i < 10; i++) {
        test_input_address(&outer, i % 3, (i + 1) % 4, (i + 2) % 8);
        test_output_address(&outer, i % 3, (i + 1) % 4, (i + 2) % 8, i * 10);
        
        if (i % 3 == 0) {
            test_mixed_address(&outer, i);
            test_operand_address(&outer, i);
        }
        
        if (i % 4 == 0) {
            test_opaddr_addr(&outer);
            test_other_address(&outer);
        }
        
        if (i % 5 == 0) {
            test_other_reload(&outer);
        }
        
        /* Update indices to create varying patterns */
        g_index1 = (g_index1 * 3 + 7) % 8;
        g_index2 = (g_index2 * 5 + 11) % 8;
        g_index3 = (g_index3 * 7 + 13) % 8;
    }
    
    /* Compute checksum to ensure values are used */
    for (i = 0; i < 3; i++) {
        checksum += outer.middle[i].offset;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 8; k++) {
                checksum += outer.middle[i].inner[j].data[k];
            }
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
