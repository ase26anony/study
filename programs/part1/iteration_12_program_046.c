/* reload_stress.c
 * This program is designed to stress GCC's reload pass by creating
 * complex register allocation scenarios that require many reloads.
 * It specifically aims to trigger the initialization of reload
 * descriptors in reload.cc lines 1381-1399.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int g_seed = 42;

/* Function to get a volatile value to prevent optimizations */
static inline int get_volatile_int(void) {
    return g_seed;
}

/* ========== Function 1: Explicit register variables with mismatched constraints ========== */
__attribute__((noinline))
static int func1_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Declare explicit register variables - these compete for specific registers */
    register int r12_var asm("r12") = a + 1;
    register int r13_var asm("r13") = b + 2;
    register int r14_var asm("r14") = c + 3;
    register int r15_var asm("r15") = d + 4;
    
    int result;
    
    /* Inline asm with mismatched constraints:
     * Output is "=r" (register), but inputs include memory constraints "m"
     * This can force reloads when the chosen register conflicts */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "subl %[in3], %%eax\n\t"
        "imull %[in4], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)          /* Output in register */
        : [in1] "rm" (r12_var),        /* Input can be register OR memory */
          [in2] "rm" (r13_var),
          [in3] "rm" (r14_var),
          [in4] "rm" (r15_var)
        : "eax", "cc", "memory"
    );
    
    /* Use the explicit register variables again to keep them live */
    asm volatile ("" : : "r" (r12_var), "r" (r13_var), "r" (r14_var), "r" (r15_var));
    
    return result + e + f;
}

