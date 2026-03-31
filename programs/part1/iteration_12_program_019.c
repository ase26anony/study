/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_explicit_registers(int arg1, int arg2) {
    /* Explicit register variables create fixed register constraints */
    register int x asm("r12") = arg1 + g_volatile_seed;
    register int y asm("r13") = arg2 - g_volatile_seed;
    register int z asm("r14") = 0;
    
    /* Inline assembly with conflicting constraints */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "movl %[y], %[z]\n\t"
        : [z] "=r" (z)          /* Output in register */
        : [x] "r,m" (x),        /* Input with alternative constraints */
          [y] "0,r" (y)         /* Input matching output constraint */
        : "cc", "memory"
    );
    
    /* More assembly with mismatched modes */
    unsigned char byte_val = (unsigned char)z;
    unsigned long long qword_val;
    
    asm volatile (
        "movzbl %[byte], %k[qword]\n\t"
        "shlq $32, %[qword]\n\t"
        : [qword] "=r" (qword_val)
        : [byte] "r,m" (byte_val)
        : "cc"
    );
    
    /* Force spill by clobbering many registers */
    asm volatile (
        "nop"
        : 
        : "r" (x), "r" (y), "r" (z)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc", "memory"
    );
}

/* Function using volatile addresses and memory constraints */
__attribute__((noinline))
static void use_volatile_addresses(volatile int* ptr1, volatile short* ptr2) {
    volatile int local_vol1 = *ptr1 + g_volatile_seed;
    volatile short local_vol2 = *ptr2 - g_volatile_seed;
    
    /* Take addresses of volatile variables */
    volatile int* addr1 = &local_vol1;
    volatile short* addr2 = &local_vol2;
    
    /* Pointer arithmetic to create complex addresses */
    volatile int* complex_addr1 = addr1 + (g_volatile_seed & 0x3);
    volatile short* complex_addr2 = addr2 + (g_volatile_seed & 0x7);
    
    /* Inline assembly with memory output and immediate input */
    /* This often requires reloads */
    asm volatile (
        "movl %[imm], (%[mem])\n\t"
        : [mem] "=m" (*complex_addr1)
        : [imm] "i" (0x12345678)
        : "memory"
    );
    
    /* Mixed size operations with memory constraints */
    unsigned char byte_data = (unsigned char)*complex_addr1;
    unsigned short word_data;
    
    asm volatile (
        "movb %[byte], %%al\n\t"
        "movw %%ax, %[word]\n\t"
        : [word] "=m" (*complex_addr2)
        : [byte] "r,m" (byte_data)
        : "ax", "memory"
    );
    
    /* More complex addressing with index */
    volatile int array[10];
    volatile int index = g_volatile_seed % 10;
    
    asm volatile (
        "imull $4, %[idx]\n\t"
        "addl %[base], %[idx]\n\t"
        "movl $99, (%[idx])\n\t"
        : 
        : [base] "r" (array), [idx] "r" (index)
        : "memory", "cc"
    );
}

/* Function with mixed types and mode conversions */
__attribute__((noinline))
static void mixed_type_operations(long arg) {
    /* Mix different integer types */
    char c1 = (char)(arg & 0xFF);
    short s1 = (short)((arg >> 8) & 0xFFFF);
    int i1 = (int)(arg >> 16);
    long long ll1 = (long long)arg * 3;
    
    /* Operations causing mode changes */
    volatile char vc = c1;
    volatile short vs = s1;
    volatile int vi = i1;
    volatile long long vll = ll1;
    
    /* Mixed operations requiring extensions/truncations */
    long long result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    volatile int counter = 5;
    while (counter--) {
        /* Each iteration uses different type combinations */
        result += (long long)vc * (counter + 1);
        result += (long long)vs * (counter + 2);
        result += (long long)vi * (counter + 3);
        result -= (long long)vll / (counter + 4);
        
        /* Bitfield operations */
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
        } bits = {vc & 0x7, (vc >> 3) & 0x1F, vs & 0xFF};
        
        result ^= ((long long)bits.a << 32) | bits.b | (bits.c << 8);
        
        /* Union causing type punning */
        union {
            float f;
            int i;
        } u;
        u.i = vi + counter;
        result += (long long)u.i;
    }
    
    /* Final assembly with many constraints */
    long long final_result;
    asm volatile (
        "movq %[in], %%rax\n\t"
        "shrq $3, %%rax\n\t"
        "addq %%rax, %[out]\n\t"
        "movq %[out], %%rbx\n\t"
        : [out] "=r,m" (final_result)
        : [in] "r,m,i" (result)
        : "rax", "rbx", "cc"
    );
    
    /* Use the result to prevent elimination */
    g_volatile_seed += (int)(final_result & 0x7FFFFFFF);
}

