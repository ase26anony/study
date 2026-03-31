/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Force generation of ZERO_EXTRACT and STRICT_LOW_PART patterns */
struct bitfield_pack {
    unsigned int a : 3;    /* 3-bit field */
    unsigned int b : 12;   /* 12-bit field */
    unsigned int c : 1;    /* 1-bit field */
    unsigned int d : 8;    /* 8-bit field */
    unsigned int e : 7;    /* 7-bit field */
    unsigned int f : 1;    /* 1-bit field - total 32 bits */
} __attribute__((packed));

/* Union for bit manipulation */
union bit_manipulator {
    struct bitfield_pack fields;
    uint32_t raw;
    uint16_t halves[2];
    uint8_t bytes[4];
};

/* Force SUBREG generation for multi-word operations */
typedef struct {
    volatile uint64_t ll_data;      /* long long - may need multiple registers */
    volatile double fp_data;        /* double - may need multiple registers */
} multiword_t;

/* Complex memory access pattern */
#define ARRAY_SIZE 128
#define STRIDE 7

int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union bit_manipulator manip = {0};
    volatile multiword_t mw = {0};
    volatile uint32_t array[ARRAY_SIZE];
    
    /* Initialize array with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? (argc % 10) + 5 : 10;
    
    /* Result accumulator */
    uint32_t result = 0;
    
    /* Main loop - creates complex control flow */
    for (volatile int i = 0; i < iterations; i++) {
        /* ===== ZERO_EXTRACT and STRICT_LOW_PART patterns ===== */
        
        /* 1. Direct bit-field assignment (may generate STRICT_LOW_PART) */
        bf.a = (i & 0x7);                    /* 3-bit field */
        bf.b = (i * 3) & 0xFFF;              /* 12-bit field */
        bf.c = (i >> 1) & 0x1;               /* 1-bit field */
        
        /* 2. Bit-field extraction using union (may generate ZERO_EXTRACT) */
        manip.fields = bf;
        uint32_t extracted = manip.raw & 0x1FFF;  /* Extract lower 13 bits */
        
        /* 3. Complex bit manipulation with shifting */
        uint32_t temp = (extracted << 5) | (extracted >> 8);
        bf.d = (temp >> 3) & 0xFF;           /* 8-bit field assignment */
        
        /* 4. Cross-field operations */
        bf.e = (bf.b + bf.d) & 0x7F;         /* 7-bit field */
        bf.f = (bf.a ^ bf.c) & 0x1;          /* 1-bit field */
        
        /* ===== SUBREG patterns for multi-word operations ===== */
        
        /* 5. 64-bit operations on 32-bit targets (forces SUBREG splitting) */
        mw.ll_data += (uint64_t)extracted * 0x100000001ULL;
        
        /* 6. Double precision operations */
        mw.fp_data = mw.fp_data * 1.5 + (double)(i & 0xF);
        
        /* 7. Mix 32-bit and 64-bit operations */
        uint32_t low_part = (uint32_t)(mw.ll_data & 0xFFFFFFFF);
        uint32_t high_part = (uint32_t)(mw.ll_data >> 32);
        mw.ll_data = ((uint64_t)(high_part ^ low_part) << 32) | (low_part + high_part);
        
        /* ===== Complex memory addressing with bit-field indexing ===== */
        
        /* 8. Array access with bit-field derived index */
        int idx = ((bf.b * STRIDE) + bf.d) % ARRAY_SIZE;
        
        /* 9. Read-modify-write with bit manipulation */
        uint32_t array_val = array[idx];
        array_val ^= (extracted << (bf.a * 2));
        array_val |= (1 << (bf.e & 0x1F));
        array[idx] = array_val;
        
        /* 10. Misaligned access simulation via pointer arithmetic */
        uint8_t *byte_ptr = (uint8_t*)&array[idx];
        uint32_t reconstructed = (byte_ptr[0] << 24) | (byte_ptr[1] << 16) |
                                 (byte_ptr[2] << 8) | byte_ptr[3];
        
        /* ===== Control flow based on bit-field results ===== */
        
        /* 11. Branch on bit-field parity */
        if (bf.f) {
            /* When f=1, use different operations */
            mw.ll_data >>= (bf.a + 1);
            bf.e = (bf.e * 3) & 0x7F;
            
            /* Complex array access pattern */
            int alt_idx = (idx * 2 + bf.b) % ARRAY_SIZE;
            array[alt_idx] = array[idx] ^ reconstructed;
        } else {
            /* When f=0, alternate path */
            mw.ll_data <<= (bf.b & 0x7);
            bf.d = (bf.d + bf.e) & 0xFF;
            
            /* Different addressing mode */
            int alt_idx = (idx + ARRAY_SIZE/2) % ARRAY_SIZE;
            array[alt_idx] = array[idx] | 0x80000000;
        }
        
        /* 12. Branch based on high vs low word comparison */
        if (high_part > low_part) {
            /* Swap operations */
            uint64_t temp_ll = mw.ll_data;
            mw.ll_data = ((temp_ll >> 32) | (temp_ll << 32));
            bf.a = bf.b & 0x7;
        }
        
        /* 13. Switch based on bit-field combination */
        uint8_t switch_val = (bf.a << 5) | (bf.d & 0x1F);
        switch (switch_val & 0x3) {
            case 0:
                mw.fp_data += 1.0;
                array[idx] &= 0xFFFF;
                break;
            case 1:
                mw.fp_data -= 0.5;
                array[idx] |= 0xFFFF0000;
                break;
            case 2:
                mw.fp_data *= 0.75;
                array[idx] ^= 0xAAAAAAAA;
                break;
            case 3:
                mw.fp_data /= 2.0;
                array[idx] = ~array[idx];
                break;
        }
        
        /* Accumulate results */
        result += extracted + low_part + (uint32_t)mw.fp_data + array[idx];
    }
    
    /* Final aggregation to prevent dead code elimination */
    result += manip.raw + (uint32_t)(mw.ll_data >> 32) + (uint32_t)mw.ll_data;
    
    /* Use inline assembly to potentially generate specific patterns */
    __asm__ volatile (
        "/* Potential ZERO_EXTRACT/STRICT_LOW_PART trigger */"
        :
        : "r" (result), "r" (mw.ll_data)
        : "memory"
    );
    
    printf("Result: %u\n", result);
    return (int)(result & 0xFF);
}
