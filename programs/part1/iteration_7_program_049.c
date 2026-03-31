#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[10] = {0};
volatile uint64_t global_long = 0x9876543210ABCDEFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use inline assembly with bit-field constraints to generate ZERO_EXTRACT */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    /* Alternative: try to write to specific bits using bitfield syntax */
    struct bitfield {
        uint32_t low16 : 16;
        uint32_t high16 : 16;
    } *bf = (struct bitfield *)dest;
    
    __asm__ volatile (
        "movw %w[low], %w[val]\n\t"
        : [low] "=r" (bf->low16)
        : [val] "r" ((uint16_t)src)
        : "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *dest, uint32_t src) {
    /* Assembly that modifies only part of a register */
    __asm__ volatile (
        "add %k[dest], %k[src]\n\t"  /* %k modifier for 32-bit register name */
        : [dest] "+&r" (*dest)       /* Early clobber to force specific allocation */
        : [src] "r" (src)
        : "cc"
    );
    
    /* Another attempt with explicit low-part operation */
    uint32_t low_part;
    __asm__ volatile (
        "movl %[src], %[low]\n\t"
        "andl $0xFFFF, %[low]\n\t"
        : [low] "=&r" (low_part)     /* Early clobber constraint */
        : [src] "r" (src)
        : "cc"
    );
    *dest = low_part;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem) {
    /* Write to sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    
    __asm__ volatile (
        "movw $0xABCD, %[mem]\n\t"
        : [mem] "=m" (*short_ptr)
        :
        : "memory"
    );
    
    /* Access different sized subregions of the same memory */
    volatile uint8_t *byte_ptr = (volatile uint8_t *)mem;
    __asm__ volatile (
        "movb $0x42, %[byte]\n\t"
        : [byte] "=m" (byte_ptr[1])
        :
        : "memory"
    );
}

/* Function to test complex pattern combining multiple elements */
static void test_combined_pattern(volatile uint32_t *arr, int idx) {
    /* Complex inline assembly with multiple constraints */
    uint32_t temp;
    __asm__ volatile (
        "movl (%[arr], %[idx], 4), %[temp]\n\t"
        "andl $0x0000FFFF, %[temp]\n\t"
        "movl %[temp], (%[arr], %[idx], 4)\n\t"
        : [temp] "=&r" (temp)        /* Early clobber */
        : [arr] "r" (arr), [idx] "r" ((uint32_t)idx)
        : "memory"
    );
}

/* Main driver function */
int main() {
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    volatile uint32_t local_int = 0xDEADBEEF;
    test_zero_extract(&local_int, 0xCAFEBABE);
    checksum += local_int;
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    volatile uint64_t local_long = 0x1122334455667788ULL;
    test_strict_low_part(&local_long, 0x8899AABB);
    checksum += (uint32_t)local_long + (uint32_t)(local_long >> 32);
    
    /* Test SUBREG of MEM patterns */
    printf("Testing SUBREG of MEM patterns...\n");
    volatile uint32_t mem_buffer[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    test_subreg_mem(mem_buffer);
    for (int i = 0; i < 4; i++) {
        checksum += mem_buffer[i];
    }
    
    /* Test combined patterns */
    printf("Testing combined patterns...\n");
    volatile uint32_t array[8] = {0};
    for (int i = 0; i < 8; i++) {
        test_combined_pattern(array, i);
        checksum += array[i];
    }
    
    /* Test with global variables */
    printf("Testing with global variables...\n");
    test_zero_extract(&global_int, 0x55555555);
    test_strict_low_part(&global_long, 0x66666666);
    test_subreg_mem(global_bytes);
    
    checksum += global_int;
    checksum += (uint32_t)global_long + (uint32_t)(global_long >> 32);
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    /* Test with short array (unaligned access possibilities) */
    printf("Testing with short array...\n");
    for (int i = 0; i < 10; i++) {
        volatile uint16_t *ptr = &global_short_array[i];
        __asm__ volatile (
            "movw $0x%c[val], %[ptr]\n\t"
            : [ptr] "=m" (*ptr)
            : [val] "i" ((uint16_t)(0x1000 + i))
            : "memory"
        );
        checksum += global_short_array[i];
    }
    
    /* Force compiler to keep all computations */
    printf("Final checksum: %u\n", checksum);
    
    return (int)(checksum & 0xFF);
}
