/* reload_stress.c
 * This program is designed to stress GCC's reload pass by creating
 * situations that require many register reloads and complex operand handling.
 * The goal is to trigger the initialization of reload descriptors in reload.cc,
 * specifically targeting lines 1381-1399.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int vol_seed = 12345;

/* Function to get a volatile value to prevent constant folding */
static inline int get_volatile_int(void) {
    return vol_seed;
}

/* NOINLINE functions to prevent optimization across calls */

/* Function 1: Use explicit register variables with conflicting constraints */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + get_volatile_int();
    register int y asm("r13") = b + get_volatile_int();
    int result;
    
    /* Inline asm with mismatched constraints:
     * Output is "=r" (register) but we also use memory constraint alternatives
     * Inputs are in specific registers but asm clobbers them
     */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=r,m" (result)  /* Multiple alternatives: register or memory */
        : [x] "r,m" (x), [y] "r,m" (y)  /* Inputs can be register or memory */
        : "rax", "r12", "r13", "cc", "memory"
    );
    
    return result;
}

/* Function 2: Use volatile addresses with memory constraints */
__attribute__((noinline))
static void use_volatile_addresses(volatile int* arr, int size) {
    volatile int local_vol;
    int i;
    
    /* Prevent loop counter optimization */
    volatile int vol_counter = size;
    
    /* Complex addressing with volatile index */
    for (i = 0; i < vol_counter; i++) {
        int idx = (i + get_volatile_int()) % size;
        
        /* Inline asm with memory output and immediate input */
        asm volatile (
            "movl %[imm], %[mem]\n\t"
            : [mem] "=m" (arr[idx])  /* Memory output constraint */
            : [imm] "i" (0xDEADBEEF)  /* Immediate input (may need reload) */
            : "memory"
        );
        
        /* Another asm with mismatched operand sizes */
        asm volatile (
            "movb %[val], %%al\n\t"
            "movb %%al, %[out]\n\t"
            : [out] "=m" (local_vol)  /* Memory output for char */
            : [val] "r" (i & 0xFF)    /* Register input, char-sized */
            : "rax", "memory"
        );
    }
}

/* Function 3: Mixed type computations causing mode changes */
__attribute__((noinline))
static long mixed_type_computations(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations that change machine modes */
    long result = 0;
    
    /* char -> int -> long chain with volatile accesses */
    result += (long)(vc + get_volatile_int());  /* char promoted to int, then to long */
    result += (long)(vs * vi);  /* short * int promoted to int, then to long */
    result += vl >> (vc & 0x3);  /* long shift with char count */
    
    /* Pointer arithmetic with different types */
    volatile char* char_ptr = (volatile char*)&vc;
    volatile int* int_ptr = (volatile int*)&vi;
    
    /* Complex addressing that may need base+index reloads */
    int idx = get_volatile_int() & 0x3;
    result += char_ptr[idx] + int_ptr[0];
    
    /* Inline asm with subreg-style operations */
    asm volatile (
        "movzbl %[char_in], %%eax\n\t"      /* zero extend char to int */
        "movswl %[short_in], %%ebx\n\t"     /* sign extend short to int */
        "addl %%ebx, %%eax\n\t"
        "cltq\n\t"                          /* sign extend eax to rax */
        "addq %%rax, %[long_out]\n\t"
        : [long_out] "+r" (result)
        : [char_in] "m" (vc), [short_in] "m" (vs)
        : "rax", "rbx", "cc", "memory"
    );
    
    return result;
}

/* Function 4: Heavy register clobbering around many variables */
__attribute__((noinline))
static int heavy_clobbering(int a, int b, int c, int d, int e, int f) {
    int r1 = a + get_volatile_int();
    int r2 = b + get_volatile_int();
    int r3 = c + get_volatile_int();
    int r4 = d + get_volatile_int();
    int r5 = e + get_volatile_int();
    int r6 = f + get_volatile_int();
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "movl %[v1], %%eax\n\t"
        "movl %[v2], %%ebx\n\t"
        "movl %[v3], %%ecx\n\t"
        "movl %[v4], %%edx\n\t"
        "movl %[v5], %%esi\n\t"
        "movl %[v6], %%edi\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "addl %%esi, %%eax\n\t"
        "addl %%edi, %%eax\n\t"
        "movl %%eax, %[v1]\n\t"
        : [v1] "+m" (r1)  /* Memory constraint to force reloads */
        : [v2] "m" (r2), [v3] "m" (r3), [v4] "m" (r4),
          [v5] "m" (r5), [v6] "m" (r6)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

/* Function 5: Complex constraints with multiple alternatives */
__attribute__((noinline))
static int complex_constraints(int* ptr, volatile int* vptr) {
    int temp1 = get_volatile_int();
    int temp2 = get_volatile_int();
    int result;
    
    /* Multiple alternative constraints that may force reloads */
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl %[temp1], %%eax\n\t"
        "movl %%eax, (%[vptr])\n\t"
        "movl (%[vptr]), %%eax\n\t"
        "addl %[temp2], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r,m" (result)          /* Output: register OR memory */
        : [ptr] "r,m" (ptr),                /* Input: register OR memory */
          [vptr] "r,m" (vptr),
          [temp1] "r,i,m" (temp1),          /* Three alternatives! */
          [temp2] "r,i,m" (temp2)
        : "rax", "cc", "memory"
    );
    
    return result;
}

int main(int argc, char** argv) {
    /* Initialize with volatile values to prevent constant folding */
    int base = get_volatile_int();
    if (argc > 1) base += atoi(argv[1]);
    
    /* Many local variables of different types */
    char c1 = base & 0xFF;
    short s1 = base & 0xFFFF;
    int i1 = base;
    long l1 = base * 100L;
    
    volatile int varray[10];
    for (int i = 0; i < 10; i++) {
        varray[i] = base + i;
    }
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    sum += use_explicit_registers(base, base + 1);
    
    use_volatile_addresses(varray, 10);
    for (int i = 0; i < 10; i++) {
        sum += varray[i];
    }
    
    sum += mixed_type_computations(c1, s1, i1, l1);
    
    sum += heavy_clobbering(base, base+1, base+2, base+3, base+4, base+5);
    
    volatile int vtemp = base;
    sum += complex_constraints(&i1, &vtemp);
    
    /* Additional stress: loop with many variables */
    volatile int loop_counter = 100;
    for (volatile int j = 0; j < loop_counter; j++) {
        /* Many variables in loop to increase register pressure */
        int a = j + base;
        char b = (j * 3) & 0xFF;
        short c = (j * 5) & 0xFFFF;
        long d = j * 7L;
        
        /* Mixed operations */
        a += b;
        c -= a & 0xFFFF;
        d += c * 100L;
        
        /* Pointer arithmetic with volatile */
        volatile int* ptr = &varray[j % 10];
        *ptr += a;
        
        sum += a + b + c + (int)d;
    }
    
    /* Final checksum to prevent elimination */
    printf("Result: %d\n", sum);
    return sum & 0xFF;
}
