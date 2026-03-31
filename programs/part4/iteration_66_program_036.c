/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test_reload_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int base;
    long offset;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
volatile struct Outer* volatile global_struct_ptr = NULL;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer* outer, int idx1, int idx2) {
    /* Complex addressing: outer->arrays[idx1].data[idx2] */
    int val;
    
    /* Inline asm with memory input constraint and complex address */
    asm volatile (
        "movl %[mem], %[val]\n\t"
        : [val] "=r" (val)
        : [mem] "m" (outer->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Use the value to prevent optimization */
    global_index = val;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer* outer, int idx1, int idx2, int value) {
    /* Complex addressing for output */
    asm volatile (
        "movl %[val], %[mem]\n\t"
        : [mem] "=m" (outer->arrays[idx1].data[idx2])
        : [val] "r" (value)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS together */
void test_mixed_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    int temp;
    
    /* Read from one complex address */
    asm volatile (
        "movl %[in_mem], %[temp]\n\t"
        : [temp] "=r" (temp)
        : [in_mem] "m" (outer->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Write to another complex address */
    asm volatile (
        "movl %[temp], %[out_mem]\n\t"
        : [out_mem] "=m" (outer->arrays[idx3].data[idx1])
        : [temp] "r" (temp + 1)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS */
void test_inpaddr_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    /* Even more complex addressing with pointer arithmetic */
    struct Inner* inner_ptr = &outer->arrays[idx1];
    
    /* Use the pointer in addressing with offset */
    asm volatile (
        "movl (%[ptr], %[idx2], 4), %[result]\n\t"
        : [result] "=r" (global_offset)
        : [ptr] "r" (&inner_ptr->data[0]),
          [idx2] "r" (idx2)
        : "memory"
    );
    
    /* Another complex access using the result */
    asm volatile (
        "addl %[offset], %[mem]\n\t"
        : [mem] "+m" (outer->arrays[idx3].extra)
        : [offset] "r" (global_offset)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    /* Complex output addressing with computed base */
    int* base_ptr = outer->arrays[idx1].data;
    int offset = idx2 * sizeof(int);
    
    asm volatile (
        "movl %[val], (%[base], %[offset], 1)\n\t"
        : 
        : [base] "r" (base_ptr),
          [offset] "r" (offset),
          [val] "r" (idx3)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void helper_function(int* addr) {
    /* Force address computation before call */
    *addr += 1;
}

void test_operand_address(struct Outer* outer, int idx1, int idx2) {
    /* Complex address passed as function argument */
    helper_function(&outer->arrays[idx1].data[idx2]);
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(struct Outer* outer, int idx1, int idx2, int idx3) {
    /* Multiple complex addresses in different contexts */
    int* ptr1 = &outer->arrays[idx1].data[idx2];
    int* ptr2 = &outer->arrays[idx3].data[idx1];
    
    /* Use both addresses in operations */
    asm volatile (
        "movl (%[ptr1]), %%eax\n\t"
        "addl %%eax, (%[ptr2])\n\t"
        : 
        : [ptr1] "r" (ptr1),
          [ptr2] "r" (ptr2)
        : "eax", "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct Outer* outer, int idx1, int idx2, int idx3, int idx4) {
    /* Multiple levels of indirection and computation */
    struct Inner* inner1 = &outer->arrays[idx1];
    struct Inner* inner2 = &outer->arrays[idx2];
    
    /* Complex addressing across multiple structures */
    int val1, val2;
    
    asm volatile (
        "movl (%[base1], %[idx3], 4), %[val1]\n\t"
        "movl (%[base2], %[idx4], 4), %[val2]\n\t"
        "addl %[val1], %[val2]\n\t"
        : [val1] "=r" (val1),
          [val2] "=r" (val2)
        : [base1] "r" (&inner1->data[0]),
          [idx3] "r" (idx3),
          [base2] "r" (&inner2->data[0]),
          [idx4] "r" (idx4)
        : "memory"
    );
    
    /* Store result in yet another complex location */
    outer->arrays[idx3].data[idx4] = val1 + val2;
}

/* Function to force RELOAD_OTHER */
void test_other_reload(struct Outer* outer, int idx1, int idx2) {
    /* Multiple memory operations with register pressure */
    register int r1 asm("r10");
    register int r2 asm("r11");
    register int r3 asm("r12");
    register int r4 asm("r13");
    
    /* Force many values into registers */
    r1 = outer->base;
    r2 = outer->offset;
    r3 = idx1;
    r4 = idx2;
    
    /* Complex asm with many constraints */
    asm volatile (
        "leal (%[r1], %[r2]), %%eax\n\t"
        "addl %[r3], %%eax\n\t"
        "subl %[r4], %%eax\n\t"
        "movl %%eax, %[mem]\n\t"
        : [mem] "=m" (outer->arrays[0].extra)
        : [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3),
          [r4] "r" (r4)
        : "eax", "memory"
    );
}

/* Main driver function */
int main() {
    /* Allocate and initialize test structure */
    struct Outer* test_struct = (struct Outer*)malloc(sizeof(struct Outer));
    if (!test_struct) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with test data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            test_struct->arrays[i].data[j] = i * 100 + j;
        }
        test_struct->arrays[i].extra = i * 1000;
    }
    test_struct->base = 42;
    test_struct->offset = 17;
    
    global_struct_ptr = test_struct;
    
    int checksum = 0;
    
    /* Test each reload type */
    test_input_address(test_struct, 1, 2);
    checksum += global_index;
    
    test_output_address(test_struct, 2, 3, 999);
    checksum += test_struct->arrays[2].data[3];
    
    test_mixed_address(test_struct, 0, 1, 2);
    checksum += test_struct->arrays[2].data[0];
    
    test_inpaddr_address(test_struct, 1, 2, 3);
    checksum += global_offset;
    
    test_outaddr_address(test_struct, 2, 3, 777);
    checksum += test_struct->arrays[2].data[3];
    
    test_operand_address(test_struct, 3, 4);
    checksum += test_struct->arrays[3].data[4];
    
    test_opaddr_addr(test_struct, 0, 5, 1);
    checksum += test_struct->arrays[1].data[0];
    
    test_other_address(test_struct, 0, 1, 2, 3);
    checksum += test_struct->arrays[2].data[3];
    
    test_other_reload(test_struct, 7, 3);
    checksum += test_struct->arrays[0].extra;
    
    /* Print checksum to ensure all code executes */
    printf("Final checksum: %d\n", checksum);
    
    free(test_struct);
    return 0;
}
