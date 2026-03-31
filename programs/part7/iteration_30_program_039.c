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
    register int reg_var1 asm ("r12") = 1;
    register int reg_var2 asm ("r13") = 2;
    auto int auto_vars[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    volatile int vol_var = 100;
    struct misaligned_data packed = {.c = 'A', .i = 1234, .l = 5678};
    long *aliased_ptr = (long *)&packed.i;  /* Potentially misaligned */
    
    /* Complex base/index registers for addressing modes */
    long base_array[32];
    for (int i = 0; i < 32; i++) base_array[i] = i * 3;
    
    long *base_ptr = &base_array[16];
    long index_reg = 8;
    long scale_factor = 4;
    long displacement = 0x100;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        /* Change displacement to vary addressing modes */
        displacement = (iter * 16) & 0xFF;
        scale_factor = 2 + (iter % 3);
        
        /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode that may need temporary register */
        asm volatile (
            "mov %[dst], qword ptr [%[base] + %[index]*%[scale] + %[disp]] \n\t"
            : [dst] "=r" (reg_var1)
            : [base] "r" (base_ptr),
              [index] "r" (index_reg),
              [scale] "i" (scale_factor),
              [disp] "i" (displacement),
              "m" (*base_ptr)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex address computation */
        long output_array[16];
        long output_index = iter & 7;
        
        asm volatile (
            "mov qword ptr [%[base] + %[index]*8 + %[disp]], %[src] \n\t"
            : "=m" (output_array[output_index])
            : [base] "r" (output_array),
              [index] "r" (output_index),
              [disp] "i" (0),
              [src] "r" (reg_var1)
            : "memory"
        );
        
        /* RELOAD_FOR_INPUT with many operands causing register pressure */
        /* More input operands than available registers in a class */
        int temp1, temp2, temp3, temp4, temp5, temp6;
        
        asm volatile (
            "add %[a], %[b] \n\t"
            "sub %[c], %[d] \n\t"
            "imul %[e], %[f] \n\t"
            "mov %[out1], %[a] \n\t"
            "mov %[out2], %[c] \n\t"
            "mov %[out3], %[e] \n\t"
            : [out1] "=&r" (temp1),
              [out2] "=&r" (temp2),
              [out3] "=&r" (temp3)
            : [a] "r" (auto_vars[0]),
              [b] "r" (auto_vars[1]),
              [c] "r" (auto_vars[2]),
              [d] "r" (auto_vars[3]),
              [e] "r" (auto_vars[4]),
              [f] "r" (auto_vars[5]),
              "m" (auto_vars[0]),
              "m" (auto_vars[1]),
              "m" (auto_vars[2]),
              "m" (auto_vars[3]),
              "m" (auto_vars[4]),
              "m" (auto_vars[5])
            : "cc", "memory"
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of already complex operand */
        long *addr_of_mem;
        
        asm volatile (
            "lea %[addr], [%[base] + %[index]*4] \n\t"
            "mov %[val], dword ptr [%[addr]] \n\t"
            : [addr] "=r" (addr_of_mem),
              [val] "=r" (vol_var)
            : [base] "r" (base_array),
              [index] "r" (iter),
              "m" (base_array[0])
            : "memory"
        );
        
        /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Mixed constraints with early clobber and memory operands */
        long other_temp1, other_temp2;
        
        asm volatile (
            "mov %[t1], %[in1] \n\t"
            "add %[t1], %[in2] \n\t"
            "mov %[t2], %[ptr] \n\t"
            "add %[t2], %[t1] \n\t"
            "mov [%[t2]], %[t1] \n\t"
            : [t1] "=&r" (other_temp1),
              [t2] "=&r" (other_temp2),
              "=m" (*aliased_ptr)
            : [in1] "r" (reg_var1),
              [in2] "r" (reg_var2),
              [ptr] "r" (aliased_ptr),
              "m" (*aliased_ptr)
            : "memory"
        );
        
        /* Update variables to create dependencies between iterations */
        reg_var1 = (reg_var1 * 1103515245 + 12345) & 0x7FFFFFFF;
        reg_var2 = (reg_var2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        vol_var += iter;
        global_counter += other_temp1 + other_temp2;
        
        /* Force spill/reload by using all variables */
        auto_vars[iter & 7] = temp1 + temp2 + temp3 + vol_var;
    }
    
    /* Compute checksum to ensure all operations have effect */
    unsigned long checksum = 0;
    checksum += reg_var1;
    checksum += reg_var2;
    checksum += vol_var;
    checksum += global_counter;
    
    for (int i = 0; i < 8; i++) {
        checksum += auto_vars[i];
    }
    
    checksum += packed.i;
    checksum += packed.l;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
