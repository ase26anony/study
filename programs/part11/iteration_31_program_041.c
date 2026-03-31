/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    unsigned int a : 3;    /* 3-bit field */
    unsigned int b : 12;   /* 12-bit field */
    unsigned int c : 1;    /* 1-bit field */
    unsigned int d : 8;    /* 8-bit field */
    unsigned int e : 8;    /* 8-bit field */
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_struct bits;
    unsigned int word;
    unsigned short halves[2];
    unsigned char bytes[4];
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Function with inline assembly to mimic STRICT_LOW_PART behavior */
static unsigned long long asm_low_part(unsigned long long x) {
    unsigned int low, high;
    
    /* Split 64-bit into high/low parts */
    high = (unsigned int)(x >> 32);
    low = (unsigned int)(x & 0xFFFFFFFF);
    
    /* Inline asm that operates on low 32 bits */
    asm volatile (
        "addl $1, %0\n\t"
        "movl %0, %1"
        : "+r" (low), "=r" (high)
        : "0" (low), "1" (high)
        : "cc"
    );
    
    return ((unsigned long long)high << 32) | low;
}

int main(int argc, char **argv) {
    volatile struct bitfield_struct bf = {0};
    volatile union mixed_access ma = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[256];
    volatile int *ptr_array = array;
    int i, j, result = 0;
    volatile int loop_limit;
    
    /* Use argc to prevent constant propagation */
    loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    /* Initialize array with pattern */
    for (i = 0; i < 256; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.a = (i & 0x7);                     /* 3-bit field */
        bf.b = (i * 7) & 0xFFF;               /* 12-bit field */
        bf.c = (i & 0x1);                     /* 1-bit field */
        
        /* Extract bit-field values using masking (may generate ZERO_EXTRACT) */
        unsigned int extracted = ((unsigned int)bf.b << 3) | bf.a;
        
        /* Access through union with type punning */
        ma.bits = bf;
        unsigned int masked = ma.word & 0x7FF;  /* Mask lower 11 bits */
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        /* Operations on long long (64-bit) on 32-bit targets */
        ll_var += (long long)extracted;
        ll_var ^= (long long)masked << 16;
        
        /* Double operations that may use multiple registers */
        dbl_var += (double)(i & 0xF) * 0.1;
        dbl_var = dbl_var * 0.99;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i & 0x3F, 4, (extracted & 0x1F));
        if (idx >= 0 && idx < 256) {
            /* Read-modify-write with bit manipulation */
            int old_val = array[idx];
            
            /* Create pattern that might generate SUBREG on MEM */
            array[idx] = (old_val & 0xFFFF0000) | 
                        ((old_val & 0xFFFF) + (extracted & 0xFFFF));
            
            /* Another access with pointer arithmetic */
            ptr_array[idx] ^= (masked << 8);
        }
        
        /* 4. Conditional based on bit-field parity and long long comparison */
        if (bf.c) {  /* Odd iteration */
            /* Check high vs low word of long long */
            unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
            unsigned int high_part = (unsigned int)(ll_var >> 32);
            
            if (high_part > low_part) {
                /* More bit-field manipulation */
                bf.d = (high_part - low_part) & 0xFF;
                bf.e = (low_part + i) & 0xFF;
                
                /* Use inline assembly for low-part operation */
                ll_var = asm_low_part(ll_var);
            } else {
                /* Different bit-field operation */
                bf.d = (low_part >> 8) & 0xFF;
                bf.e = (high_part << 1) & 0xFF;
                
                /* Force memory access with complex address */
                int alt_idx = ((i * 3) + (bf.d * 2)) & 0xFF;
                if (alt_idx >= 0 && alt_idx < 256) {
                    array[alt_idx] += bf.e;
                }
            }
        } else {  /* Even iteration */
            /* Mix double and integer operations */
            int int_from_dbl = (int)dbl_var;
            bf.d = (int_from_dbl ^ extracted) & 0xFF;
            bf.e = (int_from_dbl >> 4) & 0xFF;
            
            /* Another complex array access pattern */
            for (j = 0; j < 4; j++) {
                int nested_idx = (idx + j * 16) & 0xFF;
                if (nested_idx >= 0 && nested_idx < 256) {
                    array[nested_idx] -= bf.d + (j * bf.e);
                }
            }
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r" (i) : : "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i += 8) {
        result += array[i];
    }
    
    result += (int)(ll_var & 0xFFFFFFFF);
    result += (int)(ll_var >> 32);
    result += (int)dbl_var;
    result += bf.a + bf.b + bf.c + bf.d + bf.e;
    
    /* Use result to affect return value */
    return (result & 0xFF) + (argc > 0 ? 0 : 1);
}

/* Additional function to create more RTL patterns */
static void extra_patterns(void) {
    volatile struct {
        unsigned int x : 5;
        unsigned int y : 10;
        unsigned int z : 17;
    } s1 = {0}, s2 = {0};
    
    volatile long long ll1, ll2;
    volatile int arr[64];
    
    /* Pattern 1: Multiple bit-field assignments */
    for (int i = 0; i < 32; i++) {
        s1.x = i & 0x1F;
        s1.y = (i * 3) & 0x3FF;
        s1.z = (i * 100) & 0x1FFFF;
        
        /* Copy with masking */
        s2.x = s1.x;
        s2.y = s1.y & 0x1FF;  /* Different mask */
        s2.z = s1.z | 0x1000;
        
        /* Use in array index */
        int idx = (s1.x + s2.y) & 0x3F;
        arr[idx] = s1.z + s2.z;
    }
    
    /* Pattern 2: Long long operations that may split */
    ll1 = 0xFEDCBA9876543210LL;
    for (int i = 0; i < 16; i++) {
        ll2 = ll1 >> (i * 2);
        ll1 = ll1 + ll2;
        ll1 = ll1 ^ (ll2 << 8);
        
        /* Access different halves */
        unsigned short *half_ptr = (unsigned short*)&ll1;
        for (int j = 0; j < 4; j++) {
            half_ptr[j] += i + j;
        }
    }
}
