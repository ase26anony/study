/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function to test ZERO_EXTRACT patterns */
unsigned int test_zero_extract(struct bitfield_struct *bf, unsigned int *arr, int n) {
    unsigned int sum = 0;
    
    /* Pattern 1: Bit-field extraction from structure */
    for (int i = 0; i < n; i++) {
        /* Multiple bit-field accesses that should generate ZERO_EXTRACT */
        sum += bf[i].field1;
        sum += bf[i].field2 << 3;
        sum += (bf[i].field3 & 0x3) * 2;
        
        /* Complex bit-field comparison */
        if (bf[i].field4 > 1000) {
            sum += bf[i].field4 >> 4;
        }
    }
    
    /* Pattern 2: Explicit bit-field extraction from integers */
    for (int i = 0; i < n; i++) {
        /* This should generate ZERO_EXTRACT: (x >> shift) & mask */
        unsigned int val = arr[i];
        unsigned int low_bits = (val >> 0) & 0xFF;        /* Extract byte 0 */
        unsigned int mid_bits = (val >> 8) & 0xFF;        /* Extract byte 1 */
        unsigned int high_bits = (val >> 16) & 0xFF;      /* Extract byte 2 */
        unsigned int top_bits = (val >> 24) & 0xFF;       /* Extract byte 3 */
        
        /* Combine with arithmetic to prevent optimization */
        sum += low_bits * 1;
        sum += mid_bits * 3;
        sum += high_bits * 5;
        sum += top_bits * 7;
        
        /* Variable shift to prevent constant folding */
        int shift = (i & 0x3);  /* 0-3 */
        unsigned int extracted = (val >> shift) & ((1 << (shift + 1)) - 1);
        sum += extracted;
    }
    
    /* Pattern 3: Bit-field in conditional expression */
    for (int i = 0; i < n; i++) {
        /* Comparison of bit-field with constant */
        if (bf[i].field1 == 10) {
            sum += 100;
        }
        if (bf[i].field2 != 0) {
            sum += bf[i].field2;
        }
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Function to test STRICT_LOW_PART patterns */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int n) {
    unsigned int sum = 0;
    
    /* Pattern 1: Partial register updates with char/short */
    for (int i = 0; i < n; i++) {
        /* These assignments should generate STRICT_LOW_PART
           as we're writing partial registers */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Arithmetic that keeps values in registers */
        s = (s * 3 + 7) & 0xFFFF;
        c = (c + i) & 0xFF;
        
        /* Write back partial results */
        short_arr[i] = s;
        char_arr[i] = c;
        
        sum += s + c;
    }
    
    /* Pattern 2: Volatile pointers for partial writes */
    volatile short *vol_short = short_arr;
    volatile char *vol_char = char_arr;
    
    for (int i = 0; i < n; i += 2) {
        /* Volatile writes to partial registers */
        *vol_short = (short)(sum & 0xFFFF);
        *vol_char = (char)(sum & 0xFF);
        
        /* Prevent optimization */
        sum += *vol_short + *vol_char;
        
        vol_short++;
        vol_char++;
    }
    
    /* Pattern 3: Function arguments and returns with small types */
    for (int i = 0; i < n; i++) {
        /* Casting between types of different sizes */
        int temp = short_arr[i];
        temp = (temp * 17 + 23) & 0xFFFFFF;
        
        /* Assignment back to short - partial register update */
        short_arr[i] = (short)temp;
        sum += temp;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning (SUBREG generation) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function to test SUBREG patterns */
unsigned int test_subreg(union type_pun *data, int n) {
    unsigned int sum = 0;
    
    /* Pattern 1: Union-based type punning */
    for (int i = 0; i < n; i++) {
        /* Access different views of the same data - should generate SUBREG */
        data[i].full = i * 0x01020304;
        
        /* Access sub-parts through union members */
        sum += data[i].halves[0];      /* Low 16 bits */
        sum += data[i].halves[1];      /* High 16 bits */
        sum += data[i].bytes[1];       /* Byte 1 */
        sum += data[i].parts.low;      /* Low part through struct */
        sum += data[i].parts.high;     /* High part through struct */
    }
    
    /* Pattern 2: Explicit casting between different integer sizes */
    for (int i = 0; i < n; i++) {
        uint32_t val = data[i].full;
        
        /* Cast to smaller types - should generate SUBREG */
        uint16_t low16 = (uint16_t)(val & 0xFFFF);
        uint16_t high16 = (uint16_t)((val >> 16) & 0xFFFF);
        uint8_t byte0 = (uint8_t)(val & 0xFF);
        uint8_t byte3 = (uint8_t)((val >> 24) & 0xFF);
        
        /* Use all parts to prevent dead code elimination */
        sum += low16;
        sum += high16 * 2;
        sum += byte0 * 3;
        sum += byte3 * 4;
        
        /* Recombine with shifts */
        uint32_t recombined = (high16 << 16) | low16;
        sum += recombined & 0xFF;
    }
    
    /* Pattern 3: Memory accesses with different sized views */
    for (int i = 0; i < n; i++) {
        /* Pointer casting for type-punning */
        uint32_t *as_int = &data[i].full;
        uint16_t *as_short = (uint16_t *)as_int;
        uint8_t *as_byte = (uint8_t *)as_int;
        
        /* Access through different typed pointers */
        sum += *as_int;
        sum += as_short[0] + as_short[1];
        sum += as_byte[0] + as_byte[1] + as_byte[2] + as_byte[3];
    }
    
    return sum;
}

/* ========== Combined patterns with memory references ========== */

/* Complex structure combining multiple patterns */
struct complex_data {
    struct bitfield_struct bf;
    union type_pun pun;
    short short_val;
    char char_val;
    int array[4];
};

/* Function that combines all patterns with complex memory addressing */
unsigned int test_combined(struct complex_data *data, int n) {
    unsigned int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* ZERO_EXTRACT: Bit-field access */
        sum += data[i].bf.field1;
        sum += data[i].bf.field2 << 2;
        
        /* STRICT_LOW_PART: Partial register update */
        data[i].short_val = (short)(sum & 0xFFFF);
        data[i].char_val = (char)(sum & 0xFF);
        
        /* SUBREG: Type-punning through union */
        sum += data[i].pun.halves[0];
        sum += data[i].pun.bytes[2];
        
        /* Complex memory addressing modes */
        for (int j = 0; j < 4; j++) {
            /* Array access with index - creates MEM with address expression */
            sum += data[i].array[j] * (j + 1);
            
            /* Pointer arithmetic */
            int *ptr = &data[i].array[j];
            sum += *(ptr + (j % 2)) >> 1;
        }
        
        /* Mixed operations */
        data[i].bf.field3 = (sum >> 4) & 0x7;  /* ZERO_EXTRACT in store */
        data[i].short_val += data[i].bf.field1; /* STRICT_LOW_PART update */
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    /* Use argc to prevent constant folding */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    /* Allocate test data */
    struct bitfield_struct *bf_array = 
        (struct bitfield_struct *)malloc(n * sizeof(struct bitfield_struct));
    unsigned int *int_array = (unsigned int *)malloc(n * sizeof(unsigned int));
    short *short_array = (short *)malloc(n * sizeof(short));
    char *char_array = (char *)malloc(n);
    union type_pun *pun_array = (union type_pun *)malloc(n * sizeof(union type_pun));
    struct complex_data *complex_array = 
        (struct complex_data *)malloc((n/4) * sizeof(struct complex_data));
    
    /* Initialize with pseudo-random data using volatile seed */
    for (int i = 0; i < n; i++) {
        /* Use volatile to prevent compile-time computation */
        int seed = g_volatile_seed + i;
        
        /* Initialize bitfield structure */
        bf_array[i].field1 = (seed >> 0) & 0x1F;
        bf_array[i].field2 = (seed >> 5) & 0x7F;
        bf_array[i].field3 = (seed >> 12) & 0x7;
        bf_array[i].field4 = (seed * 17) & 0x1FFFF;
        
        /* Initialize integer array */
        int_array[i] = seed * 0x1234567;
        
        /* Initialize short and char arrays */
        short_array[i] = (short)(seed * 3);
        char_array[i] = (char)(seed * 5);
        
        /* Initialize union array */
        pun_array[i].full = seed * 0x89ABCDEF;
        
        /* Initialize complex array (for fewer elements) */
        if (i < n/4) {
            complex_array[i].bf = bf_array[i];
            complex_array[i].pun = pun_array[i];
            complex_array[i].short_val = short_array[i];
            complex_array[i].char_val = char_array[i];
            for (int j = 0; j < 4; j++) {
                complex_array[i].array[j] = seed * (j + 1);
            }
        }
    }
    
    unsigned int total_sum = 0;
    
    /* Run individual pattern tests */
    total_sum += test_zero_extract(bf_array, int_array, n);
    total_sum += test_strict_low_part(short_array, char_array, n);
    total_sum += test_subreg(pun_array, n);
    total_sum += test_combined(complex_array, n/4);
    
    /* Final computation to use all results */
    for (int i = 0; i < n; i++) {
        total_sum += bf_array[i].field1;
        total_sum += short_array[i];
        total_sum += char_array[i];
        total_sum += pun_array[i].halves[0];
        if (i < n/4) {
            total_sum += complex_array[i].short_val;
        }
    }
    
    /* Clean up */
    free(bf_array);
    free(int_array);
    free(short_array);
    free(char_array);
    free(pun_array);
    free(complex_array);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", total_sum);
    
    return (int)(total_sum & 0x7FFFFFFF);
}
