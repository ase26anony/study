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
    register int z asm("r14") = c ^ d;
    
    int result;
    
    /* Inline assembly with mismatched constraints to force reloads */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "subl %[z], %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r,m" (result)      /* Multiple alternatives */
        : [x] "r,m" (x),                /* Input with alternatives */
          [y] "r,m" (y),
          [z] "r,m" (z)
        : "eax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Force more register pressure */
    asm volatile (
        "imull %[e], %[result]"
        : [result] "+r,m" (result)
        : [e] "r,m" (e)
        : "cc"
    );
    
    return result + f;
}

/* Function using volatile addresses and memory constraints */
__attribute__((noinline))
void func_volatile_addresses(volatile int* arr, int size) {
    volatile char c1 = 25;
    volatile short s1 = 1000;
    volatile long l1 = 99999;
    
    /* Take addresses of volatile variables */
    volatile char* pc = &c1;
    volatile short* ps = &s1;
    volatile long* pl = &l1;
    
    /* Complex addressing with volatile indices */
    volatile int idx = g_volatile_seed % size;
    
    /* Inline assembly with memory output and immediate input */
    for (int i = 0; i < 3; i++) {
        asm volatile (
            "movl %[imm], (%[addr])"
            : 
            : [imm] "i" (0xDEADBEEF),   /* Immediate constraint */
              [addr] "r" (&arr[idx + i]) /* Register containing address */
            : "memory"
        );
    }
    
    /* Mixed mode operations with memory constraints */
    long temp;
    asm volatile (
        "movsbl (%[char_ptr]), %%eax\n\t"
        "movswl (%[short_ptr]), %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cltq\n\t"                      /* Sign extend to 64-bit */
        "addq (%[long_ptr]), %%rax\n\t"
        "movq %%rax, %[temp]"
        : [temp] "=m" (temp)            /* Memory output */
        : [char_ptr] "r" (pc),
          [short_ptr] "r" (ps),
          [long_ptr] "r" (pl)
        : "rax", "rbx", "rcx", "cc", "memory"
    );
    
    /* Store result back through volatile pointer */
    *pl = temp;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
int func_mixed_types(int iterations) {
    volatile char cv = 7;
    volatile short sv = 30000;
    volatile int iv = 1000000;
    volatile long long llv = 0x123456789ABCDEF;
    
    /* Union to force type punning */
    union {
        int i;
        float f;
        char c[4];
    } pun;
    
    int sum = 0;
    volatile int counter = iterations;
    
    while (counter-- > 0) {
        /* Operations causing mode changes */
        char c_temp = cv + counter;
        short s_temp = sv - c_temp;     /* char to short promotion */
        int i_temp = iv * s_temp;       /* short to int promotion */
        long long ll_temp = llv / (i_temp + 1);
        
        /* Bitfield operations */
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 24;
        } bits = {c_temp & 0x7, (c_temp >> 3) & 0x1F, i_temp & 0xFFFFFF};
        
        /* Type punning through union */
        pun.i = bits.c;
        pun.c[0] = bits.a;
        
        /* Inline assembly with subreg operations */
        int result;
        asm volatile (
            "movzbl %[char_val], %%eax\n\t"     /* Zero extend char */
            "movswl %[short_val], %%ebx\n\t"    /* Sign extend short */
            "addl %%ebx, %%eax\n\t"
            "addl %[int_val], %%eax\n\t"
            "movl %%eax, %[result]"
            : [result] "=r,m" (result)
            : [char_val] "r,m" (c_temp),
              [short_val] "r,m" (s_temp),
              [int_val] "r,m" (pun.i)
            : "eax", "ebx", "cc"
        );
        
        sum += result;
        
        /* Pointer arithmetic with mixed types */
        char* ptr = (char*)&ll_temp;
        ptr += counter % 8;
        cv = *ptr;
    }
    
    return sum;
}

/* Function with complex addressing modes */
__attribute__((noinline))
int func_complex_addressing(int* base, int offset1, int offset2) {
    volatile int voff1 = offset1;
    volatile int voff2 = offset2;
    
    /* Complex address calculation */
    int* addr1 = base + voff1;
    int* addr2 = base + voff2;
    int* addr3 = addr1 + (voff2 / 2);
    
    int results[3];
    
    /* Multiple memory accesses with different addressing */
    asm volatile (
        "movl (%[a1]), %%eax\n\t"
        "addl (%[a2]), %%eax\n\t"
        "addl (%[a3]), %%eax\n\t"
        "movl %%eax, %[r0]"
        : [r0] "=m" (results[0])
        : [a1] "r" (addr1),
          [a2] "r" (addr2),
          [a3] "r" (addr3)
        : "eax", "memory"
    );
    
    /* More constraints with immediate offsets */
    asm volatile (
        "movl 4(%[base]), %%eax\n\t"
        "imull %[imm], %%eax\n\t"
        "movl %%eax, %[r1]"
        : [r1] "=m" (results[1])
        : [base] "r" (base),
          [imm] "i" (g_volatile_seed)
        : "eax", "memory"
    );
    
    return results[0] + results[1];
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant folding */
    int seed = g_volatile_seed + argc;
    
    /* Array with volatile index access */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = (i * seed) & 0xFF;
    }
    
    /* Call functions to stress reload pass */
    int r1 = func_explicit_registers(seed, seed+1, seed+2, seed+3, seed+4, seed+5);
    
    func_volatile_addresses(array, 100);
    
    int r2 = func_mixed_types(10 + (seed % 5));
    
    int r3 = func_complex_addressing(array, seed % 50, (seed * 3) % 50);
    
    /* Compute checksum to prevent elimination */
    int checksum = r1 + r2 + r3;
    for (int i = 0; i < 100; i++) {
        checksum ^= array[i];
    }
    
    printf("Result: %d (seed: %d)\n", checksum, seed);
    
    return checksum != 0 ? 0 : 1;
}
