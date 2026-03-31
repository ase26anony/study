/* reload_coverage.c - Comprehensive test for GCC reload pass coverage */
#include <stdio.h>
#include <stdint.h>

/* Packed struct to force misalignment */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

int main(void) {
    /* Declare variables with different types and storage */
    int arr[256];
    long *ptr_arr[16];
    struct misaligned_data packed;
    volatile int vol_var = 0;
    register int reg_var asm("r12") = 42; /* Try to bind to specific reg */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        ptr_arr[i] = (long*)&arr[i * 16];
    }
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    int result = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx1 = (iter * 17) % 256;
        int idx2 = (iter * 23) % 256;
        int idx3 = (iter * 37) % 256;
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Multiple output operands with early-clobber */
        asm volatile (
            "addl %[in1], %[out1]\n\t"
            "imull %[in2], %[out2]\n\t"
            "orl %[in3], %[out3]"
            : [out1] "=&r" (arr[idx1]),  /* Early-clobber output */
              [out2] "=&r" (arr[idx2]),  /* Another early-clobber */
              [out3] "=r" (arr[idx3])    /* Regular output */
            : [in1] "rm" (iter),         /* Register or memory */
              [in2] "rm" (global_counter),
              [in3] "rm" (reg_var)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_INPUT_ADDRESS with complex addressing */
        /* Multiple memory operands with index registers */
        long complex_addr_result;
        asm volatile (
            "movq (%[base1], %[idx1], 8), %[res]\n\t"
            "addq (%[base2], %[idx2], 4), %[res]\n\t"
            "subq %[disp], %[res]"
            : [res] "=r" (complex_addr_result)
            : [base1] "r" (arr),          /* Base register */
              [idx1] "r" (idx1),          /* Index register */
              [base2] "r" (arr),          /* Another base */
              [idx2] "r" (idx2),          /* Another index */
              [disp] "rm" (packed.l)      /* Displacement */
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        /* Taking address of memory operand that needs reload */
        long* addr_of_mem;
        asm volatile (
            "leaq (%[base], %[idx], 4), %[addr]\n\t"
            "movq %[addr], (%[ptr_arr])"
            : [addr] "=&r" (addr_of_mem),   /* Address computation */
              "=m" (ptr_arr[iter % 16])     /* Memory output */
            : [base] "r" (arr),
              [idx] "r" (idx3),
              [ptr_arr] "r" (&ptr_arr[iter % 16])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Nested address computation */
        long double_indirect;
        asm volatile (
            "movq (%[ptr]), %[temp]\n\t"
            "movq (%[temp]), %[result]"
            : [result] "=r" (double_indirect),
              [temp] "=&r" (reg_var)       /* Temporary for address */
            : [ptr] "r" (&ptr_arr[iter % 8])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
        /* Many operands to cause register pressure */
        int temp1, temp2, temp3, temp4, temp5;
        asm volatile (
            "movl %%eax, %[t1]\n\t"
            "movl %%ebx, %[t2]\n\t"
            "movl %%ecx, %[t3]\n\t"
            "movl %%edx, %[t4]\n\t"
            "movl %%esi, %[t5]"
            : [t1] "=r" (temp1),
              [t2] "=r" (temp2),
              [t3] "=r" (temp3),
              [t4] "=r" (temp4),
              [t5] "=r" (temp5)
            : /* No inputs, but clobber many registers */
            : "eax", "ebx", "ecx", "edx", "esi", "memory"
        );
        
        /* Mix with volatile variable access */
        vol_var = iter;
        global_counter++;
        
        /* Use computed values to prevent dead code elimination */
        result ^= arr[idx1] ^ arr[idx2] ^ arr[idx3];
        result ^= (int)complex_addr_result;
        result ^= (int)(uintptr_t)addr_of_mem;
        result ^= (int)double_indirect;
        result ^= temp1 ^ temp2 ^ temp3 ^ temp4 ^ temp5;
    }
    
    /* Compute final checksum */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (long)(uintptr_t)ptr_arr[i];
    }
    checksum += packed.i + packed.l;
    checksum += vol_var + global_counter + result;
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
