/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char tail;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main() {
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 12345;  /* Fixed register to create pressure */
    auto int auto_var = 67890;
    volatile int vol_var = 13579;
    static long static_array[16];
    
    /* Misaligned struct */
    struct misaligned_data packed = {.c = 'A', .i = -42, .l = 999999999, .tail = 'Z'};
    
    /* Pointers with different properties */
    int *restrict ptr1 = &auto_var;
    volatile int *ptr2 = &vol_var;
    char *byte_ptr = (char *)&packed + 1;  /* Misaligned pointer */
    
    /* Array for complex addressing */
    long base_array[100];
    for (int i = 0; i < 100; i++) {
        base_array[i] = i * 3 + 1;
    }
    
    /* Index variables with different types */
    char index8 = 10;
    short index16 = 20;
    int index32 = 30;
    long index64 = 40;
    
    /* Result accumulator */
    unsigned long long checksum = 0;
    
    /* Loop to vary constraints and trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Modify variables to prevent optimization */
        reg_var += iteration;
        auto_var ^= iteration * 7;
        vol_var = iteration;
        global_counter++;
        
        /* 
         * STRATEGY 1: Complex addressing modes with multiple memory operands
         * Forces: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS
         */
        asm volatile (
            /* Complex addressing: [base + index*scale + displacement] */
            "mov %[val1], %[base1]\n\t"
            "add %[val2], %[base2]\n\t"
            : [val1] "+r" (auto_var), [val2] "+r" (reg_var)
            : [base1] "m" (base_array[index32 * 2 + index16]), 
              [base2] "m" (global_array[index64 * 1 + 8]),
              "r" (index32), "r" (index64)
            : "memory", "cc"
        );
        
        /* 
         * STRATEGY 2: Multiple operands exceeding available registers
         * Forces: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER
         */
        long temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "lea (%[in2], %[in3], 4), %[out2]\n\t"
            "imul %[in4], %[out3]\n\t"
            : [out1] "=&r" (temp1),   /* Early clobber */
              [out2] "=&r" (temp2),   /* Early clobber */
              [out3] "=r" (temp3),
              [out4] "=r" (temp4),
              [out5] "=r" (temp5),
              [out6] "=r" (temp6),
              [out7] "=r" (temp7),
              [out8] "=r" (temp8)
            : [in1] "r" (auto_var),
              [in2] "r" (reg_var),
              [in3] "r" (index32),
              [in4] "m" (static_array[5]),  /* Memory operand */
              [in5] "r" (vol_var),
              [in6] "r" (global_counter),
              [in7] "i" (256),              /* Immediate */
              [in8] "m" (packed.i)          /* Misaligned memory */
            : "memory", "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r13", "r14", "r15"
        );
        
        /* 
         * STRATEGY 3: Nested address computation
         * Forces: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR
         */
        long *addr_ptr;
        asm volatile (
            /* Take address of complex memory operand */
            "lea %[complex_addr], %[ptr]\n\t"
            /* Use that address in another operation */
            "mov (%[ptr]), %[val]\n\t"
            : [ptr] "=r" (addr_ptr),
              [val] "=r" (temp1)
            : [complex_addr] "m" (base_array[index32 + index16 * 2 + 8]),
              "0" (addr_ptr)  /* Same as output */
            : "memory"
        );
        
        /* 
         * STRATEGY 4: Output address reloads
         * Forces: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS
         */
        long output_array[4] = {0};
        asm volatile (
            /* Store with complex addressing */
            "mov %[src], %[dst1]\n\t"
            "mov %[src2], %[dst2]\n\t"
            : [dst1] "=m" (output_array[index8 + 1]),
              [dst2] "=m" (global_array[index16 * 2])
            : [src] "r" (auto_var),
              [src2] "r" (reg_var),
              "r" (index8), "r" (index16)
            : "memory"
        );
        
        /* 
         * STRATEGY 5: Mixed data types and alignment
         * Forces: RELOAD_FOR_OTHER_ADDRESS
         */
        asm volatile (
            "movzbl %[char_ptr], %%eax\n\t"
            "addl %%eax, %[int_val]\n\t"
            "movslq %[int_val], %%rax\n\t"
            "addq %%rax, %[long_val]\n\t"
            : [int_val] "+r" (auto_var),
              [long_val] "+r" (reg_var)
            : [char_ptr] "m" (*byte_ptr),
              "m" (packed.l)  /* Misaligned long */
            : "memory", "cc", "rax"
        );
        
        /* Vary constraints each iteration */
        switch (iteration % 4) {
            case 0:
                /* More memory constraints */
                asm volatile (
                    "mov %1, %0\n\t"
                    : "=r" (temp1)
                    : "m" (base_array[index32]), "m" (global_array[index64])
                    : "memory"
                );
                break;
            case 1:
                /* More register constraints */
                asm volatile (
                    "imul %1, %0\n\t"
                    "add %2, %0\n\t"
                    : "+r" (reg_var)
                    : "r" (auto_var), "r" (vol_var), "r" (global_counter)
                    : "cc"
                );
                break;
            case 2:
                /* Mixed constraints with early clobber */
                asm volatile (
                    "mov %2, %0\n\t"
                    "add %3, %1\n\t"
                    : "=&r" (temp1), "+r" (temp2)
                    : "m" (static_array[3]), "r" (index32),
                      "m" (packed.i)  /* Misaligned */
                    : "memory"
                );
                break;
            case 3:
                /* Output to memory with complex address */
                asm volatile (
                    "mov %1, %0\n\t"
                    : "=m" (base_array[index64 / 2 + index32])
                    : "r" (reg_var)
                    : "memory"
                );
                break;
        }
        
        /* Update checksum with all modified values */
        checksum += auto_var;
        checksum += reg_var;
        checksum += vol_var;
        checksum += temp1;
        checksum += (unsigned long)byte_ptr;
        checksum += iteration;
    }
    
    /* Final computation to ensure all asm has side effects */
    checksum += global_counter * 1000;
    checksum += (unsigned long long)&packed;
    
    /* Print result to prevent dead code elimination */
    printf("Reload coverage test checksum: %llu\n", checksum);
    
    return (checksum > 1000000) ? 0 : 1;
}
