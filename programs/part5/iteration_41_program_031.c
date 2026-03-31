/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_seed = 42;

/* ===== ZERO_EXTRACT patterns ===== */

/* Bit-field structure for ZERO_EXTRACT */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple ZERO_EXTRACT patterns */
    unsigned int mask = (1 << 5) - 1;  /* Not constant folded */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift */
    result += (x >> shift) & mask;
    
    /* Pattern 2: Multiple extractions */
    result += (x & 0xFF00) >> 8;
    result += (x & 0x00FF0000) >> 16;
    
    /* Pattern 3: Variable width extraction */
    unsigned int width = (g_seed % 8) + 1;
    unsigned int var_mask = (1 << width) - 1;
    result += (x >> (width * 2)) & var_mask;
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Pattern 1: Bit-field assignment */
    s->field1 = val & 0x1F;
    
    /* Pattern 2: Bit-field comparison */
    if (s->field2 == (val & 0x7F)) {
        result += s->field2;
    }
    
    /* Pattern 3: Bit-field arithmetic */
    result += s->field3 * 2;
    result += s->field4 >> 4;
    
    /* Pattern 4: Nested bit-field operations */
    s->field1 = (s->field2 + s->field3) & 0x1F;
    
    return result;
}

/* ===== STRICT_LOW_PART patterns ===== */

/* Function 3: Partial register updates */
unsigned int test_strict_low_part(short *arr, int size, char *chars) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern 1: char assignment in loop (partial register update) */
        char c = chars[i];
        result += c;
        
        /* Pattern 2: short assignment with arithmetic */
        short s = arr[i];
        s = (s * 2) & 0x7FFF;  /* Force partial update */
        result += s;
        
        /* Pattern 3: Pointer to volatile short */
        volatile short *vs = &arr[i];
        *vs = s + 1;  /* Generates STRICT_LOW_PART store */
        
        /* Pattern 4: Mixed-size operations */
        int temp = s;
        temp = temp * 3 + c;
        arr[i] = (short)(temp & 0xFFFF);  /* Partial write back */
    }
    
    return result;
}

/* Function 4: Inline assembly for partial updates */
unsigned int test_strict_low_part_asm(int x) {
    unsigned int result = x;
    
    /* Pattern 1: Byte register operation */
    unsigned char b1, b2;
    b1 = (x >> 8) & 0xFF;
    b2 = x & 0xFF;
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    asm volatile (
        "addb %1, %0\n\t"
        : "+r"(b1)
        : "r"(b2)
        : "cc"
    );
    
    result = (result & 0xFFFFFF00) | b1;
    
    /* Pattern 2: 16-bit operation */
    unsigned short s1 = x & 0xFFFF;
    unsigned short s2 = (x >> 16) & 0xFFFF;
    
    asm volatile (
        "addw %1, %0\n\t"
        : "+r"(s1)
        : "r"(s2)
        : "cc"
    );
    
    result = (result << 16) | s1;
    return result;
}

/* ===== SUBREG patterns ===== */

/* Function 5: Union for type-punning */
unsigned int test_subreg_union(unsigned int x) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t b[4];
    } u;
    
    u.i = x;
    
    /* Pattern 1: Access sub-parts through union */
    unsigned int result = 0;
    result += u.s[0];  /* SUBREG for half-word access */
    result += u.s[1];
    result += u.b[0] << 8;
    result += u.b[3];
    
    /* Pattern 2: Modify sub-parts */
    u.s[1] = (u.s[0] + u.s[1]) & 0xFFFF;
    u.b[2] = u.b[1] ^ 0xAA;
    
    return u.i + result;
}

/* Function 6: Casting between types */
unsigned int test_subreg_casts(int *arr, int size) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pattern 1: Cast to smaller type */
        short s = (short)(arr[i] & 0xFFFF);
        
        /* Pattern 2: Cast back with extension */
        int extended = (int)s * 2;
        
        /* Pattern 3: Memory access with different types */
        char *byte_ptr = (char *)&arr[i];
        for (int j = 0; j < 4; j++) {
            result += byte_ptr[j] << (j * 2);
        }
        
        /* Pattern 4: Mixed operations */
        result += extended + (arr[i] >> 16);
    }
    
    return result;
}

/* ===== Complex memory references ===== */

/* Function 7: Memory references with addressing modes */
unsigned int test_complex_memory(int *base, int index1, int index2) {
    unsigned int result = 0;
    
    /* Pattern 1: Array indexing with computation */
    result += base[index1 * 2 + 3];
    result += base[index2 % 16];
    
    /* Pattern 2: Pointer arithmetic */
    int *ptr = base + index1;
    for (int i = 0; i < 4; i++) {
        result += ptr[i] * i;
    }
    
    /* Pattern 3: Structure with bit-fields in array */
    struct BitFieldStruct bfs[4];
    for (int i = 0; i < 4; i++) {
        bfs[i].field1 = (base[i] >> i) & 0x1F;
        bfs[i].field2 = (base[i + 1] * 3) & 0x7F;
        result += bfs[i].field1 + bfs[i].field2;
    }
    
    return result;
}

/* ===== Main test driver ===== */

int main(int argc, char **argv) {
    /* Initialize test data with some randomness */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : g_seed;
    srand(seed);
    
    const int ARRAY_SIZE = 64;
    
    /* Test data arrays */
    int int_array[ARRAY_SIZE];
    short short_array[ARRAY_SIZE];
    char char_array[ARRAY_SIZE];
    struct BitFieldStruct bfs;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() ^ (rand() << 16);
        short_array[i] = (short)(rand() & 0xFFFF);
        char_array[i] = (char)(rand() & 0xFF);
    }
    
    /* Initialize bit-field structure */
    bfs.field1 = rand() & 0x1F;
    bfs.field2 = rand() & 0x7F;
    bfs.field3 = rand() & 0x07;
    bfs.field4 = rand() & 0x1FFFF;
    
    unsigned int checksum = 0;
    
    /* Test ZERO_EXTRACT patterns */
    checksum += test_zero_extract_int(int_array[0], 3);
    checksum += test_zero_extract_struct(&bfs, rand());
    
    /* Test STRICT_LOW_PART patterns */
    checksum += test_strict_low_part(short_array, ARRAY_SIZE / 2, char_array);
    checksum += test_strict_low_part_asm(int_array[1]);
    
    /* Test SUBREG patterns */
    checksum += test_subreg_union(int_array[2]);
    checksum += test_subreg_casts(int_array, 8);
    
    /* Test complex memory references */
    checksum += test_complex_memory(int_array, 4, 8);
    
    /* Additional mixed pattern tests */
    for (int i = 0; i < 16; i++) {
        /* Mix different patterns in loop */
        unsigned int temp = int_array[i];
        
        /* ZERO_EXTRACT */
        temp = (temp >> (i % 8)) & ((1 << 5) - 1);
        
        /* STRICT_LOW_PART simulation */
        short_array[i] = (short)(temp & 0x7FFF);
        
        /* SUBREG through union */
        union { int i; short s[2]; } u;
        u.i = int_array[i];
        temp += u.s[0] + u.s[1];
        
        /* Complex memory reference */
        temp += int_array[(i * 3) % ARRAY_SIZE];
        
        checksum += temp;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Resource patterns checksum: %u\n", checksum);
    
    /* Return non-zero if checksum looks reasonable */
    return (checksum > 1000) ? 0 : 1;
}
