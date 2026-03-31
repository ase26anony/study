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
    /* Declare diverse variables with different storage characteristics */
    register int reg_var asm("ebx") = 1;  /* Try to bind to specific reg */
    int auto_var = 2;
    volatile int vol_var = 3;
    long long_var = 4;
    char char_var = 5;
    int *ptr_var = &auto_var;
    
    /* Array with complex indexing */
    int arr[100] = {0};
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    /* Packed/misaligned struct */
    struct misaligned_data packed = {.c = 'A', .i = 100, .l = 1000};
    
    /* Address-taken variables */
    int addr_taken1 = 100, addr_taken2 = 200;
    int *addr1 = &addr_taken1;
    int *addr2 = &addr_taken2;
    
    /* Loop with varying constraints to trigger different reload types */
    for (int iter = 0; iter < 10; iter++) {
        int idx = iter * 7 % 100;
        int scale = (iter % 4) + 1;
        
        /* Vary constraints each iteration */
        const char *constraint_r = "r";
        const char *constraint_m = "m";
        const char *constraint_rm = "rm";
        const char *constraint_memory = "memory";
        
        switch (iter % 4) {
            case 0: {
                /* Complex addressing mode: base + index*scale + displacement */
                /* Likely triggers: RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    "movl %[arrbase], %%eax\n\t"
                    "movl %[index], %%ecx\n\t"
                    "movl (%[base],%%ecx,%[scale],4), %%edx\n\t"
                    "addl %%edx, %[sum]\n\t"
                    : [sum] "+rm" (global_sum)
                    : [arrbase] "r" (arr), 
                      [base] "r" (&arr[10]),  /* Complex base */
                      [index] "r" (idx),
                      [scale] "r" (scale)
                    : "eax", "ecx", "edx", "memory", "cc"
                );
                break;
            }
            
            case 1: {
                /* Multiple operands with early-clobber and overlapping constraints */
                /* Likely triggers: RELOAD_FOR_INPUT, RELOAD_FOR_OUTPUT, RELOAD_OTHER */
                int tmp1, tmp2, tmp3;
                asm volatile (
                    "movl %[in1], %[out1]\n\t"
                    "imull %[in2], %[out1]\n\t"
                    "addl %[in3], %[out2]\n\t"
                    "leal (%[out1],%[out2],2), %[out3]\n\t"
                    : [out1] "=&r" (tmp1),  /* Early clobber */
                      [out2] "=&r" (tmp2),  /* Early clobber */
                      [out3] "=r" (tmp3)
                    : [in1] "rm" (auto_var + iter),
                      [in2] "rm" (vol_var),
                      [in3] "rm" (char_var)
                    : "cc"
                );
                global_sum += tmp1 + tmp2 + tmp3;
                break;
            }
            
            case 2: {
                /* Nested address computation - address of a memory operand */
                /* Likely triggers: RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
                long *complex_ptr;
                asm volatile (
                    "leaq %[packed_i], %[ptr]\n\t"
                    "movq %[ptr], %%rax\n\t"
                    "addq $4, %%rax\n\t"
                    "movq %%rax, %[ptr]\n\t"
                    : [ptr] "=r" (complex_ptr)
                    : [packed_i] "m" (packed.i)
                    : "rax", "memory"
                );
                
                /* Use the computed address */
                if (complex_ptr) {
                    asm volatile (
                        "movl (%[ptr]), %%eax\n\t"
                        "addl %%eax, %[sum]\n\t"
                        : [sum] "+rm" (global_sum)
                        : [ptr] "r" (complex_ptr)
                        : "eax", "memory"
                    );
                }
                break;
            }
            
            case 3: {
                /* Maximum register pressure with mixed types */
                /* Likely triggers: RELOAD_FOR_OTHER_ADDRESS, various others */
                char c1, c2, c3;
                short s1, s2;
                int i1, i2, i3, i4, i5, i6, i7, i8;
                long l1, l2;
                
                /* Force many temporaries */
                asm volatile (
                    "movb %[cv], %%al\n\t"
                    "movb %%al, %[c1]\n\t"
                    "movw %[iter], %%bx\n\t"
                    "movw %%bx, %[s1]\n\t"
                    "movl %[auto], %%ecx\n\t"
                    "movl %%ecx, %[i1]\n\t"
                    "movl %[vol], %%edx\n\t"
                    "movl %%edx, %[i2]\n\t"
                    "movl %[longv], %%esi\n\t"
                    "movl %%esi, %[i3]\n\t"
                    "movq %[ptrv], %%rdi\n\t"
                    "movq %%rdi, %[l1]\n\t"
                    : [c1] "=r" (c1),
                      [s1] "=r" (s1),
                      [i1] "=r" (i1),
                      [i2] "=r" (i2),
                      [i3] "=r" (i3),
                      [l1] "=r" (l1)
                    : [cv] "rm" (char_var),
                      [iter] "rm" ((short)iter),
                      [auto] "rm" (auto_var),
                      [vol] "rm" (vol_var),
                      [longv] "rm" ((int)long_var),
                      [ptrv] "rm" ((long)ptr_var)
                    : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
                );
                
                /* Use results to prevent elimination */
                global_sum += c1 + s1 + i1 + i2 + i3 + (int)l1;
                break;
            }
        }
        
        /* Additional complex addressing with output address reloads */
        if (iter % 3 == 0) {
            long output_addr;
            /* Likely triggers: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
            asm volatile (
                "leaq %[output], %[addr]\n\t"
                "movq $0x12345678, (%[addr])\n\t"
                : [addr] "=r" (output_addr)
                : [output] "m" (arr[idx])
                : "memory"
            );
        }
        
        /* Mix memory and register constraints in same statement */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val2]\n\t"
            : [val2] "+rm" (auto_var)
            : [val1] "irm" (iter),  /* Immediate, register, or memory */
              [val3] "rm" (reg_var)
            : "cc"
        );
        
        global_counter++;
    }
    
    /* Compute checksum to ensure all assembly executed */
    long checksum = global_sum + global_counter + auto_var + long_var + char_var;
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: %ld\n", checksum);
    printf("Final auto_var: %d\n", auto_var);
    printf("Global sum: %ld\n", global_sum);
    
    return (int)(checksum % 256);
}
