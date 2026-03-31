/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test_reload
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
    long offset;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
volatile struct Outer* volatile global_struct = NULL;

/* Test function for RELOAD_FOR_INPUT_ADDRESS */
int test_input_address(struct Outer* s, int idx1, int idx2, int idx3) {
    int result = 0;
    
    /* Complex addressing that requires input address reloads */
    for (int i = 0; i < 4; i++) {
        /* Multiple index computations in address */
        int complex_idx = (idx1 + i) * (idx2 + 1) + idx3;
        
        /* Inline assembly with memory input constraint and complex address */
        asm volatile (
            "addl %%ecx, %%eax\n\t"
            : "=a"(result)
            : "a"(result), 
              "m"(s->arrays[i].data[complex_idx % 8]),  /* Memory input with complex address */
              "c"(s->arrays[i].extra)
            : "memory"
        );
    }
    
    return result;
}

/* Test function for RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(int* buffer, int* indices, int count) {
    /* Complex output addressing */
    for (int i = 0; i < count; i++) {
        int offset = (indices[i] << 2) + (i * 16);
        
        /* Inline assembly with memory output constraint */
        asm volatile (
            "movl %%eax, %0\n\t"
            : "=m"(buffer[offset])  /* Memory output with computed address */
            : "a"(i * 100)
            : "memory"
        );
    }
}

/* Test function for RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct Outer* s, int idx) {
    /* Function call with complex address expression */
    helper_function(&s->arrays[idx % 4].data[(idx * 3) % 8]);
}

void helper_function(int* ptr) {
    /* Dummy function to force address computation */
    *ptr = *ptr + 1;
}

/* Test function for mixed reload types */
int test_mixed_reloads(struct Outer* s, int* out_buf, int idx) {
    int temp = 0;
    
    /* Mixed input/output with different addressing */
    for (int i = 0; i < 4; i++) {
        int input_idx = (idx + i * 3) % 4;
        int output_idx = (idx * 2 + i) % 4;
        
        /* Complex input address computation */
        int input_val = s->arrays[input_idx].data[(i + idx) % 8];
        
        /* Inline assembly with both input and output memory constraints */
        asm volatile (
            "imull %%ecx, %%eax\n\t"
            "addl %%eax, %1\n\t"
            : "+a"(temp), "=m"(out_buf[output_idx * 8 + i])  /* Output address */
            : "c"(input_val), 
              "m"(s->arrays[output_idx].data[i]),  /* Input address */
              "m"(s->base)  /* Another input */
            : "memory"
        );
    }
    
    return temp;
}

/* Test function for RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_address_of_address(int* data, int size) {
    int* volatile ptrs[10];
    
    for (int i = 0; i < size && i < 10; i++) {
        /* Taking address of array element with complex index */
        int complex_index = (i * 7 + 3) % size;
        ptrs[i] = &data[complex_index];
        
        /* Using the address in inline assembly */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m"(*ptrs[i])  /* Memory operand at computed address */
            : "r"(ptrs[i])    /* Register containing address */
            : "eax", "memory"
        );
    }
}

/* Test function for RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
int test_other_reloads(struct Outer* s1, struct Outer* s2, int idx) {
    int result = 0;
    
    /* Multiple complex memory operations in sequence */
    for (int i = 0; i < 3; i++) {
        /* First operation with one addressing mode */
        int val1 = s1->arrays[(idx + i) % 4].data[i];
        
        /* Second operation with different addressing */
        int val2 = s2->arrays[i].data[(idx * 2 + i) % 8];
        
        /* Third operation mixing both */
        asm volatile (
            "leal (%1, %2, 4), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r"(result)
            : "r"(val1), "r"(val2),
              "m"(s1->offset),  /* Additional memory operand */
              "m"(s2->base)     /* Another memory operand */
            : "eax", "memory"
        );
    }
    
    return result;
}

/* Main driver function */
int main() {
    /* Initialize test data */
    struct Outer struct1, struct2;
    int buffer[256];
    int indices[32];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            struct1.arrays[i].data[j] = i * 100 + j;
            struct2.arrays[i].data[j] = i * 200 + j * 2;
        }
        struct1.arrays[i].extra = i * 50;
        struct2.arrays[i].extra = i * 75;
    }
    struct1.base = 1000;
    struct2.base = 2000;
    struct1.offset = 500;
    struct2.offset = 750;
    
    /* Initialize indices */
    for (int i = 0; i < 32; i++) {
        indices[i] = (i * 3) % 256;
        buffer[i] = i;
    }
    
    int checksum = 0;
    
    /* Call test functions to trigger different reload types */
    checksum += test_input_address(&struct1, 1, 2, 3);
    
    test_output_address(buffer, indices, 16);
    
    test_operand_address(&struct2, 5);
    
    checksum += test_mixed_reloads(&struct1, buffer, 2);
    
    test_address_of_address(buffer, 256);
    
    checksum += test_other_reloads(&struct1, &struct2, 3);
    
    /* Compute final checksum from buffer */
    for (int i = 0; i < 256; i++) {
        checksum += buffer[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
