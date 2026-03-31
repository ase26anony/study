/* reload_coverage.c - Comprehensive test to trigger all reload types in GCC's reload pass */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    long l;
    char pad[3];
};

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};

int main(void) {
    /* Declare variables with different types and storage */
    int i, j, k;
    long base1 = 0x1000, base2 = 0x2000;
    volatile int vol_int = 42;
    char *char_ptr = (char*)&base1;
    int *int_ptr = &vol_int;
    long *long_ptr = &base2;
    
    /* Array with complex indexing */
    long data_array[64];
    for (i = 0; i < 64; i++) {
        data_array[i] = i * 3 + 1;
    }
    
    /* Packed/misaligned struct */
    struct misaligned_data md;
    md.c = 'A';
    md.i = 0xDEADBEEF;
    md.l = 0xCAFEBABE;
    
    /* Variables for address computations */
    long index_reg, scale_reg, disp_reg;
    long *addr_reg;
    
    /* Loop to vary constraints and trigger different reload types */
    for (i = 0; i < 10; i++) {
        /* Vary register pressure by using different iteration patterns */
        index_reg = i * 8;
        scale_reg = 2 + (i % 3);
        disp_reg = i * 16;
        addr_reg = &data_array[i * 4];
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex addressing mode with multiple components */
        asm volatile (
            "mov %[val1], %[offset]\n\t"
            "add %[val2], %[offset]\n\t"
            : [offset] "=r" (j)
            : [val1] "r" (index_reg),
              [val2] "r" (disp_reg),
              "m" (data_array[base1 + index_reg * scale_reg + disp_reg])  /* Complex memory operand */
            : "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Early-clobber output with overlapping inputs */
        asm volatile (
            "lea (%[base], %[index], %[scale]), %[addr]\n\t"
            "mov (%[addr]), %[tmp]\n\t"
            "add %[inc], %[tmp]\n\t"
            "mov %[tmp], (%[addr])\n\t"
            : [addr] "=&r" (addr_reg), [tmp] "=&r" (k)
            : [base] "r" (long_ptr),
              [index] "r" (index_reg),
              [scale] "i" (4),
              [inc] "r" (i),
              "m" (*long_ptr)  /* Memory clobber */
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Taking address of a complex memory operand */
        long **addr_of_addr = &addr_reg;
        asm volatile (
            "mov %[src], (%[dest])\n\t"
            : 
            : [src] "r" (&data_array[index_reg * 2 + i]),
              [dest] "r" (addr_of_addr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_INPUT with many operands exceeding register file */
        /* Mix of register and memory constraints */
        asm volatile (
            "add %[a], %[b]\n\t"
            "sub %[c], %[d]\n\t"
            "imul %[e], %[f]\n\t"
            : [b] "+r" (j), [d] "+r" (k), [f] "+r" (vol_int)
            : [a] "r" (i),
              [c] "r" (index_reg),
              [e] "r" (scale_reg),
              "m" (md.i),      /* Misaligned memory operand */
              "m" (md.l),      /* Another memory operand */
              "m" (global_array[i]),  /* Global memory operand */
              "m" (*char_ptr)  /* Char pointer dereference */
            : "cc", "rax", "rcx", "rdx"  /* Explicit clobbers for x86 */
        );
        
        /* Force RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
        /* Volatile asm with many constraints and memory clobber */
        asm volatile (
            "movq %%rsp, %%rax\n\t"
            "andq $-16, %%rax\n\t"
            "movq %[size], %%rcx\n\t"
            "1:\n\t"
            "movq (%%rax), %%rdx\n\t"
            "addq %%rdx, %[sum]\n\t"
            "addq $8, %%rax\n\t"
            "loop 1b\n\t"
            : [sum] "+r" (global_counter)
            : [size] "r" (8L),
              [ptr] "r" (data_array),
              "m" (data_array[0]),
              "m" (data_array[8]),
              "m" (data_array[16]),
              "m" (data_array[24])
            : "rax", "rcx", "rdx", "cc", "memory"
        );
        
        /* Mix data types to increase pressure */
        char char_var = (char)i;
        short short_var = (short)(i * 100);
        int int_var = i * 1000;
        long long_var = i * 10000L;
        
        /* Another complex asm with mixed types */
        asm volatile (
            "movsx %[chr], %[tmp32]\n\t"
            "add %[sht], %[tmp32]\n\t"
            "add %[intv], %[tmp32]\n\t"
            "mov %[tmp32], %[out]\n\t"
            : [out] "=r" (j), [tmp32] "=&r" (k)
            : [chr] "r" (char_var),
              [sht] "r" (short_var),
              [intv] "r" (int_var),
              [lng] "m" (long_var)  /* Memory operand for long */
            : "cc"
        );
        
        /* Update checksum to prevent dead code elimination */
        global_counter += j + k + vol_int;
        data_array[i] = global_counter;
    }
    
    /* Compute final checksum */
    long checksum = 0;
    for (i = 0; i < 64; i++) {
        checksum += data_array[i];
    }
    checksum += global_counter + vol_int + md.i + md.l;
    
    printf("Checksum: %ld\n", checksum);
    
    /* Use the results to prevent optimization */
    if (checksum > 1000) {
        printf("Test completed successfully.\n");
    }
    
    return 0;
}
