/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function to test ZERO_EXTRACT patterns */
unsigned int test_zero_extract(struct bitfield_struct *bf, unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    /* Pattern 1: Bit-field extraction from structure */
    for (int i = 0; i < size; i++) {
        /* Force bit-field reads that generate ZERO_EXTRACT */
        sum += bf[i].field1;
        sum += bf[i].field2;
        sum += bf[i].field3;
        sum += bf[i].field4;
        
        /* Pattern 2: Explicit bit-field extraction using shift and mask */
        unsigned int val = arr[i];
        sum += (val >> 3) & 0x1F;        /* Extract bits 3-7 */
        sum += (val >> 8) & 0xFF;        /* Extract bits 8-15 */
        sum += (val >> 16) & 0x7FF;      /* Extract bits 16-26 */
        
        /* Pattern 3: Combined mask and shift (common ZERO_EXTRACT pattern) */
        sum += (val & 0xFF00) >> 8;      /* Extract middle byte */
        sum += (val & 0xF0F0) >> 4;      /* Complex extraction pattern */
    }
    
    /* Pattern 4: Bit-field comparison (generates ZERO_EXTRACT in condition) */
    for (int i = 0; i < size; i++) {
        if (bf[i].field1 == (g_volatile_seed & 0x1F)) {
            sum += bf[i].field2;
        }
        if (bf[i].field3 > 500) {
            sum += bf[i].field4;
        }
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Function to test STRICT_LOW_PART patterns */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int size) {
    unsigned int sum = 0;
    
    /* Pattern 1: Partial register updates through char/short assignments */
    for (int i = 0; i < size; i++) {
        /* These assignments may generate STRICT_LOW_PART for partial register writes */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Arithmetic that keeps values in registers, then partial writes back */
        s = (s * 3 + g_volatile_seed) & 0xFFFF;
        c = (c + i) & 0xFF;
        
        /* Store back (potential STRICT_LOW_PART for byte/word stores) */
        short_arr[i] = s;
        char_arr[i] = c;
        
        sum += s + c;
    }
    
    /* Pattern 2: Volatile pointer to short (force memory store of low part) */
    volatile short *vol_short_ptr = (volatile short *)&short_arr[0];
    for (int i = 0; i < size; i++) {
        *vol_short_ptr = (short)(g_volatile_seed + i);
        sum += *vol_short_ptr;
        vol_short_ptr++;
    }
    
    /* Pattern 3: Function with small integer parameters (register partial updates) */
    for (int i = 0; i < size; i++) {
        /* Casting to smaller types in expressions */
        int temp = g_volatile_seed + i * 7;
        short partial = (short)(temp & 0xFFFF);
        
        /* This assignment might use STRICT_LOW_PART */
        short_arr[i % size] = partial;
        sum += partial;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning (generates SUBREG accesses) */
union type_pun {
    uint32_t full;
    uint16_t half[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function to test SUBREG patterns */
unsigned int test_subreg(union type_pun *data, int size) {
    unsigned int sum = 0;
    
    /* Pattern 1: Union-based type punning */
    for (int i = 0; i < size; i++) {
        /* Access different views of the same register/memory */
        sum += data[i].half[0];      /* SUBREG for low 16 bits */
        sum += data[i].half[1];      /* SUBREG for high 16 bits */
        sum += data[i].bytes[1];     /* SUBREG for single byte */
        
        /* Modify through one view, read through another */
        data[i].half[0] = (uint16_t)(sum & 0xFFFF);
        sum += data[i].full;         /* Read full 32-bit value */
    }
    
    /* Pattern 2: Casting between different-sized integers */
    for (int i = 0; i < size; i++) {
        uint32_t val = data[i].full;
        
        /* These casts generate SUBREG operations */
        uint16_t low_part = (uint16_t)val;
        uint16_t high_part = (uint16_t)(val >> 16);
        
        /* Recombine with SUBREG operations */
        data[i].parts.low = low_part + g_volatile_seed;
        data[i].parts.high = high_part - g_volatile_seed;
        
        sum += data[i].full;
    }
    
    /* Pattern 3: SIMD-like operations using unions */
    for (int i = 0; i < size - 1; i++) {
        /* Swap halves between adjacent elements */
        uint16_t temp = data[i].half[1];
        data[i].half[1] = data[i + 1].half[0];
        data[i + 1].half[0] = temp;
        
        sum += data[i].full + data[i + 1].full;
    }
    
    return sum;
}

/* ========== Complex memory address patterns ========== */

/* Function combining all patterns with complex addressing */
unsigned int test_complex_memory(int *base_arr, struct bitfield_struct *bf_arr, 
                                 union type_pun *pun_arr, int size) {
    unsigned int sum = 0;
    
    /* Complex addressing modes that interact with the RTL patterns */
    for (int i = 0; i < size; i++) {
        int idx = (i * g_volatile_seed) % size;
        
        /* Memory reference with index calculation */
        int *ptr = &base_arr[idx];
        
        /* Combine with bit-field extraction from memory */
        sum += (bf_arr[idx].field2 << 3) + *ptr;
        
        /* Type punning with memory access */
        union type_pun *pun_ptr = &pun_arr[idx];
        sum += pun_ptr->half[0] - pun_ptr->half[1];
        
        /* Update memory with partial write */
        *ptr = (short)(sum & 0xFFFF);  /* Potential STRICT_LOW_PART */
        
        /* Another complex address calculation */
        sum += base_arr[(idx + 1) % size] + bf_arr[(idx + 2) % size].field3;
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    const int SIZE = 100;
    unsigned int final_sum = 0;
    
    /* Initialize test data with non-constant values */
    struct bitfield_struct *bf_arr = 
        (struct bitfield_struct *)malloc(SIZE * sizeof(struct bitfield_struct));
    unsigned int *int_arr = (unsigned int *)malloc(SIZE * sizeof(unsigned int));
    short *short_arr = (short *)malloc(SIZE * sizeof(short));
    char *char_arr = (char *)malloc(SIZE * sizeof(char));
    union type_pun *pun_arr = (union type_pun *)malloc(SIZE * sizeof(union type_pun));
    int *base_arr = (int *)malloc(SIZE * sizeof(int));
    
    /* Seed with pseudo-random but volatile-influenced values */
    for (int i = 0; i < SIZE; i++) {
        /* Use volatile seed to prevent constant folding */
        int val = g_volatile_seed + i * 1103515245;
        
        bf_arr[i].field1 = val & 0x1F;
        bf_arr[i].field2 = (val >> 5) & 0x7F;
        bf_arr[i].field3 = (val >> 12) & 0x3FF;
        bf_arr[i].field4 = (val >> 22) & 0x3FF;
        
        int_arr[i] = val;
        short_arr[i] = (short)(val & 0xFFFF);
        char_arr[i] = (char)(val & 0xFF);
        pun_arr[i].full = val;
        base_arr[i] = val;
    }
    
    /* Run all tests to trigger different RTL patterns */
    final_sum += test_zero_extract(bf_arr, int_arr, SIZE);
    final_sum += test_strict_low_part(short_arr, char_arr, SIZE);
    final_sum += test_subreg(pun_arr, SIZE);
    final_sum += test_complex_memory(base_arr, bf_arr, pun_arr, SIZE);
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %u\n", final_sum);
    
    /* Cleanup */
    free(bf_arr);
    free(int_arr);
    free(short_arr);
    free(char_arr);
    free(pun_arr);
    free(base_arr);
    
    return (final_sum > 0) ? 0 : 1;
}
