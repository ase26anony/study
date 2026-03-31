#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline, optimize("O0")))
int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - compete for specific registers */
    register int x1 asm("r12") = a + 1;
    register int x2 asm("r13") = b + 2;
    register int x3 asm("r14") = c + 3;
    register int x4 asm("r15") = d + 4;
    
    /* Mixed operations with different types */
    char c1 = (char)(x1 & 0xFF);
    short s1 = (short)(x2 & 0xFFFF);
    long l1 = (long)x3 * (long)x4;
    
    /* Inline assembly with mismatched constraints */
    int result;
    asm volatile (
        /* Output constraint: memory, input: register - may need reload */
        "movl %[in1], %[out]\n\t"
        "addl %[in2], %[out]\n\t"
        "subl %[in3], %[out]\n\t"
        : [out] "=m" (result)          /* Memory output */
        : [in1] "r" (x1),              /* Register input */
          [in2] "r" (x2),
          [in3] "r" (x3)
        : "memory", "cc", "rax", "rbx", "rcx", "rdx"
    );
    
    /* More operations with mode changes */
    result += (int)c1 + (int)s1 + (int)(l1 & 0xFFFFFFFF);
    
    /* Another asm with immediate input and memory output */
    int temp;
    asm volatile (
        "movl $0x12345678, %0\n\t"
        "addl %%r12d, %0\n\t"
        : "=r" (temp)                  /* Register output */
        :                               /* No inputs */
        : "cc"
    );
    
    return result + temp + e + f;
}

/* Function using volatile addresses and complex addressing */
__attribute__((noinline))
int func_volatile_addressing(int idx1, int idx2) {
    volatile int varr[32];
    volatile char cvarr[64];
    volatile short svarr[48];
    
    /* Initialize with non-constant indices */
    for (int i = 0; i < 32; i++) {
        varr[i] = g_volatile_seed + i * 3;
    }
    
    /* Complex address calculations */
    int* ptr1 = (int*)&varr[idx1 % 32];
    char* ptr2 = (char*)&cvarr[idx2 % 64];
    short* ptr3 = (short*)&svarr[(idx1 + idx2) % 48];
    
    /* Inline assembly with memory constraints and clobbers */
    int sum = 0;
    asm volatile (
        /* Multiple memory accesses with different addressing modes */
        "movl (%[p1]), %%eax\n\t"
        "addw (%[p3]), %%ax\n\t"
        "movsbl (%[p2]), %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[sum]\n\t"
        : [sum] "=rm" (sum)            /* Register or memory output */
        : [p1] "r" (ptr1),
          [p2] "r" (ptr2),
          [p3] "r" (ptr3)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
    );
    
    /* Pointer arithmetic creating complex addressing */
    int* volatile_ptr = (int*)((uintptr_t)ptr1 + (uintptr_t)ptr2 - (uintptr_t)ptr3);
    
    /* Another asm with immediate to memory */
    asm volatile (
        "movl $0xDEADBEEF, (%0)\n\t"
        : 
        : "r" (volatile_ptr)
        : "memory"
    );
    
    return sum + *ptr1 + idx1 + idx2;
}

/* Function with mixed types and mode conversions */
__attribute__((noinline))
int func_mixed_types(volatile int loop_count) {
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = g_volatile_seed;
    
    /* Mixed type operations in loop */
    long long accumulator = 0;
    volatile int counter = loop_count;
    
    while (counter-- > 0) {
        /* Operations causing mode changes */
        char c_val = u.c[counter % 4];
        short s_val = u.s[counter % 2];
        int i_val = u.i;
        
        /* Inline asm with alternative constraints */
        long long temp;
        asm volatile (
            /* Alternative constraints: register or memory for input */
            "movsx %w[char_in], %[temp]\n\t"
            "add %[short_in], %[temp]\n\t"
            "add %[int_in], %[temp]\n\t"
            : [temp] "=&r" (temp)      /* Early clobber output */
            : [char_in] "rm" ((int)c_val),   /* Register or memory */
              [short_in] "rm" ((int)s_val),
              [int_in] "rm" (i_val)
            : "cc"
        );
        
        accumulator += temp;
        
        /* Change union contents */
        u.i += (int)accumulator;
    }
    
    /* Final asm with many clobbers */
    int result;
    asm volatile (
        "movq %[acc], %%rax\n\t"
        "shrq $32, %%rax\n\t"
        "addl %[acc], %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=rm" (result)
        : [acc] "rm" (accumulator)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    return result;
}

/* Function creating register pressure with many variables */
__attribute__((noinline))
int func_register_pressure(int a, int b, int c, int d, int e, int f,
                           int g, int h, int i, int j, int k, int l) {
    /* Many local variables of different types */
    char c1 = a & 0xFF;
    char c2 = b & 0xFF;
    short s1 = c & 0xFFFF;
    short s2 = d & 0xFFFF;
    int i1 = e;
    int i2 = f;
    long l1 = (long)g * h;
    long l2 = (long)i * j;
    int* p1 = &i1;
    int* p2 = &i2;
    volatile int v1 = k;
    volatile int v2 = l;
    
    /* Complex expression mixing all variables */
    int result = (int)c1 + (int)c2 * 2;
    result += (int)s1 - (int)s2;
    result += i1 * i2;
    result += (int)(l1 & 0xFFFFFFFF) + (int)(l2 >> 32);
    result += *p1 + *p2;
    result += v1 + v2;
    
    /* Inline asm using many variables with constraints */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "addl %[r2], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "+rm" (result)
        : [r1] "rm" (result),
          [r2] "rm" (g_volatile_seed)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "r14", "r15", "memory", "cc"
    );
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Use arguments to prevent constant folding */
    int arg_base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Many local variables to increase register pressure */
    int v1 = arg_base + 1;
    int v2 = arg_base + 2;
    int v3 = arg_base + 3;
    int v4 = arg_base + 4;
    int v5 = arg_base + 5;
    int v6 = arg_base + 6;
    int v7 = arg_base + 7;
    int v8 = arg_base + 8;
    int v9 = arg_base + 9;
    int v10 = arg_base + 10;
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    sum += func_explicit_registers(v1, v2, v3, v4, v5, v6);
    sum += func_volatile_addressing(v1 % 10, v2 % 10);
    sum += func_mixed_types(v3 % 8 + 1);
    
    /* Call with many arguments to force stack usage */
    sum += func_register_pressure(
        v1, v2, v3, v4, v5, v6,
        v7, v8, v9, v10, sum & 0xFF, g_volatile_seed
    );
    
    /* More calls with different patterns */
    for (volatile int i = 0; i < 3; i++) {
        sum += func_explicit_registers(
            sum & 0xFF, 
            (sum >> 8) & 0xFF,
            v1 + i, v2 - i, v3 * i, v4 / (i + 1)
        );
        
        sum += func_volatile_addressing(i, sum % 32);
    }
    
    /* Final mixed types call */
    sum += func_mixed_types(sum % 5 + 2);
    
    /* Compute checksum and print */
    printf("Result checksum: %d\n", sum);
    
    /* Use result to prevent dead code elimination */
    return sum == 0 ? 1 : 0;
}
