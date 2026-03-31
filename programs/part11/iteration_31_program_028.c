/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to potentially generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int a : 1;
    unsigned int b : 3;
    unsigned int c : 12;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int full;
    volatile unsigned char bytes[4];
};

/* Force multi-word operations that may generate SUBREG */
typedef volatile long long vllong;
typedef volatile double vdouble;

/* Complex array with stride access */
#define ARRAY_SIZE 128
#define STRIDE 3

int main(int argc, char *argv[]) {
    volatile int i, j, limit;
    volatile unsigned int temp;
    volatile long long accumulator = 0;
    
    /* Use argc to prevent compile-time optimization */
    limit = (argc > 1) ? 100 : 50;
    
    /* Bit-field structure */
    union mixed_access data;
    data.full = 0x12345678;
    
    /* Multi-word variables */
    vllong big_val = 0x1122334455667788LL;
    vdouble fp_val = 3.141592653589793;
    
    /* Array with complex indexing */
    volatile unsigned int arr[ARRAY_SIZE];
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * 7;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations that may generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Extract bit-field 'c' (12 bits) - potential ZERO_EXTRACT */
        temp = data.bits.c;
        
        /* Modify bit-field 'b' (3 bits) - potential STRICT_LOW_PART */
        data.bits.b = (data.bits.b + 1) & 0x7;
        
        /* Cross-bit-field operation */
        data.bits.a = data.bits.c & 1;  /* Extract LSB of 12-bit field */
        
        /* Manual bit extraction using shift/mask */
        unsigned int extracted = (data.full >> 4) & 0xFFF;  /* Another ZERO_EXTRACT candidate */
        
        /* 2. Multi-word operations that may generate SUBREG */
        
        /* Operations on long long (multi-register on 32-bit) */
        big_val = big_val + 0x10001LL;
        
        /* Access low and high parts separately */
        unsigned int low_part = (unsigned int)(big_val & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(big_val >> 32);
        
        /* Recombine with bitwise operations */
        big_val = ((vllong)high_part << 32) | low_part;
        
        /* Floating point operation (also multi-word) */
        fp_val = fp_val * 1.01;
        
        /* 3. Complex memory addressing with bit-field derived index */
        
        /* Array access with stride and bit-field dependent offset */
        j = (i * STRIDE + data.bits.c) % ARRAY_SIZE;
        
        /* Read-modify-write with bit manipulation */
        arr[j] = (arr[j] ^ extracted) + temp;
        
        /* Misaligned access simulation via pointer arithmetic */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&arr[j];
        unsigned int word_from_bytes = 
            (byte_ptr[0] << 24) | 
            (byte_ptr[1] << 16) | 
            (byte_ptr[2] << 8) | 
            byte_ptr[3];
        
        /* 4. Control flow based on bit-field and multi-word comparisons */
        
        if (data.bits.a) {
            /* When bit 'a' is set */
            if (low_part > high_part) {
                /* Swap low and high parts of big_val */
                big_val = ((vllong)low_part << 32) | high_part;
            }
            /* Access array with different pattern */
            arr[(j + 1) % ARRAY_SIZE] = word_from_bytes;
        } else {
            /* When bit 'a' is clear */
            if ((data.bits.c & 0x800) != 0) {  /* Check bit 11 of field c */
                /* Invert bits in array element */
                arr[j] = ~arr[j];
            }
        }
        
        /* Switch based on bit-field 'b' value */
        switch (data.bits.b) {
            case 0:
                big_val = big_val << 1;
                break;
            case 1:
                big_val = big_val >> 1;
                break;
            case 2:
                big_val = big_val ^ 0xAAAAAAAAAAAAAAAALL;
                break;
            case 3:
                big_val = big_val | 0x5555555555555555LL;
                break;
            case 4:
                big_val = big_val & 0x3333333333333333LL;
                break;
            case 5:
                big_val = big_val + big_val;
                break;
            case 6:
                big_val = big_val - 0x100000001LL;
                break;
            case 7:
                big_val = ~big_val;
                break;
        }
        
        /* Accumulate results */
        accumulator += arr[j] + low_part + high_part + temp;
    }
    
    /* Final computation to prevent dead code elimination */
    unsigned int result = 
        (data.bits.a << 0) |
        (data.bits.b << 1) |
        (data.bits.c << 4) |
        ((unsigned int)(accumulator & 0xFFFFFFFF) << 16);
    
    /* Mix in array checksum */
    unsigned int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i += 2) {
        checksum ^= arr[i];
    }
    result ^= checksum;
    
    /* Use result to affect return value */
    return (result & 0xFF) + argc;
}
