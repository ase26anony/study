/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_explicit_registers(int arg1, int arg2) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = arg1 + g_volatile_seed;
    register int y asm("r13") = arg2 - g_volatile_seed;
    register int z asm("r14") = arg1 * arg2;
    
    int result1, result2;
    
    /* Inline asm with mismatched constraints forcing reloads */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[res1]\n\t"
        "imull %[z], %%eax\n\t"
        "movl %%eax, %[res2]"
        : [res1] "=m" (result1), [res2] "=m" (result2)
        : [x] "r,m" (x), [y] "r,m" (y), [z] "r,m" (z)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    /* Use results to prevent elimination */
    g_volatile_seed += result1 + result2;
}

/* Function with memory operands and immediate inputs */
__attribute__((noinline))
static void memory_operand_reloads(volatile int* ptr, int idx) {
    volatile int arr[16];
    volatile short sarr[32];
    volatile char carr[64];
    
    /* Complex addressing with volatile index */
    int index = idx + g_volatile_seed;
    
    /* Multiple asm statements with memory output and immediate input */
    asm volatile (
        "movl $0x12345678, %[dest]"
        : [dest] "=m" (arr[index & 0xF])
        :
        : "memory"
    );
    
    asm volatile (
        "movw $0xABCD, %[dest]"
        : [dest] "=m" (sarr[(index + 1) & 0x1F])
        :
        : "memory"
    );
    
    /* Mixed size operations requiring mode changes */
    char cval = carr[index & 0x3F];
    long long lval = (long long)cval * 0x100000001LL;
    
    asm volatile (
        "addq %[input], %[dest]\n\t"
        "movq %[dest], %[dest2]"
        : [dest] "+r" (lval), [dest2] "=m" (arr[(index + 2) & 0xF])
        : [input] "i" (0xFFFFFFFF)
        : "cc"
    );
    
    *ptr = (int)lval + arr[0] + sarr[0];
}

/* Function with mixed types and conversions */
__attribute__((noinline))
static void mixed_type_operations(int count) {
    volatile char c = g_volatile_seed & 0xFF;
    volatile short s = g_volatile_seed & 0xFFFF;
    volatile int i = g_volatile_seed;
    volatile long long ll = g_volatile_seed;
    
    /* Pointer arithmetic with different types */
    char* cp = (char*)&ll;
    short* sp = (short*)&i;
    int* ip = (int*)&ll;
    
    /* Loop with volatile counter to prevent optimization */
    volatile int v;
    for (v = 0; v < count; v++) {
        /* Operations causing mode changes and subregs */
        c = (c + v) & 0xFF;
        s = (s + (short)c * 2) & 0xFFFF;
        i = i + (int)s * 3;
        ll = ll + (long long)i * 4;
        
        /* Pointer accesses with different alignments */
        cp[v & 7] = c;
        sp[(v + 1) & 3] = s;
        ip[(v + 2) & 1] = i;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "mov %[cval], %%al\n\t"
            "mov %[sval], %%bx\n\t"
            "mov %[ival], %%ecx\n\t"
            "mov %[llval], %%rdx\n\t"
            "add %%al, %%bl\n\t"
            "add %%bx, %%cx\n\t"
            "add %%ecx, %%edx"
            :
            : [cval] "r" ((int)c), [sval] "r" ((int)s), 
              [ival] "r" (i), [llval] "r" (ll)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "cc"
        );
    }
    
    g_volatile_seed = c + s + i + (int)ll;
}

/* Function with union and bitfield operations */
__attribute__((noinline))
static void complex_bitfield_ops(void) {
    union {
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
            unsigned int d : 16;
        } bits;
        uint32_t full;
    } u;
    
    u.full = g_volatile_seed;
    
    /* Operations on bitfields requiring extractions */
    u.bits.a = (u.bits.a + 1) & 0x7;
    u.bits.b = (u.bits.b * 3) & 0x1F;
    u.bits.c = (u.bits.c << 1) | (u.bits.a & 1);
    u.bits.d = u.bits.d + (u.bits.b << 8) + u.bits.c;
    
    /* Inline asm with memory constraint and register input */
    uint32_t temp = u.full;
    asm volatile (
        "bswap %[val]\n\t"
        "rol $8, %[val]\n\t"
        "xor %%ecx, %[val]"
        : [val] "+r" (temp)
        :
        : "rcx", "cc"
    );
    
    /* Store with complex addressing */
    volatile uint32_t* vptr = &u.full;
    asm volatile (
        "movl %[src], (%[dst])"
        :
        : [src] "r" (temp), [dst] "r" (vptr)
        : "memory"
    );
    
    g_volatile_seed = u.full;
}

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    volatile int counter = 0;
    
    /* Declare many variables of different types */
    char c1 = base & 0xFF;
    char c2 = (base >> 8) & 0xFF;
    short s1 = base & 0xFFFF;
    short s2 = (base >> 16) & 0xFFFF;
    int i1 = base;
    int i2 = base * 2;
    int i3 = base * 3;
    long long ll1 = (long long)base * 1000000;
    long long ll2 = (long long)base * 2000000;
    
    /* Take addresses to create pointer variables */
    char* cp1 = &c1;
    char* cp2 = &c2;
    short* sp1 = &s1;
    short* sp2 = &s2;
    int* ip1 = &i1;
    int* ip2 = &i2;
    int* ip3 = &i3;
    long long* lp1 = &ll1;
    long long* lp2 = &ll2;
    
    /* Call functions repeatedly with different arguments */
    for (counter = 0; counter < 10; counter++) {
        use_explicit_registers(i1 + counter, i2 - counter);
        memory_operand_reloads(ip1, counter);
        mixed_type_operations(5 + (counter & 3));
        complex_bitfield_ops();
        
        /* Update variables to create data dependencies */
        c1 = (c1 + c2) & 0xFF;
        c2 = (c2 + counter) & 0xFF;
        s1 = (s1 + s2 + c1) & 0xFFFF;
        s2 = (s2 + counter * 2) & 0xFFFF;
        i1 = i1 + i2 + i3 + s1;
        i2 = i2 * 3 - i1;
        i3 = i3 + counter * 7;
        ll1 = ll1 + ll2 + i1;
        ll2 = ll2 * 5 - ll1;
    }
    
    /* Compute checksum to ensure no code is eliminated */
    long long checksum = (long long)c1 + c2 + s1 + s2 + i1 + i2 + i3 + ll1 + ll2;
    checksum += (long long)(uintptr_t)cp1 + (uintptr_t)cp2;
    checksum += (long long)(uintptr_t)sp1 + (uintptr_t)sp2;
    checksum += (long long)(uintptr_t)ip1 + (uintptr_t)ip2 + (uintptr_t)ip3;
    checksum += (long long)(uintptr_t)lp1 + (uintptr_t)lp2;
    checksum += g_volatile_seed;
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
