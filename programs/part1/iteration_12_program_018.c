/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of functions */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global volatile variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile char g_volatile_char = 'A';
volatile short g_volatile_short = 100;
volatile long g_volatile_long = 999999L;
volatile void* g_volatile_ptr = (void*)0x1000;

/* Complex struct with bitfields to create mode mismatches */
struct mixed_types {
    unsigned char c1 : 3;
    unsigned char c2 : 5;
    unsigned short s1;
    unsigned int i1;
    unsigned long l1;
    void* ptr;
} __attribute__((packed));

/* Function 1: Explicit register variables with conflicting constraints */
NOINLINE USED static int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables that conflict with each other */
    register int x asm("r12");
    register int y asm("r13");
    register int z asm("r14");
    
    /* Force them to be used with different values */
    x = a + g_volatile_int;
    y = b * g_volatile_char;
    z = c ^ g_volatile_short;
    
    /* Inline assembly with mismatched constraints */
    int result;
    asm volatile (
        /* Output constraint: memory, input constraints: register/immediate */
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "xorl %[z], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m" (result)  /* Memory output */
        : [x] "r,m" (x),          /* Register OR memory input */
          [y] "r,i" (y),          /* Register OR immediate */
          [z] "r,m" (z)           /* Register OR memory */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    return result + d + e + f;
}

