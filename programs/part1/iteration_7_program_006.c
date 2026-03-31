#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[10] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[32] = {0};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Try to generate ZERO_EXTRACT by writing to specific bits */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    /* Another attempt with bitfield operation */
    struct bitfield {
        uint32_t low: 8;
        uint32_t mid: 8;
        uint32_t high: 16;
    } __attribute__((packed));
    
    volatile struct bitfield bf;
    __asm__ volatile (
        "mov %[low], %b[src]\n\t"
        : [low] "=r" (bf.low)
        : [src] "r" (src)
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *dest, uint32_t src) {
    /* Assembly that should only affect low part of register */
    __asm__ volatile (
        "add %0, %1\n\t"
        : "+&r" (*dest)      /* Early clobber to force specific allocation */
        : "r" (src)
        : "cc"
    );
    
    /* Another variant with explicit low-part constraint */
    uint32_t low_part;
    __asm__ volatile (
        "movl %1, %0\n\t"
        : "=r" (low_part)
        : "r" (src)
    );
    *dest = low_part;
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem, uint16_t value) {
    /* Access memory through different-sized views */
    volatile uint32_t *as_int = (volatile uint32_t *)mem;
    volatile uint16_t *as_short = (volatile uint16_t *)mem;
    volatile uint8_t *as_byte = (volatile uint8_t *)mem;
    
    /* Write to sub-region of memory using inline assembly */
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=m" (*as_short)
        : "r" (value)
        : "memory"
    );
    
    /* Another access with offset */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=m" (as_byte[1])
        : "r" ((uint8_t)(value >> 8))
        : "memory"
    );
}

/* Function to test complex pattern with array access */
static void test_complex_pattern(volatile uint16_t *arr, int idx, uint32_t value) {
    /* Mix of operations that might generate SUBREG MEM */
    volatile uint32_t temp;
    
    __asm__ volatile (
        "movl %[val], %[tmp]\n\t"
        "movw %w[tmp], (%[arr], %[idx], 2)\n\t"
        : [tmp] "=&r" (temp)
        : [val] "r" (value),
          [arr] "r" (arr),
          [idx] "r" ((long)idx)
        : "memory"
    );
    
    /* Access with pointer arithmetic */
    volatile uint16_t *elem = &arr[idx];
    __asm__ volatile (
        "incw %0\n\t"
        : "+m" (*elem)
        :: "memory"
    );
}

/* Main driver function */
int main() {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for GCC coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 0xABCD1234);
    checksum += global_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long, 0x87654321);
    checksum += (global_long & 0xFFFFFFFF);
    
    /* Test SUBREG MEM patterns */
    test_subreg_mem(&global_int, 0x5678);
    checksum += global_int;
    
    /* Test complex patterns with arrays */
    test_complex_pattern(global_short_array, 3, 0x12345678);
    for (int i = 0; i < 10; i++) {
        checksum += global_short_array[i];
    }
    
    /* Additional tests with byte arrays */
    for (int i = 0; i < 32; i += 2) {
        __asm__ volatile (
            "movw $0xAA55, %0\n\t"
            : "=m" (global_byte_array[i])
            :: "memory"
        );
        checksum += global_byte_array[i];
    }
    
    /* Test with mixed types and casts */
    volatile uint32_t local_var = 0;
    volatile uint16_t *short_ptr = (volatile uint16_t*)&local_var;
    
    __asm__ volatile (
        "movw $0x1234, %0\n\t"
        : "=m" (*short_ptr)
        :: "memory"
    );
    checksum += local_var;
    
    /* Force early-clobber pattern */
    uint32_t a = 1, b = 2, c = 3;
    __asm__ volatile (
        "add %1, %0\n\t"
        "add %2, %0\n\t"
        : "+&r" (a)
        : "r" (b), "r" (c)
        : "cc"
    );
    checksum += a;
    
    printf("Final checksum: %u\n", checksum);
    return 0;
}
