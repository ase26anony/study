/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
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

/* Global variables to prevent optimization */
volatile int global_index = 0;
volatile int* volatile global_ptr = NULL;
struct outer_struct global_nested[16];

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer_struct* arr, int idx1, int idx2, int idx3) {
    /* Complex addressing: arr[idx1].inner[idx2].member_array[idx3] */
    asm volatile(
        "/* Input address computation */\n\t"
        : /* no outputs */
        : "m" (arr[idx1].inner[idx2].member_array[idx3]),
          "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    /* Multiple levels of indirection */
    int temp = idx1 * idx2 + idx3;
    asm volatile(
        "/* Nested input address */\n\t"
        : 
        : "m" (arr[temp >> 2].inner[temp & 3].member_array[global_index]),
          "r" (temp)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer_struct* arr, int idx1, int idx2, int idx3, int value) {
    /* Complex output addressing */
    asm volatile(
        "/* Output address computation */\n\t"
        : "=m" (arr[idx1].inner[idx2].member_array[idx3])
        : "r" (value), "r" (idx1), "r" (idx2), "r" (idx3)
        : "memory"
    );
    
    /* Output with shifted index */
    int offset = (idx1 << 2) + idx2;
    asm volatile(
        "/* Shifted output address */\n\t"
        : "=m" (arr->inner[0].member_array[offset])
        : "r" (value), "r" (offset)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_mixed_address(struct outer_struct* arr, int* results, int count) {
    for (int i = 0; i < count; i++) {
        int idx1 = (i * 3) % 16;
        int idx2 = (i * 5) % 4;
        int idx3 = (i * 7) % 8;
        
        /* Mixed input/output with complex addressing */
        asm volatile(
            "/* Mixed input/output addressing */\n\t"
            : "=m" (results[i]),
              "=m" (arr[idx1].inner[idx2].member_array[idx3])
            : "m" (arr[(i+1)%16].inner[(i+2)%4].member_array[(i+3)%8]),
              "r" (i), "r" (idx1), "r" (idx2), "r" (idx3)
            : "memory"
        );
    }
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void helper_function(int* addr1, int* addr2, int* addr3) {
    /* Force address computations before call */
    asm volatile(
        "/* Using computed addresses */\n\t"
        : 
        : "r" (addr1), "r" (addr2), "r" (addr3)
        : "memory"
    );
}

void test_operand_address(struct outer_struct* arr, int idx) {
    /* Complex address expressions as function arguments */
    helper_function(
        &arr[idx].inner[global_index % 4].member_array[0],
        &arr[(idx + 1) % 16].inner[(global_index + 1) % 4].member_array[3],
        &arr[(idx + 2) % 16].base_value
    );
    
    /* More complex nested addressing */
    helper_function(
        &arr[idx >> 2].inner[idx & 3].member_array[global_index & 7],
        &global_nested[global_index].inner[0].member_array[idx],
        (int*)&global_nested[idx].index_hint
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct outer_struct* arr, int* buffer, int size) {
    /* Unusual addressing patterns */
    for (int i = 0; i < size; i++) {
        /* Address computation with multiple operations */
        int complex_idx = ((i * 11) + (global_index * 13)) % 16;
        
        asm volatile(
            "/* Other address pattern */\n\t"
            : "=m" (buffer[i])
            : "m" (arr[complex_idx].inner[i % 4].member_array[(i * 17) % 8]),
              "r" (i), "r" (complex_idx)
            : "memory"
        );
        
        /* Chain of address computations */
        if (i > 0) {
            asm volatile(
                "/* Chained address computation */\n\t"
                : 
                : "m" (buffer[i - 1]),
                  "m" (arr[complex_idx].base_value)
                : "memory"
            );
        }
    }
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(struct outer_struct* arr, int idx) {
    /* Address of address computation */
    int** addr_ptr = &global_ptr;
    
    asm volatile(
        "/* Address of address */\n\t"
        : "=m" (*addr_ptr)
        : "m" (arr[idx].inner[0].volatile_ptr),
          "r" (addr_ptr), "r" (idx)
        : "memory"
    );
    
    /* Complex pointer chain */
    volatile int*** ptr_chain = (volatile int***)&global_nested[0].inner[0].volatile_ptr;
    asm volatile(
        "/* Pointer chain */\n\t"
        : 
        : "m" (ptr_chain),
          "m" (arr[idx].inner[1].member_array[0])
        : "memory"
    );
}

/* Main driver function */
int main() {
    /* Initialize test data */
    struct outer_struct test_array[16];
    int results[32];
    
    for (int i = 0; i < 16; i++) {
        test_array[i].base_value = i * 100;
        test_array[i].index_hint = i;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                test_array[i].inner[j].member_array[k] = i * 1000 + j * 100 + k;
            }
            test_array[i].inner[j].volatile_ptr = &global_index;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        global_nested[i].base_value = i * 200;
        global_nested[i].index_hint = i * 2;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                global_nested[i].inner[j].member_array[k] = i * 2000 + j * 200 + k;
            }
            global_nested[i].inner[j].volatile_ptr = &global_index;
        }
    }
    
    /* Call test functions with various parameters to trigger different reload types */
    
    /* Test 1: Input address reloads */
    for (int i = 0; i < 8; i++) {
        test_input_address(test_array, i, i % 4, i % 8);
    }
    
    /* Test 2: Output address reloads */
    for (int i = 0; i < 8; i++) {
        test_output_address(test_array, i, (i + 1) % 4, (i + 2) % 8, i * 50);
    }
    
    /* Test 3: Mixed address reloads */
    test_mixed_address(test_array, results, 16);
    
    /* Test 4: Operand address reloads */
    for (int i = 0; i < 4; i++) {
        test_operand_address(test_array, i * 3);
    }
    
    /* Test 5: Other address reloads */
    test_other_address(test_array, results + 16, 16);
    
    /* Test 6: Opaddr address reloads */
    for (int i = 0; i < 4; i++) {
        test_opaddr_addr(test_array, i * 2);
    }
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += test_array[i].base_value;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 8; k++) {
                checksum += test_array[i].inner[j].member_array[k];
            }
        }
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
