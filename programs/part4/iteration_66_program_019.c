/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of critical variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex nested structure to force address computations */
struct Inner {
    int data[8];
    volatile int* volatile_ptr;
};

struct Outer {
    struct Inner arrays[4];
    int offsets[16];
    volatile int index_reg;
};

/* Global variables to increase register pressure */
volatile int global_index1, global_index2;
struct Outer* volatile global_struct;

/* Function to trigger RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(struct Outer* s, int idx1, int idx2) {
    /* Complex addressing: s->arrays[idx1].data[idx2] */
    /* Force address computation into reloads */
    int val;
    
    /* Inline asm with memory input operand using complex address */
    asm volatile (
        "movl %[mem], %[val]\n\t"
        : [val] "=r" (val)
        : [mem] "m" (s->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Use the value to prevent optimization */
    s->arrays[0].data[0] = val;
}

/* Function to trigger RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(struct Outer* s, int base, int offset) {
    /* Complex output addressing: s->offsets[(base << 2) + offset] */
    int temp = s->index_reg;
    
    /* Inline asm with memory output operand at complex address */
    asm volatile (
        "movl %[temp], %[mem]\n\t"
        : [mem] "=m" (s->offsets[(base << 2) + offset])
        : [temp] "r" (temp)
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_INPADDR_ADDRESS */
NOOPT void test_inpaddr_address(struct Outer* s, int i, int j, int k) {
    /* Nested addressing with pointer arithmetic */
    volatile int* ptr = &s->arrays[i].data[j];
    
    /* Address computation that needs reloading */
    asm volatile (
        "addl $1, %[ptr]\n\t"
        "movl (%[ptr]), %%eax\n\t"
        : 
        : [ptr] "r" (ptr), "m" (*ptr)
        : "eax", "memory"
    );
}

/* Function to trigger RELOAD_FOR_OUTADDR_ADDRESS */
NOOPT void test_outaddr_address(struct Outer* s, int idx) {
    /* Output address with complex computation */
    struct Inner* inner_ptr = &s->arrays[idx];
    
    /* Force address computation before store */
    asm volatile (
        "movl $0x1234, 4(%[ptr])\n\t"
        : 
        : [ptr] "r" (inner_ptr)
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_OPERAND_ADDRESS */
NOOPT void test_operand_address(struct Outer* s, int a, int b, int c) {
    /* Multiple complex address computations in one expression */
    int* addr1 = &s->arrays[a].data[b];
    int* addr2 = &s->offsets[c];
    
    /* Mixed input/output with different addressing */
    asm volatile (
        "movl (%[addr1]), %%eax\n\t"
        "addl %%eax, (%[addr2])\n\t"
        : 
        : [addr1] "r" (addr1), [addr2] "r" (addr2)
        : "eax", "memory"
    );
}

/* Function to trigger RELOAD_FOR_OTHER_ADDRESS */
NOOPT void test_other_address(struct Outer* s, int x, int y, int z) {
    /* Complex addressing with multiple index registers */
    int complex_idx = (x * y) + z;
    
    /* Force spill of address computation registers */
    asm volatile (
        "leal (%[idx1],%[idx2],4), %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %[out]\n\t"
        : [out] "=m" (s->arrays[0].data[0])
        : [idx1] "r" (&s->arrays[0]), [idx2] "r" (complex_idx)
        : "eax", "ebx", "memory"
    );
}

/* Function to trigger RELOAD_OTHER */
NOOPT void test_other_reload(struct Outer* s, int p1, int p2, int p3) {
    /* Multiple memory operands with register pressure */
    register int r1 asm("ebx") = p1;
    register int r2 asm("ecx") = p2;
    register int r3 asm("edx") = p3;
    
    /* Complex asm with many operands */
    asm volatile (
        "imull %[r2], %[r1]\n\t"
        "addl %[r3], %[r1]\n\t"
        "movl %[r1], %[mem1]\n\t"
        "movl %[r2], %[mem2]\n\t"
        : [mem1] "=m" (s->offsets[0]),
          [mem2] "=m" (s->offsets[1])
        : [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3)
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
NOOPT void test_mixed_reloads(struct Outer* s) {
    volatile int idx = s->index_reg;
    
    /* Chain of operations requiring different reload types */
    for (int i = 0; i < 4; i++) {
        /* Input address reload */
        int val;
        asm volatile (
            "movl %[in], %[val]\n\t"
            : [val] "=r" (val)
            : [in] "m" (s->arrays[i].data[idx])
            : "memory"
        );
        
        /* Output address reload */
        int out_idx = (i * 3) + (idx & 7);
        asm volatile (
            "movl %[val], %[out]\n\t"
            : [out] "=m" (s->offsets[out_idx])
            : [val] "r" (val)
            : "memory"
        );
        
        /* Operand address reload */
        int* ptr = &s->arrays[i].data[0];
        asm volatile (
            "addl $1, (%[ptr])\n\t"
            : 
            : [ptr] "r" (ptr)
            : "memory"
        );
    }
}

/* Main driver function */
int main() {
    /* Allocate and initialize test structure */
    struct Outer* s = (struct Outer*)malloc(sizeof(struct Outer));
    if (!s) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            s->arrays[i].data[j] = i * 100 + j;
        }
        s->arrays[i].volatile_ptr = &s->index_reg;
    }
    
    for (int i = 0; i < 16; i++) {
        s->offsets[i] = i * 10;
    }
    
    s->index_reg = 3;
    global_struct = s;
    
    /* Call test functions with various parameters to trigger different reloads */
    test_input_address(s, 1, 2);
    test_output_address(s, 2, 1);
    test_inpaddr_address(s, 0, 3, 0);
    test_outaddr_address(s, 2);
    test_operand_address(s, 1, 2, 3);
    test_other_address(s, 1, 2, 3);
    test_other_reload(s, 10, 20, 30);
    test_mixed_reloads(s);
    
    /* Compute checksum to ensure all operations executed */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += s->arrays[i].data[j];
        }
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += s->offsets[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(s);
    return 0;
}
