/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
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
volatile int global_counter = 0;
volatile long global_sum = 0;

int main() {
    /* Declare variables with different types and storage */
    int arr[100];
    long *ptr_arr[50];
    char buffer[256];
    volatile int vol_var = 42;
    struct misaligned_data packed;
    double fp_data[10];
    register int reg_var asm("r12") = 100; /* Suggest register */
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3;
        if (i < 50) ptr_arr[i] = (long*)&arr[i];
        if (i < 10) fp_data[i] = i * 1.5;
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 100;
        long *base_ptr = (long*)&arr[0];
        long *index_ptr = (long*)&arr[idx];
        int scale = (iter % 4) + 1;
        long displacement = iter * 16;
        
        /* Vary constraints each iteration */
        const char *constraint1 = (iter % 3 == 0) ? "=m" : "=r";
        const char *constraint2 = (iter % 3 == 1) ? "r" : "m";
        const char *clobber_reg = (iter % 2) ? "r13" : "r14";
        
        /* 
         * Complex addressing mode: [base + index*scale + displacement]
         * Forces RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS
         */
        asm volatile (
            "mov %[dst], %[src]\n\t"
            : [dst] "=r" (arr[idx])
            : [src] "m" (*(long*)((char*)base_ptr + (long)index_ptr * scale + displacement))
            : "memory"
        );
        
        /* 
         * Multiple operands with conflicting constraints
         * Forces RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER
         */
        long temp1, temp2, temp3;
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[out1]\n\t"
            "mov %[in1], %[out2]\n\t"
            "add %[in2], %[out3]\n\t"
            : [out1] "=&r" (temp1),  /* Early clobber */
              [out2] "=r" (temp2),
              [out3] "=r" (temp3)
            : [base] "r" (base_ptr),
              [index] "r" (index_ptr),
              [scale] "i" (scale),
              [in1] "r" (vol_var),
              [in2] "rm" (displacement)  /* Register or memory */
            : "cc", clobber_reg
        );
        
        /* 
         * Nested address computation
         * Forces RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR
         */
        long *addr_of_mem;
        asm volatile (
            "lea %[mem_operand], %[addr]\n\t"
            : [addr] "=r" (addr_of_mem)
            : [mem_operand] "m" (*(struct misaligned_data*)
                                 ((char*)&packed + iter * 2))
            : "memory"
        );
        
        /* 
         * Output address reload with complex addressing
         * Forces RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS
         */
        asm volatile (
            "mov %[val], %[complex_addr]\n\t"
            : [complex_addr] "=m" (*(int*)((char*)buffer + idx * scale + iter * 4))
            : [val] "r" (iter)
            : "memory"
        );
        
        /* 
         * Mixed data types with memory clobber
         * Forces RELOAD_FOR_OTHER_ADDRESS
         */
        char char_val;
        int int_val;
        double double_val;
        
        asm volatile (
            "movzb %[char_in], %[int_out]\n\t"
            "cvtsi2sd %[int_in], %[fp_out]\n\t"
            : [int_out] "=r" (int_val),
              [fp_out] "=x" (double_val)
            : [char_in] "m" (buffer[idx]),
              [int_in] "rm" (arr[idx])
            : "memory", "xmm0", "xmm1"
        );
        
        /* Use computed values to prevent dead code elimination */
        global_counter += iter;
        global_sum += temp1 + temp2 + temp3 + int_val + (long)double_val;
        arr[idx] ^= (int)(uintptr_t)addr_of_mem;
    }
    
    /* Compute checksum to ensure all operations have side effects */
    unsigned long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum = (checksum * 31 + arr[i]) & 0xFFFFFFFF;
    }
    checksum = (checksum + global_sum + global_counter) & 0xFFFFFFFF;
    
    /* Use packed struct to force potential misaligned reloads */
    checksum ^= packed.i;
    checksum ^= (packed.l >> 32) ^ (packed.l & 0xFFFFFFFF);
    
    printf("Checksum: 0x%08lx\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global sum: %ld\n", global_sum);
    
    return (checksum != 0) ? 0 : 1;
}
