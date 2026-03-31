/* reload_coverage.c - Test program to trigger specific reload types in GCC */

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
    int matrix[3][3];
    volatile int* volatile_index;
} OuterStruct;

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_ptr = NULL;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(OuterStruct* os, int idx1, int idx2, int idx3) {
    /* Complex addressing that requires multiple reloads */
    int val;
    
    /* Force input address reload with complex computation */
    asm volatile (
        "movl %[input], %[output]\n\t"
        : [output] "=r" (val)
        : [input] "m" (os->inner[idx1].data[(idx2 << 2) + idx3 + os->inner[idx1].offset])
        : "memory"
    );
    
    /* Another complex input address */
    asm volatile (
        ""
        :
        : "m" (os->matrix[idx1][idx2 + global_index]),
          "m" (os->inner[(idx1 + idx2) % 4].ptr[idx3])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct* os, int* arr, int idx1, int idx2) {
    /* Complex output addressing */
    int temp = idx1 * idx2;
    
    /* Force output address reload */
    asm volatile (
        "movl %[val], %[output]\n\t"
        : [output] "=m" (os->inner[idx1].data[(idx2 << 1) + temp % 8])
        : [val] "r" (temp)
        : "memory"
    );
    
    /* Mixed input/output with complex addresses */
    asm volatile (
        "addl $1, %[out]\n\t"
        : [out] "=m" (arr[(idx1 * 3 + idx2 * 7) % 16])
        : [in] "m" (os->matrix[idx1 % 3][idx2 % 3])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(OuterStruct* os, int idx) {
    /* Taking address of complex expression forces operand address reload */
    int* complex_addr = &os->inner[(idx + global_index) % 4].data[
        (idx * 2 + os->inner[idx % 4].offset) % 8
    ];
    
    /* Use in inline asm */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[addr])\n\t"
        :
        : [addr] "r" (complex_addr)
        : "eax", "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(OuterStruct* os, int* arr, int idx) {
    /* Complex addressing for both input and output */
    int* volatile vptr = arr;
    
    /* This should trigger inpaddr/outaddr reloads */
    asm volatile (
        "movl (%[in]), %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, (%[out])\n\t"
        :
        : [in] "r" (&vptr[(idx << 2) + os->inner[idx % 4].offset]),
          [out] "r" (&os->inner[(idx + 1) % 4].data[idx % 8])
        : "eax", "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(OuterStruct* os, int idx1, int idx2, int idx3) {
    /* Multiple complex address computations in one statement */
    int result = 
        os->inner[idx1].data[idx2] +
        os->matrix[idx1 % 3][idx2 % 3] +
        *(os->inner[idx2].ptr + idx3) +
        os->inner[idx3].data[(idx1 + idx2) % 8];
    
    /* Use result to prevent optimization */
    asm volatile (
        ""
        :
        : "r" (result)
        : "memory"
    );
}

/* Function to force RELOAD_OTHER type */
void test_other_reload(OuterStruct* os, int* arr, int n) {
    /* Loop with complex addressing that forces various reloads */
    for (int i = 0; i < n; i++) {
        /* Mix different addressing modes */
        int idx1 = (i * 3) % 4;
        int idx2 = (i * 5) % 8;
        int idx3 = (i * 7) % 3;
        
        /* Multiple asm statements with different constraints */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (arr[(idx1 << 3) + idx2])
            : [in1] "m" (os->inner[idx1].data[idx2]),
              [in2] "m" (os->matrix[idx3][idx1 % 3])
            : "eax", "memory"
        );
        
        /* Update structure with complex address */
        os->inner[idx2 % 4].ptr = &arr[(idx1 + idx2 + idx3) % 16];
    }
}

/* Main driver function */
int main() {
    /* Initialize test data */
    OuterStruct os;
    int array[32];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].data[j] = i * 10 + j;
        }
        os.inner[i].ptr = &array[i * 4];
        os.inner[i].offset = i * 2;
    }
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            os.matrix[i][j] = i * 3 + j;
        }
    }
    
    os.volatile_index = &global_index;
    
    for (int i = 0; i < 32; i++) {
        array[i] = i;
    }
    
    global_ptr = array;
    
    /* Call test functions with various parameters to trigger different reload types */
    test_input_address(&os, 1, 2, 3);
    test_output_address(&os, array, 2, 3);
    test_operand_address(&os, 1);
    test_inpaddr_outaddr(&os, array, 2);
    test_other_address(&os, 0, 1, 2);
    test_other_reload(&os, array, 8);
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += array[i];
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += os.inner[i].data[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
