/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct Inner {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct Outer {
    struct Inner inner_struct[4];
    int outer_array[16];
    volatile int index;
};

/* Global variables to increase register pressure */
volatile int global_index1 = 0;
volatile int global_index2 = 0;
struct Outer* volatile global_struct_ptr = NULL;

/* Function to prevent optimization */
static int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct Outer* nested, int i, int j, int k) {
    /* Complex addressing: nested[i].inner_struct[j].member_array[k] */
    int val;
    
    /* Force address computation with multiple register values */
    asm volatile (
        "movl %[val], %[result]\n\t"
        : [result] "=r" (val)
        : [val] "m" (nested[i].inner_struct[j].member_array[k]),
          "r" (i), "r" (j), "r" (k)
        : "memory"
    );
    
    /* Use the result to prevent dead code elimination */
    global_index1 = use_result(val);
    
    /* Additional complex addressing in loop */
    for (int idx = 0; idx < 4; idx++) {
        int temp = nested[idx].inner_struct[idx].member_array[(i + j + k) & 7];
        asm volatile ("" : : "m" (nested[idx]), "r" (temp) : "memory");
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct Outer* nested, int* results, int count) {
    /* Complex output addressing with shifting */
    for (int i = 0; i < count; i++) {
        int offset = (i << 2) + nested[i % 4].index;
        
        /* Force output address reload */
        asm volatile (
            "movl %[input], %[output]\n\t"
            : [output] "=m" (results[offset])
            : [input] "r" (i * 100),
              "r" (offset), "r" (results)
            : "memory"
        );
        
        /* Chain address computations */
        int complex_offset = offset + nested[(i + 1) % 4].index;
        asm volatile (
            "addl $1, %[dest]\n\t"
            : [dest] "=m" (nested[i % 4].outer_array[complex_offset & 15])
            : "r" (complex_offset)
            : "memory"
        );
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void test_operand_address(struct Outer* nested, int idx1, int idx2, int idx3) {
    /* Force address computation before function call */
    volatile int* addr1 = &nested[idx1].inner_struct[idx2].member_array[idx3];
    volatile int* addr2 = &nested[idx2].outer_array[idx1];
    
    /* Mixed addressing in inline asm */
    asm volatile (
        "movl (%[addr1]), %%eax\n\t"
        "addl (%[addr2]), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m" (nested[0].index)
        : [addr1] "r" (addr1),
          [addr2] "r" (addr2)
        : "eax", "memory"
    );
    
    /* Complex address as function argument simulation */
    int sum = *addr1 + *addr2 + nested[idx3].index;
    global_index2 = use_result(sum);
}

/* Test RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
void test_inpaddr_address(struct Outer* nested, int base) {
    /* Multiple memory inputs with complex addressing */
    for (int i = 0; i < 8; i++) {
        int idx1 = (base + i) & 3;
        int idx2 = (base * 2 + i) & 3;
        int idx3 = (base * 3 + i) & 7;
        
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "subl %[in2], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (nested[idx1].index)
            : [in1] "m" (nested[idx1].inner_struct[idx2].member_array[idx3]),
              [in2] "m" (nested[idx2].outer_array[idx1]),
              "r" (idx1), "r" (idx2), "r" (idx3)
            : "eax", "memory"
        );
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(struct Outer* nested, int* buffer, int size) {
    /* Multiple output addresses with register pressure */
    register int r1 asm("r10") = size;
    register int r2 asm("r11") = nested[0].index;
    
    for (int i = 0; i < size; i++) {
        int out_idx = (i * r1 + r2) & 31;
        int in_idx = (i + r2) & 15;
        
        /* Multiple outputs with complex addressing */
        asm volatile (
            "movl %[in_val], %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%ebx\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out1] "=m" (buffer[out_idx]),
              [out2] "=m" (nested[i & 3].outer_array[in_idx])
            : [in_val] "r" (i),
              "r" (out_idx), "r" (in_idx)
            : "eax", "ebx", "memory"
        );
        
        r2 = (r2 * 1103515245 + 12345) & 0x7fffffff;
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct Outer* nested, int iterations) {
    /* Mixed operations creating various reload types */
    volatile int temp[16];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force spill/reload of address registers */
        for (int i = 0; i < 8; i++) {
            int* addr1 = &nested[i & 3].inner_struct[(i >> 1) & 3].member_array[i & 7];
            int* addr2 = &temp[(i * 3) & 15];
            
            /* Complex asm with multiple memory operands */
            asm volatile (
                "movl (%[src]), %%ecx\n\t"
                "addl $42, %%ecx\n\t"
                "movl %%ecx, (%[dst])\n\t"
                "addl $1, %[counter]\n\t"
                : [counter] "+m" (global_index1)
                : [src] "r" (addr1),
                  [dst] "r" (addr2)
                : "ecx", "memory"
            );
        }
        
        /* Address computation that can't be done in one instruction */
        int complex_index = (nested[0].index * 3 + nested[1].index * 5) & 7;
        nested[2].inner_struct[3].member_array[complex_index] = iter;
    }
}

/* Main driver function */
int main() {
    /* Allocate and initialize test structures */
    struct Outer* nested = (struct Outer*)calloc(4, sizeof(struct Outer));
    int* buffer = (int*)malloc(256 * sizeof(int));
    
    if (!nested || !buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < 4; i++) {
        nested[i].index = i * 100;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                nested[i].inner_struct[j].member_array[k] = i * 1000 + j * 100 + k;
            }
            for (int k = 0; k < 16; k++) {
                nested[i].outer_array[k] = i * 500 + j * 50 + k;
            }
        }
    }
    
    for (int i = 0; i < 256; i++) {
        buffer[i] = i;
    }
    
    global_struct_ptr = nested;
    
    printf("Starting reload coverage tests...\n");
    
    /* Execute tests to trigger different reload types */
    test_input_address(nested, 1, 2, 3);
    printf("Test 1 complete\n");
    
    test_output_address(nested, buffer, 8);
    printf("Test 2 complete\n");
    
    test_operand_address(nested, 0, 1, 2);
    printf("Test 3 complete\n");
    
    test_inpaddr_address(nested, 5);
    printf("Test 4 complete\n");
    
    test_outaddr_address(nested, buffer, 4);
    printf("Test 5 complete\n");
    
    test_other_address(nested, 3);
    printf("Test 6 complete\n");
    
    /* Compute checksum to ensure all code executed */
    int checksum = 0;
    checksum += global_index1;
    checksum += global_index2;
    checksum += nested[0].index;
    checksum += buffer[10];
    
    for (int i = 0; i < 4; i++) {
        checksum += nested[i].inner_struct[0].member_array[0];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully\n");
    
    /* Cleanup */
    free(nested);
    free(buffer);
    
    return 0;
}
