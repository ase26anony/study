/* reload_coverage.c - Program to trigger GCC reload pass edge cases */
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
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed_data[8];
    volatile int volatile_var = 42;
    register int reg_var asm ("r12") = 100; /* Try to reserve a register */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 16; i++) ptr_arr[i] = (long*)&arr[i * 16];
    for (int i = 0; i < 8; i++) {
        packed_data[i].c = 'A' + i;
        packed_data[i].i = i * 1000;
        packed_data[i].l = i * 10000LL;
    }
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = (iter * 17) % 256;
        int scale = 1 << (iter % 4);
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex addressing mode with multiple components */
        asm volatile (
            "mov %[val1], %[idx]\n\t"
            "add %[val1], %[base], %[val1], lsl #2\n\t"
            "ldr %[out1], [%[val1], %[offset], lsl #2]\n\t"
            : [out1] "=r" (arr[idx]),
              [val1] "=&r" (reg_var)  /* Early clobber */
            : [base] "r" (arr),
              [idx] "r" (idx),
              [offset] "r" (iter),
              "m" (*arr)              /* Memory constraint */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex address computation */
        long* complex_ptr = &arr[idx] + iter;
        asm volatile (
            "str %[in2], [%[ptr], %[disp], lsl #2]\n\t"
            "add %[ptr], %[ptr], #16\n\t"
            : "=m" (*complex_ptr),
              [ptr] "+&r" (complex_ptr)  /* Early clobber output */
            : [in2] "r" (iter * 7),
              [disp] "r" (scale),
              "m" (*complex_ptr)
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of memory operand with complex addressing */
        int* addr_of_mem;
        asm volatile (
            "adr %[addr], %[mem]\n\t"
            "ldr %[temp], [%[addr]]\n\t"
            "add %[temp], %[temp], %[inc]\n\t"
            "str %[temp], [%[addr]]\n\t"
            : [addr] "=&r" (addr_of_mem),
              [temp] "=&r" (reg_var),
              [mem] "+m" (packed_data[iter % 8].i)
            : [inc] "r" (iter)
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        /* Input address that itself needs reloading */
        int* nested_addr = &arr[idx] + scale;
        asm volatile (
            "ldr %[out3], [%[addr]], #4\n\t"
            "str %[out3], [%[dest]]\n\t"
            : [out3] "=r" (volatile_var),
              [addr] "+&r" (nested_addr),  /* Early clobber */
              [dest] "+m" (global_counter)
            : "r" (nested_addr),           /* Duplicate constraint */
              "m" (*nested_addr)
            : "memory", "cc"
        );
        
        /* Force RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Many operands to cause register pressure */
        asm volatile (
            "mov %[r0], %[a0]\n\t"
            "add %[r1], %[a1], %[a2], lsl %[s0]\n\t"
            "mul %[r2], %[a3], %[a4]\n\t"
            "sub %[r3], %[a5], %[a6]\n\t"
            "orr %[r4], %[a7], %[a8]\n\t"
            : [r0] "=&r" (arr[0]),
              [r1] "=&r" (arr[1]),
              [r2] "=&r" (arr[2]),
              [r3] "=&r" (arr[3]),
              [r4] "=&r" (arr[4])
            : [a0] "r" (iter),
              [a1] "r" (arr[5]),
              [a2] "r" (arr[6]),
              [a3] "r" (arr[7]),
              [a4] "r" (arr[8]),
              [a5] "r" (arr[9]),
              [a6] "r" (arr[10]),
              [a7] "r" (arr[11]),
              [a8] "r" (arr[12]),
              [s0] "r" (scale),
              "m" (arr[5]), "m" (arr[6]), "m" (arr[7]), "m" (arr[8]),
              "m" (arr[9]), "m" (arr[10]), "m" (arr[11]), "m" (arr[12])
            : "memory", "cc"
        );
        
        /* Mix memory and register constraints for same variable */
        int mixed_use = iter * 11;
        asm volatile (
            "add %[var], %[var], %[inc]\n\t"
            "str %[var], [%[ptr]]\n\t"
            : [var] "+r" (mixed_use),
              [ptr] "+&r" (ptr_arr[iter % 16])
            : [inc] "r" (scale),
              "m" (mixed_use),      /* Memory constraint on register var */
              "m" (*ptr_arr[iter % 16])
            : "memory", "cc"
        );
        
        global_counter += iter;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += packed_data[i].i + packed_data[i].l;
    }
    checksum += volatile_var + global_counter + reg_var;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 1000);
}
