#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create register pressure with explicit register variables */
__attribute__((noinline))
int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - conflicting with normal allocation */
    register int x1 asm("r12") = a + g_volatile_seed;
    register int x2 asm("r13") = b * 2;
    register int x3 asm("r14") = c ^ 0xFF;
    register int x4 asm("r15") = d | 0x1234;
    
    int result;
    
    /* Inline assembly with mismatched constraints */
    asm volatile (
        "addl %[x2], %[x1]\n\t"
        "subl %[x3], %[x1]\n\t"
        "xorl %[x4], %[x1]\n\t"
        "movl %[x1], %[result]"
        : [result] "=r,m" (result)      /* Alternative constraints */
        : [x1] "0,r" (x1),              /* Matching constraint with alternative */
          [x2] "r,m" (x2),
          [x3] "r,m" (x3),
          [x4] "r,m" (x4)
        : "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi"  /* Clobber many registers */
    );
    
    return result + e + f;
}

/* Function using volatile addresses and memory constraints */
__attribute__((noinline))
void func_volatile_addresses(volatile int* arr, int size) {
    volatile char c_var = 65;
    volatile short s_var = 1000;
    volatile long l_var = 999999;
    
    /* Take addresses of volatile variables */
    volatile char* c_ptr = &c_var;
    volatile short* s_ptr = &s_var;
    volatile long* l_ptr = &l_var;
    
    /* Pointer arithmetic to create complex addresses */
    volatile char* c_ptr2 = c_ptr + (g_volatile_seed & 0xF);
    volatile short* s_ptr2 = s_ptr + (g_volatile_seed & 0x7);
    volatile long* l_ptr2 = l_ptr + (g_volatile_seed & 0x3);
    
    /* Inline assembly with memory output and immediate input */
    for (volatile int i = 0; i < size && i < 10; i++) {
        asm volatile (
            "movl %[imm], %[mem]\n\t"
            "addl %%eax, %[mem2]\n\t"
            "orl %%ebx, %[mem3]"
            : [mem] "=m" (*c_ptr2),     /* Memory output constraint */
              [mem2] "+m" (*s_ptr2),
              [mem3] "+m" (*l_ptr2)
            : [imm] "i" (0xDEADBEEF),   /* Immediate input */
              "a" (arr[i]),             /* Input in eax */
              "b" (i)                   /* Input in ebx */
            : "cc", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
        );
        
        /* More pointer arithmetic */
        c_ptr2 += (i & 1);
        s_ptr2 += (i & 2) >> 1;
        l_ptr2 += (i & 4) >> 2;
    }
}

/* Function with mixed types and mode changes */
__attribute__((noinline))
long func_mixed_types(char c1, short s1, int i1, long l1) {
    volatile char vc = c1;
    volatile short vs = s1;
    volatile int vi = i1;
    volatile long vl = l1;
    
    /* Union to force type punning */
    union {
        char c[8];
        short s[4];
        int i[2];
        long l;
        void* p;
    } u;
    
    /* Mixed type operations causing mode changes */
    u.l = 0;
    u.c[0] = vc;
    u.s[1] = vs + vc;           /* char to short promotion */
    u.i[0] = vi * u.s[1];       /* short to int promotion */
    u.l = u.i[0] + vl;          /* int to long promotion */
    
    /* Cast between integer and pointer types */
    uintptr_t addr = (uintptr_t)&u;
    addr += (vc << 8) | (vs & 0xFF);
    void* ptr = (void*)addr;
    
    /* Bit-field operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {vc & 0x7, (vs >> 3) & 0x1F, vi & 0xFF, (unsigned int)vl & 0xFFFF};
    
    /* Complex expression with many temporaries */
    long result = (long)u.l 
                + (long)(bits.a * bits.b) 
                + (long)(bits.c << bits.d)
                + (long)((uintptr_t)ptr & 0xFFFF);
    
    /* Inline assembly with subreg operations */
    asm volatile (
        "movzbl %[char], %%eax\n\t"     /* Zero extend char to 32-bit */
        "movswl %[short], %%ebx\n\t"    /* Sign extend short to 32-bit */
        "addl %%ebx, %%eax\n\t"
        "cltq\n\t"                      /* Sign extend eax to rax */
        "addq %%rax, %[result]\n\t"
        "movq %[result], %[result]"     /* Force reload */
        : [result] "+r,m" (result)      /* Alternative constraints */
        : [char] "r,m" (vc),
          [short] "r,m" (vs)
        : "rax", "rbx", "cc", "rdx", "rcx", "rdi", "rsi"
    );
    
    return result;
}

/* Function creating complex addressing modes */
__attribute__((noinline))
int func_complex_addressing(int* base, volatile int index1, volatile int index2) {
    int result = 0;
    
    /* Complex array access with volatile indices */
    for (volatile int i = 0; i < 8; i++) {
        /* Address calculation that may need reloads */
        int* addr1 = base + index1 + i;
        int* addr2 = base + index2 + (i * 2);
        int* addr3 = addr1 + (index1 & 0x3);
        
        /* Multiple memory accesses in one asm */
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "addl (%[addr2]), %%eax\n\t"
            "movl %%eax, (%[addr3])\n\t"
            "addl %%eax, %[result]"
            : [result] "+r,m" (result)
            : [addr1] "r,m" (addr1),
              [addr2] "r,m" (addr2),
              [addr3] "r,m" (addr3)
            : "rax", "cc", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = g_volatile_seed + argc;
    
    /* Many local variables of different types */
    char c1 = seed & 0xFF;
    short s1 = (seed * 2) & 0xFFFF;
    int i1 = seed * 3;
    long l1 = seed * 1000L;
    
    int arr[32];
    for (volatile int i = 0; i < 32; i++) {
        arr[i] = (i * seed) & 0xFFF;
    }
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    sum += func_explicit_registers(c1, s1, i1, l1 & 0xFFFF, 
                                   (l1 >> 16) & 0xFFFF, (l1 >> 32) & 0xFFFF);
    
    func_volatile_addresses(arr, 32);
    
    for (volatile int i = 0; i < 8; i++) {
        sum += func_mixed_types(c1 + i, s1 - i, i1 ^ i, l1 + i * 1000);
    }
    
    volatile int idx1 = seed & 0x7;
    volatile int idx2 = (seed >> 3) & 0x7;
    sum += func_complex_addressing(arr, idx1, idx2);
    
    /* Compute checksum from all modified data */
    long checksum = sum;
    for (volatile int i = 0; i < 32; i++) {
        checksum += arr[i];
    }
    
    /* Use checksum to prevent elimination */
    printf("Checksum: %ld\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
