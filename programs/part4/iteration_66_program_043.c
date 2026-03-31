/* test_reload_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-strict-aliasing test_reload_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex data structures to force address computations */
struct InnerStruct {
    int member_array[8];
    volatile int* volatile_ptr;
};

struct OuterStruct {
    struct InnerStruct inner[4];
    int base_value;
    volatile int index_hint;
};

/* Global variables to increase register pressure */
volatile int global_index = 0;
volatile int* global_ptr = NULL;
struct OuterStruct global_struct;

/* Function to prevent optimization */
static void use_value(int val) {
    asm volatile("" : : "r"(val) : "memory");
}

/* Test RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct OuterStruct* os, int idx1, int idx2) {
    /* Complex addressing: array[(index << 2) + struct.member] */
    int val;
    
    /* Force address computation with multiple registers */
    asm volatile(
        "movl (%[addr]), %[val]\n\t"
        : [val] "=r"(val)
        : [addr] "r"(&os->inner[idx1].member_array[(idx2 << 2) + os->base_value])
        : "memory"
    );
    
    use_value(val);
    
    /* Another complex input address */
    int* volatile ptr = &os->inner[global_index].member_array[0];
    asm volatile(
        ""
        :
        : "m"(*ptr), "r"(idx1), "r"(idx2)
        : "memory"
    );
}

/* Test RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct OuterStruct* os, int idx, int value) {
    /* Complex output address computation */
    int offset = (idx * 3) + os->base_value;
    
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : "=m"(os->inner[idx >> 1].member_array[offset & 7])
        : [val] "r"(value), [addr] "r"(&os->inner[idx >> 1].member_array[offset & 7])
        : "memory"
    );
    
    /* Mixed input/output with different addressing */
    volatile int* out_ptr = &os->inner[idx].member_array[0];
    asm volatile(
        "addl $1, %0\n\t"
        : "=m"(*out_ptr)
        : "m"(*out_ptr)
        : "memory"
    );
}

/* Test RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
void test_addr_address(struct OuterStruct* os, int idx) {
    /* Force address-of-address computation */
    int** addr_ptr = &os->inner[idx].volatile_ptr;
    int* target;
    
    /* Input address address */
    asm volatile(
        "movq (%[addr_ptr]), %[target]\n\t"
        : [target] "=r"(target)
        : [addr_ptr] "r"(addr_ptr)
        : "memory"
    );
    
    /* Output address address */
    int* new_target = (int*)((uintptr_t)target + sizeof(int));
    asm volatile(
        "movq %[new_target], (%[addr_ptr])\n\t"
        : "=m"(*addr_ptr)
        : [new_target] "r"(new_target), [addr_ptr] "r"(addr_ptr)
        : "memory"
    );
}

/* Test RELOAD_FOR_OPERAND_ADDRESS */
void test_operand_address(struct OuterStruct* os, int idx1, int idx2) {
    /* Complex address passed as function argument (inlined) */
    int* complex_addr = &os->inner[idx1].member_array[
        (idx2 * os->base_value) % 8
    ];
    
    /* Force operand address reload */
    asm volatile(
        "call use_value\n\t"
        :
        : "D"(complex_addr), "S"(*complex_addr)
        : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
    );
}

/* Test RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
void test_other_address(struct OuterStruct* os) {
    /* Multiple memory operands with complex addressing */
    int temp1, temp2, temp3;
    
    asm volatile(
        "movl (%[addr1]), %[t1]\n\t"
        "movl (%[addr2]), %[t2]\n\t"
        "addl %[t1], %[t2]\n\t"
        "movl %[t2], (%[addr3])\n\t"
        : [t1] "=&r"(temp1), [t2] "=&r"(temp2), "=m"(os->inner[0].member_array[7])
        : [addr1] "r"(&os->inner[1].member_array[global_index]),
          [addr2] "r"(&os->inner[2].member_array[os->base_value]),
          [addr3] "r"(&os->inner[0].member_array[7]),
          "m"(os->inner[1].member_array[global_index]),
          "m"(os->inner[2].member_array[os->base_value])
        : "memory"
    );
    
    /* Force other reload types with memory clobber */
    asm volatile(
        ""
        : "=m"(os->inner[3].member_array[0]),
          "=m"(os->inner[3].member_array[1])
        : "m"(os->inner[3].member_array[2]),
          "m"(os->inner[3].member_array[3])
        : "memory"
    );
}

/* Test RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(struct OuterStruct* os, int idx) {
    /* Address of an address computation */
    int (*addr_array)[8] = &os->inner[idx].member_array;
    int* element_ptr;
    
    asm volatile(
        "leaq (%[base], %[idx], 4), %[ptr]\n\t"
        : [ptr] "=r"(element_ptr)
        : [base] "r"(*addr_array), [idx] "r"(idx)
        : "memory"
    );
    
    /* Use the computed address */
    asm volatile(
        "movl $42, (%[ptr])\n\t"
        : "=m"(**addr_array)
        : [ptr] "r"(element_ptr)
        : "memory"
    );
}

/* Mixed test combining multiple reload types */
void test_mixed_reloads(struct OuterStruct* os, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Vary indices to prevent optimization */
        int idx1 = (i * 3) % 4;
        int idx2 = (i * 5) % 8;
        int idx3 = (i * 7) % 4;
        
        /* Chain different reload types */
        test_input_address(os, idx1, idx2);
        test_output_address(os, idx3, i);
        
        if (i % 2 == 0) {
            test_addr_address(os, idx1);
        }
        
        if (i % 3 == 0) {
            test_operand_address(os, idx2, idx3);
        }
        
        /* Update structure to create dependencies */
        os->base_value = (os->base_value + i) & 3;
        global_index = (global_index + 1) & 7;
    }
}

/* Main driver */
int main() {
    /* Initialize test data */
    struct OuterStruct os;
    int checksum = 0;
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            os.inner[i].member_array[j] = i * 10 + j;
        }
        os.inner[i].volatile_ptr = &os.inner[i].member_array[0];
    }
    os.base_value = 2;
    os.index_hint = 0;
    
    global_ptr = &os.inner[0].member_array[0];
    
    /* Run tests to trigger different reload types */
    printf("Starting reload coverage tests...\n");
    
    /* Individual tests for specific reload types */
    test_input_address(&os, 1, 2);
    test_output_address(&os, 2, 42);
    test_addr_address(&os, 3);
    test_operand_address(&os, 0, 1);
    test_other_address(&os);
    test_opaddr_addr(&os, 2);
    
    /* Mixed test to increase register pressure */
    test_mixed_reloads(&os, 10);
    
    /* Compute checksum to ensure code isn't optimized away */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += os.inner[i].member_array[j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}
