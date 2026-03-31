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
    int arr[256];
    long *ptr_arr[128];
    char buffer[512];
    volatile int volatile_var = 42;
    struct misaligned_data packed;
    register int reg_var asm ("r12") = 100; /* Hint at register usage */
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) arr[i] = i * 3;
    for (int i = 0; i < 128; i++) ptr_arr[i] = (long*)&arr[i*2];
    for (int i = 0; i < 512; i++) buffer[i] = i % 256;
    
    packed.c = 'A';
    packed.i = 0xDEADBEEF;
    packed.l = 0xCAFEBABE12345678ULL;
    
    long checksum = 0;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 256;
        int scale = 1 + (iter % 4);
        long *base_ptr = (long*)&arr[0];
        char *char_base = &buffer[0];
        
        /* Force RELOAD_FOR_INPUT and RELOAD_FOR_OUTPUT */
        /* Multiple operands exceeding available registers */
        asm volatile (
            "mov %[out1], %[in1]\n\t"
            "add %[out1], %[in2]\n\t"
            "imul %[out2], %[in3]\n\t"
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            : [out1] "=&r" (arr[idx]),      /* Early clobber output */
              [out2] "=r" (arr[idx+1]),
              [addr] "=r" (reg_var)         /* Address computation result */
            : [in1] "r" (volatile_var),
              [in2] "r" (iter),
              [in3] "rm" (arr[idx+64]),     /* Memory or register */
              [base] "r" (base_ptr),
              [index] "r" ((long)idx),
              [scale] "i" (scale)
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode with displacement */
        long complex_addr_result;
        asm volatile (
            "mov (%[addr_expr]), %[result]\n\t"
            : [result] "=r" (complex_addr_result)
            : [addr_expr] "r" (&arr[idx * 2 + scale * 4 + 16])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output with complex address */
        asm volatile (
            "mov %[val], (%[complex_out])\n\t"
            : 
            : [val] "r" (iter),
              [complex_out] "r" (&ptr_arr[iter % 8][scale * 2])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of memory operand that itself needs reload */
        long *nested_addr;
        asm volatile (
            "lea (%[base], %[index], 8), %[addr_out]\n\t"
            : [addr_out] "=r" (nested_addr)
            : [base] "r" (base_ptr),
              [index] "r" ((long)(iter * 3 % 128)),
              "m" (*ptr_arr[iter % 8])  /* Memory constraint forces address reload */
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
        /* Multiple memory operands with clobbers */
        asm volatile (
            "movq (%[src1]), %%rax\n\t"
            "addq (%[src2]), %%rax\n\t"
            "movq %%rax, (%[dst])\n\t"
            "incq (%[counter])\n\t"
            : 
            : [src1] "r" (&packed.l),
              [src2] "r" (&arr[iter * 5 % 256]),
              [dst] "r" (&ptr_arr[4][0]),
              [counter] "r" (&global_counter)
            : "rax", "memory", "cc"
        );
        
        /* Mix data types to increase pressure */
        char char_result;
        asm volatile (
            "mov (%[char_ptr]), %b[result]\n\t"
            "add %[offset], %b[result]\n\t"
            : [result] "=r" (char_result)
            : [char_ptr] "r" (char_base + idx * 3),
              [offset] "i" (iter),
              "m" (buffer[0])  /* Whole array as memory operand */
        );
        
        /* Update checksum with all modified values */
        checksum += arr[idx] + arr[idx+1] + complex_addr_result 
                  + *ptr_arr[4] + char_result + reg_var;
        
        /* Vary constraints each iteration */
        if (iter % 3 == 0) {
            /* Force more register pressure */
            asm volatile (
                "mov %[in1], %[out1]\n\t"
                "mov %[in2], %[out2]\n\t"
                "mov %[in3], %[out3]\n\t"
                "mov %[in4], %[out4]\n\t"
                : [out1] "=r" (arr[0]), [out2] "=r" (arr[1]),
                  [out3] "=r" (arr[2]), [out4] "=r" (arr[3])
                : [in1] "rm" (iter), [in2] "rm" (iter+1),
                  [in3] "rm" (iter+2), [in4] "rm" (iter+3)
                : "cc"
            );
        }
    }
    
    /* Final computation to ensure all asm statements have effect */
    for (int i = 0; i < 256; i++) {
        checksum += arr[i];
    }
    checksum += global_counter + packed.i + (long)packed.c;
    
    printf("Checksum: %ld\n", checksum);
    return (checksum != 0) ? 0 : 1;
}
