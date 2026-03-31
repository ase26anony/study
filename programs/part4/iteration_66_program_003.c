/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test_reload
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct inner_struct {
    int data[8];
    int extra;
};

struct outer_struct {
    struct inner_struct inner[4];
    int base;
    int offset;
};

/* Global arrays to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
struct outer_struct global_array[16];
int output_buffer[256];

/* Function to prevent dead code elimination */
__attribute__((noinline)) 
int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
__attribute__((noinline))
void test_input_address(int idx1, int idx2, int idx3) {
    volatile int* restrict ptr = (volatile int*)&global_array[0];
    
    /* Complex addressing that requires input address reload */
    int val1 = global_array[idx1].inner[idx2].data[idx3];
    int val2 = global_array[idx2].inner[idx3].data[idx1];
    
    /* Inline assembly with memory input and complex address */
    asm volatile (
        "/* Input address computation */\n\t"
        : /* no outputs */
        : "m" (global_array[(idx1 + idx2) % 16].inner[(idx2 + idx3) % 4].data[(idx1 + idx3) % 8]),
          "m" (global_array[idx3].inner[idx1].data[idx2])
        : "memory"
    );
    
    use_result(val1 + val2);
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
__attribute__((noinline))
void test_output_address(int base_idx, int offset1, int offset2) {
    /* Complex output addressing */
    int* out_ptr = &output_buffer[base_idx];
    
    /* Multiple output addresses with computation */
    asm volatile (
        "/* Output address reload test */\n\t"
        : "=m" (out_ptr[offset1 * 2]),
          "=m" (out_ptr[offset2 * 3 + 1]),
          "=m" (out_ptr[(offset1 + offset2) * 4])
        : /* no inputs */
        : "memory"
    );
    
    /* More complex: output to computed struct location */
    global_array[base_idx % 8].inner[(offset1 + offset2) % 4].data[offset1 % 8] = 
        base_idx + offset1 + offset2;
}

/* Test RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT_ADDRESS together */
__attribute__((noinline))
void test_mixed_io(int idx, int offset) {
    volatile int temp;
    
    /* Mixed input/output with addressing */
    asm volatile (
        "/* Mixed input/output addressing */\n\t"
        : "=m" (global_array[idx].inner[offset % 4].data[(idx + offset) % 8]),
          "=r" (temp)
        : "m" (global_array[(idx + 1) % 16].inner[(offset + 1) % 4].data[idx % 8]),
          "r" (idx),
          "r" (offset)
        : "memory"
    );
    
    /* Chain of dependent addresses */
    int* ptr1 = &global_array[idx].base;
    int* ptr2 = &global_array[offset].offset;
    
    asm volatile (
        "/* Chained address computation */\n\t"
        : "=m" (*ptr1),
          "=m" (*ptr2)
        : "m" (global_array[(idx + offset) % 16].base),
          "r" (ptr1),
          "r" (ptr2)
        : "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
__attribute__((noinline))
void test_operand_address(struct outer_struct* arr, int i, int j, int k) {
    /* Function call with complex address argument - forces operand address reload */
    int result = use_result(arr[i].inner[j].data[k]);
    
    /* Multiple complex addresses in expressions */
    int val = arr[(i + j) % 16].inner[(j + k) % 4].data[(i + k) % 8] +
              arr[(i + k) % 16].inner[(i + j) % 4].data[(j + k) % 8];
    
    /* Inline asm with address constraints */
    int* addr1 = &arr[i].inner[j].data[k];
    int* addr2 = &arr[j].inner[k].data[i];
    
    asm volatile (
        "/* Operand address reloads */\n\t"
        : /* no outputs */
        : "r" (addr1),
          "r" (addr2),
          "m" (*addr1),
          "m" (*addr2)
        : "memory"
    );
    
    use_result(val + result);
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
__attribute__((noinline))
void test_addr_of_addr(int idx) {
    /* Taking address of memory location with complex addressing */
    int** ptr_to_ptr;
    int* base_ptr;
    
    /* Complex chain: address of struct member's array element */
    base_ptr = &global_array[idx % 16].inner[(idx + 1) % 4].data[(idx * 2) % 8];
    ptr_to_ptr = &base_ptr;
    
    /* Inline asm that uses both the address and the address of the address */
    asm volatile (
        "/* Address of address computation */\n\t"
        : "=m" (*base_ptr),
          "=m" (ptr_to_ptr)
        : "m" (global_array[(idx + 2) % 16].inner[(idx + 3) % 4].data[(idx * 3) % 8]),
          "r" (base_ptr),
          "r" (ptr_to_ptr)
        : "memory"
    );
    
    /* Multiple levels of indirection */
    int val = **ptr_to_ptr;
    use_result(val);
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
__attribute__((noinline))
void test_other_address_types(int n) {
    volatile int temp_array[32];
    int i, j;
    
    /* Loop with complex addressing that may trigger various reload types */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 4; j++) {
            /* Complex expression mixing array and struct accesses */
            int idx = (i * j + global_index) % 16;
            int offset = (i + j + global_offset) % 4;
            
            /* This complex addressing may require "other" reload types */
            temp_array[i * 4 + j] = 
                global_array[idx].inner[offset].data[(i + j) % 8] +
                global_array[(idx + 1) % 16].inner[(offset + 1) % 4].data[(i * j) % 8];
            
            /* Inline asm with multiple memory operands */
            asm volatile (
                "/* Other address types */\n\t"
                : "=m" (temp_array[(i * 4 + j + 1) % 32])
                : "m" (global_array[idx].inner[offset].base),
                  "m" (global_array[(idx + 2) % 16].inner[(offset + 2) % 4].offset),
                  "r" (i),
                  "r" (j)
                : "memory"
            );
        }
    }
    
    /* Compute checksum to use results */
    int sum = 0;
    for (i = 0; i < 32 && i < n * 4; i++) {
        sum += temp_array[i];
    }
    use_result(sum);
}

/* Main driver that exercises all test functions */
int main(int argc, char** argv) {
    int i, j, k;
    
    /* Initialize test data */
    for (i = 0; i < 16; i++) {
        global_array[i].base = i * 100;
        global_array[i].offset = i * 10;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 8; k++) {
                global_array[i].inner[j].data[k] = i * 1000 + j * 100 + k;
            }
            global_array[i].inner[j].extra = i + j;
        }
    }
    
    for (i = 0; i < 256; i++) {
        output_buffer[i] = 0;
    }
    
    /* Seed with command line or random values */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Execute tests with various parameters to trigger different reload patterns */
    int total = 0;
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 3; k++) {
                int idx1 = (i + j + k) % 16;
                int idx2 = (i * 2 + j + k) % 16;
                int idx3 = (i + j * 3 + k) % 16;
                
                test_input_address(idx1, idx2, idx3);
                test_output_address(idx1, idx2, idx3);
                test_mixed_io(idx1, idx2);
                test_operand_address(global_array, idx1, idx2 % 4, idx3 % 8);
                test_addr_of_addr(idx1);
                
                total += idx1 + idx2 + idx3;
            }
        }
    }
    
    /* Test with loop-based patterns */
    test_other_address_types(8);
    
    /* Final computation using all modified data */
    int final_sum = 0;
    for (i = 0; i < 16; i++) {
        final_sum += global_array[i].base + global_array[i].offset;
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 8; k++) {
                final_sum += global_array[i].inner[j].data[k];
            }
        }
    }
    
    for (i = 0; i < 256; i++) {
        final_sum += output_buffer[i];
    }
    
    printf("Final checksum: %d (total: %d)\n", final_sum, total);
    
    return (final_sum > 0) ? 0 : 1;
}
