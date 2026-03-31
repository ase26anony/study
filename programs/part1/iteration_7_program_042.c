#include <stdint.h>
#include <stdio.h>

/* Global volatile variables to prevent optimization */
volatile uint32_t global_int = 0x12345678;
volatile uint16_t global_short_array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
volatile uint64_t global_long = 0xFFFFFFFFFFFFFFFFULL;
volatile uint8_t global_byte_array[16] = {0};

/* Function 1: Generate ZERO_EXTRACT pattern */
static void test_zero_extract(volatile uint32_t *dest, uint32_t src) {
    /* Write to specific bits of a 32-bit value using bit-field constraints */
    __asm__ volatile (
        "mov %0, %1\n\t"
        : "=r" (*dest)  /* Output constraint for register */
        : "r" (src)     /* Input constraint */
        : "memory"
    );
}

/* Function 2: Generate ZERO_EXTRACT with memory destination */
static void test_zero_extract_mem(volatile uint32_t *dest, uint32_t src) {
    /* Using bit-field assignment to memory with specific constraints */
    __asm__ volatile (
        "or %0, %1\n\t"
        : "=m" (*dest)  /* Memory destination - may generate ZERO_EXTRACT */
        : "r" (src)
        : "memory"
    );
}

/* Function 3: Generate STRICT_LOW_PART pattern */
static void test_strict_low_part(volatile uint16_t *dest, uint16_t src) {
    /* Operation that only modifies low part of register */
    __asm__ volatile (
        "addw %0, %1\n\t"  /* 'w' suffix for 16-bit operation on x86 */
        : "=r" (*dest)
        : "r" (src), "0" (*dest)
        : "cc"
    );
}

/* Function 4: Generate STRICT_LOW_PART with complex constraints */
static void test_strict_low_part_complex(volatile uint32_t *a, volatile uint32_t *b) {
    uint32_t temp;
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "=r" (temp)      /* Output in register */
        : "r" (*a), "0" (*b)
        : "cc"
    );
    *a = temp;
}

/* Function 5: Generate SUBREG of MEM pattern */
static void test_subreg_mem(volatile uint32_t *int_ptr) {
    /* Access sub-register of memory through pointer casting */
    volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
    
    __asm__ volatile (
        "movw %1, %0\n\t"
        : "=m" (*short_ptr)    /* Write to 16-bit sub-register of 32-bit memory */
        : "r" ((uint16_t)0xABCD)
        : "memory"
    );
}

/* Function 6: Generate SUBREG of MEM with early clobber */
static void test_subreg_mem_earlyclobber(volatile uint64_t *long_ptr) {
    volatile uint32_t *int_ptr = (volatile uint32_t *)long_ptr;
    
    __asm__ volatile (
        "movl %%eax, %0\n\t"
        "addl %%eax, %1\n\t"
        : "=&m" (*int_ptr), "=m" (*(int_ptr + 1))  /* Early clobber on first operand */
        : 
        : "eax", "memory"
    );
}

/* Function 7: Complex pattern mixing SUBREG and ZERO_EXTRACT */
static void test_mixed_pattern(volatile uint64_t *data) {
    volatile uint8_t *byte_ptr = (volatile uint8_t *)data;
    
    /* Write to specific byte within the 64-bit value */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=m" (byte_ptr[3])    /* Access byte 3 of 64-bit value */
        : "r" ((uint8_t)0x42)
        : "memory"
    );
}

/* Function 8: Test with array and pointer arithmetic */
static void test_array_access(volatile uint16_t *arr, int index) {
    /* Access array element - may generate SUBREG(MEM) */
    __asm__ volatile (
        "incw %0\n\t"
        : "=m" (arr[index])
        : 
        : "memory"
    );
}

/* Function 9: Test with bit-field structure (may generate ZERO_EXTRACT) */
struct bitfield_struct {
    uint32_t a : 8;
    uint32_t b : 8;
    uint32_t c : 8;
    uint32_t d : 8;
};

static void test_bitfield_struct(volatile struct bitfield_struct *bf) {
    /* This may generate ZERO_EXTRACT when compiled */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=m" (bf->b)    /* Access specific bit-field */
        : "r" ((uint8_t)0x99)
        : "memory"
    );
}

/* Main driver function */
int main() {
    volatile uint32_t local_int = 0xDEADBEEF;
    volatile uint16_t local_short = 0x1234;
    volatile uint64_t local_long = 0x1122334455667788ULL;
    volatile uint32_t local_array[4] = {0xA, 0xB, 0xC, 0xD};
    struct bitfield_struct bf = {0};
    
    uint32_t checksum = 0;
    
    /* Test all patterns */
    test_zero_extract(&local_int, 0x5555AAAA);
    checksum += local_int;
    
    test_zero_extract_mem(&global_int, 0x3333CCCC);
    checksum += global_int;
    
    test_strict_low_part(&local_short, 0x1111);
    checksum += local_short;
    
    test_strict_low_part_complex(&local_int, &global_int);
    checksum += local_int;
    
    test_subreg_mem(&local_int);
    checksum += local_int;
    
    test_subreg_mem_earlyclobber(&local_long);
    checksum += (uint32_t)(local_long >> 32) + (uint32_t)local_long;
    
    test_mixed_pattern(&global_long);
    checksum += (uint32_t)(global_long >> 32) + (uint32_t)global_long;
    
    test_array_access(global_short_array, 2);
    for (int i = 0; i < 8; i++) {
        checksum += global_short_array[i];
    }
    
    test_bitfield_struct(&bf);
    checksum += *(uint32_t*)&bf;
    
    /* Access all global variables to prevent dead code elimination */
    checksum += global_int;
    for (int i = 0; i < 16; i++) {
        checksum += global_byte_array[i];
    }
    
    printf("Checksum: %u\n", checksum);
    return 0;
}
