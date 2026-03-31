/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL expressions */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to trigger ZERO_EXTRACT and STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int a : 1;    /* 1-bit field */
    unsigned int b : 3;    /* 3-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 16;   /* 16-bit field */
} __attribute__((packed));

/* Union for accessing bit-fields in different ways */
union bitfield_access {
    struct bitfield_pack fields;
    volatile uint32_t raw;
};

/* Function to force complex addressing modes */
static int complex_index(int idx, int stride, int offset) {
    return idx * stride + offset;
}

int main(int argc, char *argv[]) {
    volatile int i, j, k;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile union bitfield_access bf = { .raw = 0 };
    
    /* Medium-sized volatile array with complex access patterns */
    volatile int arr[256];
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? 100 : 50;
    
    /* Main loop with mixed operations */
    for (i = 0; i < iterations; i++) {
        /* 1. Bit-field manipulations to trigger ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Assignment to bit-field - may generate STRICT_LOW_PART */
        bf.fields.a = (i & 1);
        bf.fields.b = (i & 7);
        bf.fields.c = (i * 3) & 0xFFF;
        
        /* Extract and combine bit-fields with masking/shifting */
        uint32_t extracted = (bf.fields.c << 4) | (bf.fields.b << 1) | bf.fields.a;
        
        /* Complex bit-field operation with masking */
        bf.fields.d = (extracted * 17) & 0xFFFF;
        
        /* 2. Multi-word operations to trigger SUBREG expressions */
        
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var += (long long)(bf.raw);
        ll_var ^= (ll_var << 13) | (ll_var >> 51); /* Rotate */
        
        /* Double operations that may use multiple registers */
        dbl_var = dbl_var * 1.01 + (double)(i & 0xF);
        
        /* 3. Complex array access with addressing modes */
        
        /* Non-trivial indexing calculation */
        int idx = complex_index(i, 7, bf.fields.c & 0x1F);
        idx = idx & 0xFF; /* Ensure within bounds */
        
        /* Read-modify-write with bit manipulation */
        arr[idx] = (arr[idx] ^ (extracted << 8)) + (i & 0xFF);
        
        /* Access with pointer arithmetic and casting */
        volatile uint8_t *byte_ptr = (volatile uint8_t *)&arr[idx];
        byte_ptr[1] ^= bf.fields.b; /* Modify specific byte */
        
        /* 4. Conditional control flow based on bit-field results */
        
        /* Branch based on parity of bit-field */
        if (bf.fields.a) { /* Odd iteration */
            /* Different operations for odd iterations */
            ll_var -= (long long)arr[idx];
            bf.fields.c = (bf.fields.c + 1) & 0xFFF;
        } else { /* Even iteration */
            /* Different operations for even iterations */
            ll_var += (long long)arr[idx];
            bf.fields.c = (bf.fields.c - 1) & 0xFFF;
        }
        
        /* Branch based on comparison of high vs low parts */
        uint32_t ll_low = (uint32_t)(ll_var & 0xFFFFFFFF);
        uint32_t ll_high = (uint32_t)(ll_var >> 32);
        
        if (ll_low > ll_high) {
            /* Swap operations */
            long long temp = ll_var;
            ll_var = ((long long)ll_low << 32) | ll_high;
            dbl_var = -dbl_var;
        }
        
        /* 5. Inline assembly to mimic specific patterns */
        /* This asm operates on low 32 bits of ll_var, potentially creating
           STRICT_LOW_PART-like constraints */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (ll_low)
            : "r" (bf.raw)
            : "cc"
        );
        
        /* Recombine with high part */
        ll_var = ((long long)ll_high << 32) | ll_low;
        
        /* Additional bit-field extraction with masking */
        /* This may generate ZERO_EXTRACT for the mask operation */
        uint32_t masked = bf.raw & 0x00000FFF; /* Extract lower 12 bits */
        bf.fields.c = masked >> 4; /* Shift to fit in c field */
        
        /* Access misaligned data through pointer casting */
        if (i & 2) {
            /* Potentially unaligned access */
            volatile uint16_t *unaligned = (volatile uint16_t *)((uintptr_t)&arr[idx] + 1);
            *unaligned ^= 0x55AA;
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    int result = 0;
    
    /* Sum array elements */
    for (i = 0; i < 256; i++) {
        result += arr[i];
    }
    
    /* Incorporate bit-field values */
    result += bf.fields.a * 1000;
    result += bf.fields.b * 100;
    result += bf.fields.c * 10;
    result += bf.fields.d;
    
    /* Incorporate parts of long long */
    result += (int)(ll_var & 0xFFFFFFFF);
    result += (int)(ll_var >> 32);
    
    printf("Result: %d\n", result);
    return result & 0xFF; /* Return non-zero to indicate execution */
}
