/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile globals to prevent constant propagation */
volatile int volatile_seed = 0x12345678;
volatile int volatile_index = 3;
volatile short volatile_short = 0xABCD;
volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
int generate_zero_extract(void) {
    volatile int input = volatile_seed;
    int result = 0;
    
    /* Pattern 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int mid:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = input;
    result = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual masking and shifting */
    int temp = input;
    if (volatile_index > 0) {
        /* Extract bits 8-15 */
        result += (temp >> 8) & 0xFF;  /* May generate ZERO_EXTRACT */
    }
    
    /* Pattern 3: Multiple extractions in loop */
    for (int i = 0; i < 3; i++) {
        int shift = i * 8;
        result += (input >> shift) & 0xFF;
    }
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
int generate_strict_low_part(void) {
    volatile int input = volatile_seed;
    int result = 0;
    
    /* Pattern 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } s;
    
    s.rest = input;
    s.low_byte = input & 0xFF;  /* Should generate STRICT_LOW_PART */
    result = s.low_byte;
    
    /* Pattern 2: Pointer to low part */
    short *ptr = (short*)&input;
    if (volatile_char > 0) {
        *ptr = volatile_short;  /* Modifies low 16 bits */
        result += *ptr;
    }
    
    /* Pattern 3: Union modification */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u;
    
    u.full = input;
    u.parts.low = volatile_short;  /* STRICT_LOW_PART candidate */
    result += u.full & 0xFFFF;
    
    use_int(result);
    return result;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
int generate_subreg_mem(void) {
    volatile int base = 100;
    int result = 0;
    
    /* Pattern 1: Array with sub-register access */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = base + i;
    }
    
    /* Access through different pointer types */
    short *short_ptr = (short*)((char*)array + volatile_index);
    *short_ptr = volatile_short;  /* MEM with SUBREG addressing */
    result = *short_ptr;
    
    /* Pattern 2: Complex addressing mode */
    int *int_ptr = &array[volatile_index & 7];
    *int_ptr = volatile_seed;
    result += *int_ptr;
    
    /* Pattern 3: Nested pointer arithmetic */
    char *char_ptr = (char*)array;
    for (int i = 0; i < 5; i++) {
        int *aliased = (int*)(char_ptr + i * sizeof(int));
        *aliased = i;  /* MEM with address computation */
        result += *aliased;
    }
    
    use_ptr(array);
    return result;
}

/* Function 4: Combined patterns */
__attribute__((noinline))
int generate_combined(void) {
    volatile int input = volatile_seed;
    int result = 0;
    
    /* Combined ZERO_EXTRACT and STRICT_LOW_PART */
    union {
        uint32_t full;
        struct {
            uint16_t low16;
            uint16_t high16;
        } words;
        struct {
            uint8_t b0;
            uint8_t b1;
            uint8_t b2;
            uint8_t b3;
        } bytes;
    } data;
    
    data.full = input;
    
    /* ZERO_EXTRACT pattern */
    uint16_t extracted = (data.full >> 8) & 0xFFFF;
    
    /* STRICT_LOW_PART pattern */
    data.words.low16 = extracted;  /* Modifies only low 16 bits */
    
    /* MEM_P with SUBREG pattern */
    int buffer[4];
    short *buf_ptr = (short*)buffer;
    buf_ptr[volatile_index & 3] = data.words.low16;
    
    /* Complex control flow */
    switch (volatile_char & 3) {
        case 0:
            result = data.bytes.b0;
            break;
        case 1:
            result = data.bytes.b1;
            break;
        case 2:
            result = data.bytes.b2;
            break;
        case 3:
            result = data.bytes.b3;
            break;
    }
    
    result += buffer[0];
    use_int(result);
    return result;
}

/* Function 5: Nested patterns with control flow */
__attribute__((noinline))
int generate_nested(void) {
    volatile int counter = volatile_seed & 0xF;
    int result = 0;
    int accum = 0;
    
    while (counter-- > 0) {
        /* Create ZERO_EXTRACT in loop */
        int temp = volatile_seed + accum;
        int low_byte = temp & 0xFF;  /* Potential ZERO_EXTRACT */
        
        /* Create STRICT_LOW_PART in loop */
        struct {
            char small;
            int large;
        } item;
        
        item.large = accum;
        item.small = low_byte;  /* STRICT_LOW_PART candidate */
        
        /* Create MEM_P with addressing */
        static int storage[8];
        int idx = accum & 7;
        short *ptr = (short*)((char*)storage + idx * sizeof(int));
        *ptr = item.small;  /* MEM with SUBREG */
        
        accum += storage[idx] + item.small;
    }
    
    result = accum;
    
    /* Conditional based on volatile */
    if (volatile_char > 64) {
        /* Additional ZERO_EXTRACT */
        union {
            long long big;
            int parts[2];
        } u;
        u.big = (long long)volatile_seed * volatile_seed;
        result += u.parts[0] & 0xFFFF;  /* Extract low 16 bits */
    }
    
    use_int(result);
    return result;
}

/* Main function to drive all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    checksum ^= generate_zero_extract();
    checksum ^= generate_strict_low_part();
    checksum ^= generate_subreg_mem();
    checksum ^= generate_combined();
    checksum ^= generate_nested();
    
    printf("Final checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum & 0xFF;
}

/* Dummy definitions for external functions to satisfy linker */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
void __attribute__((weak)) use_long(long x) { (void)x; }
