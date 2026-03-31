/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile void *g_volatile_ptr = NULL;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile char *base) {
    volatile int offset = 7;
    int result;
    
    /* Force base+index addressing with volatile components */
    asm volatile (
        "movl (%[base],%[idx],4), %[res]\n\t"
        "addl %[offset], %[res]"
        : [res] "=r" (result)
        : [base] "r" (base), [idx] "r" (idx), [offset] "ri" (offset)
        : "memory", "cc"
    );
    
    return result;
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static void register_conflict(void) {
    /* Explicit register variables that conflict with inline asm constraints */
    register uint64_t r12_var asm("r12") = g_volatile_seed + 1;
    register uint64_t r13_var asm("r13") = g_volatile_seed + 2;
    register uint64_t r14_var asm("r14") = g_volatile_seed + 3;
    
    uint64_t result1, result2;
    
    /* Inline asm with multiple alternative constraints and clobbers */
    asm volatile (
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out1]\n\t"
        "imulq %[in3], %%rax\n\t"
        "movq %%rax, %[out2]"
        : [out1] "=rm" (result1), [out2] "=rm" (result2)  /* r/m alternatives */
        : [in1] "rm" (r12_var), [in2] "rm" (r13_var), [in3] "rm" (r14_var)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Use results to prevent elimination */
    g_volatile_seed += (int)(result1 ^ result2);
}

/* Function with memory output and immediate input constraints */
__attribute__((noinline))
static void memory_constraints(volatile int *mem_out, volatile short *mem_out2) {
    int imm_val = 0x12345678;
    short imm_val2 = 0xABCD;
    
    /* Force reloads by using memory outputs with immediate inputs */
    asm volatile (
        "movl %[imm], (%[out])\n\t"
        "movw %[imm2], (%[out2])"
        : 
        : [out] "r" (mem_out), [imm] "i" (imm_val),
          [out2] "r" (mem_out2), [imm2] "i" (imm_val2)
        : "memory"
    );
    
    /* Additional asm with register constraints that conflict */
    asm volatile (
        "movl %%eax, %%ebx\n\t"
        "leal 1(%%ebx), %%ecx"
        : 
        : "a" (imm_val)
        : "rbx", "rcx", "cc"
    );
}

/* Function with mixed data types causing mode changes */
__attribute__((noinline))
static long long type_mixing(char c, short s, int i, long long ll) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    
    /* Operations that cause mode changes and extensions */
    long long result = (long long)vc;      /* zero/sign extension */
    result += (long long)vs << 8;          /* shift with different size */
    result ^= (long long)vi * 256;         /* multiplication with promotion */
    result |= ll & 0xFF;                   /* mixing sizes */
    
    /* Force subreg operations */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = vi;
    result += u.s[0] * u.c[1];  /* mixed access within union */
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static void *pointer_arithmetic(void *base, volatile int offset) {
    volatile char *volatile_ptr = (volatile char *)base;
    void *result;
    
    /* Complex addressing with volatile offset */
    asm volatile (
        "movq %[base], %%rax\n\t"
        "addq %[offset], %%rax\n\t"
        "movq %%rax, %[result]"
        : [result] "=rm" (result)  /* r/m constraint */
        : [base] "rm" (volatile_ptr), [offset] "rm" ((long long)offset)
        : "rax", "cc"
    );
    
    return result;
}

/* Function creating high register pressure */
__attribute__((noinline))
static int high_pressure(volatile int a, volatile int b, 
                         volatile int c, volatile int d) {
    /* Many local variables to increase register pressure */
    int v1 = a + 1;
    int v2 = b * 2;
    int v3 = c ^ 0xFF;
    int v4 = d >> 3;
    int v5 = a * b;
    int v6 = c + d;
    int v7 = v1 ^ v2;
    int v8 = v3 | v4;
    int v9 = v5 & v6;
    int v10 = v7 - v8;
    
    /* Use all variables in complex expression */
    int result = v1 + v2 - v3 * v4 + v5 / (v6 + 1) + 
                 (v7 << 2) | (v8 >> 1) ^ v9 & v10;
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "movl %[v1], %%eax\n\t"
        "addl %[v2], %%eax\n\t"
        "movl %%eax, %%ebx\n\t"
        "subl %[v3], %%ebx\n\t"
        "movl %%ebx, %%ecx\n\t"
        "imull %[v4], %%ecx"
        : 
        : [v1] "rm" (v1), [v2] "rm" (v2), 
          [v3] "rm" (v3), [v4] "rm" (v4)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    return result;
}

/* Main function that orchestrates all stress tests */
int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    
    /* Array with volatile elements for complex addressing */
    volatile char buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(seed + i);
    }
    
    /* Various typed variables */
    char c_var = 'A' + (seed % 26);
    short s_var = 1000 + seed;
    int i_var = seed * 17;
    long long ll_var = (long long)seed * 123456789LL;
    
    /* Pointer variables */
    volatile int mem1, mem2;
    volatile short mem3;
    int *ptr1 = &mem1;
    volatile int *volatile ptr2 = &mem2;
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    for (volatile int i = 0; i < 10; i++) {
        /* Complex addressing */
        sum += complex_addressing(i, buffer);
        
        /* Register conflict */
        register_conflict();
        
        /* Memory constraints */
        memory_constraints(&mem1, &mem3);
        sum += mem1 + mem3;
        
        /* Type mixing */
        ll_var = type_mixing(c_var + i, s_var + i, i_var + i, ll_var);
        sum += (int)(ll_var & 0xFFFFFFFF);
        
        /* Pointer arithmetic */
        void *ptr_result = pointer_arithmetic(buffer, i * 4);
        sum += (int)((uintptr_t)ptr_result & 0xFF);
        
        /* High pressure */
        sum += high_pressure(seed + i, seed - i, seed * i, seed ^ i);
        
        /* Modify variables for next iteration */
        c_var++;
        s_var += 2;
        i_var *= 3;
    }
    
    /* Final computation to ensure nothing is eliminated */
    int final_result = sum ^ g_volatile_seed;
    
    /* Use all variables one more time */
    final_result += c_var + s_var + i_var + (int)(ll_var & 0xFF);
    final_result += (int)((uintptr_t)g_volatile_ptr & 0xFF);
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
