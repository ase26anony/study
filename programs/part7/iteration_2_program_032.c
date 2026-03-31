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
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Use union with bitfields to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 12;
            unsigned int high: 12;
        } bits;
    } u;
    
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    int result = u.bits.mid;           /* Should generate ZERO_EXTRACT */
    result += u.bits.low << 4;         /* Another extraction with shift */
    result |= (u.bits.high & 0xF) << 16; /* Masked extraction */
    
    /* Complex control flow */
    if (volatile_seed & 1) {
        result ^= u.bits.low;
    } else {
        result |= u.bits.mid << 8;
    }
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        short low_word;
        int full;
    } data;
    
    int temp = volatile_seed;
    
    /* These assignments should generate STRICT_LOW_PART */
    data.low_byte = temp & 0xFF;           /* Modify only low 8 bits */
    data.low_word = temp & 0xFFFF;         /* Modify only low 16 bits */
    
    /* Through pointer with type punning */
    char *byte_ptr = (char*)&data.full;
    byte_ptr[0] = volatile_char;           /* Modify single byte */
    
    /* In a loop to create multiple RTL instances */
    for (int i = 0; i < 3; i++) {
        if (volatile_index & (1 << i)) {
            data.low_byte ^= i;
        }
    }
    
    use_int(data.full);
    return data.full;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    volatile int *volatile_ptr = &volatile_seed;
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Complex pointer arithmetic for MEM_P with address computation */
    int index = volatile_index & 0xF;
    
    /* Access through different types - should generate SUBREG */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = volatile_short;  /* MEM with SUBREG store */
    
    /* Another SUBREG access */
    char *char_ptr = (char*)&array[5];
    char_ptr[2] = volatile_char;  /* MEM with byte SUBREG */
    
    /* Pointer to pointer with offset */
    int **ptr_to_ptr = (int**)&array[8];
    *ptr_to_ptr = &array[index];  /* Complex MEM access */
    
    /* Switch with different MEM patterns */
    switch (volatile_index & 3) {
        case 0:
            ((unsigned char*)array)[7] = 0xAA;
            break;
        case 1:
            ((short*)array)[3] = 0xBBBB;
            break;
        case 2:
            ((int*)array)[1] = 0xCCCCCCCC;
            break;
        default:
            /* Nested pointer access */
            int *indirect = &array[volatile_index & 7];
            *indirect = volatile_seed;
            break;
    }
    
    use_ptr(array);
    return array[0] + array[1];
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    union {
        uint32_t dword;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
        uint8_t bytes[4];
    } data;
    
    data.dword = volatile_seed;
    
    /* Mix ZERO_EXTRACT and STRICT_LOW_PART */
    int result = 0;
    
    /* Loop with combined operations */
    for (int i = 0; i < 4; i++) {
        /* ZERO_EXTRACT pattern */
        uint8_t byte = data.bytes[i];  /* Should be ZERO_EXTRACT */
        
        /* STRICT_LOW_PART pattern */
        if (i & 1) {
            data.words.low = byte;  /* Modify low word only */
        } else {
            data.bytes[i ^ 1] = byte;  /* Modify single byte */
        }
        
        /* MEM_P with SUBREG through pointer */
        volatile uint8_t *mem_ptr = &data.bytes[(i + 1) & 3];
        *mem_ptr ^= byte;
        
        result += byte;
    }
    
    /* Conditional with nested patterns */
    if (volatile_seed > 0) {
        /* More complex MEM addressing */
        uint16_t *word_ptr = (uint16_t*)((char*)&data + 1);
        *word_ptr = result & 0xFFFF;  /* Unaligned access - more complex RTL */
    }
    
    use_int(result);
    return result + data.dword;
}

/* Function 5: Deeply nested patterns */
__attribute__((noinline))
static int pattern_deep_nesting(void) {
    struct {
        int buffer[8];
        union {
            long long qword;
            struct {
                int first;
                int second;
            } dwords;
        } u;
    } context;
    
    /* Initialize with volatile values */
    for (int i = 0; i < 8; i++) {
        context.buffer[i] = volatile_seed * i;
    }
    
    /* Complex expression with multiple patterns */
    int idx = volatile_index;
    
    /* Chain of operations that should generate target RTL */
    do {
        /* ZERO_EXTRACT from memory */
        int val = context.buffer[idx & 7];
        int low_bits = val & 0xFF;  /* ZERO_EXTRACT pattern */
        
        /* STRICT_LOW_PART to memory */
        context.buffer[(idx + 1) & 7] = low_bits;  /* Store only low bits */
        
        /* SUBREG access through pointer */
        short *half_ptr = (short*)&context.buffer[idx & 7];
        half_ptr[1] = volatile_short;  /* SUBREG in MEM */
        
        /* Update index with complex expression */
        idx = (idx * 13 + 7) & 0xF;
        
    } while (idx != (volatile_index & 0xF));
    
    /* Final MEM_P with address computation */
    int *final_ptr = &context.buffer[volatile_index & 3];
    *final_ptr += context.u.dwords.first;
    
    use_long(context.u.qword);
    return context.buffer[0];
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call all pattern functions */
    checksum ^= pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum += pattern_combined();
    checksum ^= pattern_deep_nesting();
    
    /* Use volatile to ensure all calls happen */
    if (volatile_seed) {
        printf("Checksum: %d\n", checksum);
    } else {
        printf("Alternative checksum: %d\n", ~checksum);
    }
    
    return checksum & 0xFF;
}

/* Dummy definitions for external functions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
