/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 * Or for more aggressive reloads: gcc -O3 -fno-inline -fno-gcse test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct inner {
    int data[8];
    volatile int *ptr;
};

struct outer {
    struct inner arrays[4];
    volatile long index;
    int padding[16];  /* Increase structure size */
};

/* Global variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
struct outer global_struct;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer *s, int idx1, int idx2, int idx3) {
    /* Complex addressing with multiple indices */
    int value;
    
    /* Force input address reload with inline asm */
    asm volatile (
        "movl %[input], %[output]\n\t"
        : [output] "=r" (value)
        : [input] "m" (s->arrays[idx1].data[(idx2 << 2) + idx3])
        : "memory"
    );
    
    /* Another complex access pattern */
    asm volatile (
        ""
        :
        : "m" (s->arrays[s->index].data[global_index + idx1]),
          "m" (s->arrays[idx2].data[idx3])
        : "memory"
    );
    
    global_index = value;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer *s, int idx1, int idx2, int idx3) {
    /* Complex output addressing */
    int temp = idx1 + idx2 + idx3;
    
    /* Force output address reload */
    asm volatile (
        "movl %[val], %[output]\n\t"
        : [output] "=m" (s->arrays[idx1].data[(idx2 * 3) + idx3])
        : [val] "r" (temp)
        : "memory"
    );
    
    /* Mixed input/output with different addressing */
    asm volatile (
        "addl $1, %[out]\n\t"
        : [out] "=m" (s->padding[global_offset + idx1])
        : [in] "m" (s->arrays[idx2].data[idx3])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void helper_func(int *addr1, int *addr2, int *addr3) {
    /* Force address computations before call */
    asm volatile (
        ""
        :
        : "r" (addr1), "r" (addr2), "r" (addr3)
        : "memory"
    );
}

void test_operand_address(struct outer *s, int i, int j, int k) {
    /* Complex address expressions as function arguments */
    helper_func(
        &s->arrays[i].data[(j << 1) + k],
        &s->arrays[j].data[(k << 2) + i],
        &s->arrays[k].data[(i << 1) + j]
    );
    
    /* More complex nested addressing */
    helper_func(
        &s->arrays[s->index].data[global_index],
        &s->padding[i + j * 2],
        &s->arrays[global_offset].ptr[(i * j) + k]
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_inpaddr_outaddr(struct outer *s, int idx1, int idx2, int idx3) {
    volatile int *ptr1, *ptr2, *ptr3;
    int temp;
    
    /* Setup pointers with complex addressing */
    ptr1 = &s->arrays[idx1].data[(idx2 << 1) + idx3];
    ptr2 = &s->arrays[idx2].data[(idx3 << 2) + idx1];
    ptr3 = &s->arrays[idx3].data[(idx1 << 1) + idx2];
    
    /* Force inpaddr address reload */
    asm volatile (
        "movl (%[addr]), %[val]\n\t"
        : [val] "=r" (temp)
        : [addr] "r" (ptr1)
        : "memory"
    );
    
    /* Force outaddr address reload */
    asm volatile (
        "movl %[val], (%[addr])\n\t"
        :
        : [val] "r" (temp + 1), [addr] "r" (ptr2)
        : "memory"
    );
    
    /* Mixed addressing modes */
    asm volatile (
        "movl (%[in]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, (%[out])\n\t"
        :
        : [in] "r" (&s->arrays[idx1].ptr[idx2]),
          [out] "r" (&s->arrays[idx3].ptr[idx1])
        : "eax", "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct outer *s, int idx1, int idx2, int idx3) {
    int temp_array[16];
    volatile int *volatile ptr;
    
    /* Complex pointer arithmetic */
    ptr = &s->arrays[0].data[0];
    ptr += (idx1 * idx2) + (idx3 << 1);
    
    /* Force other address reloads */
    asm volatile (
        ""
        : "=m" (*ptr), "=m" (temp_array[idx1 + idx2])
        : "m" (s->arrays[idx2].data[idx3]),
          "m" (s->padding[global_index])
        : "memory"
    );
    
    /* Multiple memory operands with different addressing */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out1] "=m" (s->arrays[idx1].data[idx2]),
          [out2] "=m" (temp_array[idx3])
        : [in1] "m" (s->arrays[idx2].data[idx3]),
          [in2] "m" (s->padding[idx1])
        : "eax", "memory"
    );
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(struct outer *s, int iterations) {
    int i, j, k;
    
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                /* Alternate between different reload patterns */
                if ((i + j + k) % 4 == 0) {
                    test_input_address(s, i & 3, j, k);
                } else if ((i + j + k) % 4 == 1) {
                    test_output_address(s, j, k, i & 3);
                } else if ((i + j + k) % 4 == 2) {
                    test_inpaddr_outaddr(s, k, i & 3, j);
                } else {
                    test_other_address(s, i & 3, j, k);
                }
                
                /* Force operand address reloads periodically */
                if ((i * j * k) % 8 == 0) {
                    test_operand_address(s, i & 3, j, k);
                }
            }
        }
        
        /* Update volatile variables to prevent optimization */
        global_index = (global_index + i) & 7;
        global_offset = (global_offset + 1) & 15;
        s->index = (s->index + 1) & 3;
    }
}

/* Main driver function */
int main() {
    struct outer *s;
    int i, checksum = 0;
    
    /* Allocate and initialize test structure */
    s = (struct outer *)malloc(sizeof(struct outer));
    if (!s) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize structure with non-zero values */
    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 8; j++) {
            s->arrays[i].data[j] = (i * 8) + j;
        }
        s->arrays[i].ptr = (volatile int *)&s->arrays[(i + 1) % 4].data[0];
    }
    
    for (i = 0; i < 16; i++) {
        s->padding[i] = i * 2;
    }
    
    s->index = 0;
    global_struct = *s;
    
    printf("Starting reload coverage test...\n");
    
    /* Run tests to trigger various reload types */
    test_input_address(s, 1, 2, 3);
    test_output_address(s, 2, 3, 1);
    test_operand_address(s, 3, 1, 2);
    test_inpaddr_outaddr(s, 0, 2, 1);
    test_other_address(s, 1, 0, 2);
    
    /* Comprehensive mixed test */
    test_mixed_reloads(s, 3);
    
    /* Compute checksum to ensure code wasn't optimized away */
    for (i = 0; i < 4; i++) {
        int j;
        for (j = 0; j < 8; j++) {
            checksum += s->arrays[i].data[j];
        }
    }
    
    for (i = 0; i < 16; i++) {
        checksum += s->padding[i];
    }
    
    checksum += global_index + global_offset + s->index;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    free(s);
    return 0;
}
