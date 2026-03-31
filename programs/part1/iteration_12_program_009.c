/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
static int use_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables that conflict with normal allocation */
    register int x1 asm("r12") = a + 1;
    register int x2 asm("r13") = b + 2;
    register int x3 asm("r14") = c + 3;
    register int x4 asm("r15") = d + 4;
    
    int result;
    
    /* Inline assembly with mismatched constraints */
    /* Input in register, output to memory with complex addressing */
    volatile int mem_output;
    int* ptr = &mem_output;
    
    /* Force reloads with conflicting constraints */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=m" (*ptr)          /* Memory output */
        : [in1] "r" (x1),            /* Register input */
          [in2] "rm" (x2)            /* Register or memory - may force reload */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
    );
    
    /* More assembly with immediate inputs and memory outputs */
    volatile int mem_array[4];
    asm volatile (
        "movl $0x%0, %1\n\t"
        "addl $0x%2, %1\n\t"
        : "=m" (mem_array[0]), "=m" (mem_array[1])
        : "i" (g_volatile_seed), "i" (255)
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Mixed constraints that may require reloads */
    asm volatile (
        "mov %[src], %%rax\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %%rax, %[dst]\n\t"
        : [dst] "=rm" (result)
        : [src] "rmi" (x3),          /* Register, memory, or immediate */
          "b" (x4)                   /* Fixed register constraint */
        : "rax", "rcx", "rdx", "memory"
    );
    
    return result + mem_output + mem_array[0];
}

/* Function using volatile addresses and complex addressing modes */
__attribute__((noinline))
static long use_volatile_addresses(volatile char* base, int offset) {
    volatile short s_arr[8];
    volatile int i_arr[4];
    volatile long l_arr[2];
    
    /* Take addresses of volatile variables */
    short* s_ptr = (short*)&s_arr[g_volatile_seed & 3];
    int* i_ptr = (int*)&i_arr[g_volatile_seed & 1];
    long* l_ptr = (long*)&l_arr[0];
    
    /* Complex address calculation that may not fit in addressing mode */
    char* addr = (char*)base + offset * 3 + (g_volatile_seed & 7);
    
    long result = 0;
    
    /* Assembly with memory output and register input */
    asm volatile (
        "movzbq (%[addr]), %%rax\n\t"    /* Zero extend byte to quadword */
        "movswq (%[sptr]), %%rbx\n\t"    /* Sign extend word to quadword */
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "=m" (*l_ptr)
        : [addr] "r" (addr),
          [sptr] "r" (s_ptr)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
    );
    
    /* Another with immediate to memory */
    asm volatile (
        "movl $0x%0, %1\n\t"
        : "=m" (i_arr[2])
        : "i" (0xDEADBEEF)
        : "rax", "memory"
    );
    
    result = *l_ptr + *i_ptr;
    return result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static int mixed_type_operations(volatile char c_in, volatile short s_in) {
    /* Mix types to create subreg/zero_extend operations */
    char c1 = c_in;
    short s1 = s_in;
    int i1 = g_volatile_seed;
    long l1 = (long)i1 * 2;
    
    /* Operations that change modes */
    int i2 = (int)c1;          /* char to int - may require zero/sign extend */
    long l2 = (long)s1;        /* short to long */
    char c2 = (char)(i1 & 0xFF); /* int to char - truncation */
    
    /* Use in arithmetic with different sizes */
    long l3 = l1 + i2 + l2;
    int i3 = (int)l3 + c2;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_count = 3;
    int sum = 0;
    
    for (volatile int j = 0; j < loop_count; j = j + 1) {
        /* Mixed operations in loop */
        char loop_char = (char)(i3 + j);
        short loop_short = (short)(s1 + j * 2);
        int loop_int = i2 + (int)loop_char + (int)loop_short;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "mov %[val], %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %[sum]\n\t"
            : [sum] "+rm" (sum)
            : [val] "rm" (loop_int)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "cc", "memory"
        );
    }
    
    /* Final conversion with possible reload */
    short final_short = (short)(sum & 0xFFFF);
    return (int)final_short + i3;
}

/* Function using bitfields and unions for complex RTL */
__attribute__((noinline))
static int use_bitfields_and_unions(int x, int y) {
    /* Union causing type punning */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = x * y + g_volatile_seed;
    
    /* Bitfield structure */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits;
    
    bits.a = x & 0x7;
    bits.b = y & 0x1F;
    bits.c = (x + y) & 0xFF;
    bits.d = g_volatile_seed & 0xFFFF;
    
    /* Access bitfields and union members in assembly */
    int result;
    asm volatile (
        "mov %[union_i], %%eax\n\t"
        "add %[bits_c], %%al\n\t"      /* Mixing sizes: add 8-bit to 32-bit */
        "movzx %[bits_a], %%ebx\n\t"   /* Zero extend 3-bit field */
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %[result]\n\t"
        : [result] "=rm" (result)
        : [union_i] "rm" (u.i),
          [bits_c] "rm" ((int)bits.c),
          [bits_a] "rm" ((int)bits.a)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Use argc to prevent constant folding */
    int base = argc > 1 ? (int)(argv[0][0]) : 100;
    
    /* Many local variables of different types */
    volatile char c1 = (char)(base + 1);
    volatile short s1 = (short)(base + 2);
    volatile int i1 = base + 3;
    volatile long l1 = base + 4;
    volatile int* ptr1 = &i1;
    volatile char* cptr = (char*)ptr1;
    
    int checksum = 0;
    
    /* Call functions repeatedly with different arguments */
    for (int i = 0; i < 5; i++) {
        checksum += use_explicit_registers(i1 + i, s1, c1, l1, base, i);
        checksum += use_volatile_addresses(cptr, i);
        checksum += mixed_type_operations(c1 + i, s1 + i);
        checksum += use_bitfields_and_unions(i1 + i, base - i);
        
        /* Modify volatiles to change values */
        c1 += 1;
        s1 += 2;
        i1 += 3;
        l1 += 4;
    }
    
    /* Additional stress with many variables in scope */
    {
        int v1 = checksum, v2 = v1 * 2, v3 = v2 / 3, v4 = v3 + 4;
        short vs1 = v1 & 0xFFFF, vs2 = v2 & 0xFFFF;
        char vc1 = v1 & 0xFF, vc2 = v2 & 0xFF;
        
        /* One more assembly block clobbering everything */
        asm volatile (
            "mov %[v1], %%eax\n\t"
            "add %[v2], %%eax\n\t"
            "add %[v3], %%eax\n\t"
            "add %[v4], %%eax\n\t"
            "movzx %[vs1], %%ebx\n\t"
            "add %%ebx, %%eax\n\t"
            "movzx %[vc1], %%ecx\n\t"
            "add %%ecx, %%eax\n\t"
            "mov %%eax, %[checksum]\n\t"
            : [checksum] "+rm" (checksum)
            : [v1] "rm" (v1), [v2] "rm" (v2),
              [v3] "rm" (v3), [v4] "rm" (v4),
              [vs1] "rm" ((int)vs1),
              [vc1] "rm" ((int)vc1)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13",
              "r14", "r15", "cc", "memory"
        );
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
