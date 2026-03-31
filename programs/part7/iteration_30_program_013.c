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
    long index_var = 7;
    long displacement = 4096;
    char char_buffer[64];
    double fp_var = 3.14159;
    
    /* Packed/misaligned struct */
    struct misaligned_data packed = { .c = 'A', .i = -42, .l = 999999999, .trailing = 'Z' };
    
    /* Pointer with complex computation */
    long *base_ptr = &global_array[16];
    
    /* Array with address taken */
    int addr_taken_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int *addr_ptr = &addr_taken_array[0];
    
    /* Loop to vary constraints and create register pressure */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary index and displacement each iteration */
        index_var = (iteration * 3) % 16;
        displacement = 128 * iteration;
        
        /* Complex addressing mode 1: [base + index*scale + displacement] */
        /* Should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "movq %[base], %%r10\n\t"
            "movq %[index], %%r11\n\t"
            "leaq (%[base], %[index], 8), %%r12\n\t"
            "addq %[disp], %%r12\n\t"
            "movq (%%r12), %%r13\n\t"
            "addq %%r13, %[sum]\n\t"
            : [sum] "+r" (reg_var)
            : [base] "r" (base_ptr),
              [index] "r" (index_var),
              [disp] "r" (displacement),
              "m" (*base_ptr)
            : "r10", "r11", "r12", "r13", "cc", "memory"
        );
        
        /* Multiple operands with conflicting constraints */
        /* Should trigger RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in4], %%ebx\n\t"
            "subl %%eax, %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out1] "=&r" (auto_var),  /* Early clobber */
              [out2] "=&r" (vol_var)    /* Early clobber */
            : [in1] "r" (iteration),
              [in2] "r" (reg_var),
              [in3] "rm" (global_counter),  /* Register or memory */
              [in4] "rm" (packed.i),        /* Misaligned access */
              "m" (global_array[0])         /* Memory constraint */
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Nested address computation */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        long complex_addr;
        asm volatile (
            "leaq (%[base], %[idx], 4), %[addr]\n\t"
            "movq %[addr], %%r14\n\t"
            "addq $16, %%r14\n\t"
            "movq (%%r14), %%r15\n\t"
            "addq %%r15, %[total]\n\t"
            : [addr] "=&r" (complex_addr),  /* Early clobber output */
              [total] "+r" (reg_var)
            : [base] "r" (base_ptr),
              [idx] "r" (index_var),
              "m" (*(base_ptr + index_var))
            : "r14", "r15", "cc", "memory"
        );
        
        /* Output address reload scenario */
        /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        long output_target;
        asm volatile (
            "movq %[src], %%rax\n\t"
            "leaq %[dst], %%rbx\n\t"
            "movq %%rax, (%%rbx)\n\t"
            "addq $8, %%rbx\n\t"
            "movq %%rax, (%%rbx)\n\t"
            : "=m" (output_target)  /* Memory output */
            : [src] "r" (reg_var),
              [dst] "m" (output_target)  /* Memory input for address */
            : "rax", "rbx", "cc", "memory"
        );
        
        /* Mixed data types with alignment issues */
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movzbl %[char], %%eax\n\t"
            "movl %[int], %%ebx\n\t"
            "movq %[long], %%rcx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addq %%rcx, %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=m" (fp_var)  /* Store in double (different size) */
            : [char] "m" (packed.c),   /* Char from packed struct */
              [int] "m" (packed.i),    /* Possibly misaligned int */
              [long] "m" (packed.l),   /* Possibly misaligned long */
              "m" (char_buffer[0])     /* Additional memory constraint */
            : "rax", "rbx", "rcx", "cc", "memory"
        );
        
        /* Create register pressure by using many temporaries */
        int tmp1, tmp2, tmp3, tmp4, tmp5;
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "movl %[d], %%edx\n\t"
            "movl %[e], %%esi\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "addl %%esi, %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %%ebx, %[out2]\n\t"
            "movl %%ecx, %[out3]\n\t"
            "movl %%edx, %[out4]\n\t"
            "movl %%esi, %[out5]\n\t"
            : [out1] "=&r" (tmp1),
              [out2] "=&r" (tmp2),
              [out3] "=&r" (tmp3),
              [out4] "=&r" (tmp4),
              [out5] "=&r" (tmp5)
            : [a] "r" (iteration),
              [b] "r" (iteration + 1),
              [c] "r" (iteration + 2),
              [d] "r" (iteration + 3),
              [e] "r" (iteration + 4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "cc"
        );
        
        /* Update global to prevent dead code elimination */
        global_counter += iteration;
    }
    
    /* Compute checksum from all modified variables */
    unsigned long checksum = 0;
    checksum += reg_var;
    checksum += vol_var;
    checksum += auto_var;
    checksum += (unsigned long)fp_var;
    
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    
    checksum += global_counter;
    checksum += packed.i;
    checksum += packed.l;
    
    printf("Final checksum: %lu\n", checksum);
    printf("Reload coverage test completed.\n");
    
    return (int)(checksum % 256);
}
