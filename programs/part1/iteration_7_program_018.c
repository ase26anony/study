#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint64_t global_64 = 0x123456789ABCDEF0ULL;
volatile uint32_t global_32 = 0xDEADBEEF;
volatile uint16_t global_16[8] = {0x1111, 0x2222, 0x3333, 0x4444, 
                                  0x5555, 0x6666, 0x7777, 0x8888};
volatile uint8_t global_8[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

/* Function to test ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize) {
    uint32_t temp;
    
    /* Inline assembly that should generate ZERO_EXTRACT RTL */
    __asm__ volatile (
        /* Writing to specific bits of a memory location */
        "btsl %[pos], %[mem]\n\t"
        : [mem] "+m" (*ptr)
        : [pos] "Ir" (bitpos)
        : "cc", "memory"
    );
    
    /* Another pattern with explicit bitfield operation */
    __asm__ volatile (
        /* Using bitfield insert/extract instructions if available */
        "bfi %0, %1, %2, %3\n\t"
        : "+r" (temp)
        : "r" (0xAA), "I" (bitpos), "I" (bitsize)
        : "cc"
    );
    
    /* Store the result back */
    __asm__ volatile (
        "mov %1, %0\n\t"
        : "=m" (*ptr)
        : "r" (temp)
        : "memory"
    );
}

/* Function to test STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr) {
    uint32_t low_part;
    uint64_t full_val;
    
    /* Assembly that modifies only low part of register */
    __asm__ volatile (
        /* 'q' modifier for DImode register access in 32-bit mode */
        "add{l} {%1, %0|%0, %1}\n\t"
        : "+r" (full_val)
        : "r" (0x1000UL)
        : "cc"
    );
    
    /* Constraint suggesting only low 16 bits are modified */
    __asm__ volatile (
        "add{w} {%1, %k0|%k0, %1}\n\t"
        : "=r" (low_part)
        : "r" (0x5555), "0" (low_part)
        : "cc"
    );
    
    /* Combine results with memory store */
    __asm__ volatile (
        "mov %1, %0\n\t"
        : "=m" (*ptr)
        : "r" (full_val)
        : "memory"
    );
}

/* Function to test SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *ptr) {
    /* Cast to different types to force SUBREG MEM access */
    volatile uint16_t *as_short = (volatile uint16_t *)ptr;
    volatile uint8_t *as_byte = (volatile uint8_t *)ptr;
    
    /* Access memory through different-sized views */
    __asm__ volatile (
        /* Write to 16-bit sub-region of 32-bit memory */
        "mov{w} %1, %0\n\t"
        : "=m" (*as_short)
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
    
    /* Another access with offset */
    __asm__ volatile (
        /* Write to 8-bit sub-region with early-clobber */
        "mov{b} {%1, %0|%0, %1}\n\t"
        : "=m" (as_byte[2]), "=&r" (*(volatile uint8_t *)ptr)
        : "i" (0xEF)
        : "memory"
    );
    
    /* Complex pattern with multiple sub-register accesses */
    uint32_t temp;
    __asm__ volatile (
        /* Load 32-bit, modify low 16, store back */
        "mov{l} %1, %0\n\t"
        "add{w} $0x1111, %w0\n\t"
        "mov{l} %0, %1\n\t"
        : "=&r" (temp), "+m" (*(volatile uint32_t *)ptr)
        :
        : "cc", "memory"
    );
}

/* Additional test with array access and pointer arithmetic */
static void test_complex_mem_access(void) {
    volatile uint32_t buffer[4] = {0};
    volatile uint16_t *half_ptr;
    
    /* Force SUBREG MEM through pointer casting */
    half_ptr = (volatile uint16_t *)((char *)buffer + 1);
    
    __asm__ volatile (
        /* Unaligned 16-bit write to 32-bit memory */
        "mov{w} %1, %0\n\t"
        : "=m" (*half_ptr)
        : "r" ((uint16_t)0x1234)
        : "memory"
    );
    
    /* Test with bitfield in struct (can generate ZERO_EXTRACT) */
    struct bitfield {
        uint32_t a:8;
        uint32_t b:16;
        uint32_t c:8;
    } __attribute__((packed));
    
    volatile struct bitfield bf = {0};
    
    __asm__ volatile (
        /* This may generate ZERO_EXTRACT for bitfield assignment */
        "or{l} $0x00FF0000, %0\n\t"
        : "+m" (*(volatile uint32_t *)&bf)
        :
        : "cc", "memory"
    );
}

/* Main driver function */
int main(void) {
    uint64_t checksum = 0;
    
    printf("Testing RTL pattern generation for coverage...\n");
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&global_32, 5, 8);
    checksum += global_32;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&global_64);
    checksum += (global_64 & 0xFFFFFFFF) + (global_64 >> 32);
    
    /* Test SUBREG MEM patterns */
    test_subreg_mem(&global_32);
    checksum += global_32;
    
    /* Test complex memory access */
    test_complex_mem_access();
    
    /* Add array elements to checksum */
    for (int i = 0; i < 8; i++) {
        checksum += global_16[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += global_8[i];
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
