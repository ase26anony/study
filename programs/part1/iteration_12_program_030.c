#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables that conflict with normal allocation */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b * 2;
    register int z asm("r14") = c ^ 0xFF;
    
    int result;
    
    /* Inline assembly with mismatched constraints to force reloads */
    asm volatile (
        /* Output constraint: memory, input constraints: register or memory */
        "movl %[in1], %[out]\n\t"
        "addl %[in2], %[out]\n\t"
        "xorl %[in3], %[out]"
        : [out] "=m" (result)          /* Output to memory */
        : [in1] "r,m" (x),             /* Input: register OR memory */
          [in2] "r,m" (y),
          [in3] "r,m" (z)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    return result + d + e + f;
}

/* Function using volatile addresses and complex addressing modes */
__attribute__((noinline))
void func_volatile_addressing(volatile int* arr, int idx, long val) {
    volatile char c1 = (char)(val & 0xFF);
    volatile short s1 = (short)((val >> 8) & 0xFFFF);
    volatile int i1 = (int)(val >> 16);
    
    /* Take addresses of volatile variables */
    char* pc1 = (char*)&c1;
    short* ps1 = (short*)&s1;
    int* pi1 = (int*)&i1;
    
    /* Complex pointer arithmetic that can't be folded */
    int offset = idx * (g_volatile_seed % 16);
    
    /* Inline assembly with memory output and immediate input */
    asm volatile (
        "movb %[imm1], (%[addr1])\n\t"
        "movw %[imm2], (%[addr2],%[idx],2)\n\t"
        "movl %[imm3], (%[addr3],%[idx],4)"
        : /* No outputs (all memory side effects) */
        : [addr1] "r" (pc1 + offset),
          [addr2] "r" (ps1),
          [addr3] "r" (pi1),
          [imm1] "i" (0xAA),
          [imm2] "i" (0xBBBB),
          [imm3] "i" (0xCCCCCCCC),
          [idx] "r" (idx)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
    );
    
    /* Store to array with complex addressing */
    arr[idx * 2 + (g_volatile_seed & 1)] = *pi1 + *ps1 + *pc1;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
long func_mixed_types(char c, short s, int i, long l) {
    volatile int counter = 0;
    long result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (counter = 0; counter < 4; counter++) {
        /* Mixed type operations causing mode changes */
        char c_tmp = c + counter;
        short s_tmp = s * c_tmp;           /* char promoted to int for multiplication */
        int i_tmp = i + (int)s_tmp;        /* short promoted to int */
        long l_tmp = l + (long)i_tmp;      /* int promoted to long */
        
        /* Bitfield operations */
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
        } bits = {c_tmp & 0x7, (s_tmp >> 2) & 0x1F, i_tmp & 0xFF};
        
        /* Union causing type punning */
        union {
            uint32_t u32;
            uint16_t u16[2];
            uint8_t u8[4];
        } converter;
        
        converter.u32 = i_tmp;
        
        /* Inline assembly with subreg-like operations */
        asm volatile (
            "movzbl %[byte], %[temp]\n\t"      /* zero extend byte to long */
            "addq %[temp], %[accum]\n\t"
            "movzwl %[word], %[temp]\n\t"      /* zero extend word to long */
            "addq %[temp], %[accum]\n\t"
            "movl %[dword], %[temp]\n\t"       /* mov 32-bit to 64-bit (implicit zero extend) */
            "addq %[temp], %[accum]"
            : [accum] "+r" (result)
            : [byte] "r" (converter.u8[0]),
              [word] "r" (converter.u16[1]),
              [dword] "r" (converter.u32),
              [temp] "r" (l_tmp)
            : "cc"
        );
        
        c += (char)(bits.a + bits.b);
        s += (short)(bits.c * 2);
    }
    
    return result;
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
int func_high_register_pressure(int a1, int a2, int a3, int a4, 
                                int a5, int a6, int a7, int a8) {
    /* Many local variables of different types */
    char c1 = a1 & 0xFF;
    char c2 = a2 & 0xFF;
    short s1 = a3 & 0xFFFF;
    short s2 = a4 & 0xFFFF;
    int i1 = a5;
    int i2 = a6;
    long l1 = a7;
    long l2 = a8;
    
    /* Pointer variables */
    char* pc1 = &c1;
    char* pc2 = &c2;
    short* ps1 = &s1;
    int* pi1 = &i1;
    long* pl1 = &l1;
    
    /* Complex expression with many intermediate values */
    int temp1 = *pc1 + *pc2;
    int temp2 = *ps1 * temp1;
    int temp3 = *pi1 / (temp2 ? temp2 : 1);
    long temp4 = *pl1 + temp3;
    
    /* Inline assembly using many registers */
    asm volatile (
        "imull %[v1], %[v2]\n\t"
        "addl %[v2], %[v3]\n\t"
        "movslq %[v3], %[v4]\n\t"      /* sign extend 32-bit to 64-bit */
        "addq %[v5], %[v4]\n\t"
        "movq %[v4], %[v6]"
        : [v2] "+r" (temp2),
          [v3] "+r" (temp3),
          [v4] "+r" (temp4),
          [v6] "=m" (l2)              /* Output to memory */
        : [v1] "r,m" (temp1),         /* Input: register OR memory */
          [v5] "r,m" (l1)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "cc", "memory"
    );
    
    return (int)(temp4 + l2);
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = g_volatile_seed + argc;
    
    /* Many local variables to increase register pressure */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    char c1 = (char)v1;
    short s1 = (short)v2;
    long l1 = (long)v3;
    
    /* Array with volatile accesses */
    volatile int arr[32];
    for (int i = 0; i < 32; i++) {
        arr[i] = seed + i;
    }
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    sum += func_explicit_registers(v1, v2, v3, v4, v5, v6);
    sum += func_explicit_registers(v2, v3, v4, v5, v6, v7);
    
    func_volatile_addressing((int*)arr, v1 % 16, l1);
    func_volatile_addressing((int*)arr, v2 % 16, l1 + 1000);
    
    sum += (int)func_mixed_types(c1, s1, v4, l1);
    sum += (int)func_mixed_types(c1 + 1, s1 + 1, v5, l1 + 1);
    
    sum += func_high_register_pressure(v1, v2, v3, v4, v5, v6, v7, v8);
    sum += func_high_register_pressure(v8, v7, v6, v5, v4, v3, v2, v1);
    
    /* Additional complex operations in main */
    {
        /* Union and pointer casting */
        union {
            long l;
            int i[2];
            char c[8];
        } u;
        
        u.l = sum;
        
        /* Pointer arithmetic with different types */
        char* ptr = u.c;
        for (volatile int i = 0; i < 8; i++) {
            ptr[i] += (char)(seed + i);
        }
        
        /* Final inline assembly with many clobbers */
        asm volatile (
            "movq %[val], %%rax\n\t"
            "rolq $32, %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            "movq %[sum], %[out]"
            : [out] "=rm" (sum)
            : [val] "rm" (u.l),
              [sum] "0" (sum)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13",
              "r14", "r15", "cc", "memory"
        );
    }
    
    /* Use array values to prevent elimination */
    for (int i = 0; i < 32; i++) {
        sum += arr[i];
    }
    
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
