/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
    unsigned int padding : 32 - (1 + 3 + 12 + 16);
};

/* Union for type-punning to access different views of the same memory */
union data_union {
    struct bitfield_struct bits;
    uint32_t raw;
    uint16_t halves[2];
    uint8_t bytes[4];
};

/* Packed structure to force misaligned accesses */
struct __attribute__((packed)) packed_struct {
    char prefix;
    struct bitfield_struct bf;
    char suffix;
};

/* Function with inline assembly to explicitly generate low-part operations */
static inline uint32_t manipulate_low_part(uint64_t value) {
    uint32_t result;
    /* Inline asm that operates on low 32 bits, potentially generating STRICT_LOW_PART */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)value)  /* Cast to 32-bit to use low part */
        : "cc"
    );
    return result;
}

/* Function to force SUBREG generation for 64-bit operations on 32-bit targets */
static uint64_t process_64bit(uint64_t a, uint64_t b) {
    volatile uint64_t temp = a;
    
    /* Operations that might be split into high/low register pairs */
    temp = temp + b;
    temp = temp ^ (b << 16);
    temp = temp | (a >> 32);
    
    /* Comparison forcing separate high/low word handling */
    if ((temp & 0xFFFFFFFF) > (b & 0xFFFFFFFF)) {
        temp = temp + 0x100000000ULL;
    }
    
    return temp;
}

/* Function using complex array indexing */
static void complex_array_access(volatile uint32_t *arr, int size, int stride) {
    for (int i = 0; i < size; i++) {
        /* Complex addressing mode: arr[i*stride + constant] */
        uint32_t idx = i * stride + 7;
        if (idx < size) {
            /* Read-modify-write with bit manipulation */
            uint32_t val = arr[idx];
            val = (val & 0xFFFF0000) | ((val & 0xFFFF) + i);
            arr[idx] = val;
        }
    }
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? 10 : 5;
    volatile uint32_t accumulator = 0;
    
    /* Volatile variables to prevent optimization */
    volatile struct bitfield_struct bf_var = {0};
    volatile union data_union data_union_var = {0};
    volatile uint64_t ll_var = 0x123456789ABCDEF0ULL;
    volatile double dbl_var = 3.141592653589793;
    volatile uint32_t array[128];
    
    /* Initialize array with pattern */
    for (int i = 0; i < 128; i++) {
        array[i] = i * 0x01010101;
    }
    
    /* Main loop with mixed operations */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf_var.flag1 = i & 1;
        bf_var.small = (i * 3) & 0x7;
        bf_var.medium = (i * 17) & 0xFFF;
        bf_var.large = (i * 257) & 0xFFFF;
        
        /* Extract bit-field values using masking/shifting */
        uint32_t extracted = ((bf_var.medium << 4) & 0xFF00) | bf_var.small;
        
        /* 2. Multi-word operations (potential SUBREG) */
        ll_var = process_64bit(ll_var, (uint64_t)extracted);
        
        /* Double operations on 32-bit targets use multiple registers */
        dbl_var = dbl_var * 1.1 + (double)(i & 0xF);
        
        /* 3. Complex memory addressing with bit manipulation */
        uint32_t idx = ((extracted * 13) + i) & 0x7F;
        
        /* Potential ZERO_EXTRACT from memory location */
        uint32_t mem_val = array[idx];
        uint32_t low_bits = mem_val & 0xFF;
        uint32_t high_bits = (mem_val >> 24) & 0xFF;
        
        /* Modify array element - complex addressing in RTL */
        array[idx] = (mem_val & 0x00FFFF00) | (low_bits << 24) | high_bits;
        
        /* 4. Control flow based on bit-field results */
        if (bf_var.flag1) {
            /* Branch 1: More bit manipulation */
            uint32_t temp = data_union_var.raw;
            temp = (temp << bf_var.small) | (temp >> (32 - bf_var.small));
            data_union_var.raw = temp;
            
            /* Use inline assembly for explicit low-part operation */
            uint32_t asm_result = manipulate_low_part(ll_var);
            accumulator += asm_result;
        } else {
            /* Branch 2: Different operations */
            if ((ll_var >> 32) > (ll_var & 0xFFFFFFFF)) {
                /* High word greater than low word */
                ll_var = ll_var ^ 0xFFFFFFFF00000000ULL;
            }
            
            /* Access packed structure (potential misaligned access) */
            struct packed_struct ps;
            ps.prefix = 'A' + i;
            ps.bf = bf_var;
            ps.suffix = 'Z' - i;
            
            accumulator += ps.bf.medium + ps.bf.large;
        }
        
        /* 5. Complex array function call */
        complex_array_access(array, 128, 3 + (i & 3));
        
        /* Mix in double operations */
        if (dbl_var > 10.0) {
            dbl_var = dbl_var / 2.0;
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    uint32_t final_result = accumulator;
    
    for (int i = 0; i < 128; i++) {
        final_result ^= array[i];
    }
    
    final_result += (uint32_t)ll_var;
    final_result += (uint32_t)(ll_var >> 32);
    final_result += (uint32_t)(dbl_var * 1000.0);
    
    /* Use result to affect return value */
    return (final_result & 0xFF) == 0 ? 0 : 1;
}
