/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int vol_seed = 42;

/* Function prototypes with noinline to prevent inlining */
int __attribute__((noinline)) func_explicit_registers(int a, int b, int c);
void __attribute__((noinline)) func_volatile_addresses(volatile int* arr, int idx);
long __attribute__((noinline)) func_mixed_types(char c, short s, int i, long l);
int __attribute__((noinline)) func_complex_constraints(int x, int y, int z);

/* Global volatile variables to force memory operations */
volatile int g_vol1 = 1;
volatile int g_vol2 = 2;
volatile char g_char_array[256];
volatile short g_short_array[128];
volatile int g_int_array[64];
volatile long g_long_array[32];

/* Function using explicit register variables with conflicting constraints */
int __attribute__((noinline)) func_explicit_registers(int a, int b, int c) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int r1 asm("r12") = a + vol_seed;
    register int r2 asm("r13") = b * 3;
    register int r3 asm("r14") = c - vol_seed;
    
    int result;
    
    /* Inline asm with mismatched constraints to force reloads */
    __asm__ volatile (
        /* Output constraint doesn't match input register classes */
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "subl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)      /* Output: register constraint */
        : "m" (r1),          /* Input: memory constraint (but r1 is in register) */
          "r" (r2),          /* Input: register constraint */
          "i" (123)          /* Input: immediate (may need reload) */
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
    
    /* More asm with clobbered registers to force spills */
    __asm__ volatile (
        "movl %1, %%ebx\n\t"
        "imull %%ebx, %%ebx\n\t"
        "addl %%ebx, %0\n\t"
        : "+r" (result)
        : "r" (r3)
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "cc"
    );
    
    return result;
}

/* Function using volatile addresses with complex addressing modes */
void __attribute__((noinline)) func_volatile_addresses(volatile int* arr, int idx) {
    volatile int local_vol = idx * 2;
    volatile char* char_ptr = (volatile char*)&local_vol;
    volatile long* long_ptr = (volatile long*)arr;
    
    /* Complex address calculation that may need base+index reload */
    int complex_idx = (idx * 3 + vol_seed) % 64;
    
    /* Inline asm with memory output and immediate input - likely needs reload */
    __asm__ volatile (
        "movl %1, (%0)\n\t"
        : 
        : "r" (&arr[complex_idx]),  /* Base+index addressing */
          "i" (0xDEADBEEF)          /* Immediate value */
        : "memory", "%rax", "%rbx"
    );
    
    /* Multiple memory operations with different constraints */
    int temp;
    __asm__ volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (g_vol1)            /* Memory output */
        : "r" (&arr[idx]),         /* Memory input via register */
          "r" (&g_vol2)            /* Memory input via register */
        : "%eax", "%rcx", "%rdx", "memory"
    );
    
    /* Pointer arithmetic that creates complex addresses */
    volatile int* ptr = arr + (idx & 0xF);
    for (int i = 0; i < 4; i++) {
        /* Mix of constraints: memory output, register input */
        __asm__ volatile (
            "movl %1, (%0)\n\t"
            : 
            : "r" (ptr + i),
              "r" (i * 0x100)
            : "memory", "%rax"
        );
    }
}

