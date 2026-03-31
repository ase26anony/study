/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile long global_counter = 0;
volatile int global_index = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    char buffer[256];
    double doubles[20];
    volatile int volatile_var = 42;
    struct misaligned_data packed_data[10];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
        if (i < 50) ptr_arr[i] = (long*)&arr[i];
        if (i < 20) doubles[i] = i * 1.5;
        if (i < 10) {
            packed_data[i].c = i;
            packed_data[i].i = i * 100;
            packed_data[i].l = i * 1000L;
        }
    }
    
    /* Complex addressing mode variables */
    long base_reg = (long)&arr[0];
    long index_reg = 10;
    long scale = 4;
    long displacement = 8;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 50; iter++) {
        /* Change constraints each iteration */
        int constraint_type = iter % 8;
        volatile_var = iter;
        
        switch (constraint_type) {
            case 0: /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                /* Complex addressing with multiple levels of indirection */
                asm volatile (
                    "mov %[val1], %[out1] \n\t"
                    "lea (%[base], %[index], %[scale]), %[addr] \n\t"
                    "mov (%[addr], %[disp]), %[out2] \n\t"
                    : [out1] "=&r" (arr[iter]), 
                      [out2] "=r" (arr[iter + 1]),
                      [addr] "=&r" (base_reg)
                    : [base] "r" (base_reg),
                      [index] "r" (index_reg),
                      [scale] "r" (scale),
                      [disp] "r" (displacement),
                      [val1] "m" (packed_data[iter % 10].i)
                    : "memory", "cc"
                );
                break;
                
            case 1: /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                /* Output with complex address computation */
                asm volatile (
                    "lea (%[base], %[idx], 8), %[tmp] \n\t"
                    "mov %[val], (%[tmp], %[off], 2) \n\t"
                    "add $1, %[idx] \n\t"
                    : [tmp] "=&r" (index_reg),
                      [idx] "+&r" (global_index)
                    : [base] "r" (base_reg),
                      [off] "r" (displacement),
                      [val] "r" (iter)
                    : "memory", "cc"
                );
                break;
                
            case 2: /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                /* Taking address of memory operand that needs reload */
                {
                    long *addr_ptr;
                    asm volatile (
                        "lea %[mem], %[addr] \n\t"
                        "mov (%[addr]), %[val] \n\t"
                        "add %[inc], %[val] \n\t"
                        "mov %[val], (%[addr]) \n\t"
                        : [addr] "=&r" (addr_ptr),
                          [val] "=&r" (global_counter)
                        : [mem] "m" (packed_data[iter % 5].l),
                          [inc] "r" ((long)iter)
                        : "memory", "cc"
                    );
                }
                break;
                
            case 3: /* RELOAD_FOR_INPUT with many operands */
                /* Many input operands to cause register pressure */
                asm volatile (
                    "add %[a], %[sum] \n\t"
                    "add %[b], %[sum] \n\t"
                    "add %[c], %[sum] \n\t"
                    "add %[d], %[sum] \n\t"
                    "add %[e], %[sum] \n\t"
                    "imul %[f], %[sum] \n\t"
                    : [sum] "+&r" (arr[iter])
                    : [a] "r" (arr[iter + 1]),
                      [b] "r" (arr[iter + 2]),
                      [c] "r" (arr[iter + 3]),
                      [d] "r" (arr[iter + 4]),
                      [e] "r" (arr[iter + 5]),
                      [f] "r" (iter)
                    : "cc"
                );
                break;
                
            case 4: /* RELOAD_OTHER with mixed constraints */
                /* Early clobber with overlapping constraints */
                asm volatile (
                    "mov %[in1], %[out1] \n\t"
                    "add %[in2], %[out1] \n\t"
                    "mov %[out1], %[out2] \n\t"
                    "sub %[in3], %[out2] \n\t"
                    : [out1] "=&r" (arr[iter]),
                      [out2] "=&r" (arr[iter + 10])
                    : [in1] "m" (packed_data[iter % 10].i),
                      [in2] "r" (volatile_var),
                      [in3] "rm" (iter * 2)
                    : "cc"
                );
                break;
                
            case 5: /* RELOAD_FOR_OTHER_ADDRESS with memory clobber */
                /* Complex memory operation with full clobber */
                {
                    long complex_addr;
                    asm volatile (
                        "mov %[base], %[addr] \n\t"
                        "add %[idx], %[addr] \n\t"
                        "shl $3, %[addr] \n\t"
                        "mov (%[addr]), %[tmp] \n\t"
                        "add %[tmp], %[total] \n\t"
                        : [addr] "=&r" (complex_addr),
                          [total] "+&r" (global_counter)
                        : [base] "r" (base_reg),
                          [idx] "r" (index_reg)
                        : "memory", "cc"
                    );
                }
                break;
                
            case 6: /* Multiple address reloads with nesting */
                /* Nested address computation */
                {
                    long *ptr1, *ptr2;
                    asm volatile (
                        "lea (%[base], %[idx1], 4), %[p1] \n\t"
                        "lea (%[p1], %[idx2], 2), %[p2] \n\t"
                        "mov (%[p2]), %[val] \n\t"
                        "add %[val], (%[p1]) \n\t"
                        : [p1] "=&r" (ptr1),
                          [p2] "=&r" (ptr2),
                          [val] "=&r" (displacement)
                        : [base] "r" (arr),
                          [idx1] "r" (iter),
                          [idx2] "r" (iter * 2)
                        : "memory", "cc"
                    );
                }
                break;
                
            case 7: /* Maximum register pressure */
                /* Many output registers forcing spills */
                asm volatile (
                    "mov %[in1], %[out1] \n\t"
                    "mov %[in2], %[out2] \n\t"
                    "mov %[in3], %[out3] \n\t"
                    "mov %[in4], %[out4] \n\t"
                    "add %[out1], %[out2] \n\t"
                    "add %[out3], %[out4] \n\t"
                    "imul %[out2], %[out4] \n\t"
                    : [out1] "=&r" (arr[iter]),
                      [out2] "=&r" (arr[iter + 1]),
                      [out3] "=&r" (arr[iter + 2]),
                      [out4] "=&r" (arr[iter + 3])
                    : [in1] "m" (packed_data[0].i),
                      [in2] "m" (packed_data[1].i),
                      [in3] "m" (packed_data[2].i),
                      [in4] "m" (packed_data[3].i)
                    : "cc"
                );
                break;
        }
        
        /* Modify addressing parameters for next iteration */
        index_reg = (index_reg + 1) % 40;
        displacement = (displacement + 4) % 32;
        scale = (scale == 4) ? 8 : 4;
    }
    
    /* Compute checksum to ensure all operations have effect */
    long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += arr[i];
    }
    checksum += global_counter;
    checksum += volatile_var;
    
    for (int i = 0; i < 10; i++) {
        checksum += packed_data[i].i;
        checksum += packed_data[i].l;
    }
    
    printf("Checksum: %ld\n", checksum);
    return (checksum != 0) ? 0 : 1;
}
