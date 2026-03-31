/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    struct {
        int x;
        int y[4];
    } nested;
} InnerStruct;

typedef struct {
    InnerStruct arrays[16];
    volatile int* volatile_ptr;
    int padding[3];
} OuterStruct;

/* Global variables to increase register pressure */
volatile int global_index = 0;
volatile int global_offset = 0;
OuterStruct global_struct;

/* Function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(OuterStruct *os, int idx1, int idx2, int idx3) {
    /* Complex addressing that requires input address reload */
    int result;
    
    /* Force address computation with multiple registers */
    asm volatile(
        "movl (%[base], %[idx1], 4), %[res]\n\t"
        : [res] "=r" (result)
        : [base] "r" (&os->arrays[idx2].nested.y[idx3]),
          [idx1] "r" (idx1)
        : "memory"
    );
    
    /* More complex addressing with shift */
    asm volatile(
        ""
        : 
        : "m" (os->arrays[(idx1 << 1) + idx2].data[idx3]),
          "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    use(&result);
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct *os, int idx1, int idx2, int idx3) {
    /* Complex addressing for output */
    int temp = idx1 + idx2 + idx3;
    
    /* Output to memory with complex address */
    asm volatile(
        "movl %[val], (%[base], %[idx], 4)\n\t"
        : "=m" (os->arrays[idx2].nested.y[idx1 + idx3])
        : [val] "r" (temp),
          [base] "r" (os->arrays),
          [idx] "r" (idx1)
        : "memory"
    );
    
    /* Another output with different addressing */
    asm volatile(
        ""
        : "=m" (os->arrays[idx1].data[(idx2 << 2) + idx3])
        : "r" (temp), "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and mixed types */
void test_inpaddr_address(OuterStruct *os, int idx1, int idx2) {
    /* Mixed input/output with address computations */
    int input_val, output_val;
    
    /* Input with complex address, output to simple location */
    asm volatile(
        "movl (%[in_addr]), %[tmp]\n\t"
        "addl $1, %[tmp]\n\t"
        "movl %[tmp], %[out]\n\t"
        : [out] "=m" (os->arrays[idx1].data[0]),
          [tmp] "=&r" (output_val)
        : [in_addr] "r" (&os->arrays[idx2].nested.y[idx1]),
          "m" (os->arrays[idx2].nested.y[idx1])
        : "memory"
    );
    
    /* Chain of dependencies */
    int idx3 = idx1 + idx2;
    asm volatile(
        ""
        : "=m" (os->arrays[idx3].data[global_index])
        : "m" (os->arrays[idx1].nested.y[idx2]),
          "r" (idx3), "r" (global_index)
        : "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(OuterStruct *os, int idx) {
    /* Force address computation before function-like asm */
    int (*func_ptr)(int) = (int (*)(int))&use;
    
    /* Complex address as operand */
    asm volatile(
        "call *%[func]\n\t"
        : 
        : [func] "r" (func_ptr),
          "D" (&os->arrays[idx].nested.y[global_index * 2])  /* RDI on x86 */
        : "memory", "rax", "rcx", "rdx", "rsi", "r8", "r9", "r10", "r11"
    );
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(OuterStruct *os, int idx1, int idx2, int idx3) {
    /* Multiple interdependent asm statements */
    int temp1, temp2, temp3;
    
    /* First asm computes address used in second */
    asm volatile(
        "lea (%[base], %[idx1], 8), %[t1]\n\t"
        : [t1] "=r" (temp1)
        : [base] "r" (os->arrays),
          [idx1] "r" (idx1)
        : "memory"
    );
    
    /* Second uses that address and produces another */
    asm volatile(
        "movl (%[addr], %[idx2], 4), %[t2]\n\t"
        "lea (%[t2], %[idx3], 2), %[t3]\n\t"
        : [t2] "=r" (temp2),
          [t3] "=r" (temp3)
        : [addr] "r" (temp1),
          [idx2] "r" (idx2),
          [idx3] "r" (idx3)
        : "memory"
    );
    
    /* Third uses both computed values */
    asm volatile(
        "addl %[t2], %[t3]\n\t"
        "movl %[t3], (%[base], %[idx1], 4)\n\t"
        : "=m" (os->arrays[idx2].data[idx3])
        : [t2] "r" (temp2),
          [t3] "r" (temp3),
          [base] "r" (os->arrays),
          [idx1] "r" (idx1)
        : "memory"
    );
}

/* Test RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(OuterStruct *os, int idx1, int idx2) {
    /* Output address computation */
    volatile int* addr;
    
    /* Compute output address in register */
    asm volatile(
        "lea (%[base], %[idx1], 8), %[addr]\n\t"
        : [addr] "=r" (addr)
        : [base] "r" (&os->arrays[idx2].data[0]),
          [idx1] "r" (idx1)
        : "memory"
    );
    
    /* Use computed address for output */
    int value = idx1 * idx2;
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : "=m" (*addr)
        : [val] "r" (value),
          [addr] "r" (addr)
        : "memory"
    );
}

/* Test RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(OuterStruct *os, int idx1, int idx2, int idx3) {
    /* Complex operand address with multiple computations */
    int* addr1;
    int* addr2;
    
    /* Compute two different addresses */
    asm volatile(
        "lea (%[base1], %[idx1], 4), %[a1]\n\t"
        "lea (%[base2], %[idx2], 4), %[a2]\n\t"
        : [a1] "=r" (addr1),
          [a2] "=r" (addr2)
        : [base1] "r" (&os->arrays[idx3].data[0]),
          [base2] "r" (&os->arrays[idx1].nested.y[0]),
          [idx1] "r" (idx2),
          [idx2] "r" (idx3)
        : "memory"
    );
    
    /* Use both addresses in operation */
    int result;
    asm volatile(
        "movl (%[a1]), %[res]\n\t"
        "addl (%[a2]), %[res]\n\t"
        : [res] "=r" (result)
        : [a1] "r" (addr1),
          [a2] "r" (addr2)
        : "memory"
    );
    
    /* Store result using complex address */
    asm volatile(
        "movl %[res], (%[base], %[idx], 4)\n\t"
        : "=m" (os->arrays[idx2].data[idx3])
        : [res] "r" (result),
          [base] "r" (os->arrays),
          [idx] "r" (idx1)
        : "memory"
    );
}

/* Main driver that exercises all test functions */
int main() {
    /* Initialize test structure */
    OuterStruct os;
    int i, j, k;
    
    /* Initialize with some data */
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            os.arrays[i].data[j] = i * 100 + j;
        }
        for (j = 0; j < 4; j++) {
            os.arrays[i].nested.y[j] = i * 50 + j;
        }
        os.arrays[i].nested.x = i * 10;
    }
    
    /* Create varying indices to prevent constant propagation */
    volatile int idx1 = 3;
    volatile int idx2 = 7;
    volatile int idx3 = 11;
    volatile int idx4 = 5;
    volatile int idx5 = 9;
    
    /* Exercise all test functions with different parameters */
    test_input_address(&os, idx1, idx2, idx3);
    test_output_address(&os, idx2, idx3, idx4);
    test_inpaddr_address(&os, idx3, idx4);
    test_operand_address(&os, idx4);
    test_other_address(&os, idx1, idx3, idx5);
    test_outaddr_address(&os, idx2, idx4);
    test_opaddr_addr(&os, idx1, idx4, idx5);
    
    /* Mix in a loop with varying indices to increase pressure */
    int sum = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 2; k++) {
                test_input_address(&os, i, j, k);
                test_output_address(&os, j, k, i);
                sum += os.arrays[i].data[j] + os.arrays[j].nested.y[k];
            }
        }
    }
    
    /* Final computation to use results and prevent dead code elimination */
    printf("Result checksum: %d\n", sum);
    
    return 0;
}