/* ========== Function 2: Memory addresses with complex addressing modes ========== */
__attribute__((noinline))
static void func2_memory_ops(volatile int* arr, int idx1, int idx2, int idx3) {
    volatile int local1, local2, local3;
    volatile int* ptr1 = &local1;
    volatile int* ptr2 = &local2;
    
    /* Complex address calculation that may not fit in a single addressing mode */
    int offset = get_volatile_int() % 16;
    
    /* Multiple asm statements with memory output operands and immediate inputs */
    /* This can force reloads when immediates need to go to memory */
    asm volatile (
        "movl %[imm], (%[addr])"
        : 
        : [imm] "i" (0x12345678),      /* Immediate constraint */
          [addr] "r" (ptr1 + offset)   /* Complex address in register */
        : "memory"
    );
    
    asm volatile (
        "addl %%eax, (%[base], %[index], 4)"
        : 
        : [base] "r" (arr),            /* Base register */
          [index] "r" (idx1),          /* Index register */
          "a" (idx2)                   /* In eax register */
        : "memory"
    );
    
    /* Three-address memory operation with mismatched constraints */
    int temp;
    asm volatile (
        "movl (%[src]), %%ecx\n\t"
        "leal (%%ecx, %[imm]), %%edx\n\t"
        "movl %%edx, (%[dst])"
        : 
        : [src] "r" (&arr[idx2]),      /* Memory input */
          [dst] "r" (&arr[idx3]),      /* Memory output */
          [imm] "i" (100)              /* Immediate */
        : "ecx", "edx", "memory"
    );
    
    /* Force spill/reload by clobbering many registers */
    asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* ========== Function 3: Mixed type operations causing mode changes ========== */
__attribute__((noinline))
static long func3_mixed_types(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Mixed type computations that create subreg/zero_extend operations */
    long result = 0;
    
    /* char -> long extension may require reload */
    result += (long)vc * 256L;
    
    /* short -> int -> long with arithmetic */
    result += (long)(vs * vi);
    
    /* Complex expression with mixed types */
    for (volatile int j = 0; j < 4; j++) {
        /* Loop counter is volatile to prevent optimization */
        char temp_c = vc + j;
        short temp_s = vs - j;
        int temp_i = vi * (temp_c + 1);
        
        /* This mixed operation may require mode conversions */
        result += (long)temp_i / (temp_s + 1);
        
        /* Bitfield-like operation */
        result ^= ((long)temp_c << 16) | ((long)temp_s << 32);
    }
    
    /* Union to force type punning */
    union {
        int i;
        char c[4];
        short s[2];
    } pun;
    
    pun.i = vi;
    result += pun.c[0] + pun.s[1];
    
    /* Pointer casting creating complex addressing */
    int* ptr = (int*)((uintptr_t)&vi & ~0x3);
    result += *ptr;
    
    return result;
}

/* ========== Function 4: Many variables with register pressure ========== */
__attribute__((noinline))
static int func4_register_pressure(int a, int b, int c, int d, int e,
                                   int f, int g, int h, int i, int j) {
    /* Many local variables to increase register pressure */
    int v1 = a * b;
    int v2 = c + d;
    int v3 = e - f;
    int v4 = g ^ h;
    int v5 = i | j;
    int v6 = a ^ c;
    int v7 = b & d;
    int v8 = e << 2;
    int v9 = f >> 1;
    int v10 = g * 3;
    int v11 = h + 7;
    int v12 = i - 5;
    int v13 = j * 2;
    int v14 = a + b + c;
    int v15 = d + e + f;
    
    /* Use all variables in a complex asm with many clobbers */
    /* This forces the compiler to spill/reload around the asm */
    int result;
    asm volatile (
        "movl %[v1], %%eax\n\t"
        "addl %[v2], %%eax\n\t"
        "subl %[v3], %%eax\n\t"
        "addl %[v4], %%eax\n\t"
        "subl %[v5], %%eax\n\t"
        "addl %[v6], %%eax\n\t"
        "imull %[v7], %%eax\n\t"
        "addl %[v8], %%eax\n\t"
        "subl %[v9], %%eax\n\t"
        "addl %[v10], %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=rm" (result)      /* Can be register OR memory */
        : [v1] "rm" (v1), [v2] "rm" (v2), [v3] "rm" (v3),
          [v4] "rm" (v4), [v5] "rm" (v5), [v6] "rm" (v6),
          [v7] "rm" (v7), [v8] "rm" (v8), [v9] "rm" (v9),
          [v10] "rm" (v10)
        : "eax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9",
          "r10", "r11", "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    /* Use remaining variables to keep them live */
    result += v11 + v12 + v13 + v14 + v15;
    
    return result;
}

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    /* Use argc and argv to prevent constant folding */
    int base = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Declare many local variables of different types */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = base, i2 = base + 1, i3 = base + 2, i4 = base + 3;
    int i5 = base + 4, i6 = base + 5, i7 = base + 6, i8 = base + 7;
    long l1 = base * 10L, l2 = base * 20L, l3 = base * 30L;
    
    volatile int vol1 = get_volatile_int();
    volatile int vol2 = vol1 + 1;
    volatile int vol3 = vol2 * 2;
    
    /* Array with volatile index access */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = base + i;
    }
    
    /* Call functions repeatedly to create complex reload scenarios */
    int sum = 0;
    
    /* Function 1: Explicit register variables */
    sum += func1_explicit_registers(i1, i2, i3, i4, i5, i6);
    
    /* Function 2: Memory operations with complex addressing */
    func2_memory_ops(arr, vol1 % 50, vol2 % 50, vol3 % 50);
    sum += arr[vol1 % 50] + arr[vol2 % 50];
    
    /* Function 3: Mixed type operations */
    sum += (int)func3_mixed_types(c1, s1, i1, l1);
    sum += (int)func3_mixed_types(c2, s2, i2, l2);
    sum += (int)func3_mixed_types(c3, s3, i3, l3);
    
    /* Function 4: Register pressure */
    sum += func4_register_pressure(i1, i2, i3, i4, i5, i6, i7, i8, 
                                   vol1, vol2);
    sum += func4_register_pressure(i2, i3, i4, i5, i6, i7, i8, i1,
                                   vol2, vol3);
    sum += func4_register_pressure(i3, i4, i5, i6, i7, i8, i1, i2,
                                   vol3, vol1);
    
    /* Additional complex inline asm with many operands */
    int temp1, temp2, temp3;
    asm volatile (
        "movl %[a], %%eax\n\t"
        "leal (%%eax, %[b], 2), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "imull %[c], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "addl %[d], %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=rm" (temp1), [out2] "=rm" (temp2), [out3] "=rm" (temp3)
        : [a] "rm" (i1), [b] "rm" (i2), [c] "rm" (i3), [d] "rm" (i4)
        : "eax", "ebx", "cc"
    );
    
    sum += temp1 + temp2 + temp3;
    
    /* Final checksum to ensure nothing is optimized away */
    printf("Result checksum: %d\n", sum);
    
    return (sum > 0) ? 0 : 1;
}
