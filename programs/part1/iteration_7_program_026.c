#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Write to specific bit-field (bits 4-11) using inline assembly */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)
        : [src] "r" (src)
        : "memory"
    );
    
    /* Alternative: Bit-field operation that might generate ZERO_EXTRACT */
    uint32_t mask = 0xFF0;  /* Bits 4-11 */
    __asm__ volatile (
        "and %[val], %[mask]\n\t"
        "or %[dest], %[val]\n\t"
        : [dest] "+r" (*dest)
        : [val] "r" (src), [mask] "r" (mask)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *dest, uint32_t src) {
    /* Operation that only modifies low 32 bits of 64-bit register */
    __asm__ volatile (
        "addl %[src], %k[dest]\n\t"  /* 'l' suffix for 32-bit, %k for low 32-bit */
        : [dest] "+r" (*dest)
        : [src] "r" (src)
        : "cc"
    );
    
    /* Another pattern using explicit constraint modifiers */
    uint32_t low_part;
    __asm__ volatile (
        "mov %k[out], %[in]\n\t"  /* Move to low 32 bits only */
        : [out] "=&r" (low_part)  /* Early clobber to force new register */
        : [in] "r" (src)
    );
    *dest = (*dest & 0xFFFFFFFF00000000ULL) | low_part;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr, volatile uint16_t *short_ptr) {
    /* Write to memory through different-sized pointers */
    uint16_t temp = 0xABCD;
    
    /* Access 16-bit sub-region of 32-bit memory location */
    __asm__ volatile (
        "movw %[ptr], %[val]\n\t"  /* 'w' suffix for word (16-bit) */
        : [ptr] "=m" (*(volatile uint16_t *)int_ptr)  /* Cast to access sub-region */
        : [val] "r" (temp)
        : "memory"
    );
    
    /* Another pattern: accessing byte within word */
    uint8_t byte_val = 0x42;
    __asm__ volatile (
        "movb %b[val], %[ptr]\n\t"  /* 'b' suffix for byte */
        : [ptr] "=m" (*(volatile uint8_t *)short_ptr)
        : [val] "r" (byte_val)
        : "memory"
    );
}

/* Complex pattern mixing multiple operations */
static void test_mixed_patterns(volatile uint64_t *data) {
    /* Create a situation with register pressure and sub-register accesses */
    uint32_t parts[4];
    
    for (int i = 0; i < 4; i++) {
        parts[i] = i * 0x11111111;
    }
    
    /* Multiple operations that might generate SUBREG patterns */
    __asm__ volatile (
        "movl %[p0], 0(%[dst])\n\t"
        "movw %w[p1], 4(%[dst])\n\t"  /* %w for 16-bit sub-register */
        "movb %b[p2], 6(%[dst])\n\t"  /* %b for 8-bit sub-register */
        "addl %[p3], 8(%[dst])\n\t"
        : 
        : [dst] "r" (data), 
          [p0] "r" (parts[0]),
          [p1] "r" (parts[1]),
          [p2] "r" (parts[2]),
          [p3] "r" (parts[3])
        : "memory"
    );
}

/* Test with bit-field structures (likely to generate ZERO_EXTRACT) */
struct bitfield_struct {
    uint32_t low : 8;
    uint32_t mid : 16;
    uint32_t high : 8;
};

static void test_bitfield_struct(volatile struct bitfield_struct *bf) {
    /* Operations on bit-fields often generate ZERO_EXTRACT */
    uint32_t temp = 0x55AA;
    
    __asm__ volatile (
        "or %[bf], %[val]\n\t"
        : [bf] "+r" (*(volatile uint32_t *)bf)
        : [val] "r" (temp)
        : "cc"
    );
}

int main() {
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint16_t local_short = 0xCAFE;
    volatile uint64_t local_long = 0x123456789ABCDEF0ULL;
    
    printf("Starting coverage test...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&local_int, 0x87654321);
    test_zero_extract(&global_int, 0x5555AAAA);
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&local_long, 0x11111111);
    test_strict_low_part(&global_long, 0x22222222);
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&local_int, &local_short);
    test_subreg_mem(&global_int, global_short_array);
    
    /* Test mixed patterns */
    test_mixed_patterns(&local_long);
    
    /* Test bit-field structures */
    volatile struct bitfield_struct bf = {0};
    test_bitfield_struct(&bf);
    
    /* Create checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    checksum += local_int;
    checksum += local_short;
    checksum += local_long;
    checksum += global_int;
    checksum += global_long;
    
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    checksum += *(uint32_t *)&bf;
    
    printf("Checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Test completed.\n");
    
    return 0;
}
