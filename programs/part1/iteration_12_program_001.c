/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_explicit_registers(int a, int b) {
    /* Explicit register variables create fixed register constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    register int z asm("r14") = 0;
    
    /* Inline assembly with mismatched constraints */
    asm volatile (
        "addl %[x], %[z]\n\t"
        "subl %[y], %[z]\n\t"
        : [z] "+r" (z)          /* Output operand in register */
        : [x] "rm" (x),         /* Input can be register or memory */
          [y] "rm" (y)          /* Another input with alternative constraint */
        : "cc"                  /* Clobber flags */
    );
    
    /* More assembly with conflicting constraints */
    int temp;
    asm volatile (
        "movl %[z], %[temp]\n\t"
        "imull %[x], %[temp]\n\t"
        : [temp] "=r" (temp)    /* Output must be register */
        : [z] "rm" (z),         /* Input can be register or memory */
          [x] "r" (x)           /* Input must be register - may force reload */
        : "cc"
    );
    
    /* Store result to volatile to prevent elimination */
    *(volatile int*)&g_volatile_seed = temp;
}

/* Function using volatile addresses and memory constraints */
__attribute__((noinline))
static void use_volatile_addresses(volatile int* ptr1, volatile short* ptr2) {
    volatile char c = 127;
    volatile long long ll = 0x123456789ABCDEF0LL;
    
    /* Take addresses of volatile variables */
    volatile char* cp = &c;
    volatile long long* llp = &ll;
    
    /* Pointer arithmetic creates complex addressing */
    int offset = g_volatile_seed & 0xF;
    volatile char* cp2 = cp + offset;
    volatile long long* llp2 = llp - offset;
    
    /* Inline assembly with memory output and immediate input */
    asm volatile (
        "movb %[imm], (%[mem])\n\t"
        : [mem] "=m" (*cp2)     /* Memory output constraint */
        : [imm] "i" (65)        /* Immediate input - may need reload if not constant */
        : "memory"
    );
    
    /* Assembly with multiple clobbered registers */
    asm volatile (
        "movq (%[src]), %%rax\n\t"
        "addq $0x1000, %%rax\n\t"
        "movq %%rax, (%[dst])\n\t"
        : 
        : [src] "r" (llp), [dst] "r" (llp2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
    );
    
    /* Mixed size operations */
    short s = *ptr2;
    asm volatile (
        "movw %w[s], (%[addr])\n\t"
        : 
        : [s] "r" (s), [addr] "r" (ptr2)
        : "memory"
    );
}

/* Function with mixed types and mode changes */
__attribute__((noinline))
static void mixed_type_operations(unsigned count) {
    volatile char cv = 'A';
    volatile short sv = 1000;
    volatile int iv = 1000000;
    volatile long long llv = 0;
    
    /* Loop with volatile counter to prevent optimization */
    volatile unsigned i = 0;
    for (i = 0; i < count; i = i + 1) {
        /* Mixed type operations requiring extensions/truncations */
        char c_temp = cv + (char)i;
        short s_temp = sv + (short)(c_temp * 2);
        int i_temp = iv + (int)(s_temp * 3);
        long long ll_temp = llv + (long long)(i_temp * 4LL);
        
        /* Bitfield operations */
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 24;
        } bf;
        
        bf.a = i & 0x7;
        bf.b = (i >> 3) & 0x1F;
        bf.c = i_temp & 0xFFFFFF;
        
        /* Union causing type punning */
        union {
            float f;
            uint32_t u;
        } u;
        
        u.u = (bf.c << 8) | (bf.b << 3) | bf.a;
        
        /* Inline assembly with subreg operations */
        uint32_t union_val = u.u;
        asm volatile (
            "movl %[val], %%eax\n\t"
            "shrl $1, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (union_val)  /* Output can be reg or mem */
            : [val] "rm" (union_val)   /* Input can be reg or mem */
            : "eax", "cc"
        );
        
        /* Update volatiles to prevent elimination */
        cv = c_temp;
        sv = s_temp;
        iv = i_temp;
        llv = ll_temp;
    }
}

/* Function with complex constraints and many operands */
__attribute__((noinline))
static void many_operands_constraints(int a, int b, int c, int d, 
                                      int e, int f, int g, int h) {
    /* Many variables to increase register pressure */
    int v1 = a + b;
    int v2 = c + d;
    int v3 = e + f;
    int v4 = g + h;
    int v5 = a * c;
    int v6 = b * d;
    int v7 = e * g;
    int v8 = f * h;
    
    /* Inline assembly with many operands and alternatives */
    asm volatile (
        "addl %[v2], %[v1]\n\t"
        "subl %[v3], %[v4]\n\t"
        "imull %[v5], %[v6]\n\t"
        "orl %[v7], %[v8]\n\t"
        : [v1] "+r,m" (v1), [v4] "+r,m" (v4),
          [v6] "+r,m" (v6), [v8] "+r,m" (v8)
        : [v2] "r,m,i" (v2), [v3] "r,m,i" (v3),
          [v5] "r,m,i" (v5), [v7] "r,m,i" (v7)
        : "cc"
    );
    
    /* More assembly with output memory constraints */
    int results[4];
    asm volatile (
        "movl %[v1], %[r0]\n\t"
        "movl %[v4], %[r1]\n\t"
        "movl %[v6], %[r2]\n\t"
        "movl %[v8], %[r3]\n\t"
        : [r0] "=m" (results[0]),
          [r1] "=m" (results[1]),
          [r2] "=m" (results[2]),
          [r3] "=m" (results[3])
        : [v1] "r" (v1), [v4] "r" (v4),
          [v6] "r" (v6), [v8] "r" (v8)
        : "memory"
    );
    
    /* Use results to prevent elimination */
    g_volatile_seed = results[0] + results[1] + results[2] + results[3];
}

int main(int argc, char* argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    
    /* Many local variables of different types */
    char c1 = 'a' + (seed & 0xF);
    short s1 = 100 + (seed & 0xFF);
    int i1 = 1000 + seed;
    long long ll1 = 10000LL + seed * 2LL;
    
    volatile int* ptr1 = (volatile int*)&i1;
    volatile short* ptr2 = (volatile short*)&s1;
    
    /* Call functions multiple times with different arguments */
    for (int j = 0; j < 10; j++) {
        use_explicit_registers(seed + j, seed - j);
        use_volatile_addresses(ptr1, ptr2);
        mixed_type_operations(5 + (j % 3));
        many_operands_constraints(
            seed, seed + 1, seed + 2, seed + 3,
            seed + 4, seed + 5, seed + 6, seed + 7
        );
        
        /* Modify variables to change inputs */
        c1 += j;
        s1 -= j;
        i1 *= (j + 1);
        ll1 >>= 1;
    }
    
    /* Compute checksum to ensure no code is eliminated */
    unsigned long checksum = (unsigned char)c1 + s1 + i1 + (unsigned long)ll1;
    checksum += g_volatile_seed;
    
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
