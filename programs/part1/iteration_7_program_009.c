#include <stdint.h>
#include <stdio.h>

// Global volatile variables to prevent optimization
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[10] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFF;
volatile uint8_t global_byte_array[16] = {0};

// Function to generate ZERO_EXTRACT pattern
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    // Try to write to specific bits of a variable
    // Using bit-field constraints to potentially generate ZERO_EXTRACT
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    // Another attempt with explicit bit manipulation
    uint32_t mask = 0x0000FF00;
    __asm__ volatile (
        "and %[dest], %[mask]\n\t"
        "or %[dest], %[src]\n\t"
        : [dest] "+r" (*dest)
        : [src] "r" (src & mask), [mask] "r" (mask)
        : "cc"
    );
}

// Function to generate STRICT_LOW_PART pattern
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    // Assembly that modifies only part of a register
    // The "=r" constraint with early clobber might generate STRICT_LOW_PART
    __asm__ volatile (
        "addw %[dest], %[src]\n\t"
        : [dest] "=&r" (*dest)
        : [src] "r" (src)
        : "cc"
    );
    
    // Another pattern with 8-bit operation on 16-bit variable
    __asm__ volatile (
        "movb %b[dest], %b[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : 
    );
}

// Function to generate SUBREG of MEM pattern
static void test_subreg_mem(volatile uint32_t *int_ptr, volatile uint16_t *short_ptr) {
    // Access memory through different sized pointers
    // This should generate SUBREG when accessing part of memory
    
    // Write to short within int (SUBREG of MEM)
    __asm__ volatile (
        "movw %[short], %[value]\n\t"
        : [short] "=m" (*(volatile uint16_t *)int_ptr)
        : [value] "r" ((uint16_t)0xABCD)
        : "memory"
    );
    
    // Another SUBREG pattern with byte access
    __asm__ volatile (
        "movb %[byte], %[val]\n\t"
        : [byte] "=m" (*(volatile uint8_t *)&global_int)
        : [val] "r" ((uint8_t)0x42)
        : "memory"
    );
    
    // Complex pattern with multiple memory accesses
    uint32_t temp;
    __asm__ volatile (
        "movl (%[ptr]), %[temp]\n\t"
        "movw %w[temp], %[short]\n\t"
        : [temp] "=&r" (temp), [short] "=m" (*short_ptr)
        : [ptr] "r" (int_ptr)
        : "memory"
    );
}

// Function to test all patterns with array access
static void test_complex_patterns(void) {
    volatile uint32_t local_array[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile uint64_t local_long = 0x1122334455667788;
    
    // Test ZERO_EXTRACT with bit-field in array element
    for (int i = 0; i < 4; i++) {
        test_zero_extract(&local_array[i], 0x00FF00FF);
    }
    
    // Test STRICT_LOW_PART with array of shorts
    volatile uint16_t short_array[8];
    for (int i = 0; i < 8; i++) {
        test_strict_low_part(&short_array[i], (uint16_t)(i * 0x1111));
    }
    
    // Test SUBREG of MEM with type punning
    test_subreg_mem(&local_array[0], (volatile uint16_t *)&local_long);
    
    // Additional complex pattern: bit-field extraction from memory
    uint32_t extracted;
    __asm__ volatile (
        "movl %[mem], %[extracted]\n\t"
        "andl $0x0000FFFF, %[extracted]\n\t"
        : [extracted] "=r" (extracted)
        : [mem] "m" (local_array[0])
        : 
    );
}

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    // Test ZERO_EXTRACT patterns
    test_zero_extract(&global_int, 0xA5A5A5A5);
    checksum += global_int;
    
    // Test STRICT_LOW_PART patterns
    for (int i = 0; i < 10; i++) {
        test_strict_low_part(&global_short_array[i], (uint16_t)(i * 0x101));
        checksum += global_short_array[i];
    }
    
    // Test SUBREG of MEM patterns
    test_subreg_mem(&global_int, &global_short_array[0]);
    checksum += global_int;
    
    // Test complex patterns
    test_complex_patterns();
    
    // Access all global variables to ensure they're not optimized away
    checksum += global_long;
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("(This value varies based on architecture and compiler)\n");
    
    return 0;
}
