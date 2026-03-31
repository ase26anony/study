/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Force generation of ZERO_EXTRACT and STRICT_LOW_PART through bit-field operations */
struct bitfield_struct {
    volatile unsigned int field1 : 1;
    volatile unsigned int field2 : 3;
    volatile unsigned int field3 : 12;
    volatile unsigned int field4 : 8;
    volatile unsigned int field5 : 8;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    volatile uint32_t word;
    volatile struct bitfield_struct bits;
    volatile uint8_t bytes[4];
};

/* Complex array with stride access */
#define ARRAY_SIZE 256
#define STRIDE 7

/* Function to create complex addressing modes */
static int complex_index(int i, int offset) {
    return (i * STRIDE + offset) & (ARRAY_SIZE - 1);
}

/* Inline assembly to force specific register usage patterns */
static inline uint32_t manipulate_low_part(uint64_t value) {
    uint32_t result;
    /* This asm may generate STRICT_LOW_PART patterns */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((uint32_t)value)
        : "cc"
    );
    return result;
}

static inline uint64_t extract_bits(uint64_t value, int start, int length) {
    uint64_t mask = (1ULL << length) - 1;
    return (value >> start) & mask;
}

int main(int argc, char *argv[]) {
    volatile union mixed_access data;
    volatile uint64_t large_int = 0x123456789ABCDEF0ULL;
    volatile double fp_value = 3.141592653589793;
    volatile int32_t complex_array[ARRAY_SIZE];
    volatile int loop_limit = (argc > 1) ? 100 : 50;
    volatile int i, j;
    int result = 0;
    
    /* Initialize data */
    data.word = 0xDEADBEEF;
    for (i = 0; i < ARRAY_SIZE; i++) {
        complex_array[i] = i * 3;
    }
    
    /* Main loop with mixed operations to generate various RTL patterns */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        data.bits.field1 = i & 1;
        data.bits.field2 = (i >> 1) & 0x7;
        data.bits.field3 = (i * 17) & 0xFFF;
        
        /* Extract and combine bit-fields */
        uint32_t extracted = (data.bits.field3 << 4) | data.bits.field2;
        data.bits.field4 = extracted & 0xFF;
        data.bits.field5 = (extracted >> 8) & 0xFF;
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        /* Force 64-bit operations on 32-bit target */
        large_int = large_int * 0x1234567ULL + i;
        
        /* Cross-type operations that may generate SUBREG */
        fp_value = fp_value * (double)(large_int & 0xFFFFFFFF) + (double)(large_int >> 32);
        
        /* 3. Complex memory addressing with bit-field dependent indexing */
        int idx = complex_index(i, data.bits.field2);
        
        /* Read-modify-write with bit manipulation */
        complex_array[idx] = (complex_array[idx] & ~0xFF) | data.bits.field4;
        
        /* Another complex access pattern */
        idx = complex_index(i, data.bits.field3 & 0x1F);
        complex_array[idx] ^= (data.bits.field5 << 8) | data.bits.field4;
        
        /* 4. Conditional operations based on bit-field results */
        if (data.bits.field1) {
            /* Use inline assembly that might generate STRICT_LOW_PART */
            uint32_t low_part = manipulate_low_part(large_int);
            complex_array[i % ARRAY_SIZE] += low_part;
        } else {
            /* Extract specific bit ranges (potential ZERO_EXTRACT) */
            uint64_t bits_20_31 = extract_bits(large_int, 20, 12);
            complex_array[i % ARRAY_SIZE] -= (int32_t)bits_20_31;
        }
        
        /* 5. Additional SUBREG patterns through type punning */
        {
            volatile uint32_t *ptr32 = (volatile uint32_t *)&large_int;
            volatile uint16_t *ptr16 = (volatile uint16_t *)&large_int;
            
            /* Access different parts of the 64-bit value */
            ptr32[0] = ptr32[0] + ptr16[1];  /* High 16 bits added to low 32 bits */
            ptr32[1] = ptr32[1] ^ ptr16[0];  /* Low 16 bits XORed with high 32 bits */
        }
        
        /* 6. Switch statement with bit-field dependent cases */
        switch (data.bits.field2) {
            case 0:
                complex_array[complex_index(i, 0)] = data.word;
                break;
            case 1:
            case 2:
                data.word = complex_array[complex_index(i, 1)] 
                          + complex_array[complex_index(i, 2)];
                break;
            case 3:
            case 4:
                /* More bit manipulation */
                data.bits.field3 = (data.bits.field3 << 1) | data.bits.field1;
                break;
            default:
                /* Force memory access with complex address */
                int base = data.bits.field3 % (ARRAY_SIZE / 2);
                for (j = 0; j < 4; j++) {
                    complex_array[base + j] += i * j;
                }
                break;
        }
        
        /* 7. Pointer arithmetic with bit-field offsets */
        volatile int32_t *array_ptr = &complex_array[data.bits.field4 % 64];
        for (j = 0; j < 4; j++) {
            array_ptr[j] = array_ptr[j] * 2 + j;
        }
    }
    
    /* Aggregate results to prevent optimization */
    for (i = 0; i < ARRAY_SIZE; i++) {
        result += complex_array[i];
    }
    result += data.word;
    result += (large_int >> 32) + (large_int & 0xFFFFFFFF);
    result += (int)fp_value;
    
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}

/* Additional function to create more RTL patterns during compilation */
static void __attribute__((noinline)) extra_patterns(void) {
    volatile struct {
        unsigned int a : 5;
        unsigned int b : 11;
        unsigned int c : 16;
    } s1, s2;
    
    volatile uint64_t ll1, ll2;
    volatile double d1, d2;
    
    s1.a = 0x1F;
    s1.b = 0x7FF;
    s1.c = 0xFFFF;
    
    /* Force ZERO_EXTRACT patterns */
    s2.a = s1.b & 0x1F;           /* Extract low 5 bits of 11-bit field */
    s2.b = (s1.c >> 4) & 0x7FF;   /* Extract bits 4-14 of 16-bit field */
    s2.c = s1.a | (s1.b << 5);    /* Combine fields */
    
    /* Force SUBREG patterns with 64-bit operations */
    ll1 = 0x123456789ABCDEF0ULL;
    ll2 = ll1 >> 32;              /* High part */
    d1 = (double)ll1;
    d2 = (double)ll2;
    
    /* Mixed operations */
    ll1 = ll1 + (uint64_t)d1;
    ll2 = ll2 * (uint64_t)d2;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(s2.a), "r"(s2.b), "r"(s2.c), "r"(ll1), "r"(ll2));
}
