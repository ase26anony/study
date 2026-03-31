/* Compile with: gcc -O3 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload_test.c -o reload_test */
/* For 32-bit: gcc -O2 -m32 -fno-omit-frame-pointer reload_test.c -o reload_test_32 */

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
    
    /* Non-volatile variables with arithmetic to create live ranges */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    int nv5 = 500, nv6 = 600, nv7 = 700, nv8 = 800;
    
    /* Floating point variables to increase register pressure */
    volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
    double nf1 = 5.5, nf2 = 6.6, nf3 = 7.7, nf4 = 8.8;
    
    /* Different sized variables for mode mismatches */
    volatile char c1 = 'A', c2 = 'B';
    volatile short s1 = 1000, s2 = 2000;
    volatile long long ll1 = 0x1122334455667788LL;
    
    /* Explicit register variables to pin specific registers */
    register int reg_var1 asm ("r12") = 0xAAAA;
    register int reg_var2 asm ("r13") = 0xBBBB;
    register int reg_var3 asm ("r14") = 0xCCCC;
    
    /* Variables for address-taking */
    int addr_var1 = 9999, addr_var2 = 8888;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Result accumulator */
    volatile int checksum = 0;
    
    /* Complex control flow with multiple basic blocks */
    block1:
    {
        /* Force many values to be live across this point */
        nv1 = v1 + nv2;
        nv3 = v2 * nv4;
        nf1 = f1 + nf2;
        
        /* Inline assembly with conflicting constraints */
        /* Memory constraint while variable also used in register context */
        asm volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            : [output] "=m" (v3)          /* Memory output */
            : [input1] "r" (nv1),         /* Register input */
              [input2] "r" (nv3)          /* Another register input */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        /* Use the clobbered explicit register variables */
        reg_var1 = reg_var1 * 2;
        reg_var2 = reg_var2 + v3;
        
        /* Force address mode conflict */
        asm volatile (
            "movl (%[addr]), %%ebx\n\t"
            "addl %%ebx, %[sum]\n\t"
            : [sum] "+r" (checksum)
            : [addr] "r" (addr_ptr1)
            : "rbx", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        v1 = v3 + 1;
        v2 = v1 * 2;
        nv5 = nv6 - nv7;
    }
    
    goto block2;
    
    /* Dead code to create control flow complexity */
    {
        int dummy = 0;
        dummy = v4 + v5;  /* Never reached but affects analysis */
    }
    
    block2:
    {
        /* More arithmetic to create overlapping live ranges */
        nv2 = v4 + nv5;
        nv4 = v5 * nv6;
        nf2 = f2 + nf3;
        
        /* Inline assembly with output earlyclobber and multiple clobbers */
        /* This often forces reloads due to register conflicts */
        asm volatile (
            "movl %[in1], %%ecx\n\t"
            "imull %[in2], %%ecx\n\t"
            "movl %%ecx, %[out1]\n\t"
            "movl %[in3], %%edx\n\t"
            "addl %%ecx, %%edx\n\t"
            "movl %%edx, %[out2]\n\t"
            : [out1] "=&r" (nv7),    /* Earlyclobber - can't share reg with inputs */
              [out2] "=r" (nv8)      /* Regular output */
            : [in1] "r" (nv2),
              [in2] "r" (nv4),
              [in3] "r" (reg_var1)   /* Use explicit register variable */
            : "rax", "rcx", "rdx", "rbx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* Mixed data types in same assembly */
        asm volatile (
            "movzbl %[char_in], %%eax\n\t"
            "movw %[short_in], %%bx\n\t"
            "addw %%bx, %%ax\n\t"
            "movl %%eax, %[int_out]\n\t"
            : [int_out] "=r" (nv1)
            : [char_in] "m" (c1),
              [short_in] "m" (s1)
            : "rax", "rbx", "cc"
        );
        
        /* Force floating point reloads */
        asm volatile (
            "movsd %[fin1], %%xmm0\n\t"
            "addsd %[fin2], %%xmm0\n\t"
            "movsd %%xmm0, %[fout]\n\t"
            : [fout] "=m" (f4)
            : [fin1] "r" (nf1),
              [fin2] "r" (nf2)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "memory"
        );
        
        v4 = nv7 + nv8;
        v5 = v4 / 2;
        addr_var2 = checksum + v5;
    }
    
    goto block3;
    
    block3:
    {
        /* Complex assembly with many operands */
        long long temp_ll;
        
        asm volatile (
            "movq %[llin], %%rax\n\t"
            "shrq $32, %%rax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in1], %%ebx\n\t"
            "addl %[in2], %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            "movl %[in3], %%ecx\n\t"
            "subl %[in4], %%ecx\n\t"
            "movl %%ecx, %[out3]\n\t"
            : [out1] "=r" (nv2),
              [out2] "=r" (nv3),
              [out3] "=r" (nv4),
              "=r" (temp_ll)
            : [llin] "r" (ll1),
              [in1] "r" (v6),
              [in2] "r" (v7),
              [in3] "r" (reg_var2),
              [in4] "r" (reg_var3)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* Final checksum calculation */
        checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        checksum += nv1 + nv2 + nv3 + nv4 + nv5 + nv6 + nv7 + nv8;
        checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        checksum += c1 + c2 + s1 + s2;
        checksum += reg_var1 + reg_var2 + reg_var3;
        checksum += addr_var1 + addr_var2;
        
        /* Use all variables one more time to ensure they're live */
        asm volatile (
            "addl %[val], %[sum]\n\t"
            : [sum] "+r" (checksum)
            : [val] "r" (temp_ll)
            : "cc"
        );
    }
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
