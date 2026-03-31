/* test_reload_coverage.c
 * Designed to trigger various reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
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
    int base_array[16];
    volatile long offset;
};

/* Global volatile variables to prevent optimization */
volatile int global_index = 0;
volatile int global_offset = 0;
volatile struct outer_struct* volatile global_struct = NULL;

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) 
use_complex_address(struct inner_struct* ptr, int idx1, int idx2) {
    /* Complex addressing that may need operand address reload */
    asm volatile("" 
                 : 
                 : "r"(ptr), "r"(idx1), "r"(idx2), 
                   "m"(ptr->member_array[idx1 + idx2])
                 : "memory");
}

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void __attribute__((noinline))
test_input_address(struct outer_struct* s, int i, int j, int k) {
    /* Multiple levels of indirection requiring input address computation */
    int val;
    
    /* Complex addressing: s->inner[i].member_array[j + k] */
    asm volatile("movl (%[addr]), %[val]\n\t"
                 : [val]"=r"(val)
                 : [addr]"r"(&s->inner[i].member_array[j + k])
                 : "memory");
    
    /* Use the value to prevent dead code elimination */
    global_index += val;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void __attribute__((noinline))
test_output_address(struct outer_struct* s, int idx, int offset) {
    /* Output to memory with complex address computation */
    int result = idx * 2 + offset;
    
    /* Output to computed address: s->base_array[idx << 2] */
    asm volatile("movl %[res], (%[addr])\n\t"
                 : 
                 : [res]"r"(result), 
                   [addr]"r"(&s->base_array[(idx << 2) + s->offset])
                 : "memory");
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void __attribute__((noinline))
test_mixed_address_reloads(struct outer_struct* s, 
                          volatile int* input_ptr,
                          volatile int* output_ptr,
                          int scale) {
    int temp;
    
    /* Input with complex address (may trigger INPADDR_ADDRESS) */
    asm volatile("movl (%[in]), %[temp]\n\t"
                 : [temp]"=r"(temp)
                 : [in]"r"(&input_ptr[s->inner[scale].member_array[scale]])
                 : "memory");
    
    /* Output with different complex address (may trigger OUTADDR_ADDRESS) */
    asm volatile("movl %[temp], (%[out])\n\t"
                 : 
                 : [temp]"r"(temp + scale),
                   [out]"r"(&output_ptr[(scale * 3) + global_offset])
                 : "memory");
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void __attribute__((noinline))
test_other_address(struct outer_struct* s, int* ptr_array[], int count) {
    /* Complex addressing in loop - may trigger OTHER_ADDRESS */
    for (int i = 0; i < count; i++) {
        /* Address depends on previous computation */
        int* addr = &ptr_array[i][s->inner[i % 4].member_array[i % 8]];
        
        asm volatile("" 
                     : 
                     : "r"(addr), "m"(*addr)
                     : "memory");
    }
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void __attribute__((noinline))
test_opaddr_addr(struct outer_struct* s, int idx) {
    /* Nested addressing that may require operand address reloads */
    struct inner_struct* inner_ptr = &s->inner[idx & 3];
    
    /* Complex address of address computation */
    asm volatile("" 
                 : 
                 : "r"(&inner_ptr->member_array[idx]),
                   "r"(&inner_ptr->volatile_ptr),
                   "m"(inner_ptr->member_array[idx])
                 : "memory");
}

/* Main test driver */
int main() {
    /* Initialize test data */
    struct outer_struct data;
    volatile int input_buffer[64];
    volatile int output_buffer[64];
    int* ptr_array[8];
    
    /* Initialize structures */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            data.inner[i].member_array[j] = i * 8 + j;
        }
        data.inner[i].volatile_ptr = &input_buffer[i * 8];
    }
    
    for (int i = 0; i < 16; i++) {
        data.base_array[i] = i * 3;
    }
    
    data.offset = 4;
    
    /* Initialize buffers */
    for (int i = 0; i < 64; i++) {
        input_buffer[i] = i * 2;
        output_buffer[i] = 0;
    }
    
    for (int i = 0; i < 8; i++) {
        ptr_array[i] = (int*)&input_buffer[i * 8];
    }
    
    /* Set global variables */
    global_struct = &data;
    global_offset = 8;
    
    int checksum = 0;
    
    /* Test 1: Input address reloads */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            test_input_address(&data, i, j, i + j);
        }
    }
    checksum += global_index;
    
    /* Test 2: Output address reloads */
    for (int i = 0; i < 8; i++) {
        test_output_address(&data, i, i * 2);
    }
    
    /* Test 3: Mixed address reloads */
    for (int i = 0; i < 4; i++) {
        test_mixed_address_reloads(&data, 
                                  input_buffer, 
                                  output_buffer, 
                                  i);
    }
    
    /* Test 4: Operand address reloads */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            use_complex_address(&data.inner[i], j, i);
        }
    }
    
    /* Test 5: Other address reloads */
    test_other_address(&data, ptr_array, 8);
    
    /* Test 6: Opaddr address reloads */
    for (int i = 0; i < 8; i++) {
        test_opaddr_addr(&data, i);
    }
    
    /* Verify results by computing checksum */
    for (int i = 0; i < 16; i++) {
        checksum += data.base_array[i];
    }
    
    for (int i = 0; i < 64; i++) {
        checksum += output_buffer[i];
    }
    
    printf("Reload test checksum: %d\n", checksum);
    printf("If this prints, compilation succeeded with reload activity\n");
    
    return 0;
}