/* Function with mixed data types causing mode changes */
long __attribute__((noinline)) func_mixed_types(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations that change modes */
    long result = 0;
    
    /* char -> long extension may need reload */
    result += (long)vc * 256L;
    
    /* short -> int -> long with arithmetic */
    result += (long)(vs * vi);
    
    /* Mixed operations in asm with different sized operands */
    __asm__ volatile (
        "movsbl %1, %%eax\n\t"      /* Sign extend byte to long */
        "movswl %2, %%ebx\n\t"      /* Sign extend short to long */
        "addl %%ebx, %%eax\n\t"
        "cltq\n\t"                  /* Sign extend eax to rax */
        "addq %%rax, %0\n\t"
        : "+r" (result)
        : "m" (vc), "m" (vs)
        : "%rax", "%rbx", "%rcx", "cc"
    );
    
    /* Union to force reinterpretation */
    union {
        int i;
        char c[4];
        short s[2];
    } u;
    u.i = vi;
    
    /* Access different views of same data */
    result += u.c[0] + u.s[1];
    
    /* Bitfield operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {vc & 0x7, (vs >> 2) & 0x1F, u.c[1], (unsigned int)vl & 0xFFFF};
    
    result += bits.a + bits.b * 256 + bits.c * 65536 + bits.d;
    
    return result;
}

/* Function with multiple alternative constraints */
int __attribute__((noinline)) func_complex_constraints(int x, int y, int z) {
    int result1, result2;
    volatile int* mem_ptr = &g_vol1;
    
    /* Multiple alternative constraints: register OR memory */
    __asm__ volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %4, %%ebx\n\t"
        "subl %%ebx, %1\n\t"
        : "=r,m" (result1), "=r,m" (result2)
        : "r,m,i" (x), "r,m,i" (y), "r,m,i" (z)
        : "%eax", "%ebx", "%ecx", "%rdx", "cc"
    );
    
    /* Output to memory with register input */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*mem_ptr)
        : "r" (result1)
        : "%eax", "%rbx", "memory"
    );
    
    /* Input from memory with immediate operation */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "shll $3, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result2)
        : "m" (g_vol2)
        : "%eax", "cc"
    );
    
    return result1 + result2;
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant propagation */
    int base = vol_seed + argc;
    
    /* Many local variables of different types */
    char c1 = 'A' + (base & 0xF);
    short s1 = 1000 + (base * 2);
    int i1 = base * 100;
    long l1 = (long)base * 1000000L;
    
    char c2 = 'Z' - (base & 0xF);
    short s2 = 2000 - (base * 3);
    int i2 = base * 200;
    long l2 = (long)base * 2000000L;
    
    int arr[32];
    for (int i = 0; i < 32; i++) {
        arr[i] = (i * base) & 0xFF;
    }
    
    /* Call functions repeatedly with different arguments */
    long total = 0;
    
    total += func_explicit_registers(i1, i2, base);
    total += func_explicit_registers(s1, s2, argc);
    
    func_volatile_addresses(arr, argc);
    func_volatile_addresses((int*)g_int_array, base & 31);
    
    total += func_mixed_types(c1, s1, i1, l1);
    total += func_mixed_types(c2, s2, i2, l2);
    
    total += func_complex_constraints(base, argc, i1);
    total += func_complex_constraints(i2, s1, argc * 2);
    
    /* More complex operations to increase register pressure */
    for (volatile int i = 0; i < 8; i++) {
        /* Mix everything together */
        int temp = func_explicit_registers(i, i*2, i*3);
        total += func_mixed_types(temp & 0xFF, (temp >> 8) & 0xFFFF, 
                                 temp, (long)temp * temp);
        
        /* Complex addressing in loop */
        if (i & 1) {
            func_volatile_addresses(arr + (i * 2), i);
        }
    }
    
    /* Use all global arrays to prevent elimination */
    for (int i = 0; i < 256 && i < argc * 8; i++) {
        g_char_array[i] = (total + i) & 0xFF;
    }
    for (int i = 0; i < 128 && i < argc * 4; i++) {
        g_short_array[i] = (total + i * 256) & 0xFFFF;
    }
    for (int i = 0; i < 64 && i < argc * 2; i++) {
        g_int_array[i] = (int)(total + i * 65536);
    }
    for (int i = 0; i < 32 && i < argc; i++) {
        g_long_array[i] = total + (long)i * 0x1000000;
    }
    
    /* Final checksum calculation */
    long checksum = total;
    for (int i = 0; i < 32; i++) {
        checksum += arr[i];
        checksum += g_int_array[i % 64];
    }
    
    printf("Result: %ld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
