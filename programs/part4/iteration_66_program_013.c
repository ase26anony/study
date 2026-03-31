/* test_reload_coverage.c
 * 
 * This program creates complex addressing scenarios to trigger
 * various reload types in GCC's reload pass, specifically targeting
 * the switch cases in chain_reload_insns() in reload1.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to create nested addressing */
struct Inner {
    int data[8];
    int extra;
};

struct Outer {
    struct Inner arrays[4];
    int padding[3];
};

struct Mixed {
    int* ptr_array[16];
    long offsets[16];
    volatile int index_mask;
};

/* Global volatile variables to prevent optimization */
volatile int g_index1 = 2;
volatile int g_index2 = 3;
volatile int g_base_offset = 100;

/* Function to create RELOAD_FOR_INPUT_ADDRESS scenarios */
void test_input_address(struct Outer* outer, int idx1, int idx2, int idx3) {
    /* Complex addressing: outer[idx1].arrays[idx2].data[idx3] */
    /* This creates a multi-step address computation */
    asm volatile (
        "/* Input address computation */\n\t"
        : /* no outputs */
        : "m" (outer[idx1].arrays[idx2].data[idx3]),
          "r" (outer), "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    /* Another complex input address with shift */
    int shift_amount = 2;
    asm volatile (
        "/* Input address with shift */\n\t"
        : 
        : "m" (outer[(idx1 << shift_amount) + idx2].arrays[idx3].data[0]),
          "r" (outer), "r" (idx1), "r" (idx2), "r" (idx3), "r" (shift_amount)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OUTPUT_ADDRESS scenarios */
void test_output_address(struct Outer* outer, int* results, 
                         int idx1, int idx2, int idx3) {
    /* Complex output addressing */
    asm volatile (
        "/* Output address computation */\n\t"
        : "=m" (outer[idx1].arrays[idx2].data[idx3])
        : "r" (outer), "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    /* Output to computed address with multiple indices */
    int combined_idx = idx1 * 4 + idx2;
    asm volatile (
        "/* Output to computed address */\n\t"
        : "=m" (results[combined_idx + idx3])
        : "r" (results), "r" (combined_idx), "r" (idx3)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OPERAND_ADDRESS scenarios */
void test_operand_address(struct Mixed* mixed, int idx) {
    /* Taking address of complex expression */
    int* complex_addr = &mixed->ptr_array[mixed->offsets[idx] & mixed->index_mask][idx];
    
    asm volatile (
        "/* Operand address usage */\n\t"
        : 
        : "r" (complex_addr), "m" (*complex_addr)
        : "memory"
    );
    
    /* Address computation as function argument simulation */
    volatile int dummy;
    asm volatile (
        "/* Address computation in pseudo-call */\n\t"
        "mov %[addr], %%rax\n\t"
        "mov (%%rax), %%ebx\n\t"
        : "=m" (dummy)
        : [addr] "r" (&mixed->ptr_array[idx][mixed->offsets[idx] % 8]),
          "m" (mixed->ptr_array[idx][mixed->offsets[idx] % 8])
        : "rax", "rbx", "memory"
    );
}

/* Function to create RELOAD_FOR_INPADDR_ADDRESS scenarios */
void test_inpaddr_address(struct Outer* outer, int idx1, int idx2) {
    /* Input address that itself needs reloading */
    struct Inner* temp = &outer[idx1].arrays[idx2];
    
    asm volatile (
        "/* Input address that needs address reload */\n\t"
        : 
        : "m" (temp->data[0]), "m" (temp->data[1]),
          "r" (temp), "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Multiple levels of indirection */
    struct Inner** ptr_to_ptr = &temp;
    asm volatile (
        "/* Multiple indirection levels */\n\t"
        : 
        : "m" ((*ptr_to_ptr)->data[idx1]),
          "r" (ptr_to_ptr), "r" (idx1)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OUTADDR_ADDRESS scenarios */
void test_outaddr_address(int*** triple_ptr, int depth1, int depth2) {
    /* Output address that itself needs address computation */
    int** ptr_at_depth = triple_ptr[depth1];
    
    asm volatile (
        "/* Output address needing address reload */\n\t"
        : "=m" (ptr_at_depth[depth2])
        : "r" (ptr_at_depth), "r" (depth2)
        : "memory"
    );
}

/* Function to create RELOAD_FOR_OTHER_ADDRESS scenarios */
void test_other_address(struct Mixed* mixed, volatile int* indices, int count) {
    /* Mixed addressing in loop - creates various reload types */
    for (int i = 0; i < count; i++) {
        int idx1 = indices[i] & 0xF;
        int idx2 = indices[i + 1] & 0x7;
        
        /* This creates a chain of address computations */
        asm volatile (
            "/* Mixed addressing in loop */\n\t"
            : "=m" (mixed->ptr_array[idx1][idx2])
            : "m" (mixed->offsets[idx1]),
              "r" (mixed), "r" (idx1), "r" (idx2)
            : "memory"
        );
    }
}

/* Function to create RELOAD_FOR_OPADDR_ADDR scenarios */
void test_opaddr_addr(struct Outer* outer, int* base_indices) {
    /* Complex address of an address computation */
    int (*addr_array)[8] = &outer[base_indices[0]].arrays[base_indices[1]].data;
    
    asm volatile (
        "/* Address of address computation */\n\t"
        : 
        : "r" (addr_array), "m" (base_indices[0]), "m" (base_indices[1])
        : "memory"
    );
    
    /* Another variant with more computation */
    int offset = base_indices[2] * sizeof(struct Inner);
    struct Inner* computed = (struct Inner*)((char*)outer + offset);
    
    asm volatile (
        "/* Computed address usage */\n\t"
        : "=m" (computed->data[base_indices[0]])
        : "r" (computed), "r" (base_indices[0])
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
int test_mixed_reloads(struct Outer* outer, struct Mixed* mixed, 
                       int* results, int iterations) {
    int sum = 0;
    volatile int indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    
    for (int i = 0; i < iterations; i++) {
        /* Vary indices to prevent optimization */
        int idx1 = (indices[i % 8] + g_index1) % 4;
        int idx2 = (indices[(i + 1) % 8] + g_index2) % 4;
        int idx3 = (indices[(i + 2) % 8] + i) % 8;
        
        /* Trigger multiple reload types in sequence */
        test_input_address(outer, idx1, idx2, idx3);
        test_output_address(outer, results, idx1, idx2, idx3);
        
        if (i % 3 == 0) {
            test_operand_address(mixed, idx1);
        }
        
        if (i % 4 == 0) {
            test_inpaddr_address(outer, idx2, idx3);
        }
        
        /* Accumulate to prevent dead code elimination */
        sum += results[idx3] + outer[idx1].arrays[idx2].data[idx3];
    }
    
    return sum;
}

/* Main driver function */
int main() {
    /* Allocate and initialize test structures */
    struct Outer* outer_array = (struct Outer*)calloc(8, sizeof(struct Outer));
    struct Mixed* mixed_data = (struct Mixed*)calloc(1, sizeof(struct Mixed));
    int* results = (int*)calloc(64, sizeof(int));
    int*** triple_ptr = (int***)calloc(4, sizeof(int**));
    
    /* Initialize mixed_data */
    for (int i = 0; i < 16; i++) {
        mixed_data->ptr_array[i] = (int*)calloc(16, sizeof(int));
        mixed_data->offsets[i] = i * 2;
    }
    mixed_data->index_mask = 0xF;
    
    /* Initialize triple pointer structure */
    for (int i = 0; i < 4; i++) {
        triple_ptr[i] = (int**)calloc(4, sizeof(int*));
        for (int j = 0; j < 4; j++) {
            triple_ptr[i][j] = (int*)calloc(4, sizeof(int));
        }
    }
    
    /* Initialize base indices for opaddr test */
    int base_indices[4] = {1, 2, 0, 3};
    
    printf("Starting reload coverage tests...\n");
    
    /* Run individual tests to trigger specific reload types */
    test_input_address(outer_array, 1, 2, 3);
    test_output_address(outer_array, results, 0, 1, 2);
    test_operand_address(mixed_data, 1);
    test_inpaddr_address(outer_array, 2, 1);
    test_outaddr_address(triple_ptr, 1, 2);
    test_other_address(mixed_data, (volatile int*)base_indices, 4);
    test_opaddr_addr(outer_array, base_indices);
    
    /* Run mixed test - most likely to trigger complex reload chains */
    int final_sum = test_mixed_reloads(outer_array, mixed_data, results, 16);
    
    printf("Test completed. Final sum: %d\n", final_sum);
    
    /* Cleanup */
    for (int i = 0; i < 16; i++) {
        free(mixed_data->ptr_array[i]);
    }
    free(mixed_data);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            free(triple_ptr[i][j]);
        }
        free(triple_ptr[i]);
    }
    free(triple_ptr);
    
    free(outer_array);
    free(results);
    
    return 0;
}
