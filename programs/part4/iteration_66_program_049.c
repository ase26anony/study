/* test_reload_coverage.c
 * 
 * This program is designed to trigger various reload types in GCC's reload pass,
 * specifically targeting the switch cases in chain_reload_insns() function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify addressing modes */
#define NOOPT __attribute__((optimize("O0")))

/* Complex data structures to force complex addressing */
struct inner_struct {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct outer_struct {
    struct inner_struct inner[4];
    int base_array[16];
    volatile int index;
};

/* Global variables to increase register pressure */
volatile int global_index1 = 0;
volatile int global_index2 = 0;
volatile int* global_ptr = NULL;
struct outer_struct global_struct;

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
NOOPT void test_input_address(struct outer_struct* s, int idx1, int idx2) {
    /* Complex addressing: array[struct.member + (index << shift)] */
    int val;
    
    /* Force address computation with multiple register values */
    asm volatile (
        "movl %[result], %[val]\n\t"
        : [val] "=r" (val)
        : [result] "m" (s->inner[idx1].member_array[(idx2 << 1) + s->index]),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Another complex addressing pattern */
    asm volatile (
        ""
        :: "m" (s->base_array[idx1 * 3 + idx2]),
           "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    global_ptr = &val;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
NOOPT void test_output_address(struct outer_struct* s, int idx1, int idx2, int value) {
    /* Output to memory with complex address computation */
    asm volatile (
        "movl %[value], %[dest]\n\t"
        : [dest] "=m" (s->inner[idx1].member_array[(idx2 << 2) + global_index1])
        : [value] "r" (value),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
    
    /* Mixed input/output with different addressing */
    asm volatile (
        "addl $1, %[dest]\n\t"
        : [dest] "=m" (s->base_array[idx1 + (global_index2 * 2)])
        : "m" (s->base_array[idx2]),
          "r" (idx1), "r" (idx2)
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
NOOPT void test_operand_address(struct outer_struct* s, int idx) {
    /* Taking address of complex expression as function argument */
    void (*helper)(int*) = (void(*)(int*))0x1234; /* Dummy function pointer */
    
    /* Force address computation before "call" */
    asm volatile (
        "leal %[addr], %%eax\n\t"
        "pushl %%eax\n\t"
        "call *%[func]\n\t"
        "addl $4, %%esp\n\t"
        :: [addr] "m" (s->inner[idx].member_array[s->index]),
           [func] "r" (helper)
        : "eax", "memory"
    );
    
    /* Another operand address pattern */
    asm volatile (
        ""
        :: "r" (&s->inner[(idx + global_index1) % 4].member_array[0])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
NOOPT void test_inpaddr_outaddr(struct outer_struct* s, int idx1, int idx2) {
    int temp;
    
    /* Input address reload */
    asm volatile (
        "movl (%%ebx), %%eax\n\t"
        "movl %%eax, %[temp]\n\t"
        : [temp] "=r" (temp)
        : "b" (&s->inner[idx1].member_array[idx2 + global_index1]),
          "m" (s->inner[idx1].member_array[idx2 + global_index1])
        : "eax", "memory"
    );
    
    /* Output address reload */
    asm volatile (
        "movl %[val], (%%ecx)\n\t"
        : 
        : [val] "r" (temp + 1),
          "c" (&s->base_array[idx1 * 2 + idx2]),
          "m" (s->base_array[idx1 * 2 + idx2])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
NOOPT void test_other_address(struct outer_struct* s) {
    /* Complex addressing in loop to force various reloads */
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 4; j++) {
            /* Mix different addressing modes */
            asm volatile (
                "imull $3, %[i], %%eax\n\t"
                "addl %[j], %%eax\n\t"
                "movl (%%eax, %[base], 4), %%ebx\n\t"
                : 
                : [i] "r" (i), [j] "r" (j),
                  [base] "r" (s->base_array)
                : "eax", "ebx", "memory"
            );
        }
    }
}

/* Function to force RELOAD_OTHER type */
NOOPT void test_reload_other(struct outer_struct* s, int idx) {
    /* Multiple memory operations with register pressure */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r1 = idx * 2;
    r2 = idx * 3;
    r3 = idx * 4;
    
    /* Force spilling of all registers */
    asm volatile (
        "movl %[r1], %[mem1]\n\t"
        "movl %[r2], %[mem2]\n\t"
        "movl %[r3], %[mem3]\n\t"
        : [mem1] "=m" (s->base_array[0]),
          [mem2] "=m" (s->base_array[1]),
          [mem3] "=m" (s->base_array[2])
        : [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3)
        : "memory"
    );
    
    /* More operations to create chain */
    asm volatile (
        ""
        :: "m" (s->inner[0]), "m" (s->inner[1]), "m" (s->inner[2])
        : "memory"
    );
}

/* Main driver function */
int main() {
    struct outer_struct local_struct;
    int i, j;
    
    /* Initialize structure */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            local_struct.inner[i].member_array[j] = i * 100 + j;
        }
        local_struct.inner[i].volatile_ptr = &global_index1;
    }
    
    for (i = 0; i < 16; i++) {
        local_struct.base_array[i] = i * 10;
    }
    
    local_struct.index = 2;
    global_index1 = 1;
    global_index2 = 3;
    global_ptr = local_struct.base_array;
    
    /* Call all test functions to trigger different reload types */
    test_input_address(&local_struct, 1, 2);
    test_output_address(&local_struct, 0, 3, 999);
    test_operand_address(&local_struct, 2);
    test_inpaddr_outaddr(&local_struct, 1, 2);
    test_other_address(&local_struct);
    test_reload_other(&local_struct, 5);
    
    /* Compute checksum to ensure all operations have effect */
    int checksum = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            checksum += local_struct.inner[i].member_array[j];
        }
    }
    
    for (i = 0; i < 16; i++) {
        checksum += local_struct.base_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global index1: %d\n", global_index1);
    printf("Global index2: %d\n", global_index2);
    
    return 0;
}
