/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force complex addressing */
typedef struct {
    int data[8];
    int* ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    long long big_array[16];
} OuterStruct;

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_ptr = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(OuterStruct* os, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    asm volatile(
        "/* Input address computation */\n\t"
        "addl %[idx2], %[idx1]\n\t"
        "movl (%[base],%[idx1],4), %[res]\n\t"
        : [res] "=r" (result)
        : [base] "r" (&os->inner[idx3].data[0]),
          [idx1] "r" (idx1),
          [idx2] "r" (idx2)
        : "memory"
    );
    
    /* More complex input addressing with shift */
    int idx4 = idx1 << 2;
    asm volatile(
        "movl (%[base],%[idx],4), %0\n\t"
        : "=r" (result)
        : [base] "r" (os->inner[0].data),
          [idx] "r" (idx4)
        : "memory"
    );
    
    return result;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct* os, int* indices, int count) {
    /* Complex output addressing */
    for (int i = 0; i < count; i++) {
        int offset = indices[i] * 3;
        asm volatile(
            "/* Output address computation */\n\t"
            "movl $42, (%[base],%[offset],4)\n\t"
            : 
            : [base] "r" (os->big_array),
              [offset] "r" (offset)
            : "memory"
        );
    }
    
    /* Output address with structure member */
    int complex_offset = os->inner[0].offset;
    asm volatile(
        "movq $99, (%[base],%[offset],8)\n\t"
        :
        : [base] "r" (os->big_array),
          [offset] "r" (complex_offset)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(OuterStruct* os, int idx) {
    int temp;
    
    /* Mixed input/output with address computations */
    asm volatile(
        "/* Mixed input/output addressing */\n\t"
        "leaq (%[base],%[idx],8), %[addr]\n\t"
        "movl (%[addr]), %[temp]\n\t"
        "addl $1, %[temp]\n\t"
        "movl %[temp], 4(%[addr])\n\t"
        : [temp] "=&r" (temp), [addr] "=&r" (temp)
        : [base] "r" (os->big_array),
          [idx] "r" (idx)
        : "memory"
    );
    
    /* Another complex case */
    int* ptr = os->inner[idx].ptr;
    int offset = os->inner[idx].offset;
    
    asm volatile(
        "movl (%[ptr],%[offset],4), %0\n\t"
        "movl %0, 8(%[ptr],%[offset],4)\n\t"
        : "=&r" (temp)
        : [ptr] "r" (ptr),
          [offset] "r" (offset)
        : "memory"
    );
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(OuterStruct* os, int i, int j, int k) {
    /* Function call with complex address operand */
    helper_function(&os->inner[i].data[j * 2 + k]);
    
    /* Inline asm with operand address */
    int* addr = &os->matrix[i][j];
    asm volatile(
        "call *%[func]\n\t"
        :
        : [func] "r" ((void*)helper_function),
          "r" (addr)
        : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
}

/* Helper function for operand address test */
void helper_function(int* ptr) {
    *ptr = *ptr + 1;
}

/* Test function for RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
int test_other_address(OuterStruct* os, int* indices, int n) {
    int sum = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < n; i++) {
        int idx1 = indices[i];
        int idx2 = indices[(i + 1) % n];
        
        /* Multiple memory accesses with different addressing */
        asm volatile(
            "/* Multiple address computations */\n\t"
            "movl (%[base1],%[idx1],4), %%eax\n\t"
            "addl (%[base2],%[idx2],4), %%eax\n\t"
            "movl %%eax, (%[base3],%[idx1],4)\n\t"
            : 
            : [base1] "r" (os->inner[0].data),
              [base2] "r" (os->inner[1].data),
              [base3] "r" (os->inner[2].data),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2)
            : "memory", "eax"
        );
        
        sum += os->inner[2].data[idx1];
    }
    
    /* RELOAD_OTHER case */
    asm volatile(
        "/* Other reload type */\n\t"
        "movl $0, %%eax\n\t"
        "cpuid\n\t"
        : 
        : 
        : "memory", "eax", "ebx", "ecx", "edx"
    );
    
    return sum;
}

/* Main test driver */
int main() {
    /* Initialize test data */
    OuterStruct os;
    int indices[] = {0, 2, 1, 3, 0, 2, 1, 3};
    int n = sizeof(indices) / sizeof(indices[0]);
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].data[j] = i * 10 + j;
        }
        os.inner[i].ptr = &os.inner[(i + 1) % 4].data[0];
        os.inner[i].offset = i * 2;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            os.matrix[i][j] = i * 4 + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        os.big_array[i] = i * 100;
    }
    
    /* Run tests to trigger different reload types */
    int result1 = test_input_address(&os, 1, 2, 0);
    test_output_address(&os, indices, n);
    test_inpaddr_outaddr(&os, 2);
    test_operand_address(&os, 1, 2, 1);
    int result5 = test_other_address(&os, indices, n);
    
    /* Compute checksum to prevent optimization */
    int checksum = result1 + result5;
    for (int i = 0; i < 4; i++) {
        checksum += os.inner[i].data[0];
        checksum += (int)os.big_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
