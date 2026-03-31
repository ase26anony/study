/* Test program to trigger uncovered reload block in reload.cc */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 0x77777777;
    int nv2 = 0x88888888;
    int nv3 = 0x99999999;
    int nv4 = 0xAAAAAAAA;
    
    /* Floating point variables to increase register pressure */
    volatile double f1 = 3.14159;
    volatile double f2 = 2.71828;
    volatile float f3 = 1.41421f;
    volatile float f4 = 1.73205f;
    
    /* Different sized variables for mode mismatches */
    volatile char c1 = 'A';
    volatile short s1 = 0x1234;
    volatile long long ll1 = 0x1122334455667788LL;
    
    /* Explicit register variables to pin specific registers */
    register int reg_var1 asm ("r12") = 0xDEADBEEF;
    register int reg_var2 asm ("r13") = 0xCAFEBABE;
    register int reg_var3 asm ("r14") = 0xFEEDFACE;
    
    /* Variables for address taking */
    int addr_var1 = 0x11112222;
    int addr_var2 = 0x33334444;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Control flow with goto to create complex live ranges */
    int checksum = 0;
    
block1:
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        /* Output operands with earlyclobber to force separate registers */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out1] "=&r" (nv1),    /* Earlyclobber - can't share with inputs */
          [out2] "=&r" (nv2)     /* Another earlyclobber */
        : [in1] "r" (v1),        /* Input in register */
          [in2] "r" (v2),        /* Another input */
          [in3] "rm" (v3)        /* Input in register or memory */
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* Use the results to keep them live */
    checksum += nv1 + nv2;
    
    /* Force address computation reload */
    asm volatile (
        "movl (%[ptr]), %%ebx\n\t"
        "addl %%ebx, %[sum]\n\t"
        : [sum] "+r" (checksum)
        : [ptr] "r" (addr_ptr1)
        : "rbx", "memory", "cc"
    );
    
    /* Modify variables to keep them live across blocks */
    v1 += 1;
    v2 += 2;
    reg_var1 ^= 0x12345678;
    
    goto block2;

block2:
    /* More inline assembly with memory constraints */
    {
        int temp1, temp2;
        
        /* Assembly with both memory and register constraints */
        asm volatile (
            "movq %[llin], %%rax\n\t"
            "shrq $32, %%rax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in1], %%ebx\n\t"
            "addl %%ebx, %[out2]\n\t"
            : [out1] "=r" (temp1),
              [out2] "=r" (temp2)
            : [llin] "m" (ll1),      /* Memory constraint */
              [in1] "r" (v4)         /* Register constraint */
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        checksum += temp1 + temp2;
    }
    
    /* Use explicit register variables in assembly that clobbers them */
    asm volatile (
        "movl %[regvar], %%r12d\n\t"  /* Explicitly use r12 */
        "addl $0x100, %%r12d\n\t"
        "movl %%r12d, %[out]\n\t"
        : [out] "=r" (nv3)
        : [regvar] "r" (reg_var1)
        : "r12", "cc"  /* Clobber r12 - forces save/restore of reg_var1 */
    );
    
    /* Floating point operations to increase pressure */
    f1 = f1 * 2.0;
    f2 = f2 / 2.0;
    
    /* Mixed size operations */
    c1 = c1 + 1;
    s1 = s1 * 2;
    
    goto block3;

block3:
    /* Complex assembly with many clobbers */
    {
        long long result;
        
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "movl %[d], %%edx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "cltd\n\t"              /* Sign extend eax to edx:eax */
            "movq %%rax, %[res]\n\t"
            : [res] "=rm" (result)  /* Output in register or memory */
            : [a] "rm" (v5),        /* Multiple inputs with rm constraint */
              [b] "rm" (v6),
              [c] "rm" (v7),
              [d] "rm" (v8)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        checksum += (int)result;
    }
    
    /* Another assembly block with address of local variable */
    {
        int local_var = 0x5555;
        int *local_ptr = &local_var;
        
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            : [sum] "+r" (checksum)
            : [addr] "r" (local_ptr)
            : "rax", "memory", "cc"
        );
    }
    
    /* Force reloads by using all variables one more time */
    v3 = nv4 ^ reg_var2;
    v4 = addr_var2 | reg_var3;
    
    /* Final checksum computation */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += c1 + s1 + (int)(ll1 & 0xFFFFFFFF);
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2;
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
