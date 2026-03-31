/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile int g_volatile_index = 0;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_complex_addressing(volatile int* arr, int size, int idx) {
    volatile int* volatile_ptr = arr;
    int* aliased_ptr = (int*)volatile_ptr;
    
    /* Force base+index addressing with volatile components */
    for (volatile int i = 0; i < size; i++) {
        /* Complex address calculation that may need reloads */
        int offset = (idx * i + g_volatile_seed) % size;
        aliased_ptr[offset] = aliased_ptr[i] * 3 + offset;
    }
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b, int c) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b * 2;
    register int z asm("r14") = c - g_volatile_index;
    
    int result;
    
    /* Inline asm with multiple alternative constraints and clobbers */
    asm volatile (
        /* Output constraint: memory, input constraints: register or memory */
        "movl %[x_in], %%eax\n\t"
        "addl %[y_in], %%eax\n\t"
        "subl %[z_in], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m" (result)          /* Memory output */
        : [x_in] "r,m" (x),               /* Register OR memory */
          [y_in] "r,m" (y),
          [z_in] "r,m" (z)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
    );
    
    /* Additional asm that clobbers specific registers */
    asm volatile (
        "xor %%ebx, %%ebx\n\t"
        "xor %%ecx, %%ecx\n\t"
        "xor %%edx, %%edx\n\t"
        : : : "rbx", "rcx", "rdx", "cc"
    );
    
    return result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long mixed_type_operations(char c, short s, int i, long l) {
    /* Operations that cause mode changes */
    char c_ext = c + g_volatile_index;
    short s_ext = s * 2;
    int i_ext = i + (int)c_ext;
    long l_ext = l + (long)s_ext + (long)i_ext;
    
    /* Bit-field operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {c_ext & 0x7, (s_ext >> 2) & 0x1F, i_ext & 0xFF, l_ext & 0xFFFF};
    
    /* Union for type punning */
    union {
        uint32_t u32;
        uint16_t u16[2];
        uint8_t u8[4];
    } converter;
    
    converter.u32 = (bits.a << 29) | (bits.b << 24) | (bits.c << 16) | bits.d;
    
    /* Inline asm with memory output and immediate input */
    volatile long mem_result;
    asm volatile (
        "mov %[imm], %%rax\n\t"
        "add %[val], %%rax\n\t"
        "mov %%rax, %[out]\n\t"
        : [out] "=m" (mem_result)
        : [imm] "i" (0x12345678), [val] "r" (l_ext)
        : "rax", "memory"
    );
    
    return mem_result + converter.u32;
}

/* Function with pointer arithmetic and volatile addresses */
__attribute__((noinline))
static void pointer_arithmetic(volatile int* base, int offset) {
    volatile int* ptr1 = base + offset;
    volatile int* ptr2 = base + g_volatile_seed;
    volatile short* short_ptr = (volatile short*)base;
    
    /* Complex pointer expressions */
    int* regular_ptr = (int*)ptr1;
    int idx = g_volatile_index;
    
    /* Multiple memory accesses with different types */
    for (volatile int i = 0; i < 8; i++) {
        int temp = *ptr1 + *ptr2 + idx;
        short_ptr[i] = (short)(temp & 0xFFFF);
        regular_ptr[i] = temp >> 16;
        
        /* Inline asm with "m" constraints and clobbers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (regular_ptr[i])
            : [in1] "m" (*ptr1), [in2] "r" (i)
            : "rax", "rcx", "rdx", "memory", "cc"
        );
    }
}

/* Function that creates many local variables for register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 'A', c2 = 'B', c3 = 'C', c4 = 'D';
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long l1 = 10000, l2 = 20000, l3 = 30000, l4 = 40000;
    volatile int v1 = g_volatile_seed;
    volatile short v2 = g_volatile_index;
    
    /* Pointer variables */
    int* p1 = &i1;
    int* p2 = &i2;
    int* p3 = &i3;
    int* p4 = &i4;
    
    int sum = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Mixed operations causing mode changes */
        c1 = (c1 + c2) & 0xFF;
        s1 = (s1 + s2 + c1) & 0xFFFF;
        i1 = i1 + i2 + s1;
        l1 = l1 + l2 + i1;
        
        /* Pointer arithmetic */
        p1 = p1 + (v1 & 0x3);
        p2 = p2 + (v2 & 0x1);
        
        /* Inline asm with many clobbered registers */
        asm volatile (
            "mov %[a], %%rax\n\t"
            "mov %[b], %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[sum]\n\t"
            : [sum] "=m" (sum)
            : [a] "r" (l1), [b] "r" (l2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        /* Update volatile to prevent loop elimination */
        v1 = v1 + 1;
        v2 = v2 + 2;
    }
    
    return sum + c1 + s1 + i1 + (int)l1;
}

int main(int argc, char** argv) {
    /* Initialize with command line args to prevent constant folding */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    int iterations = argc > 2 ? atoi(argv[2]) : 10;
    
    /* Array for complex addressing */
    volatile int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = (i * base + g_volatile_seed) & 0xFF;
    }
    
    int checksum = 0;
    
    /* Call functions repeatedly to create reload situations */
    for (int i = 0; i < iterations; i++) {
        g_volatile_index = i;
        
        /* 1. Complex addressing */
        use_complex_addressing(array, 256, i);
        
        /* 2. Register conflicts */
        checksum += register_conflicts(i, i*2, i*3);
        
        /* 3. Mixed type operations */
        checksum += mixed_type_operations(
            (char)(i & 0xFF), 
            (short)((i * 10) & 0xFFFF),
            i * 100,
            (long)i * 1000
        );
        
        /* 4. Pointer arithmetic */
        pointer_arithmetic(array, i % 128);
        
        /* 5. High register pressure */
        checksum += high_register_pressure(5);
        
        /* Update array to prevent optimization */
        array[i % 256] = checksum & 0xFF;
    }
    
    /* Final computation to ensure all code is used */
    for (int i = 0; i < 256; i++) {
        checksum += array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
