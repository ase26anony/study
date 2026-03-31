/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
static int use_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - compete for specific registers */
    register int r12_val asm("r12") = a + 1;
    register int r13_val asm("r13") = b + 2;
    register int r14_val asm("r14") = c + 3;
    register int r15_val asm("r15") = d + 4;
    
    int result;
    
    /* Inline assembly with mismatched constraints */
    /* Output is memory, inputs are registers - may force reloads */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "subl %[in3], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=m" (result)          /* Memory output */
        : [in1] "r" (r12_val),         /* Register input */
          [in2] "r" (r13_val),         /* Register input */
          [in3] "r" (r14_val)          /* Register input */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Use r15_val to prevent elimination */
    asm volatile ("" : : "r" (r15_val));
    
    return result + r15_val;
}

/* Function using volatile addresses and complex addressing modes */
__attribute__((noinline))
static long use_volatile_addresses(volatile char *ptr1, volatile short *ptr2, 
                                   volatile int *ptr3, volatile long *ptr4) {
    long total = 0;
    volatile int idx = g_volatile_seed & 3;  /* Non-constant index */
    
    /* Complex addressing with volatile pointers */
    /* Multiple memory constraints with different modes */
    for (int i = 0; i < 4; i++) {
        char char_val;
        short short_val;
        int int_val;
        long long_val;
        
        /* Inline assembly with memory constraints and clobbers */
        /* Inputs are memory, outputs are memory - may need reloads */
        asm volatile (
            "movb (%[addr1]), %%al\n\t"
            "movw (%[addr2]), %%bx\n\t"
            "movl (%[addr3]), %%ecx\n\t"
            "movq (%[addr4]), %%rdx\n\t"
            "movb %%al, %[out1]\n\t"
            "movw %%bx, %[out2]\n\t"
            "movl %%ecx, %[out3]\n\t"
            "movq %%rdx, %[out4]\n\t"
            : [out1] "=m" (char_val),
              [out2] "=m" (short_val),
              [out3] "=m" (int_val),
              [out4] "=m" (long_val)
            : [addr1] "r" (ptr1 + i + idx),
              [addr2] "r" (ptr2 + i + idx),
              [addr3] "r" (ptr3 + i + idx),
              [addr4] "r" (ptr4 + i + idx)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Mixed-type operations requiring mode changes */
        total += (long)char_val + (long)short_val + (long)int_val + long_val;
    }
    
    return total;
}

/* Function with mixed data types and mode conversions */
__attribute__((noinline))
static uint64_t mixed_type_operations(volatile uint8_t *bytes, 
                                      volatile uint16_t *words,
                                      volatile uint32_t *dwords,
                                      volatile uint64_t *qwords) {
    uint64_t sum = 0;
    volatile int counter = g_volatile_seed;
    
    /* Operations that cause mode changes */
    for (int i = 0; i < 8; i++) {
        /* Read different sized values */
        uint8_t b = bytes[i];
        uint16_t w = words[i];
        uint32_t d = dwords[i];
        uint64_t q = qwords[i];
        
        /* Mixed operations requiring extensions/truncations */
        uint64_t temp;
        
        /* Inline assembly with alternative constraints */
        /* "r,m" constraints may force reload decisions */
        asm volatile (
            "movzbl %[byte], %%eax\n\t"
            "movzwl %[word], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %[dword], %%eax\n\t"
            "movl %%eax, %%eax\n\t"    /* Zero extend to 64-bit */
            "addq %[qword], %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=r,m" (temp)   /* Alternative constraints */
            : [byte] "r,m" (b),
              [word] "r,m" (w),
              [dword] "r,m" (d),
              [qword] "r,m" (q)
            : "rax", "rbx", "rcx", "cc"
        );
        
        sum += temp;
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r" (counter));
        counter++;
    }
    
    return sum;
}

/* Function with pointer arithmetic and complex addressing */
__attribute__((noinline))
static int complex_addressing(int *base, volatile int offset1, 
                              volatile int offset2, volatile int offset3) {
    int result1, result2, result3;
    
    /* Complex address calculations that may not fit in addressing modes */
    int *ptr1 = base + offset1;
    int *ptr2 = base + offset2;
    int *ptr3 = base + offset3;
    
    /* Inline assembly with multiple memory operands */
    asm volatile (
        "movl (%[p1]), %%eax\n\t"
        "addl (%[p2]), %%eax\n\t"
        "subl (%[p3]), %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%eax, %[out2]\n\t"
        "movl %%eax, %[out3]\n\t"
        : [out1] "=m" (result1),
          [out2] "=m" (result2),
          [out3] "=m" (result3)
        : [p1] "r" (ptr1),
          [p2] "r" (ptr2),
          [p3] "r" (ptr3)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    return result1 + result2 + result3;
}

/* Union to create subreg operations */
typedef union {
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
    uint32_t whole;
    uint8_t bytes[4];
} mixed_union;

__attribute__((noinline))
static uint32_t use_union_operations(mixed_union *unions, int count) {
    uint32_t total = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data */
        uint16_t low = unions[i].parts.low;
        uint16_t high = unions[i].parts.high;
        uint32_t whole = unions[i].whole;
        
        /* Operations requiring mode mixing */
        uint32_t temp;
        
        /* Assembly with mismatched operand sizes */
        asm volatile (
            "movzwl %[low], %%eax\n\t"     /* Zero extend 16 to 32 */
            "movzwl %[high], %%ebx\n\t"    /* Zero extend 16 to 32 */
            "shll $16, %%ebx\n\t"          /* Shift to high half */
            "orl %%ebx, %%eax\n\t"         /* Combine */
            "xorl %[whole], %%eax\n\t"     /* XOR with whole */
            "movl %%eax, %[result]\n\t"
            : [result] "=r,m" (temp)
            : [low] "r,m" (low),
              [high] "r,m" (high),
              [whole] "r,m" (whole)
            : "rax", "rbx", "cc"
        );
        
        total += temp;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int init_val = g_volatile_seed;
    
    /* Many local variables of different types */
    char char_array[16];
    short short_array[16];
    int int_array[32];
    long long_array[32];
    uint8_t byte_array[32];
    uint16_t word_array[32];
    uint32_t dword_array[32];
    uint64_t qword_array[32];
    mixed_union unions[8];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 32; i++) {
        if (i < 16) {
            char_array[i] = (char)(init_val + i);
            short_array[i] = (short)(init_val * i);
        }
        if (i < 8) {
            unions[i].whole = init_val * 100 + i;
            unions[i].parts.low = (uint16_t)(init_val + i);
            unions[i].parts.high = (uint16_t)(init_val * i);
        }
        int_array[i] = init_val + i * 2;
        long_array[i] = init_val * 1000L + i;
        byte_array[i] = (uint8_t)(init_val ^ i);
        word_array[i] = (uint16_t)(init_val * 3 + i);
        dword_array[i] = (uint32_t)(init_val * 7 + i);
        qword_array[i] = (uint64_t)(init_val * 11 + i);
    }
    
    uint64_t checksum = 0;
    
    /* Call functions repeatedly to increase reload pressure */
    for (int iter = 0; iter < 10; iter++) {
        /* Use function arguments to prevent optimization */
        int arg1 = argc + iter;
        int arg2 = argv[0][0] + iter;
        int arg3 = init_val + iter;
        int arg4 = arg1 * arg2;
        int arg5 = arg2 + arg3;
        int arg6 = arg3 * arg4;
        
        /* Force register pressure with explicit registers */
        checksum += use_explicit_registers(arg1, arg2, arg3, arg4, arg5, arg6);
        
        /* Use volatile addresses */
        checksum += use_volatile_addresses((volatile char *)char_array,
                                          (volatile short *)short_array,
                                          (volatile int *)int_array,
                                          (volatile long *)long_array);
        
        /* Mixed type operations */
        checksum += mixed_type_operations(byte_array, word_array, 
                                         dword_array, qword_array);
        
        /* Complex addressing */
        volatile int offset1 = (iter * 3) & 31;
        volatile int offset2 = (iter * 5) & 31;
        volatile int offset3 = (iter * 7) & 31;
        checksum += complex_addressing(int_array, offset1, offset2, offset3);
        
        /* Union operations */
        checksum += use_union_operations(unions, 8);
        
        /* Modify global volatile to prevent optimization */
        g_volatile_seed++;
    }
    
    /* Final computation to ensure nothing is eliminated */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
