/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_explicit_registers(int arg1, int arg2) {
    /* Explicit register variables competing for same register */
    register int x asm("r12") = arg1 + global_seed;
    register int y asm("r13") = arg2 * 2;
    register int z asm("r14") = global_seed;
    
    /* Inline asm with conflicting constraints */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "imull %[z], %[y]\n\t"
        : [y] "+r,m" (y)  /* Multiple alternatives forcing reload consideration */
        : [x] "r,m" (x), [z] "r,m" (z)
        : "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "memory"
    );
    
    /* More complex asm with mismatched modes */
    char char_var = y & 0xFF;
    long long_var = (long)y * 100;
    
    asm volatile (
        "movsbl %b[char], %k[long]\n\t"
        "addq %[long], %[result]\n\t"
        : [result] "=r,m" (long_var)
        : [char] "r,m" (char_var), [long] "0" (long_var)
        : "cc"
    );
    
    global_seed ^= y + long_var;
}

/* Function with memory constraints and volatile addresses */
__attribute__((noinline))
static void memory_constraints_ops(volatile int* ptr1, volatile short* ptr2) {
    volatile int local_volatile = *ptr1 + global_seed;
    volatile short short_volatile = *ptr2;
    
    /* Take addresses of volatile variables */
    int* addr1 = (int*)&local_volatile;
    short* addr2 = (short*)&short_volatile;
    
    /* Complex addressing with pointer arithmetic */
    int offset = global_seed & 0x3;
    int* complex_addr1 = addr1 + offset;
    short* complex_addr2 = addr2 + (offset * 2);
    
    /* Inline asm with memory output and register input */
    int immediate = 0x1234;
    
    asm volatile (
        "movl %[imm], (%[mem])\n\t"
        "movw %w[imm], (%[mem2])\n\t"
        : 
        : [imm] "ri" (immediate), 
          [mem] "r" (complex_addr1), 
          [mem2] "r" (complex_addr2)
        : "memory"
    );
    
    /* Mixed size operations forcing mode changes */
    char char_result = (*complex_addr1 >> 8) & 0xFF;
    long long_result = (long)char_result * (long)*complex_addr2;
    
    /* Another asm with clobbered registers */
    asm volatile (
        "mov %[char], %%al\n\t"
        "mov %[long], %%rbx\n\t"
        "imul %%rbx\n\t"
        : 
        : [char] "r" (char_result), [long] "r" (long_result)
        : "rax", "rbx", "rdx", "cc"
    );
}

/* Function with mixed types and conversions */
__attribute__((noinline))
static void mixed_type_operations(unsigned count) {
    volatile unsigned volatile_count = count;
    
    /* Union creating subreg operations */
    union {
        uint64_t full;
        struct {
            uint32_t low;
            uint32_t high;
        } parts;
        unsigned char bytes[8];
    } data;
    
    data.full = (uint64_t)global_seed << 32 | volatile_count;
    
    /* Operations causing zero/sign extension */
    for (volatile unsigned i = 0; i < volatile_count % 8; i++) {
        signed char byte_val = data.bytes[i];
        int extended = byte_val;  /* Sign extension */
        unsigned zero_extended = (unsigned)data.bytes[i];  /* Zero extension */
        
        /* Inline asm with mode mismatches */
        asm volatile (
            "movsbl %b[byte], %k[ext]\n\t"
            "addl %k[ext], %k[result]\n\t"
            : [result] "+r,m" (extended)
            : [byte] "r,m" (byte_val)
            : "cc"
        );
        
        data.parts.low ^= extended;
        data.parts.high += zero_extended;
    }
    
    /* Pointer casting creating complex RTL */
    uintptr_t int_ptr = (uintptr_t)&data;
    int_ptr += global_seed;
    
    /* Access through casted pointer */
    uint32_t* word_ptr = (uint32_t*)int_ptr;
    if ((int_ptr & 3) == 0 && word_ptr >= (uint32_t*)&data && 
        word_ptr < (uint32_t*)&data + 2) {
        *word_ptr ^= 0xDEADBEEF;
    }
    
    global_seed = data.parts.low ^ data.parts.high;
}

/* Function with bitfield operations */
__attribute__((noinline))
static void bitfield_operations(int a, int b) {
    struct bitfields {
        unsigned int small1 : 3;
        unsigned int small2 : 5;
        unsigned int small3 : 8;
        unsigned int small4 : 16;
    } bits;
    
    bits.small1 = a & 0x7;
    bits.small2 = (a >> 3) & 0x1F;
    bits.small3 = b & 0xFF;
    bits.small4 = (b >> 8) & 0xFFFF;
    
    /* Operations on bitfields causing subreg */
    unsigned int combined = bits.small1 | (bits.small2 << 3) | 
                           (bits.small3 << 8) | (bits.small4 << 16);
    
    /* Inline asm with bitfield value */
    asm volatile (
        "andl $0xFF, %[val]\n\t"
        "orl %[seed], %[val]\n\t"
        : [val] "+r,m" (combined)
        : [seed] "r,i" (global_seed)
        : "cc"
    );
    
    /* Store back to bitfields */
    bits.small1 = combined & 0x7;
    bits.small2 = (combined >> 3) & 0x1F;
    
    global_seed ^= combined;
}

/* Main function creating maximum register pressure */
int main(int argc, char* argv[]) {
    /* Many local variables of different types */
    int var1 = argc + 1;
    char var2 = (argc > 1) ? argv[1][0] : 'A';
    short var3 = (short)(global_seed * 2);
    long var4 = (long)argc * 1000;
    volatile int var5 = var1 * 2;
    volatile char var6 = var2 + 1;
    
    /* Pointers to volatiles */
    volatile int* ptr1 = &var5;
    volatile short* ptr2 = (volatile short*)&var3;
    
    /* Array with volatile index */
    int array[16];
    volatile int idx = global_seed % 16;
    for (int i = 0; i < 16; i++) {
        array[i] = i * i + global_seed;
    }
    
    /* Call functions multiple times with different args */
    use_explicit_registers(var1, array[idx]);
    memory_constraints_ops(ptr1, ptr2);
    mixed_type_operations(argc + 5);
    bitfield_operations(var1, var3);
    
    /* More complex calls in loop */
    for (volatile int i = 0; i < 3; i++) {
        use_explicit_registers(array[i], var4);
        memory_constraints_ops((volatile int*)&array[i], 
                              (volatile short*)&var3);
    }
    
    /* Compute checksum to prevent elimination */
    unsigned long checksum = 0;
    checksum += var1;
    checksum += var2;
    checksum += var3;
    checksum += var4;
    checksum += var5;
    checksum += var6;
    checksum += global_seed;
    
    for (int i = 0; i < 16; i++) {
        checksum += array[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    return (int)(checksum % 256);
}
