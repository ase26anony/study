/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
    unsigned int flag2 : 1;
    unsigned int pad : 31 - (1+3+12+16+1); /* fill to 32 bits */
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    volatile uint32_t word;
    volatile uint16_t half[2];
    volatile uint8_t bytes[4];
};

/* Force misaligned access structure */
struct misaligned {
    char preamble[3];
    struct bitfield_pack packed; /* Will be misaligned */
} __attribute__((packed));

/* Complex array access pattern */
#define ARRAY_SIZE 256
#define STRIDE 7

/* Inline assembly to force specific register constraints */
static inline uint32_t asm_low_part(uint64_t val) {
    uint32_t result;
    /* Operate on low 32 bits, potentially creating STRICT_LOW_PART */
    __asm__ volatile (
        "movl %k1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)val)  /* Only low part */
        : "cc"
    );
    return result;
}

static inline uint64_t asm_zero_extract(uint64_t val, int start, int length) {
    uint64_t result;
    /* Extract bit field - may generate ZERO_EXTRACT */
    __asm__ volatile (
        "shrq %2, %1\n\t"
        "andq %3, %1\n\t"
        "movq %1, %0"
        : "=r" (result)
        : "r" (val), "i" ((unsigned)start), "i" ((1ULL << length) - 1)
        : "cc"
    );
    return result;
}

int main(int argc, char *argv[]) {
    volatile int i, j, limit;
    volatile uint64_t accumulator = 0;
    
    /* Use argc to prevent compile-time loop unrolling */
    limit = (argc > 1) ? 100 : 50;
    
    /* Declare variables that should generate target RTL patterns */
    volatile union mixed_access mix;
    volatile struct misaligned misalign;
    volatile long long big_int = 0x123456789ABCDEF0LL;
    volatile double fp_val = 3.141592653589793;
    volatile int32_t array[ARRAY_SIZE];
    
    /* Initialize array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * STRIDE;
    }
    
    /* Initialize bit-field structure */
    mix.bits.flag1 = 1;
    mix.bits.small = 5;
    mix.bits.medium = 2047;
    mix.bits.large = 32768;
    mix.bits.flag2 = 0;
    
    /* Main loop with complex operations */
    for (i = 0; i < limit; i++) {
        uint32_t temp32;
        uint64_t temp64;
        
        /* 1. Bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART candidates) */
        /* Extract and manipulate bit-fields */
        temp32 = mix.bits.medium;  /* May generate ZERO_EXTRACT */
        mix.bits.small = (temp32 & 0x7) ^ i;  /* May generate STRICT_LOW_PART */
        
        /* Complex bit-field assignment with masking */
        mix.bits.large = (mix.bits.large + i) & 0xFFFF;  /* Strict 16-bit operation */
        
        /* 2. Multi-word operations (SUBREG candidates on 32-bit targets) */
        /* Operations on long long - may be split into SUBREGs */
        big_int = big_int + ((uint64_t)mix.bits.medium << 16);
        big_int = big_int | ((uint64_t)mix.bits.small << 32);
        
        /* Comparison forcing high/low word separation */
        if ((uint32_t)(big_int >> 32) > (uint32_t)big_int) {
            /* Swap high and low words */
            uint32_t hi = (uint32_t)(big_int >> 32);
            uint32_t lo = (uint32_t)big_int;
            big_int = ((uint64_t)lo << 32) | hi;
        }
        
        /* 3. Floating point operations (may use multiple registers) */
        fp_val = fp_val * 1.01 + (double)(i & 0xF);
        
        /* 4. Complex array addressing with bit-field derived index */
        /* Non-linear array access pattern */
        j = (mix.bits.small * STRIDE + i) % ARRAY_SIZE;
        
        /* Read-modify-write with bit manipulation */
        temp32 = array[j];
        array[j] = (temp32 ^ mix.word) + (temp32 & mix.bits.large);
        
        /* 5. Inline assembly forcing specific patterns */
        temp32 = asm_low_part(big_int);
        temp64 = asm_zero_extract(big_int, mix.bits.small, 8);
        
        /* 6. Misaligned structure access */
        misalign.packed.medium = array[i % 64];
        temp32 = misalign.packed.large;  /* Misaligned access */
        
        /* 7. Conditional based on bit-field parity */
        if (mix.bits.flag1 ^ (mix.bits.small & 1)) {
            /* Different operation path */
            mix.bits.flag2 = !mix.bits.flag2;
            big_int = big_int >> 1;
        } else {
            mix.bits.flag1 = mix.bits.flag2;
            big_int = big_int << 1;
        }
        
        /* 8. Pointer arithmetic with casting */
        uint8_t *byte_ptr = (uint8_t*)&array[j];
        uint32_t word_from_bytes = 
            (byte_ptr[0] << 24) | 
            (byte_ptr[1] << 16) | 
            (byte_ptr[2] << 8) | 
            byte_ptr[3];
        
        /* Modify via byte pointer */
        byte_ptr[(i % 4)] ^= 0xAA;
        
        /* Accumulate results */
        accumulator += temp32 + (temp64 & 0xFFFFFFFF) + word_from_bytes;
    }
    
    /* Additional complex operation after loop */
    /* Double-word shift with masking */
    uint64_t mask = (1ULL << mix.bits.medium) - 1;
    big_int = (big_int & mask) | ((big_int & ~mask) >> mix.bits.small);
    
    /* Final array reduction */
    int32_t array_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i += 8) {
        /* Strided access pattern */
        array_sum += array[i] - array[(i + 128) % ARRAY_SIZE];
    }
    
    /* Combine all results into final value */
    uint64_t final_result = 
        accumulator + 
        big_int + 
        (uint64_t)array_sum + 
        (uint64_t)mix.word + 
        (uint64_t)(fp_val * 1000);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %llu\n", (unsigned long long)final_result);
    
    /* Return based on bit-field state */
    return (mix.bits.flag1 << 1) | mix.bits.flag2;
}
