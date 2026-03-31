#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_bytes[16] = {0};

/* Function 1: Target ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr) {
    /* Try to generate ZERO_EXTRACT by writing to specific bits */
    uint32_t temp;
    
    /* Assembly that writes to specific bits of a register */
    __asm__ volatile (
        "mov %[temp], %[val]\n\t"
        "btr %[temp], %[bit]\n\t"
        "mov %[out], %[temp]"
        : [out] "=r" (temp)
        : [val] "r" (*ptr),
          [bit] "i" (5)
        : "cc"
    );
    
    /* Another attempt with bitfield operation */
    uint32_t mask = 0xFF00;
    __asm__ volatile (
        "and %[out], %[in], %[mask]"
        : [out] "=r" (temp)
        : [in] "r" (*ptr),
          [mask] "r" (mask)
    );
    
    /* Store back to force memory reference */
    *ptr = temp;
}

/* Function 2: Target STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *arr) {
    uint32_t reg32;
    uint16_t reg16;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        "movw %[out], %[in]\n\t"
        "addw %[out], $1"
        : [out] "=r" (reg16)
        : [in] "r" (arr[0])
        : "cc"
    );
    
    /* Using early-clobber to complicate register allocation */
    __asm__ volatile (
        "mov %0, %1\n\t"
        "inc %0"
        : "=&r" (reg32)  /* Early-clobber */
        : "r" (arr[1])
        : "cc"
    );
    
    /* Mixed size operations that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "addb %b[out], %b[in]"  /* %b modifier for byte access */
        : [out] "+r" (reg32)
        : [in] "r" ((uint8_t)arr[2])
        : "cc"
    );
    
    arr[0] = reg16;
}

/* Function 3: Target SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint64_t *big_mem) {
    /* Access different parts of the memory through casts */
    volatile uint32_t *as_int = (volatile uint32_t *)big_mem;
    volatile uint16_t *as_short = (volatile uint16_t *)big_mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)big_mem;
    
    /* Write to sub-region of memory */
    __asm__ volatile (
        "movl %[val], (%[ptr])"
        :
        : [ptr] "r" (as_int),
          [val] "r" (0xDEADBEEF)
        : "memory"
    );
    
    /* Access 16-bit sub-region */
    __asm__ volatile (
        "movw %[val], (%[ptr])"
        :
        : [ptr] "r" (as_short + 2),
          [val] "r" ((uint16_t)0xCAFE)
        : "memory"
    );
    
    /* Byte access within larger memory */
    __asm__ volatile (
        "movb %[val], (%[ptr])"
        :
        : [ptr] "r" (as_byte + 6),
          [val] "r" ((uint8_t)0x42)
        : "memory"
    );
}

/* Function 4: Combined pattern - SUBREG of MEM with bit operations */
static void test_combined(volatile uint8_t *mem) {
    uint32_t temp;
    
    /* Read from memory, modify sub-part, write back */
    __asm__ volatile (
        "movzbl (%[src]), %[temp]\n\t"
        "andl $0xF0, %[temp]\n\t"
        "movb %b[temp], (%[dst])"
        : [temp] "=&r" (temp)
        : [src] "r" (mem),
          [dst] "r" (mem + 8)
        : "memory"
    );
}

/* Function 5: Complex memory addressing with displacement */
static void test_complex_mem(volatile uint16_t *arr, int index) {
    uint32_t temp;
    
    /* Memory access with index and scale */
    __asm__ volatile (
        "movw (%[base], %[index], 2), %w[temp]\n\t"
        "incw %w[temp]\n\t"
        "movw %w[temp], (%[base], %[index], 2)"
        : [temp] "=&r" (temp)
        : [base] "r" (arr),
          [index] "r" ((long)index)
        : "memory", "cc"
    );
}

int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(global_short_array);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&global_long);
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    
    /* Test combined patterns */
    test_combined(global_bytes);
    for (int i = 0; i < 16; i++) {
        checksum += global_bytes[i];
    }
    
    /* Test complex memory addressing */
    test_complex_mem(global_short_array, 3);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Additional test with local volatile variables */
    volatile uint32_t local_var = 0xABCD1234;
    volatile uint16_t local_arr[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    
    test_zero_extract(&local_var);
    checksum += local_var;
    
    test_strict_low_part(local_arr);
    for (int i = 0; i < 4; i++) {
        checksum += local_arr[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("(This ensures operations aren't optimized away)\n");
    
    return 0;
}
