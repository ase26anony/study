/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad   : 16;
} __attribute__((packed));

/* Union to force specific bit manipulations */
union mixed_access {
    unsigned int full;
    struct {
        unsigned short low;
        unsigned short high;
    } parts;
    struct {
        unsigned int bits0_7 : 8;
        unsigned int bits8_15 : 8;
        unsigned int bits16_23 : 8;
        unsigned int bits24_31 : 8;
    } bytes;
};

/* Complex structure with nested bit-fields */
struct nested_bitfields {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 8;
    } inner;
    unsigned int d : 16;
};

/* Function to create complex addressing patterns */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to force specific register usage */
static void asm_bit_manipulation(volatile unsigned long long *val) {
    /* Inline asm that operates on low 32 bits */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFF, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (*val)
        : "r" (*val)
        : "%eax"
    );
}

int main(int argc, char **argv) {
    volatile int i, j, limit;
    volatile unsigned int temp;
    volatile long long result = 0;
    
    /* Use argc to create data-dependent control flow */
    limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit <= 0) limit = 100;
    
    /* Volatile structures to prevent optimization */
    volatile struct bitfield_pack bf = {0, 0, 0, 0};
    volatile union mixed_access ma = {0};
    volatile struct nested_bitfields nb = {{0, 0, 0}, 0};
    
    /* Multi-word types for SUBREG generation */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with complex access patterns */
    volatile int array[256];
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.flag1 = i & 1;
        bf.flag2 = (i >> 1) & 0x7;
        bf.value = (bf.value + i) & 0xFFF;
        
        /* Extract using bit operations */
        temp = ((unsigned int)bf.flag1 << 16) | 
               ((unsigned int)bf.flag2 << 12) | 
               bf.value;
        
        /* 2. Union-based bit manipulation */
        ma.full = temp;
        ma.parts.low = ma.parts.low ^ ma.parts.high;
        ma.bytes.bits0_7 = (ma.bytes.bits0_7 + 1) & 0xFF;
        
        /* 3. Nested bit-field operations */
        nb.inner.a = (nb.inner.a + 1) & 0xF;
        nb.inner.b = (nb.inner.b ^ nb.inner.a) & 0xF;
        nb.inner.c = (nb.inner.c + nb.inner.b) & 0xFF;
        nb.d = (nb.d << 1) | (nb.d >> 15);
        
        /* 4. Multi-word operations (potential SUBREG) */
        ll_var = ll_var + 0x100000001LL;
        dbl_var = dbl_var * 1.01;
        
        /* Force split operations on 32-bit targets */
        if ((ll_var >> 32) != (ll_var & 0xFFFFFFFF)) {
            /* Operations that might generate SUBREG */
            unsigned int low = ll_var & 0xFFFFFFFF;
            unsigned int high = ll_var >> 32;
            ll_var = ((long long)(high ^ 0x55555555) << 32) | (low ^ 0xAAAAAAAA);
        }
        
        /* 5. Complex array access with bit-derived index */
        j = complex_index(i, bf.value & 0x1F, ma.bytes.bits0_7);
        if (j >= 0 && j < 256) {
            /* Read-modify-write with bit masking */
            array[j] = (array[j] & 0xFFFF0000) | 
                      ((array[j] + temp) & 0xFFFF);
            
            /* Access with byte offset (potential misaligned access) */
            if ((j & 1) == 0) {
                char *byte_ptr = (char *)&array[j];
                byte_ptr[1] = byte_ptr[0] ^ byte_ptr[2];
            }
        }
        
        /* 6. Conditional control flow based on bit operations */
        if (bf.flag1) {
            /* Branch 1: More bit manipulations */
            result += (ll_var & 0xF) + (dbl_var > 0 ? 1 : 0);
            
            /* Potential STRICT_LOW_PART for partial register updates */
            ma.parts.low = (ma.parts.low + array[i & 0xFF]) & 0xFFFF;
        } else {
            /* Branch 2: Different operations */
            result -= (ll_var >> 32) & 0xF;
            
            /* Use inline assembly for explicit low-part manipulation */
            asm_bit_manipulation(&ll_var);
        }
        
        /* 7. Switch based on bit-field value */
        switch (bf.flag2) {
            case 0:
                array[(i * 7) & 0xFF] = result & 0xFF;
                break;
            case 1:
            case 2:
                nb.d = (nb.d + array[(i * 3) & 0xFF]) & 0xFFFF;
                break;
            case 3:
            case 4:
                /* Mixed-size operations */
                ll_var = (ll_var << 4) | (bf.value & 0xF);
                break;
            default:
                dbl_var = dbl_var / 1.1;
                break;
        }
        
        /* 8. Pointer arithmetic with type punning */
        {
            volatile unsigned int *ptr = &array[0] + (i & 0x3F);
            volatile unsigned short *short_ptr = (volatile unsigned short *)ptr;
            
            /* Access through different type views */
            *short_ptr = (*short_ptr + i) & 0xFFFF;
            *(short_ptr + 1) = (*(short_ptr + 1) ^ temp) & 0xFFFF;
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    unsigned int final_result = 0;
    final_result += bf.value;
    final_result += ma.full;
    final_result += nb.d;
    final_result += result & 0xFFFFFFFF;
    final_result += (unsigned int)dbl_var;
    
    for (i = 0; i < 256; i += 8) {
        final_result ^= array[i];
    }
    
    printf("Result: %u\n", final_result);
    return final_result & 0xFF;
}
