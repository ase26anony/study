/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char trailing;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 12345;  /* Force register usage */
    volatile int vol_var = 67890;
    int auto_var = 13579;
    long long_var = 24680;
    char char_array[64] = {0};
    int int_array[32] = {0};
    long *ptr_array[16];
    
    /* Misaligned struct */
    struct misaligned_data packed_struct = {0};
    
    /* Variables for address computations */
    int base_index = 8;
    int scale_factor = 4;
    int displacement = 0x100;
    
    /* Result accumulator to ensure side effects */
    unsigned long long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        displacement += 0x20;
        scale_factor = 2 + (iteration % 3);
        
        /* 
         * RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS
         * Complex addressing mode with multiple components
         */
        asm volatile (
            "mov %[dest], qword ptr [%[base] + %[index]*%[scale] + %[disp]] \n\t"
            : [dest] "=r" (long_var)
            : [base] "r" (int_array), 
              [index] "r" (base_index),
              [scale] "i" (scale_factor),
              [disp] "i" (displacement)
            : "memory"
        );
        
        /* 
         * RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
         * Complex store with early-clobber
         */
        int temp_index = iteration * 2;
        asm volatile (
            "mov qword ptr [%[base] + %[idx]*8 + 64], %[val] \n\t"
            : "=m" (*(long (*)[32])global_array)
            : [base] "r" (global_array),
              [idx] "r" (temp_index),
              [val] "r" (long_var),
              "m" (global_array)
            : "memory"
        );
        
        /* 
         * RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT with register pressure
         * More operands than available registers in a class
         */
        int r1, r2, r3, r4, r5, r6, r7, r8;
        asm volatile (
            "mov %0, %1 \n\t"
            "add %0, %2 \n\t"
            "mov %3, %0 \n\t"
            "imul %4, %5 \n\t"
            "add %6, %7 \n\t"
            "sub %8, %9 \n\t"
            : "=&r" (r1), "=&r" (r2), "=r" (r3), "=r" (r4),
              "=r" (r5), "=r" (r6), "=r" (r7), "=r" (r8)
            : "r" (reg_var), "r" (vol_var), 
              "r" (auto_var), "r" (iteration),
              "m" (packed_struct.i), "m" (packed_struct.l),
              "0" (r1), "1" (r2)
            : "cc"
        );
        
        /* 
         * RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
         * Taking address of complex memory operand
         */
        long *addr1, *addr2;
        asm volatile (
            "lea %0, [%[base] + %[idx]*8 + 16] \n\t"
            "lea %1, [%0 + %[off]] \n\t"
            : "=&r" (addr1), "=r" (addr2)
            : [base] "r" (global_array),
              [idx] "r" (iteration),
              [off] "r" (displacement)
            : "memory"
        );
        
        /* 
         * RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS
         * Mixed-size operands with memory clobber
         */
        char *char_ptr = char_array + iteration;
        asm volatile (
            "movzx %k0, byte ptr [%[ptr] + %[idx]*2] \n\t"
            "add %k0, dword ptr [%[arr] + %[idx]*4] \n\t"
            "mov byte ptr [%[ptr]], %b0 \n\t"
            : "=&r" (auto_var)
            : [ptr] "r" (char_ptr),
              [idx] "r" (iteration),
              [arr] "r" (int_array),
              "m" (*(char (*)[64])char_array),
              "m" (*(int (*)[32])int_array)
            : "memory", "cc"
        );
        
        /* Nested addressing with pointer chasing */
        ptr_array[iteration % 16] = &long_var;
        long **ptr_ptr = &ptr_array[iteration % 16];
        
        asm volatile (
            "mov %[val], [%[ptrptr]] \n\t"
            "mov %[val], [%[val]] \n\t"
            : [val] "=r" (long_var)
            : [ptrptr] "r" (ptr_ptr),
              "m" (ptr_array)
            : "memory"
        );
        
        /* Update checksum with all modified variables */
        checksum += reg_var + vol_var + auto_var + long_var;
        checksum += (uintptr_t)addr1 + (uintptr_t)addr2;
        checksum += r1 + r2 + r3 + r4;
        
        /* Change constraints for next iteration */
        if (iteration % 2) {
            /* Switch to memory operands */
            asm volatile (
                "add dword ptr [%[addr]], %[inc] \n\t"
                : "=m" (packed_struct.i)
                : [addr] "r" (&packed_struct.i),
                  [inc] "ri" (iteration)
                : "memory", "cc"
            );
        }
        
        global_counter++;
    }
    
    /* Final computation to ensure all asm statements have effect */
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    checksum += packed_struct.i + packed_struct.l;
    
    /* Use the result to prevent dead code elimination */
    printf("Reload coverage checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return (checksum > 1000000) ? 0 : 1;
}
