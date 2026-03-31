/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;    /* 3-bit field */
    unsigned int b : 12;   /* 12-bit field */
    unsigned int c : 1;    /* 1-bit field */
    unsigned int d : 8;    /* 8-bit field */
    unsigned int e : 7;    /* 7-bit field - odd size to force masking */
    unsigned int f : 1;    /* 1-bit field - total 32 bits */
};

/* Union to force reinterpretation of bits */
union bit_union {
    struct bitfield_struct bits;
    unsigned int full;
    volatile unsigned int vfull;
};

/* Force generation of SUBREG for multi-word operations */
typedef struct {
    volatile long long ll_part;
    volatile double dbl_part;
} multiword_t;

/* Complex array access patterns */
#define ARRAY_SIZE 128
typedef struct {
    volatile int data[ARRAY_SIZE];
    volatile int stride;
} complex_array_t;

/* Inline assembly to force specific register constraints */
static inline unsigned int asm_low_part(unsigned long long val) {
    unsigned int low;
    /* Force operation on low 32 bits only */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (low)
        : "r" ((unsigned int)val)
        : "cc"
    );
    return low;
}

static inline unsigned int asm_high_part(unsigned long long val) {
    unsigned int high;
    /* Force operation on high 32 bits only */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "xorl $0xABCD, %0"
        : "=r" (high)
        : "r" ((unsigned int)(val >> 32))
        : "cc"
    );
    return high;
}

int main(int argc, char *argv[]) {
    volatile int i, j, k;
    volatile unsigned int temp;
    volatile long long accumulator = 0;
    int result = 0;
    
    /* Control loop with volatile to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    /* Initialize structures */
    union bit_union bu;
    bu.full = 0x12345678;
    
    multiword_t mw;
    mw.ll_part = 0xFEDCBA9876543210LL;
    mw.dbl_part = 3.141592653589793;
    
    complex_array_t ca;
    ca.stride = 7;  /* Non-power-of-two stride */
    for (i = 0; i < ARRAY_SIZE; i++) {
        ca.data[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit; i++) {
        /* ===== BIT-FIELD OPERATIONS ===== */
        /* These should generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Extract and manipulate bit-fields */
        temp = bu.bits.b;  /* 12-bit extract */
        bu.bits.c = temp & 1;  /* 1-bit assignment - may be STRICT_LOW_PART */
        
        /* Combined bit-field operations */
        bu.bits.a = (bu.bits.d >> 3) & 0x7;  /* 3-bit from 8-bit */
        bu.bits.e = (bu.bits.b + bu.bits.a) & 0x7F;  /* 7-bit with masking */
        
        /* Manual bit extraction using shifts (alternative to bit-fields) */
        unsigned int manual_extract = (bu.full >> 5) & 0xFFF;  /* 12-bit extract */
        bu.bits.f = (manual_extract ^ bu.bits.c) & 1;
        
        /* ===== MULTI-WORD OPERATIONS ===== */
        /* These should generate SUBREG expressions */
        
        /* Operations on long long forcing split */
        mw.ll_part += (long long)bu.full;
        mw.ll_part ^= (long long)i << 32;  /* Mix high and low parts */
        
        /* Compare high vs low parts - forces SUBREG usage */
        long long ll_temp = mw.ll_part;
        unsigned int low_word = (unsigned int)ll_temp;
        unsigned int high_word = (unsigned int)(ll_temp >> 32);
        
        /* Double precision operations */
        mw.dbl_part += (double)low_word / 1000.0;
        mw.dbl_part -= (double)high_word / 10000.0;
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        /* Generate MEM with complex address expressions */
        
        /* Non-linear array access */
        j = (i * ca.stride + bu.bits.a) % ARRAY_SIZE;
        k = ((i + 1) * ca.stride - bu.bits.b) % ARRAY_SIZE;
        
        /* Read-modify-write with bit manipulation */
        int old_val = ca.data[j];
        /* Extract bits 4-11 from old_val (8 bits) */
        int extracted = (old_val >> 4) & 0xFF;
        /* Combine with bit-field */
        int new_val = (extracted << 8) | bu.bits.d;
        ca.data[j] = new_val;
        
        /* Another complex access pattern */
        int idx = (j * 3 + k * 5) % ARRAY_SIZE;
        ca.data[idx] ^= (bu.full >> 16) & 0xFFFF;
        
        /* ===== CONTROL FLOW BASED ON BIT OPERATIONS ===== */
        /* Force scheduling decisions */
        
        if (bu.bits.c) {  /* Branch on 1-bit field */
            /* Path 1: More bit-field manipulations */
            bu.bits.a = (bu.bits.a + 1) & 0x7;
            mw.ll_part >>= 1;
            
            /* Use inline assembly for explicit low-part operations */
            temp = asm_low_part(mw.ll_part);
            bu.bits.d = temp & 0xFF;
        } else {
            /* Path 2: Different operations */
            bu.bits.b = (bu.bits.b * 3) & 0xFFF;
            mw.ll_part <<= 2;
            
            temp = asm_high_part(mw.ll_part);
            bu.bits.e = temp & 0x7F;
        }
        
        /* Switch based on multiple bit conditions */
        unsigned int switch_val = (bu.bits.a << 2) | bu.bits.c;
        switch (switch_val & 0x7) {
            case 0:
                ca.data[i % ARRAY_SIZE] += low_word;
                break;
            case 1:
                ca.data[i % ARRAY_SIZE] -= high_word;
                break;
            case 2:
                ca.data[i % ARRAY_SIZE] ^= bu.bits.b;
                break;
            case 3:
                ca.data[i % ARRAY_SIZE] |= bu.bits.d;
                break;
            case 4:
                ca.data[i % ARRAY_SIZE] &= ~bu.bits.e;
                break;
            default:
                ca.data[i % ARRAY_SIZE] = bu.full;
                break;
        }
        
        /* Accumulate for result */
        accumulator += (long long)ca.data[i % ARRAY_SIZE];
        accumulator += (long long)bu.full;
    }
    
    /* Final computation to prevent dead code elimination */
    result = (int)(accumulator & 0xFFFFFFFF);
    result ^= (int)(accumulator >> 32);
    result += bu.bits.a + bu.bits.b + bu.bits.c + bu.bits.d + bu.bits.e + bu.bits.f;
    
    /* Use array elements */
    for (i = 0; i < 10; i++) {
        result += ca.data[i * ca.stride % ARRAY_SIZE];
    }
    
    /* Print to prevent optimization */
    printf("Result: %d (Loop limit: %d)\n", result, loop_limit);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
