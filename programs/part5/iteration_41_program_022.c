/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x) {
    unsigned int sum = 0;
    /* Multiple bit-field extractions with varying widths */
    sum += (x >> 0) & 0x1F;      /* Extract bits 0-4 */
    sum += (x >> 5) & 0x3F;      /* Extract bits 5-10 */
    sum += (x >> 11) & 0x7FF;    /* Extract bits 11-21 */
    sum += (x >> 22) & 0x3FF;    /* Extract bits 22-31 */
    return sum;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, int count) {
    unsigned int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Bit-field reads generate ZERO_EXTRACT */
        sum += s[i].a;
        sum += s[i].b;
        sum += s[i].c;
        sum += s[i].d;
        
        /* Bit-field writes also generate ZERO_EXTRACT in SET_DEST */
        s[i].a = (sum + i) & 0x7;
        s[i].b = (sum * 2 + i) & 0x1F;
    }
    return sum;
}

/* Complex bit-field operation with memory reference */
unsigned int test_zero_extract_complex(unsigned int *arr, int n) {
    unsigned int result = 0;
    for (int i = 0; i < n; i++) {
        /* Multiple extractions from memory location */
        unsigned int val = arr[i];
        result += (val & 0xFF00) >> 8;      /* High byte */
        result += (val & 0x00FF);           /* Low byte */
        result += (val >> 16) & 0xFFFF;     /* Upper half */
        
        /* Store extracted bits back to memory */
        arr[i] = ((val & 0xF) << 4) | ((val >> 4) & 0xF);
    }
    return result;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* char operations on promoted integers */
        char c = chars[i];
        int temp = c * 2;           /* Promoted to int */
        chars[i] = temp & 0xFF;     /* Only low 8 bits written back - STRICT_LOW_PART */
        sum += chars[i];
        
        /* short operations */
        short s = shorts[i];
        int temp2 = s + g_seed;     /* Promoted to int */
        shorts[i] = temp2 & 0xFFFF; /* Only low 16 bits written back */
        sum += shorts[i];
    }
    return sum;
}

/* Volatile pointer access for STRICT_LOW_PART */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int count) {
    unsigned int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Volatile write of short - often generates STRICT_LOW_PART */
        ptr[i] = (short)(g_seed + i);
        sum += ptr[i];
    }
    return sum;
}

/* Inline assembly for byte register access */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    unsigned char byte1 = 0xAA;
    unsigned char byte2 = 0x55;
    
    /* Assembly that operates on byte registers */
    __asm__ volatile (
        "movb %1, %%al\n\t"
        "xorb %2, %%al\n\t"
        "movb %%al, %0"
        : "=m"(byte1)
        : "m"(byte1), "m"(byte2)
        : "%al"
    );
    
    result = byte1;
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t word;
    uint16_t halfwords[2];
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union type_pun *data, int count) {
    unsigned int sum = 0;
    for (int i = 0; i < count; i++) {
        /* Access different views of the same register */
        sum += data[i].halfwords[0];    /* SUBREG for low 16 bits */
        sum += data[i].halfwords[1];    /* SUBREG for high 16 bits */
        sum += data[i].bytes[2];        /* SUBREG for byte access */
        
        /* Modify through one view, read through another */
        data[i].bytes[1] = (sum + i) & 0xFF;
        sum += data[i].word;            /* Full register access */
    }
    return sum;
}

/* Casting between integer sizes */
unsigned int test_subreg_casts(int *ints, short *shorts, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Cast int to short - generates SUBREG */
        short s = (short)(ints[i] & 0xFFFF);
        sum += s;
        
        /* Cast short to int and back */
        int temp = shorts[i] * 3;
        shorts[i] = (short)(temp & 0xFFFF);  /* SUBREG in store */
        sum += shorts[i];
    }
    return sum;
}

/* SIMD-like operations using unions */
unsigned int test_subreg_simd_like(void) {
    union {
        uint64_t dword;
        uint32_t words[2];
        uint16_t shorts[4];
    } vec;
    
    vec.dword = 0x0123456789ABCDEFULL;
    
    /* Extract and manipulate elements */
    unsigned int sum = 0;
    sum += vec.words[0];          /* Low 32 bits */
    sum += vec.words[1];          /* High 32 bits */
    sum += vec.shorts[1];         /* Middle 16 bits */
    
    /* Modify partial elements */
    vec.shorts[2] = (sum & 0xFFFF);
    sum += vec.dword & 0xFFFFFFFF;
    
    return sum;
}

/* ========== Complex Memory References ========== */

/* Memory references with addressing modes */
unsigned int test_complex_memory(int *base, int index, int offset) {
    unsigned int sum = 0;
    
    /* Various addressing modes that create complex MEM expressions */
    sum += base[index];                   /* Indexed */
    sum += base[index + offset];          /* Indexed with offset */
    sum += *(base + index * 2);           /* Scaled index */
    sum += base[offset & 0x3];            /* Masked index */
    
    /* Combine with bit-field extraction */
    sum += (base[index] >> 4) & 0xF;
    
    return sum;
}

/* ========== Main Test Driver ========== */

int main(int argc, char **argv) {
    unsigned int final_sum = 0;
    
    /* Initialize test data with volatile to prevent optimization */
    volatile int init_seed = g_seed;
    int data_size = 100;
    
    /* Allocate and initialize test arrays */
    unsigned int *int_array = (unsigned int*)malloc(data_size * sizeof(unsigned int));
    struct bitfield_struct *bf_array = (struct bitfield_struct*)malloc(data_size * sizeof(struct bitfield_struct));
    char *char_array = (char*)malloc(data_size * sizeof(char));
    short *short_array = (short*)malloc(data_size * sizeof(short));
    union type_pun *union_array = (union type_pun*)malloc(data_size * sizeof(union type_pun));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < data_size; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0xFFFFFFFF;
        bf_array[i].a = i & 0x7;
        bf_array[i].b = (i >> 3) & 0x1F;
        bf_array[i].c = (i >> 8) & 0xFF;
        bf_array[i].d = (i >> 16) & 0xFFFF;
        char_array[i] = (char)(i & 0xFF);
        short_array[i] = (short)((i * 100) & 0xFFFF);
        union_array[i].word = int_array[i];
    }
    
    /* Run all tests to trigger different RTL patterns */
    final_sum += test_zero_extract_int(int_array[0]);
    final_sum += test_zero_extract_struct(bf_array, data_size / 10);
    final_sum += test_zero_extract_complex(int_array, data_size / 5);
    
    final_sum += test_strict_low_part_chars(char_array, short_array, data_size / 5);
    final_sum += test_strict_low_part_volatile((volatile short*)short_array, data_size / 10);
    final_sum += test_strict_low_part_asm();
    
    final_sum += test_subreg_union(union_array, data_size / 10);
    final_sum += test_subreg_casts(int_array, short_array, data_size / 10);
    final_sum += test_subreg_simd_like();
    
    final_sum += test_complex_memory((int*)int_array, init_seed % 10, 5);
    
    /* Clean up */
    free(int_array);
    free(bf_array);
    free(char_array);
    free(short_array);
    free(union_array);
    
    /* Return final checksum to prevent dead code elimination */
    printf("Final checksum: %u\n", final_sum);
    return (int)(final_sum & 0x7FFFFFFF);
}
