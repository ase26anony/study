#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to ensure side effects are preserved */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {0};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function 1: Target ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Use bit-field assignment to potentially generate ZERO_EXTRACT */
    __asm__ volatile (
        "mov %[dest], %[src]\n\t"
        : [dest] "=r" (*dest)  /* Output constraint for register */
        : [src] "r" (src)
        : "memory"
    );
}

/* Function 1b: More explicit bit-field manipulation */
static void test_zero_extract_bitfield(volatile uint32_t *dest) {
    uint32_t temp;
    /* Attempt to write to specific bits (bits 8-15) */
    __asm__ volatile (
        "btsl $8, %[dest]\n\t"      /* Set bit 8 */
        "btrl $12, %[dest]\n\t"     /* Clear bit 12 */
        : [dest] "+r" (*dest)
        : 
        : "cc", "memory"
    );
}

/* Function 2: Target STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Use constraint that suggests only low part is modified */
    __asm__ volatile (
        "addw %[src], %[dest]\n\t"  /* Word-sized add (16-bit) */
        : [dest] "+r" (*dest)
        : [src] "r" (src)
        : "cc"
    );
}

/* Function 2b: Byte operation on 32-bit register */
static void test_strict_low_part_byte(volatile uint8_t *dest, uint8_t src) {
    /* Byte operation that might generate STRICT_LOW_PART */
    __asm__ volatile (
        "addb %[src], %[dest]\n\t"
        : [dest] "+r" (*dest)
        : [src] "r" (src)
        : "cc"
    );
}

/* Function 3: Target SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Cast to access sub-region of memory */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    /* Write to 16-bit sub-region of 32-bit memory location */
    __asm__ volatile (
        "movw $0xABCD, %[ptr]\n\t"
        : [ptr] "=m" (*short_ptr)  /* Memory operand for 16-bit access */
        : 
        : "memory"
    );
}

/* Function 3b: More complex SUBREG memory access with early-clobber */
static void test_subreg_mem_complex(volatile uint64_t *long_ptr) {
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    /* Use early-clobber to complicate register allocation */
    uint32_t temp;
    __asm__ volatile (
        "movl $0xDEADBEEF, %%eax\n\t"
        "movl %%eax, %[ptr]\n\t"
        "movl $0xCAFEBABE, %%ebx\n\t"
        "addl %%ebx, %[ptr]\n\t"
        : [ptr] "=m" (*int_ptr), "=&a" (temp)
        : 
        : "ebx", "cc", "memory"
    );
}

/* Function 3c: Access different sized subregions of array */
static void test_subreg_mem_array(volatile uint8_t *array) {
    /* Access 32-bit chunk of byte array */
    volatile uint32_t *int_view = (volatile uint32_t *)(array + 4);
    
    __asm__ volatile (
        "orl $0xFF00FF00, %[view]\n\t"
        : [view] "=m" (*int_view)
        : 
        : "memory"
    );
}

/* Function 4: Combined pattern - potentially generates multiple RTL forms */
static void test_combined_pattern(volatile uint64_t *dest) {
    volatile uint32_t *half1 = (volatile uint32_t *)dest;
    volatile uint16_t *quarter = (volatile uint16_t *)((uintptr_t)dest + 2);
    
    /* Multiple operations that might generate different RTL patterns */
    __asm__ volatile (
        "movl $0x12345678, %[h1]\n\t"
        "movw $0x9ABC, %[q]\n\t"
        : [h1] "=m" (*half1), [q] "=m" (*quarter)
        : 
        : "memory"
    );
}

/* Driver function */
int main() {
    volatile uint32_t local_int = 0xAAAAAAAA;
    volatile uint16_t local_short = 0xBBBB;
    volatile uint8_t local_byte = 0xCC;
    volatile uint64_t local_long = 0xDDDDDDDDDDDDDDDDULL;
    
    uint32_t checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract(&local_int, 0x55555555);
    test_zero_extract_bitfield(&local_int);
    checksum += local_int;
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(&local_short, 0x1111);
    test_strict_low_part_byte(&local_byte, 0x22);
    checksum += local_short + local_byte;
    
    /* Test SUBREG of MEM patterns */
    test_subreg_mem(&local_int);
    test_subreg_mem_complex(&local_long);
    test_subreg_mem_array(global_byte_array);
    checksum += local_int + (uint32_t)(local_long >> 32) + (uint32_t)local_long;
    
    /* Test combined pattern */
    test_combined_pattern(&local_long);
    checksum += (uint32_t)(local_long >> 32) + (uint32_t)local_long;
    
    /* Test with global variables */
    test_zero_extract(&global_int, 0x87654321);
    test_strict_low_part(&global_short_array[2], 0x3333);
    test_subreg_mem(&global_int);
    
    /* Add globals to checksum */
    checksum += global_int;
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