/* Function creating register pressure with many live variables */
__attribute__((noinline))
static void high_register_pressure(int iterations) {
    /* Declare many variables of different types */
    int a1 = g_volatile_seed + 1;
    char a2 = (char)(g_volatile_seed + 2);
    short a3 = (short)(g_volatile_seed + 3);
    long a4 = (long)(g_volatile_seed + 4);
    int* a5 = (int*)&g_volatile_seed;
    char* a6 = (char*)&g_volatile_seed;
    
    int b1 = a1 * 2;
    char b2 = a2 + 1;
    short b3 = a3 - 1;
    long b4 = a4 / 2;
    int* b5 = a5 + 1;
    char* b6 = a6 + 1;
    
    int c1 = b1 ^ a1;
    char c2 = b2 | a2;
    short c3 = b3 & a3;
    long c4 = b4 + a4;
    int* c5 = b5 - 1;
    char* c6 = b6 - 1;
    
    /* Use all variables in complex expressions */
    for (volatile int i = 0; i < iterations; i++) {
        a1 = (a1 * 1103515245 + 12345) & 0x7FFFFFFF;
        a2 = (char)(a1 >> 16);
        a3 = (short)(a1 >> 8);
        a4 = (long)a1 * a4;
        
        /* Pointer arithmetic with different scales */
        b5 = (int*)((char*)b5 + (a2 & 0x3));
        b6 = (char*)((int*)b6 + (a2 & 0x1));
        
        /* Mixed operations */
        c1 = (int)((long)c1 * c4 / (a4 ? a4 : 1));
        c2 = (char)(c2 + a2 + b2);
        c3 = (short)(c3 - a3 + b3);
        
        /* Inline assembly using multiple variables */
        asm volatile (
            "movl %[v1], %%eax\n\t"
            "addb %[v2], %%al\n\t"
            "addw %[v3], %%ax\n\t"
            "movl %%eax, %[v4]\n\t"
            : [v4] "=m" (c1)
            : [v1] "r,m" (a1),
              [v2] "r,m" (a2),
              [v3] "r,m" (a3)
            : "eax", "cc", "memory"
        );
    }
    
    /* Final computation using all variables */
    int checksum = a1 + a2 + a3 + (int)a4 + 
                   (int)((intptr_t)b5 >> 2) + (int)((intptr_t)b6 >> 2) +
                   c1 + c2 + c3 + (int)c4 + 
                   (int)((intptr_t)c5 >> 2) + (int)((intptr_t)c6 >> 2);
    
    g_volatile_seed = checksum & 0xFF;
}

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    volatile int* volatile_ptr = &g_volatile_seed;
    volatile short volatile_short = (short)g_volatile_seed;
    
    /* Call functions repeatedly with different arguments */
    for (int i = 0; i < 10; i++) {
        use_explicit_registers(base + i, base - i);
        use_volatile_addresses(volatile_ptr, &volatile_short);
        mixed_type_operations((long)base * i + g_volatile_seed);
        high_register_pressure(3 + (i % 5));
        
        /* Modify base to create different patterns */
        base = (base * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Compute final checksum */
    long long final_checksum = g_volatile_seed;
    final_checksum = (final_checksum * 6364136223846793005ULL) + 1442695040888963407ULL;
    
    /* Use checksum in output to prevent dead code elimination */
    printf("Result: %lld\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF) % 256;
}
