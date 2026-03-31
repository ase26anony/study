/* test_reload_coverage.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Specifically targets the switch cases in chain_reload_insns()
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Complex nested structure to force address computations */
struct inner {
    int data[8];
    int *ptr;
};

struct outer {
    struct inner arrays[4];
    int base;
    volatile int offset; /* volatile to prevent optimization */
};

/* Global variables to increase register pressure */
volatile int global_index1, global_index2;
struct outer global_struct;

/* Function to force RELOAD_FOR_OPERAND_ADDRESS */
void __attribute__((noinline)) 
use_complex_address(struct inner *addr) {
    asm volatile("" : : "r"(addr) : "memory");
}

/* Function to force RELOAD_FOR_INPUT_ADDRESS */
void test_input_address(struct outer *s, int idx1, int idx2) {
    /* Complex addressing: s->arrays[idx1].data[idx2] */
    /* This requires computing the address before accessing */
    int val;
    
    /* Force address computation with inline asm */
    asm volatile(
        "movl (%[addr]), %[val]\n\t"
        : [val] "=r"(val)
        : [addr] "r"(&s->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Use the value to prevent optimization */
    global_struct.base += val;
}

/* Function to force RELOAD_FOR_OUTPUT_ADDRESS */
void test_output_address(struct outer *s, int idx1, int idx2, int value) {
    /* Complex addressing for output */
    asm volatile(
        "movl %[val], (%[addr])\n\t"
        : 
        : [val] "r"(value), [addr] "r"(&s->arrays[idx1].data[idx2])
        : "memory"
    );
}

/* Function to force RELOAD_FOR_INPADDR_ADDRESS */
void test_inpaddr_address(struct outer *s, int idx) {
    /* Taking address of a complex expression as input */
    int *addr = &s->arrays[idx].data[idx * 2];
    
    /* Use the address in inline asm with memory constraint */
    asm volatile(
        ""
        : 
        : "m"(*addr), "r"(addr)
        : 
    );
}

/* Function to force RELOAD_FOR_OUTADDR_ADDRESS */
void test_outaddr_address(struct outer *s, int idx, int **out_addr) {
    /* Computing address for output */
    *out_addr = &s->arrays[idx].data[global_index1 + global_index2];
    
    /* Force address computation before use */
    asm volatile(
        ""
        : "=m"(**out_addr)
        : "r"(out_addr)
        : 
    );
}

/* Function to force RELOAD_FOR_OTHER_ADDRESS */
void test_other_address(struct outer *s) {
    /* Mixed addressing modes in loop */
    for (int i = 0; i < 4; i++) {
        /* Complex index calculation */
        int idx = (i * global_index1 + global_index2) & 3;
        
        /* Multiple memory accesses with complex addresses */
        asm volatile(
            "movl (%[addr1]), %%eax\n\t"
            "addl %%eax, (%[addr2])\n\t"
            : 
            : [addr1] "r"(&s->arrays[i].data[0]),
              [addr2] "r"(&s->arrays[idx].data[3])
            : "eax", "memory"
        );
    }
}

/* Function to force RELOAD_FOR_OPADDR_ADDR */
void test_opaddr_addr(void) {
    /* Address of address computation */
    int array[16];
    volatile int *ptr_array[4];
    
    for (int i = 0; i < 4; i++) {
        /* Complex addressing for pointer array */
        ptr_array[i] = &array[(i * 5 + global_index1) % 16];
        
        /* Use in inline asm */
        asm volatile(
            ""
            : "=m"(ptr_array[i])
            : "r"(&ptr_array[i])
            : 
        );
    }
}

/* Main test driver with mixed reload types */
void __attribute__((noinline))
run_comprehensive_test(struct outer *s) {
    int temp = 0;
    int *temp_addr;
    
    /* Force various reload types in sequence */
    
    /* 1. RELOAD_FOR_INPUT_ADDRESS */
    test_input_address(s, global_index1 & 3, global_index2 & 7);
    
    /* 2. RELOAD_FOR_OUTPUT_ADDRESS */
    test_output_address(s, (global_index1 + 1) & 3, 
                       (global_index2 + 2) & 7, 42);
    
    /* 3. RELOAD_FOR_INPADDR_ADDRESS */
    test_inpaddr_address(s, global_index1 & 3);
    
    /* 4. RELOAD_FOR_OUTADDR_ADDRESS */
    test_outaddr_address(s, global_index2 & 3, &temp_addr);
    
    /* 5. RELOAD_FOR_OPERAND_ADDRESS */
    use_complex_address(&s->arrays[(global_index1 + global_index2) & 3]);
    
    /* 6. RELOAD_FOR_OTHER_ADDRESS */
    test_other_address(s);
    
    /* 7. RELOAD_FOR_OPADDR_ADDR */
    test_opaddr_addr();
    
    /* Use results to prevent optimization */
    s->base += *temp_addr;
}

/* Additional test with register pressure */
void __attribute__((noinline))
high_register_pressure_test(void) {
    /* Many live variables to force spills */
    int a = global_index1;
    int b = global_index2;
    int c = a + b;
    int d = a * b;
    int e = a ^ b;
    int f = a | b;
    int g = a & b;
    
    struct outer local_struct;
    
    /* Complex addressing with many live variables */
    for (int i = 0; i < 8; i++) {
        /* Use all variables in address computation */
        int idx = (a * i + b * c + d * e + f * g) & 3;
        
        /* Force memory access with complex address */
        asm volatile(
            "addl $1, (%[addr])\n\t"
            : 
            : [addr] "r"(&local_struct.arrays[idx].data[i & 7])
            : "memory"
        );
        
        /* Modify variables to keep them live */
        a += i;
        b ^= i;
        c = a + b;
        d = a * b;
    }
    
    /* Use variables to prevent optimization */
    global_struct.base += a + b + c;
}

int main(void) {
    struct outer test_struct;
    int checksum = 0;
    
    /* Initialize test data */
    global_index1 = 1;
    global_index2 = 2;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            test_struct.arrays[i].data[j] = i * 10 + j;
        }
    }
    test_struct.base = 0;
    test_struct.offset = 3;
    
    /* Run tests multiple times with different indices */
    for (int iter = 0; iter < 10; iter++) {
        global_index1 = (global_index1 * 13 + 7) & 0xFF;
        global_index2 = (global_index2 * 17 + 11) & 0xFF;
        
        run_comprehensive_test(&test_struct);
        high_register_pressure_test();
        
        checksum += test_struct.base + global_index1 + global_index2;
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    return 0;
}
