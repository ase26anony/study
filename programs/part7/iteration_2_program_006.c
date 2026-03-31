/* test_resource.c - Generate specific RTL patterns for GCC resource.cc coverage */

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
    /* Use union with bitfields to encourage ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low_bits : 8;
            uint32_t mid_bits : 16;
            uint32_t high_bits : 8;
        } parts;
    } data;
    
    /* Volatile input prevents constant propagation */
    data.full = volatile_seed;
    
    /* These operations should generate ZERO_EXTRACT RTL */
    int result1 = data.parts.mid_bits;          /* Likely ZERO_EXTRACT */
    int result2 = data.parts.high_bits << 8;    /* Another potential */
    
    /* Force usage through external call */
    use_int(result1);
    use_int(result2);
    
    return result1 + result2;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int generate_strict_low_part(void) {
    struct {
        char low_byte;
        char mid_byte;
        short low_word;
        int full_word;
    } container;
    
    /* Initialize with volatile to prevent optimization */
    int temp = volatile_seed;
    
    /* These assignments may generate STRICT_LOW_PART */
    container.low_byte = temp & 0xFF;           /* Modify only low 8 bits */
    container.low_word = temp & 0xFFFF;         /* Modify only low 16 bits */
    
    /* Complex expression that might use STRICT_LOW_PART */
    container.mid_byte = (temp >> 8) & 0xFF;
    
    /* Force the compiler to keep all operations */
    use_short(container.low_word);
    use_int(container.full_word);
    
    return container.low_byte + container.mid_byte;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int generate_subreg_mem(void) {
    /* Create an array with complex access patterns */
    int array[16];
    volatile int idx = volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Type punning through pointers - may generate SUBREG */
    short* short_ptr = (short*)((char*)array + idx);
    *short_ptr = volatile_mask;  /* MEM access with SUBREG */
    
    /* Different type access */
    char* char_ptr = (char*)&array[4];
    char_ptr[1] = volatile_seed & 0xFF;  /* Another MEM with possible SUBREG */
    
    /* Complex addressing mode */
    int* complex_ptr = &array[idx * 2 + 1];
    *complex_ptr = volatile_seed >> 8;
    
    /* Use the results */
    use_ptr(short_ptr);
    use_int(*complex_ptr);
    
    return array[0] + array[4];
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
static int generate_combined_patterns(void) {
    int result = 0;
    volatile int control = volatile_seed & 0x3;  /* 0-3 range */
    
    /* Switch with different pattern generation in each case */
    switch (control) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a : 4;
                    uint32_t b : 12;
                    uint32_t c : 16;
                } bits;
            } u;
            
            u.val = volatile_seed;
            for (int i = 0; i < 4; i++) {
                result += u.bits.b << i;  /* ZERO_EXTRACT in loop */
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with condition */
            struct {
                unsigned char small;
                unsigned int normal;
            } s;
            
            s.normal = volatile_seed;
            if (volatile_index > 0) {
                s.small = volatile_mask;  /* STRICT_LOW_PART */
            }
            result = s.small + s.normal;
            break;
        }
        
        case 2: {
            /* SUBREG and MEM in nested structure */
            typedef struct {
                int data[4];
                short shorts[8];
            } nested_t;
            
            nested_t nested;
            volatile int offset = volatile_index & 0x3;
            
            /* Complex MEM access with SUBREG */
            *( (short*)((char*)nested.data + offset * 2) ) = volatile_seed;
            result = nested.data[offset];
            break;
        }
        
        case 3: {
            /* All patterns combined */
            union {
                uint32_t full;
                struct {
                    uint32_t part1 : 10;
                    uint32_t part2 : 10;
                    uint32_t part3 : 12;
                } split;
            } extract_union;
            
            extract_union.full = volatile_seed;
            
            struct {
                char low;
                int full;
            } strict_struct;
            
            strict_struct.full = extract_union.split.part2;
            strict_struct.low = extract_union.split.part1 & 0xFF;  /* STRICT_LOW_PART */
            
            /* MEM access through computed pointer */
            int buffer[8];
            int* mem_ptr = &buffer[volatile_index & 0x7];
            *mem_ptr = strict_struct.full + extract_union.split.part3;
            
            result = *mem_ptr + strict_struct.low;
            break;
        }
    }
    
    return result;
}

/* Function 5: Complex memory addressing with SUBREG */
__attribute__((noinline))
static int generate_complex_addressing(void) {
    /* Multi-dimensional array with stride access */
    int matrix[4][8];
    volatile int row = volatile_index & 0x3;
    volatile int col = volatile_seed & 0x7;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Access through byte pointer - forces SUBREG */
    unsigned char* byte_ptr = (unsigned char*)matrix;
    int offset = (row * 8 + col) * sizeof(int);
    
    /* Modify individual bytes - each generates MEM with possible SUBREG */
    for (int k = 0; k < 4; k++) {
        byte_ptr[offset + k] = (volatile_seed >> (k * 8)) & 0xFF;
    }
    
    /* Access as different types */
    short* short_view = (short*)&matrix[row][col];
    use_short(short_view[0]);
    use_short(short_view[1]);
    
    return matrix[row][col];
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation...\n");
    
    /* Call each pattern generator */
    checksum += generate_zero_extract();
    checksum += generate_strict_low_part();
    checksum += generate_subreg_mem();
    checksum += generate_combined_patterns();
    checksum += generate_complex_addressing();
    
    /* Mix in some volatile operations */
    checksum ^= volatile_seed;
    checksum += volatile_index;
    
    printf("Final checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

/* Dummy definitions for external functions (in same file for testing) */
void use_int(int x) { volatile_seed ^= x; }
void use_short(short x) { volatile_index += x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { volatile_mask = x & 0xFF; }
