/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile int volatile_mask = 0xFF;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int generate_zero_extract(void) {
    /* Use union with bitfields to create ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t middle: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    /* Volatile assignment prevents constant folding */
    u.full = volatile_seed;
    
    /* Extract middle bits - should generate ZERO_EXTRACT */
    uint32_t extracted = u.bits.middle;
    
    /* Use the result to prevent dead code elimination */
    use_int(extracted);
    
    /* Also test with explicit bit operations */
    uint32_t explicit_extract = (volatile_seed >> 8) & 0xFFF;
    use_int(explicit_extract);
    
    return extracted + explicit_extract;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int generate_strict_low_part(void) {
    /* Structure with small member to force partial register updates */
    struct partial_reg {
        unsigned char low_byte;
        unsigned char pad[3];
        int full_word;
    } s;
    
    /* Initialize with volatile to prevent optimization */
    int temp = volatile_seed;
    
    /* This assignment should generate STRICT_LOW_PART */
    s.low_byte = temp & 0xFF;
    
    /* Also assign through pointer cast */
    unsigned char *byte_ptr = (unsigned char*)&s.full_word;
    byte_ptr[1] = (temp >> 8) & 0xFF;  /* Modify only one byte */
    
    /* Use results */
    use_int(s.low_byte);
    use_int(s.full_word);
    
    return s.low_byte + s.full_word;
}

/* Function 3: Generate SUBREG pattern */
__attribute__((noinline))
static int generate_subreg(void) {
    /* Array for memory access patterns */
    int array[16];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Access through different pointer types - should generate SUBREG */
    volatile int idx = volatile_index;
    
    /* Access as short (half-word) */
    short *short_ptr = (short*)((char*)array + idx * sizeof(int));
    short half_val = *short_ptr;
    use_short(half_val);
    
    /* Access as char */
    unsigned char *char_ptr = (unsigned char*)&array[idx];
    unsigned char byte_val = char_ptr[2];
    use_int(byte_val);
    
    /* Complex addressing with pointer arithmetic */
    long *long_ptr = (long*)array;
    long_ptr += idx;
    use_ptr(long_ptr);
    
    return half_val + byte_val;
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
static int generate_mem_complex(void) {
    /* Multi-dimensional array for complex addressing */
    int matrix[4][4];
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = volatile_seed + i * 4 + j;
        }
    }
    
    /* Complex addressing calculation */
    volatile int row = volatile_index & 0x3;
    volatile int col = (volatile_seed >> 4) & 0x3;
    
    /* This should generate MEM with complex address expression */
    int *elem_ptr = &matrix[row][col];
    int value = *elem_ptr;
    
    /* Even more complex: pointer to pointer */
    int **ptr_to_ptr = (int**)matrix;
    use_ptr(ptr_to_ptr + row);
    
    /* Access through computed pointer */
    int *computed_ptr = *(int**)((char*)matrix + row * sizeof(int*));
    computed_ptr[col] = volatile_seed;
    
    return value + matrix[row][col];
}

/* Function 5: Combined patterns */
__attribute__((noinline))
static int generate_combined(void) {
    /* Structure combining bitfields and arrays */
    struct combined {
        union {
            uint32_t flags;
            struct {
                uint32_t a: 4;
                uint32_t b: 4;
                uint32_t c: 8;
                uint32_t d: 16;
            } parts;
        } u;
        int data[8];
    } cmb;
    
    /* Initialize with volatile */
    cmb.u.flags = volatile_seed;
    
    /* Extract part (ZERO_EXTRACT) */
    uint32_t extracted = cmb.u.parts.c;
    
    /* Store to partial register (STRICT_LOW_PART) */
    unsigned char *byte_ptr = (unsigned char*)&cmb.data[0];
    byte_ptr[volatile_index & 0x7] = extracted & 0xFF;
    
    /* Access through SUBREG */
    short *short_ptr = (short*)&cmb.data[2];
    short half_word = short_ptr[1];
    
    /* Complex MEM access */
    int *mem_ptr = &cmb.data[volatile_index & 0x3];
    *mem_ptr = half_word + extracted;
    
    return cmb.data[0] + cmb.data[2] + cmb.data[4];
}

/* Function 6: Control flow variations */
__attribute__((noinline))
static int generate_with_control_flow(void) {
    int result = 0;
    volatile int limit = 4;
    
    /* Loop with pattern generation */
    for (int i = 0; i < limit; i++) {
        /* Conditional based on volatile */
        if (volatile_seed & (1 << i)) {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t low: 8;
                    uint32_t high: 24;
                } bits;
            } u;
            u.val = volatile_seed + i;
            result += u.bits.high;
        } else {
            /* STRICT_LOW_PART in else branch */
            struct {
                char bytes[4];
            } s;
            s.bytes[i & 0x3] = (volatile_seed >> (i * 8)) & 0xFF;
            result += s.bytes[i & 0x3];
        }
    }
    
    /* Switch statement with different patterns */
    switch (volatile_index & 0x3) {
        case 0: {
            /* SUBREG pattern */
            int arr[4] = {1, 2, 3, 4};
            short *sp = (short*)arr;
            result += sp[volatile_index];
            break;
        }
        case 1: {
            /* MEM with complex address */
            int *ptr = (int*)((char*)&result + volatile_index);
            result += *ptr;
            break;
        }
        case 2: {
            /* Combined pattern */
            union {
                long l;
                int i[2];
            } u;
            u.l = volatile_seed;
            result += u.i[0] + u.i[1];
            break;
        }
        default:
            result += volatile_seed;
    }
    
    return result;
}

/* Main function that calls all pattern generators */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation test...\n");
    
    /* Call each pattern generator */
    checksum += generate_zero_extract();
    checksum += generate_strict_low_part();
    checksum += generate_subreg();
    checksum += generate_mem_complex();
    checksum += generate_combined();
    checksum += generate_with_control_flow();
    
    /* Print checksum to ensure code execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions for external functions */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
