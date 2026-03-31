/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_struct {
    volatile unsigned int a : 1;    /* 1-bit field */
    volatile unsigned int b : 3;    /* 3-bit field */
    volatile unsigned int c : 12;   /* 12-bit field */
    volatile unsigned int d : 16;   /* 16-bit field */
    volatile unsigned int pad : 0;  /* Force alignment */
};

/* Union for bit-twiddling operations */
union bit_twiddler {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* Force generation of SUBREG for multi-word operations */
typedef volatile struct {
    volatile long long ll_val;
    volatile double dbl_val;
} multiword_t;

/* Complex array with stride access */
#define ARRAY_SIZE 256
#define STRIDE 7

int main(int argc, char *argv[]) {
    volatile struct bitfield_struct bf = {0, 0, 0, 0};
    volatile union bit_twiddler twiddle = {0};
    volatile multiword_t mw = {0LL, 0.0};
    volatile int array[ARRAY_SIZE];
    volatile int i, j, temp;
    volatile int limit = (argc > 1) ? 100 : 50; /* Data-dependent limit */
    int result = 0;
    
    /* Initialize array with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with complex operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations (potential ZERO_EXTRACT/STRICT_LOW_PART) */
        bf.a = (i & 1);                     /* Single bit assignment */
        bf.b = (i & 0x7);                   /* 3-bit field */
        bf.c = (i * 3) & 0xFFF;             /* 12-bit field */
        
        /* Extract and combine bit-fields using masks */
        temp = (bf.c << 4) | (bf.b << 1) | bf.a;
        
        /* Manipulate specific bits using bitwise operations */
        twiddle.full = (uint32_t)temp;
        twiddle.parts.low = twiddle.parts.low ^ 0x55AA;  /* Modify low 16 bits */
        twiddle.parts.high = twiddle.parts.high & 0x00FF; /* Clear high 8 bits */
        
        /* 2. Multi-word operations (potential SUBREG generation) */
        mw.ll_val += (long long)twiddle.full;
        mw.ll_val = mw.ll_val << 2;         /* Shift entire long long */
        
        /* Force split operations on 32-bit targets */
        if (mw.ll_val & 0x80000000LL) {
            mw.ll_val = ~mw.ll_val;         /* Bitwise NOT on multi-word */
        }
        
        /* Double precision operations */
        mw.dbl_val += (double)(i & 0xF) * 0.1;
        mw.dbl_val = mw.dbl_val * 1.01;
        
        /* 3. Complex array access with stride (potential MEM with complex address) */
        j = (temp * STRIDE + i) % ARRAY_SIZE;
        
        /* Read-modify-write with bit manipulation */
        array[j] = (array[j] & 0xFFFF0000) | (temp & 0xFFFF);
        
        /* Access with pointer arithmetic and casting */
        volatile uint16_t *ptr = (volatile uint16_t *)((char *)array + j * sizeof(int) + 1);
        *ptr = (uint16_t)(twiddle.parts.low & 0xFF);
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf.a) {  /* Branch based on 1-bit field */
            /* Operations when LSB is 1 */
            mw.ll_val += 0x100000000LL;  /* Add to high part */
            array[j] |= 0x80000000;      /* Set high bit */
        } else {
            /* Operations when LSB is 0 */
            mw.ll_val -= 0x100000000LL;  /* Subtract from high part */
            array[j] &= 0x7FFFFFFF;      /* Clear high bit */
        }
        
        /* Compare high vs low parts of long long */
        if (((uint32_t)(mw.ll_val >> 32)) > ((uint32_t)mw.ll_val)) {
            /* Swap high and low words using inline asm hint */
            asm volatile("" : "+r" (mw.ll_val) : : "memory");
            temp = array[(j + 1) % ARRAY_SIZE];
            array[(j + 1) % ARRAY_SIZE] = array[j];
            array[j] = temp;
        }
        
        /* Additional bit-field extraction with masking */
        bf.d = (twiddle.full >> 8) & 0xFFFF;  /* Extract middle 16 bits */
        
        /* Force strict low-part operation */
        twiddle.parts.low = (twiddle.parts.low & 0xFF) | (bf.d & 0xFF00);
        
        /* Complex condition with bit-field test */
        switch (bf.b) {  /* Switch on 3-bit field */
            case 0:
                mw.dbl_val *= 0.5;
                break;
            case 1:
            case 2:
                mw.dbl_val *= 1.5;
                break;
            case 3:
            case 4:
                mw.dbl_val *= 2.0;
                break;
            default:
                mw.dbl_val *= 0.75;
                break;
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        result += array[i];
    }
    
    result += (int)bf.c;
    result += (int)bf.d;
    result += (int)(mw.ll_val & 0xFFFFFFFF);
    result += (int)(mw.ll_val >> 32);
    result += (int)(mw.dbl_val * 1000);
    
    /* Use result to affect return value */
    return result % 256;
}
