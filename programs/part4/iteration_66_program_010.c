/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int base;
    int offset;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
volatile struct Outer* volatile global_struct = NULL;

/* Function to trigger RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer* s, int idx1, int idx2) {
    /* Complex addressing: s->arrays[idx1].data[idx2] */
    /* This requires computing the address of the array element */
    int result;
    
    /* Inline asm with memory input operand using complex addressing */
    asm volatile (
        "movl %[mem], %[res]\n\t"
        : [res] "=r" (result)
        : [mem] "m" (s->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Use result to prevent optimization */
    global_index += result;
}

/* Function to trigger RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer* s, int idx1, int idx2, int value) {
    /* Complex addressing for output */
    /* This requires computing the address before storing */
    
    /* Inline asm with memory output operand using complex addressing */
    asm volatile (
        "movl %[val], %[mem]\n\t"
        : [mem] "=m" (s->arrays[idx1].data[idx2])
        : [val] "r" (value)
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_INPADDR_ADDRESS */
void test_inpaddr_address(struct Outer* s, int idx) {
    /* Taking address of a complex memory location as input */
    int* addr;
    
    /* Compute address of nested array element */
    addr = &s->arrays[idx].data[global_index];
    
    /* Use inline asm that takes the computed address as input */
    asm volatile (
        "addl $1, (%[addr])\n\t"
        : 
        : [addr] "r" (addr)
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(struct Outer* s, int idx) {
    /* Taking address of a complex memory location for output */
    int* addr;
    
    /* Compute address with shifting */
    addr = &s->arrays[idx].data[global_index << 2];
    
    /* Use inline asm that modifies through pointer */
    asm volatile (
        "subl $1, (%[addr])\n\t"
        : 
        : [addr] "r" (addr)
        : "memory"
    );
}

/* Function to trigger RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct Outer* s, int idx1, int idx2) {
    /* Passing complex address to external function */
    /* This forces address computation before call */
    extern void dummy_func(int*);
    
    /* Complex address computation */
    int* complex_addr = &s->arrays[idx1].data[idx2 + global_offset];
    
    /* Prevent inlining to force address computation before call */
    asm volatile ("" : : "r"(complex_addr) : "memory");
    
    /* The address computation happens before this call */
    dummy_func(complex_addr);
}

/* Dummy function to prevent optimization */
void dummy_func(int* p) {
    if (p) *p += 1;
}

/* Function to trigger RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct Outer* s) {
    /* Mixed input/output with complex addressing in loop */
    int i;
    
    for (i = 0; i < 4; i++) {
        int temp;
        
        /* Read from one complex address */
        asm volatile (
            "movl %[in], %[temp]\n\t"
            : [temp] "=r" (temp)
            : [in] "m" (s->arrays[i].data[global_index])
            : "memory"
        );
        
        /* Write to another complex address */
        asm volatile (
            "movl %[temp], %[out]\n\t"
            : [out] "=m" (s->arrays[(i + 1) % 4].data[global_offset])
            : [temp] "r" (temp + 1)
            : "memory"
        );
    }
}

/* Function to trigger RELOAD_OTHER */
void test_other_reload(struct Outer* s, int idx) {
    /* Multiple memory operands with register constraints */
    int reg1, reg2;
    
    /* Force register allocation conflicts */
    asm volatile (
        "movl %[in1], %[r1]\n\t"
        "movl %[in2], %[r2]\n\t"
        "addl %[r1], %[r2]\n\t"
        "movl %[r2], %[out]\n\t"
        : [r1] "=&r" (reg1), [r2] "=&r" (reg2), [out] "=m" (s->base)
        : [in1] "m" (s->arrays[idx].data[0]),
          [in2] "m" (s->arrays[idx].data[1])
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(struct Outer* s, int iterations) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Vary indices to prevent optimization */
        int idx1 = (i * 3) % 4;
        int idx2 = (i * 5) % 8;
        int idx3 = (i * 7) % 4;
        
        /* Mix different reload types in sequence */
        test_input_address(s, idx1, idx2);
        test_output_address(s, idx2, idx3, i);
        test_inpaddr_address(s, idx3);
        
        /* Update global indices to change addressing */
        global_index = (global_index + 1) % 8;
        global_offset = (global_offset + 2) % 8;
    }
}

/* Main driver function */
int main() {
    int i;
    struct Outer test_struct;
    
    /* Initialize test data */
    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 8; j++) {
            test_struct.arrays[i].data[j] = i * 10 + j;
        }
        test_struct.arrays[i].extra = i * 100;
    }
    test_struct.base = 999;
    test_struct.offset = 5;
    
    global_struct = &test_struct;
    
    printf("Starting reload coverage test...\n");
    
    /* Execute tests to trigger different reload types */
    
    /* Test 1: Input address reloads */
    printf("Testing input address reloads...\n");
    for (i = 0; i < 10; i++) {
        test_input_address(&test_struct, i % 4, (i * 2) % 8);
    }
    
    /* Test 2: Output address reloads */
    printf("Testing output address reloads...\n");
    for (i = 0; i < 10; i++) {
        test_output_address(&test_struct, i % 4, (i * 3) % 8, i * 100);
    }
    
    /* Test 3: Input address of address */
    printf("Testing inpaddr address reloads...\n");
    for (i = 0; i < 8; i++) {
        test_inpaddr_address(&test_struct, i % 4);
    }
    
    /* Test 4: Output address of address */
    printf("Testing outaddr address reloads...\n");
    for (i = 0; i < 8; i++) {
        test_outaddr_address(&test_struct, i % 4);
    }
    
    /* Test 5: Operand address */
    printf("Testing operand address reloads...\n");
    for (i = 0; i < 5; i++) {
        test_operand_address(&test_struct, i % 4, (i * 3) % 8);
    }
    
    /* Test 6: Other address */
    printf("Testing other address reloads...\n");
    test_other_address(&test_struct);
    
    /* Test 7: Other reloads */
    printf("Testing other reloads...\n");
    for (i = 0; i < 4; i++) {
        test_other_reload(&test_struct, i);
    }
    
    /* Test 8: Mixed reloads */
    printf("Testing mixed reloads...\n");
    test_mixed_reloads(&test_struct, 20);
    
    /* Compute checksum to ensure all operations executed */
    int checksum = 0;
    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 8; j++) {
            checksum += test_struct.arrays[i].data[j];
        }
        checksum += test_struct.arrays[i].extra;
    }
    checksum += test_struct.base + test_struct.offset;
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
