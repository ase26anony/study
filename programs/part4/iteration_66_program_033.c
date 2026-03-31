/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct inner_struct {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct outer_struct {
    struct inner_struct inner[4];
    int base_value;
    volatile int index_hint;
};

/* Global variables to increase register pressure */
volatile int global_index = 0;
struct outer_struct global_nested[16];
int global_array[256];

/* Function to prevent optimization */
static inline void memory_barrier(void) {
    asm volatile("" ::: "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer_struct *nested, int outer_idx, int inner_idx, int offset) {
    volatile int result = 0;
    
    /* Complex addressing that likely requires input address reload */
    for (int i = 0; i < 4; i++) {
        /* Multiple register values in address computation */
        int complex_idx = (outer_idx * 3 + inner_idx * 2 + offset) & 0xF;
        
        /* Inline asm with input memory operand using complex address */
        asm volatile(
            "addl %%ecx, %%eax\n\t"
            "movl (%%rbx,%%rax,4), %%edx"
            : "=d"(result)
            : "a"(complex_idx), 
              "b"(nested[outer_idx].inner[inner_idx].member_array),
              "c"(i),
              "m"(nested[outer_idx].inner[inner_idx].member_array[complex_idx + i])
            : "cc"
        );
        
        memory_barrier();
    }
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(int *output_array, int *index_array, int count) {
    /* Force output address reloads with computed addresses */
    for (int i = 0; i < count; i++) {
        int idx = index_array[i];
        int complex_offset = (idx << 2) + (i & 3);
        
        /* Inline asm with output memory operand at computed address */
        asm volatile(
            "movl %%esi, (%%rdi,%%rax,4)\n\t"
            : "=m"(output_array[complex_offset])
            : "D"(output_array),
              "a"(complex_offset),
              "S"(i * 7)
            : "memory"
        );
        
        /* Additional computation to prevent optimization */
        output_array[complex_offset + 1] = idx * 3;
    }
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
void helper_function(int *addr1, int *addr2, volatile int *addr3) {
    /* Force address computations before call */
    *addr1 += *addr2;
    *addr3 = *addr1;
}

void test_operand_address(struct outer_struct *nested, int idx1, int idx2) {
    /* Complex address expressions as function arguments */
    helper_function(
        &nested[idx1].inner[idx2 & 3].member_array[(idx1 + idx2) & 7],
        &global_array[(idx1 * 17 + idx2 * 13) & 0xFF],
        &nested[idx2].index_hint
    );
    
    /* Another complex address computation */
    int * volatile_ptr = nested[idx1].inner[idx2].volatile_ptr;
    if (volatile_ptr) {
        helper_function(
            volatile_ptr + (idx1 & 7),
            &global_nested[idx2].base_value,
            &nested[0].index_hint
        );
    }
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_mixed_address_reloads(int *data, int *indices, int size) {
    volatile int temp = 0;
    
    for (int i = 0; i < size; i++) {
        int idx = indices[i];
        int offset = (idx * 3 + i * 5) & 0x3F;
        
        /* Mixed input/output with complex addressing */
        asm volatile(
            "movl (%%rsi,%%rax,4), %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            "movl %%ecx, (%%rdi,%%rbx,4)"
            : "=m"(data[offset]), "=m"(global_array[i])
            : "S"(data), "D"(global_array),
              "a"(idx), "b"(offset),
              "m"(data[idx]), "m"(global_array[offset])
            : "rcx", "memory", "cc"
        );
        
        /* Force another address computation */
        int *addr = &data[(offset + i) & 0x3F];
        *addr = temp + i;
        temp = *addr;
    }
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_reload_types(struct outer_struct *nested, int count) {
    volatile int accumulator = 0;
    
    for (int i = 0; i < count; i++) {
        /* Multiple complex memory operations in sequence */
        int idx1 = (i * 3) & 0xF;
        int idx2 = (i * 5) & 0x3;
        int idx3 = (i * 7) & 0x7;
        
        /* Chain of operations requiring various reloads */
        int val1 = nested[idx1].inner[idx2].member_array[idx3];
        int val2 = global_array[(idx1 + idx2 * 4 + idx3 * 8) & 0xFF];
        
        /* Inline asm with multiple memory constraints */
        asm volatile(
            "imull %%ecx, %%eax\n\t"
            "addl %%edx, %%eax"
            : "+a"(accumulator)
            : "c"(val1), "d"(val2),
              "m"(nested[idx1].inner[idx2].member_array[idx3]),
              "m"(global_array[(idx1 + idx2 * 4 + idx3 * 8) & 0xFF])
            : "cc"
        );
        
        /* Store with complex address */
        nested[idx2].inner[idx1].member_array[idx3 & 3] = accumulator;
        
        /* Another memory operation with different addressing */
        global_nested[i & 0xF].base_value = accumulator + i;
    }
}

/* Main driver function */
int main(void) {
    /* Initialize test data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        global_nested[i].base_value = i * 100;
        global_nested[i].index_hint = i;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                global_nested[i].inner[j].member_array[k] = i * 1000 + j * 100 + k;
            }
            global_nested[i].inner[j].volatile_ptr = &global_array[(i + j * 4) & 0xFF];
        }
    }
    
    int indices[32];
    int output[128];
    
    for (int i = 0; i < 32; i++) {
        indices[i] = (i * 13) & 0x7F;
    }
    
    for (int i = 0; i < 128; i++) {
        output[i] = 0;
    }
    
    /* Execute tests to trigger different reload types */
    printf("Starting reload coverage tests...\n");
    
    /* Test 1: Input address reloads */
    test_input_address(global_nested, 3, 1, 5);
    printf("Test 1 complete\n");
    
    /* Test 2: Output address reloads */
    test_output_address(output, indices, 32);
    printf("Test 2 complete\n");
    
    /* Test 3: Operand address reloads */
    test_operand_address(global_nested, 5, 7);
    printf("Test 3 complete\n");
    
    /* Test 4: Mixed address reloads */
    test_mixed_address_reloads(global_array, indices, 16);
    printf("Test 4 complete\n");
    
    /* Test 5: Other reload types */
    test_other_reload_types(global_nested, 8);
    printf("Test 5 complete\n");
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < 128; i++) {
        checksum += output[i];
    }
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully.\n");
    
    return 0;
}
