/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int small : 3;
    unsigned int medium : 12;
    unsigned int large : 16;
    unsigned int flag2 : 1;
    unsigned int padding : 31 - (1+3+12+16+1); /* fill to 32 bits */
} __attribute__((packed));

/* Union for type-punning to force complex memory accesses */
union data_union {
    unsigned long long ull;
    struct {
        unsigned int low;
        unsigned int high;
    } parts;
    double dbl;
    unsigned char bytes[8];
};

/* Volatile variables to prevent optimization */
volatile struct bitfield_pack bf_data;
volatile unsigned long long multi_word;
volatile double fp_data;
volatile int array_data[256];
volatile int control = 1;

/* Function with inline assembly to force specific register usage */
static void asm_manipulate_bits(unsigned long long val) {
    unsigned int low, high;
    
    /* Extract low 32 bits - may generate ZERO_EXTRACT */
    low = (unsigned int)(val & 0xFFFFFFFF);
    
    /* Extract high 32 bits - may generate ZERO_EXTRACT with shift */
    high = (unsigned int)(val >> 32);
    
    /* Inline assembly that operates on low part only */
    __asm__ volatile (
        "addl $0x1, %0\n\t"
        "andl $0x7FFFFFFF, %0"
        : "+r" (low)
        : 
        : "cc"
    );
    
    /* Recombine - may generate SUBREG operations */
    multi_word = ((unsigned long long)high << 32) | low;
}

/* Function to perform complex array indexing */
static int complex_array_access(int idx, int stride) {
    volatile int* ptr;
    int result;
    
    /* Complex addressing mode with multiplication */
    ptr = &array_data[idx * stride + (idx & 7)];
    
    /* Read-modify-write with bit manipulation */
    result = *ptr;
    result ^= (result >> 16) | (result << 16);  /* byte swap effect */
    result &= 0x00FF00FF;  /* Mask specific bytes */
    *ptr = result;
    
    return result;
}

int main(int argc, char **argv) {
    int i, j, temp;
    int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    int sum = 0;
    union data_union data_conv;
    
    /* Initialize data */
    bf_data.flag1 = 1;
    bf_data.small = 5;
    bf_data.medium = 2047;
    bf_data.large = 32768;
    bf_data.flag2 = 0;
    
    multi_word = 0x123456789ABCDEF0ULL;
    fp_data = 3.141592653589793;
    
    for (i = 0; i < 256; i++) {
        array_data[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < loop_limit && control; i++) {
        /* 1. Bit-field manipulations (ZERO_EXTRACT/STRICT_LOW_PART) */
        temp = bf_data.medium;
        
        /* Extract and manipulate specific bit ranges */
        unsigned int extracted = (temp >> 4) & 0xFF;  /* ZERO_EXTRACT pattern */
        bf_data.small = extracted & 0x7;  /* STRICT_LOW_PART pattern (3 bits) */
        
        /* Toggle flag based on extracted bits */
        bf_data.flag1 = (extracted ^ (extracted >> 4)) & 1;
        
        /* 2. Multi-word operations (SUBREG generation) */
        multi_word += (unsigned long long)i * 0x10001ULL;
        
        /* Force split operations on 64-bit value */
        if ((multi_word & 0xFFFFFFFF) > (multi_word >> 32)) {
            /* Low word > high word - swap them */
            data_conv.ull = multi_word;
            temp = data_conv.parts.low;
            data_conv.parts.low = data_conv.parts.high;
            data_conv.parts.high = temp;
            multi_word = data_conv.ull;
        }
        
        /* Floating point operation that may use multiple registers */
        fp_data = fp_data * 1.01 - 0.5;
        
        /* Convert double to long long - may generate SUBREG */
        data_conv.dbl = fp_data;
        if (data_conv.ull & 1) {
            multi_word ^= data_conv.ull;
        }
        
        /* 3. Complex memory addressing with bit-field derived index */
        j = (bf_data.small * 3 + bf_data.flag1 * 7) & 0xFF;
        sum += complex_array_access(j, 3);
        
        /* 4. Conditional based on bit-field parity */
        if (bf_data.medium & 1) {
            /* Even more complex when condition is true */
            unsigned long long mask = 0xF0F0F0F0F0F0F0F0ULL;
            multi_word &= ~mask;
            
            /* Access array with stride based on multi_word */
            int stride = (multi_word & 0xF) + 1;
            for (int k = 0; k < 4; k++) {
                sum += array_data[(i * k * stride) & 0xFF];
            }
        } else {
            /* Different path */
            bf_data.large ^= (bf_data.large >> 8);
            
            /* Use inline assembly function */
            asm_manipulate_bits(multi_word);
        }
        
        /* 5. Pointer arithmetic with type punning */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&multi_word;
        for (int k = 0; k < 8; k++) {
            byte_ptr[k] ^= (i + k) & 0xFF;
        }
        
        /* 6. Switch based on bit-field value */
        switch (bf_data.small & 0x7) {
            case 0:
                array_data[i & 0xFF] += sum;
                break;
            case 1:
                array_data[i & 0xFF] -= sum;
                break;
            case 2:
                array_data[i & 0xFF] |= sum;
                break;
            case 3:
                array_data[i & 0xFF] &= sum;
                break;
            default:
                array_data[i & 0xFF] ^= sum;
                break;
        }
        
        /* Prevent loop unrolling */
        if (i % 16 == 0) {
            control = !control;
            control = !control;  /* Toggle twice to keep same value */
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    int final_result = sum;
    final_result += bf_data.small + bf_data.medium + bf_data.large;
    final_result += (int)(multi_word & 0xFFFFFFFF);
    final_result += (int)(multi_word >> 32);
    final_result += (int)fp_data;
    
    /* Use result */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
