/* reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing reload_coverage.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to create addressing complexity */
struct Inner {
    int data[8];
    volatile int* volatile_ptr;
};

struct Outer {
    struct Inner arrays[4];
    int offsets[16];
    volatile int index_reg;
};

/* Global variables to prevent optimization */
volatile int g_index1 = 0;
volatile int g_index2 = 0;
volatile struct Outer* g_outer_ptr = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer* outer, int idx1, int idx2) {
    /* Complex addressing that requires input address reload */
    int val;
    
    /* Force register pressure */
    register int r1 asm("r10") = idx1;
    register int r2 asm("r11") = idx2;
    register int r3 asm("r12") = outer->index_reg;
    
    /* Complex addressing mode that likely needs reload */
    asm volatile (
        "movl (%[base], %[idx1], 4), %[val]\n\t"
        : [val] "=r" (val)
        : [base] "r" (&outer->arrays[0].data[0]),
          [idx1] "r" ((r1 + r2) * 2 + r3)
        : "memory"
    );
    
    /* Use the value to prevent optimization */
    outer->offsets[0] = val;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer* outer, int idx1, int idx2, int value) {
    /* Complex addressing for output */
    register int r1 asm("r10") = idx1;
    register int r2 asm("r11") = idx2;
    register int r3 asm("r12") = outer->index_reg;
    
    /* Output to complex address */
    asm volatile (
        "movl %[val], (%[base], %[idx1], 4)\n\t"
        : 
        : [base] "r" (&outer->arrays[1].data[0]),
          [idx1] "r" ((r1 * 3 + r2) << 2),
          [val] "r" (value + r3)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT mixing */
void test_mixed_io(struct Outer* outer, int idx) {
    /* Mixed input/output with complex addressing */
    int temp;
    
    /* Input from one complex address */
    asm volatile (
        "movl (%[addr1]), %[temp]\n\t"
        : [temp] "=r" (temp)
        : [addr1] "r" (&outer->arrays[idx % 4].data[(idx * 7) % 8])
        : "memory"
    );
    
    /* Output to another complex address */
    asm volatile (
        "movl %[temp], (%[addr2])\n\t"
        : 
        : [addr2] "r" (&outer->offsets[(idx * 3) % 16]),
          [temp] "r" (temp + 1)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct Outer* outer, int idx) {
    /* Force operand address computation before function call */
    void use_address(int* addr) {
        *addr += 1;
    }
    
    /* Complex address expression */
    use_address(&outer->arrays[idx % 4].data[(idx * 5) % 8]);
    
    /* Another complex address */
    volatile int* ptr = &outer->offsets[(idx * 11) % 16];
    *ptr = idx;
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct Outer* outer) {
    /* Create register pressure */
    register int i asm("r10") = g_index1;
    register int j asm("r11") = g_index2;
    register int k asm("r12") = outer->index_reg;
    
    int result;
    
    /* Complex addressing with multiple computations */
    asm volatile (
        "leal (%[i], %[j], 2), %%eax\n\t"
        "addl %[k], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (result)
        : [i] "r" (i), [j] "r" (j), [k] "r" (k),
          "m" (outer->arrays[0].data[0])
        : "eax", "ebx", "memory"
    );
    
    /* Output with different complex address */
    asm volatile (
        "leal (%[i], %[j], 4), %%eax\n\t"
        "subl %[k], %%eax\n\t"
        "movl %[val], (%%eax)\n\t"
        : 
        : [i] "r" (i), [j] "r" (j), [k] "r" (k),
          [val] "r" (result),
          "m" (outer->offsets[0])
        : "eax", "memory"
    );
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct Outer* outer) {
    /* Multiple memory accesses with different addressing modes */
    volatile int* ptr1 = &outer->arrays[g_index1 % 4].data[g_index2 % 8];
    volatile int* ptr2 = &outer->offsets[(g_index1 + g_index2) % 16];
    
    /* Force address computations to stay separate */
    asm volatile (
        "movl (%0), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%1)\n\t"
        : 
        : "r" (ptr1), "r" (ptr2)
        : "eax", "memory"
    );
    
    /* Another complex access pattern */
    int idx = outer->index_reg;
    outer->arrays[idx % 4].data[(idx * 13) % 8] = 
        outer->offsets[(idx * 17) % 16];
}

/* Test function for RELOAD_OTHER */
void test_reload_other(struct Outer* outer) {
    /* Multiple constraints that don't fit clean categories */
    register int a asm("r10") = g_index1;
    register int b asm("r11") = g_index2;
    register int c asm("r12") = outer->index_reg;
    
    int temp1, temp2;
    
    /* Complex asm with multiple memory references */
    asm volatile (
        "movl (%[base1], %[a], 4), %[t1]\n\t"
        "imull %[c], %[t1]\n\t"
        "movl %[t1], (%[base2], %[b], 4)\n\t"
        "movl (%[base3]), %[t2]\n\t"
        "addl %[t1], %[t2]\n\t"
        : [t1] "=&r" (temp1), [t2] "=&r" (temp2)
        : [base1] "r" (&outer->arrays[0].data[0]),
          [base2] "r" (&outer->offsets[0]),
          [base3] "r" (&outer->arrays[1].data[0]),
          [a] "r" (a), [b] "r" (b), [c] "r" (c)
        : "memory"
    );
}

/* Main driver function */
int main() {
    /* Allocate and initialize test structure */
    struct Outer* outer = (struct Outer*)malloc(sizeof(struct Outer));
    if (!outer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            outer->arrays[i].data[j] = i * 100 + j;
        }
        outer->arrays[i].volatile_ptr = &outer->index_reg;
    }
    
    for (int i = 0; i < 16; i++) {
        outer->offsets[i] = i * 10;
    }
    
    outer->index_reg = 42;
    g_outer_ptr = outer;
    
    /* Run tests with various parameters to trigger different reload types */
    int checksum = 0;
    
    for (int i = 0; i < 100; i++) {
        g_index1 = i;
        g_index2 = i * 2;
        
        test_input_address(outer, i, i * 3);
        test_output_address(outer, i, i * 5, i * 7);
        
        if (i % 3 == 0) {
            test_mixed_io(outer, i);
        }
        
        if (i % 5 == 0) {
            test_operand_address(outer, i);
        }
        
        if (i % 7 == 0) {
            test_inpaddr_outaddr(outer);
        }
        
        if (i % 11 == 0) {
            test_other_address(outer);
        }
        
        if (i % 13 == 0) {
            test_reload_other(outer);
        }
        
        /* Update checksum to prevent optimization */
        checksum += outer->arrays[i % 4].data[i % 8];
        checksum += outer->offsets[i % 16];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully\n");
    
    free(outer);
    return 0;
}
