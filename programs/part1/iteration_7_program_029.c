#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function to generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *ptr, int bitpos, int bitsize)
{
    /* Use inline assembly with bit-field constraints */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*ptr)
        : [src] "r" (0x55AA55AA),
          "0" (*ptr)
        : "memory"
    );
    
    /* Another attempt with explicit bit-field operation */
    uint32_t mask = ((1U << bitsize) - 1) << bitpos;
    uint32_t value = 0xDEADBEEF;
    
    __asm__ volatile (
        "and %[val], %[mask]\n\t"
        "or %[out], %[val]\n\t"
        : [out] "+r" (*ptr)
        : [val] "r" (value),
          [mask] "r" (mask)
        : "cc"
    );
}

/* Function to generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint64_t *ptr)
{
    uint32_t low_part;
    
    /* Assembly that only modifies low 32 bits of 64-bit register */
    __asm__ volatile (
        "addl $0x1234, %k[low]\n\t"  /* %k modifier for 32-bit register */
        : [low] "+&r" (low_part)
        : 
        : "cc"
    );
    
    /* Force the low part into the 64-bit value */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*ptr)
        : [src] "r" (((uint64_t)low_part << 32) | low_part)
        : "memory"
    );
    
    /* Another pattern with early-clobber to force STRICT_LOW_PART */
    uint64_t temp = *ptr;
    __asm__ volatile (
        "inc %0\n\t"
        : "+&r" (temp)
        :
        : "cc"
    );
    *ptr = temp;
}

/* Function to generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile void *mem)
{
    volatile uint16_t *short_ptr = (volatile uint16_t *)mem;
    volatile uint32_t *int_ptr = (volatile uint32_t *)mem;
    
    /* Access memory through different-sized pointers */
    __asm__ volatile (
        "movw $0xABCD, %0\n\t"
        : "=m" (*short_ptr)
        :
        : "memory"
    );
    
    /* Access sub-register of memory using pointer arithmetic */
    __asm__ volatile (
        "addw $1, %0\n\t"
        : "+m" (*(volatile uint16_t *)((char *)mem + 2))
        :
        : "cc", "memory"
    );
    
    /* Complex pattern with multiple memory accesses */
    uint32_t temp;
    __asm__ volatile (
        "movl %1, %0\n\t"
        "movw %w0, %2\n\t"  /* %w for 16-bit subreg */
        : "=&r" (temp), "+m" (*int_ptr)
        : "m" (*short_ptr)
        : "memory"
    );
}

/* Function that combines all patterns */
static void test_combined_patterns(void)
{
    volatile uint32_t buffer[4] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile struct {
        uint64_t a;
        uint32_t b;
        uint16_t c;
        uint8_t d;
    } s = {0};
    
    /* Test ZERO_EXTRACT on bit-fields */
    test_zero_extract(&buffer[0], 8, 16);
    
    /* Test STRICT_LOW_PART */
    test_strict_low_part(&s.a);
    
    /* Test SUBREG of MEM */
    test_subreg_mem(&buffer[1]);
    
    /* Direct inline assembly with complex constraints */
    uint32_t x = 0;
    __asm__ volatile (
        "movl $0x12345678, %0\n\t"
        "movw %w0, %1\n\t"  /* Store low 16 bits to memory */
        : "=&r" (x), "=m" (s.c)
        :
        : "memory"
    );
}

int main(void)
{
    uint32_t checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Test 1: ZERO_EXTRACT patterns */
    test_zero_extract(&global_int, 4, 12);
    checksum += global_int;
    
    /* Test 2: STRICT_LOW_PART patterns */
    test_strict_low_part(&global_long);
    checksum += (global_long & 0xFFFFFFFF) + (global_long >> 32);
    
    /* Test 3: SUBREG of MEM patterns */
    test_subreg_mem(global_short_array);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    /* Test 4: Combined patterns */
    test_combined_patterns();
    
    /* Test 5: Additional patterns with arrays */
    volatile uint32_t local_array[8] = {0};
    for (int i = 0; i < 8; i++) {
        test_zero_extract(&local_array[i], i * 2, 8);
        checksum += local_array[i];
    }
    
    /* Test 6: Byte array with SUBREG accesses */
    volatile uint32_t *int_view = (volatile uint32_t *)global_byte_array;
    test_subreg_mem(global_byte_array);
    for (int i = 0; i < 4; i++) {
        checksum += int_view[i];
    }
    
    printf("Checksum: %u\n", checksum);
    printf("All tests completed. Compile with:\n");
    printf("  gcc -O2 -dP -da -o test_coverage test.c\n");
    printf("  gcc -O1 -fno-omit-frame-pointer -fdump-rtl-all -o test_coverage test.c\n");
    printf("  gcc -O3 -funroll-loops -fno-inline -o test_coverage test.c\n");
    
    return 0;
}
