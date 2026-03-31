/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
typedef struct {
    int data[8];
    int *ptr;
    int offset;
} InnerStruct;

typedef struct {
    InnerStruct inner[4];
    int matrix[4][4];
    long long big_array[16];
} OuterStruct;

/* Volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int *volatile global_ptr = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(OuterStruct *os, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing that requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Multiple indices in address computation */
        int val = os->inner[(idx1 + i) & 3].data[(idx2 * i + idx3) & 7];
        
        /* Inline asm with memory input constraint and complex address */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (result)
            : "m" (os->inner[(idx1 + i) & 3].data[(idx2 * i + idx3) & 7])
            : "cc"
        );
    }
    
    return result;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(OuterStruct *os, int *indices, int count) {
    /* Complex output addressing */
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        
        /* Inline asm with memory output constraint to computed address */
        asm volatile (
            "movl $0x%0, %1\n\t"
            : "=m" (os->big_array[(idx << 2) + (i * 3)])
            : "i" (i * 0x100 + 0x42)
            : "memory"
        );
    }
}

/* Test function for RELOAD_FOR_INPUT and mixed types */
int test_mixed_reloads(OuterStruct *os1, OuterStruct *os2, 
                       int base_idx, int offset) {
    int sum = 0;
    
    /* Mixed input/output with complex addressing */
    for (int i = 0; i < 4; i++) {
        /* Input address reload for os1 */
        int input_val = os1->matrix[i][(base_idx + offset) & 3];
        
        /* Output address reload for os2 */
        os2->inner[i].offset = input_val * 2;
        
        /* Inline asm with both input and output memory constraints */
        asm volatile (
            "imull %2, %1\n\t"
            "addl %1, %0\n\t"
            : "+r" (sum), "=m" (os2->inner[(i + 1) & 3].data[offset & 7])
            : "m" (os1->matrix[(i + 2) & 3][(base_idx + i) & 3]),
              "r" (i)
            : "cc", "memory"
        );
    }
    
    return sum;
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(InnerStruct *inner_array, int *indices, int n) {
    /* Function calls with complex address expressions */
    for (int i = 0; i < n; i++) {
        /* This forces operand address computation before the asm */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (*global_ptr)
            : "r" (&inner_array[indices[i] & 3].data[indices[i + 1] & 7])
            : "eax", "memory"
        );
    }
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_address_of_address(OuterStruct *os, int idx) {
    int *temp_ptr;
    
    /* Complex address of address computation */
    temp_ptr = &os->inner[idx & 3].data[0];
    
    /* Multiple levels of indirection in asm */
    asm volatile (
        "leal (%1, %2, 4), %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, (%0)\n\t"
        : 
        : "r" (temp_ptr),
          "r" (os->inner[(idx + 1) & 3].ptr),
          "r" (idx)
        : "eax", "ebx", "memory"
    );
    
    /* Another complex case */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=m" (os->inner[idx & 3].offset)
        : "r" (&os->matrix[0][0]),
          "r" (idx * sizeof(int) * 4)
        : "eax", "ebx", "memory"
    );
}

/* Test function for RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
int test_other_reloads(OuterStruct **os_array, int *offsets, int count) {
    int total = 0;
    
    /* Multiple indirect accesses with different base pointers */
    for (int i = 0; i < count; i++) {
        OuterStruct *current = os_array[i & 1];
        int offset = offsets[i];
        
        /* Complex addressing with multiple components */
        asm volatile (
            "movl (%2, %3, 8), %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movl %0, %1\n\t"
            : "+r" (total), "=m" (current->inner[offset & 3].data[(offset >> 2) & 7])
            : "r" (current->big_array),
              "r" (offset)
            : "eax", "memory"
        );
    }
    
    return total;
}

/* Main driver function */
int main() {
    /* Initialize test data */
    OuterStruct os1, os2;
    OuterStruct *os_array[2] = {&os1, &os2};
    int indices[16];
    int offsets[8];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os1.inner[i].data[j] = i * 100 + j;
            os2.inner[i].data[j] = i * 200 + j;
        }
        for (int j = 0; j < 4; j++) {
            os1.matrix[i][j] = i * 10 + j;
            os2.matrix[i][j] = i * 20 + j;
        }
        os1.inner[i].ptr = &os1.inner[i].data[0];
        os2.inner[i].ptr = &os2.inner[i].data[0];
    }
    
    for (int i = 0; i < 16; i++) {
        os1.big_array[i] = i * 1000LL;
        indices[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 8; i++) {
        offsets[i] = i * 5 + 2;
    }
    
    global_ptr = &indices[0];
    
    /* Call test functions to trigger different reload types */
    int result1 = test_input_address(&os1, 1, 2, 3);
    printf("test_input_address result: %d\n", result1);
    
    test_output_address(&os2, indices, 8);
    printf("test_output_address completed\n");
    
    int result2 = test_mixed_reloads(&os1, &os2, 2, 3);
    printf("test_mixed_reloads result: %d\n", result2);
    
    test_operand_address(os1.inner, indices, 8);
    printf("test_operand_address completed\n");
    
    test_address_of_address(&os1, 2);
    printf("test_address_of_address completed\n");
    
    int result3 = test_other_reloads(os_array, offsets, 8);
    printf("test_other_reloads result: %d\n", result3);
    
    /* Compute checksum to ensure all code executes */
    long long checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += os1.inner[i].data[j];
            checksum += os2.inner[i].data[j];
        }
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
