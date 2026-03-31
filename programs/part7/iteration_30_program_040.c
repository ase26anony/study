/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload pass */
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
volatile long global_base = 1000;
volatile int global_index = 2;
volatile long global_array[256];

int main(void) {
    /* Diverse variables with different storage characteristics */
    register int reg_var asm("r12") = 42;  /* Fixed register to create pressure */
    auto long auto_array[32];
    volatile char volatile_char = 'A';
    struct misaligned_data packed;
    long *aliased_ptr = (long *)&packed.i;  /* Potentially misaligned pointer */
    
    /* Initialize data */
    for (int i = 0; i < 32; i++) {
        auto_array[i] = i * 100;
    }
    packed.c = 'X';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE;
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iteration = 0; iteration < 10; iteration++) {
        int scale = 1 << (iteration % 4);  /* Varying scale: 1, 2, 4, 8 */
        int displacement = iteration * 16;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode that may not be directly supported */
        asm volatile (
            "/* Complex addressing input reloads */\n\t"
            "mov %[out1], [%[base] + %[index]*%[scale] + %[disp]]\n\t"
            : [out1] "=r" (reg_var)
            : [base] "r" (&global_array[0]),
              [index] "r" (global_index),
              [scale] "i" (scale),
              [disp] "i" (displacement),
              "m" (global_array[0])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex addressing */
        long temp_output;
        asm volatile (
            "/* Complex addressing output reloads */\n\t"
            "mov [%[base] + %[index]*4 + 32], %[val]\n\t"
            : "=m" (*(long(*)[1])&temp_output)  /* Memory output constraint */
            : [base] "r" (auto_array),
              [index] "r" (iteration),
              [val] "r" (reg_var + iteration)
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPUT with register pressure */
        /* Many input operands competing for registers */
        long r1, r2, r3, r4, r5, r6;
        asm volatile (
            "/* Multiple input operand reloads */\n\t"
            "add %[r1], %[a], %[b]\n\t"
            "add %[r2], %[c], %[d]\n\t"
            "add %[r3], %[e], %[f]\n\t"
            "add %[r4], %[g], %[h]\n\t"
            "add %[r5], %[i], %[j]\n\t"
            "add %[r6], %[k], %[l]\n\t"
            : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3),
              [r4] "=&r" (r4), [r5] "=&r" (r5), [r6] "=&r" (r6)
            : [a] "r" (reg_var), [b] "r" (iteration),
              [c] "r" (global_base), [d] "r" (displacement),
              [e] "r" (packed.i), [f] "r" (packed.l),
              [g] "r" (auto_array[0]), [h] "r" (auto_array[1]),
              [i] "r" (auto_array[2]), [j] "r" (auto_array[3]),
              [k] "r" (auto_array[4]), [l] "r" (auto_array[5])
            : "cc"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of a memory operand with complex addressing */
        long *addr_ptr;
        asm volatile (
            "/* Operand address reloads */\n\t"
            "lea %[addr], [%[base] + %[index]*8 + 64]\n\t"
            : [addr] "=r" (addr_ptr)
            : [base] "r" (global_array),
              [index] "r" (iteration)
        );
        
        /* Force RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Volatile asm with memory clobber and many constraints */
        asm volatile (
            "/* Other reload types with memory clobber */\n\t"
            "mov [%[ptr]], %[val]\n\t"
            "add %[val], %[val], 1\n\t"
            "mov %[out], [%[ptr]]\n\t"
            : [out] "=r" (reg_var), [ptr] "+r" (aliased_ptr)
            : [val] "r" (reg_var),
              "m" (*aliased_ptr)
            : "memory", "cc"
        );
        
        /* Mix data types to create alignment issues */
        char *char_ptr = (char *)&auto_array[iteration];
        int *int_ptr = (int *)(char_ptr + 1);  /* Misaligned access */
        
        asm volatile (
            "/* Mixed type reloads */\n\t"
            "movzx %[ch], byte ptr [%[cp]]\n\t"
            "mov %[in], dword ptr [%[ip]]\n\t"
            : [ch] "=r" (volatile_char), [in] "=r" (packed.i)
            : [cp] "r" (char_ptr), [ip] "r" (int_ptr)
            : "memory"
        );
        
        /* Update checksum with all modified values */
        checksum += reg_var + r1 + r2 + r3 + r4 + r5 + r6 + *addr_ptr + packed.i;
    }
    
    /* Final computation to ensure all asm statements have observable effect */
    checksum += volatile_char + packed.c + packed.l;
    
    printf("Checksum: %ld\n", checksum);
    return (checksum != 0) ? 0 : 1;
}