/* Function 2: Volatile addresses with memory constraints */
NOINLINE USED static long func_volatile_addresses(char* base, int offset) {
    volatile long vl1 = g_volatile_long;
    volatile int vi1 = g_volatile_int;
    volatile short vs1 = g_volatile_short;
    
    /* Take addresses of volatile variables */
    long* plong = (long*)&vl1;
    int* pint = (int*)&vi1;
    short* pshort = (short*)&vs1;
    
    /* Complex pointer arithmetic */
    char* addr1 = base + offset;
    char* addr2 = addr1 + g_volatile_char;
    char* addr3 = addr2 + g_volatile_short;
    
    long results[4];
    
    /* Multiple inline asm statements with memory constraints */
    asm volatile (
        "movq %[ptr], %%rax\n\t"
        "movq (%%rax), %%rbx\n\t"
        "addq %[imm], %%rbx\n\t"
        "movq %%rbx, %[out1]\n\t"
        : [out1] "=m" (results[0])
        : [ptr] "r" (plong),
          [imm] "i" (0x12345678)
        : "rax", "rbx", "rcx", "memory"
    );
    
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "imull %[in2], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out2] "=m" (results[1])
        : [in1] "r,m" (*pint),
          [in2] "r,i" (g_volatile_int)
        : "rax", "rdx", "memory"
    );
    
    /* Memory output with register input */
    asm volatile (
        "movzwl %[in3], %%eax\n\t"
        "movw %%ax, %[out3]\n\t"
        : [out3] "=m" (results[2])
        : [in3] "r,m" (*pshort)
        : "rax", "memory"
    );
    
    /* Force address computation into a register */
    asm volatile (
        "leaq (%[base],%[idx],1), %%rax\n\t"
        "movq %%rax, %[out4]\n\t"
        : [out4] "=m" (results[3])
        : [base] "r" (addr1),
          [idx] "r" (g_volatile_int)
        : "rax", "memory"
    );
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Function 3: Mixed type computations with mode changes */
NOINLINE USED static unsigned long func_mixed_types(struct mixed_types* mt, int iterations) {
    volatile int counter = iterations;
    unsigned long accumulator = 0;
    
    /* Force loop counter to be volatile to prevent optimization */
    while (counter > 0) {
        /* Mixed type operations causing mode changes */
        unsigned char uc = mt->c1 + mt->c2;
        unsigned short us = uc * mt->s1;  /* char to short */
        unsigned int ui = us + mt->i1;    /* short to int */
        unsigned long ul = ui ^ mt->l1;   /* int to long */
        
        /* Pointer arithmetic with different types */
        char* char_ptr = (char*)mt->ptr;
        int* int_ptr = (int*)(char_ptr + uc);
        long* long_ptr = (long*)((char*)int_ptr + us);
        
        /* Inline asm with subreg operations */
        unsigned long temp;
        asm volatile (
            "movzbl %[uc], %%eax\n\t"      /* zero extend char */
            "movzwl %[us], %%ebx\n\t"      /* zero extend short */
            "addl %%ebx, %%eax\n\t"
            "movl %[ui], %%ecx\n\t"
            "xorl %%ecx, %%eax\n\t"
            "movq %[ul], %%rdx\n\t"
            "addq %%rax, %%rdx\n\t"
            "movq %%rdx, %[temp]\n\t"
            : [temp] "=m" (temp)
            : [uc] "r,m" (uc),
              [us] "r,m" (us),
              [ui] "r,m" (ui),
              [ul] "r,m" (ul)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        accumulator += temp;
        
        /* Modify structure fields */
        mt->c1 = (mt->c1 + 1) & 0x7;
        mt->c2 = (mt->c2 - 1) & 0x1F;
        mt->s1 += g_volatile_short;
        mt->i1 ^= g_volatile_int;
        mt->l1 = (mt->l1 << 1) | (mt->l1 >> 63);
        
        counter--;
    }
    
    return accumulator;
}

/* Function 4: Complex addressing modes */
NOINLINE USED static int func_complex_addressing(int* array, int size, int idx1, int idx2, int idx3) {
    volatile int v_idx1 = idx1;
    volatile int v_idx2 = idx2;
    volatile int v_idx3 = idx3;
    
    int result = 0;
    
    /* Complex array addressing that may need base+index reloads */
    for (volatile int i = 0; i < size; i++) {
        /* Multiple array accesses with volatile indices */
        int* ptr1 = &array[v_idx1 + i];
        int* ptr2 = &array[v_idx2 + i * 2];
        int* ptr3 = &array[v_idx3 + i * 3];
        
        /* Inline asm with memory addressing */
        int temp1, temp2, temp3;
        asm volatile (
            "movl (%[p1]), %%eax\n\t"
            "movl (%[p2]), %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl (%[p3]), %%ecx\n\t"
            "subl %%ecx, %%eax\n\t"
            "movl %%eax, %[t1]\n\t"
            : [t1] "=m" (temp1)
            : [p1] "r" (ptr1),
              [p2] "r" (ptr2),
              [p3] "r" (ptr3)
            : "rax", "rbx", "rcx", "memory"
        );
        
        asm volatile (
            "movl %[t1], %%eax\n\t"
            "imull %[imm], %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+m" (result)
            : [t1] "r,m" (temp1),
              [imm] "i" (g_volatile_int)
            : "rax", "rdx", "memory"
        );
        
        /* Update volatile indices */
        v_idx1 = (v_idx1 + g_volatile_char) % size;
        v_idx2 = (v_idx2 + g_volatile_short) % size;
        v_idx3 = (v_idx3 + 1) % size;
    }
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to prevent constant propagation */
    int seed = argc > 1 ? argv[1][0] : 42;
    
    /* Declare many local variables of different types */
    int var1 = seed + 1;
    char var2 = seed + 2;
    short var3 = seed + 3;
    long var4 = seed + 4;
    int* var5 = &var1;
    char* var6 = &var2;
    long var7 = g_volatile_long;
    int var8 = g_volatile_int;
    
    /* Array with complex access patterns */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = (i * seed) ^ g_volatile_int;
    }
    
    /* Initialize mixed types struct */
    struct mixed_types mt;
    mt.c1 = 1;
    mt.c2 = 31;
    mt.s1 = 1000;
    mt.i1 = 0xDEADBEEF;
    mt.l1 = 0x123456789ABCDEF0UL;
    mt.ptr = array;
    
    /* Call functions repeatedly with different arguments */
    unsigned long checksum = 0;
    
    checksum += func_explicit_registers(var1, var2, var3, var4, var7, var8);
    checksum += func_volatile_addresses((char*)array, var1);
    
    /* Modify struct between calls */
    for (int i = 0; i < 5; i++) {
        checksum += func_mixed_types(&mt, 3);
        mt.ptr = (char*)mt.ptr + 8;  /* Change pointer */
    }
    
    checksum += func_complex_addressing(array, 100, var1 % 50, var2 % 50, var3 % 50);
    
    /* More calls with different register pressure */
    for (int i = 0; i < 10; i++) {
        checksum += func_explicit_registers(i, i*2, i*3, i*4, i*5, i*6);
        checksum += func_volatile_addresses((char*)array + i*8, i*16);
    }
    
    /* Final computation using all variables */
    checksum += var1 + var2 + var3 + var4 + (long)var5 + (long)var6 + var7 + var8;
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
