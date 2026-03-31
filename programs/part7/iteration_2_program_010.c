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

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
static int test_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.mid;  /* Should generate ZERO_EXTRACT for middle bits */
    
    /* Pattern 2: Manual masking and shifting */
    uint32_t val = volatile_seed;
    int result2 = (val >> 4) & 0xFFF;  /* Extract bits 4-15 */
    
    /* Pattern 3: Nested extraction */
    struct {
        struct {
            unsigned a: 3;
            unsigned b: 5;
            unsigned c: 8;
        } inner;
        unsigned d: 16;
    } nested;
    
    *(uint32_t*)&nested = volatile_seed;
    int result3 = nested.inner.b;
    
    /* Combine results with external use */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static int test_strict_low_part(void) {
    int temp = volatile_seed;
    
    /* Pattern 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } s;
    
    s.rest = temp >> 8;
    s.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Pattern 2: Pointer to low part */
    int value = volatile_seed;
    char *low_ptr = (char*)&value;
    *low_ptr = volatile_char;  /* Modify only low byte */
    
    /* Pattern 3: Union assignment to partial register */
    union {
        uint32_t full;
        uint16_t half[2];
    } u;
    
    u.full = volatile_seed;
    u.half[0] = volatile_short;  /* Modify low 16 bits */
    
    /* External use to preserve operations */
    use_int(s.low_byte);
    use_int(value);
    use_short(u.half[0]);
    
    return s.low_byte + *low_ptr + u.half[0];
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int test_subreg_mem(void) {
    int array[16];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Pattern 1: SUBREG through pointer casting */
    int idx = volatile_index;
    short *short_ptr = (short*)((char*)array + idx * sizeof(int));
    *short_ptr = volatile_short;  /* MEM with SUBREG addressing */
    
    /* Pattern 2: Complex addressing mode */
    int *ptr = &array[volatile_index & 7];
    ptr += (volatile_seed >> 4) & 3;
    *ptr = volatile_seed;  /* MEM with complex address */
    
    /* Pattern 3: Type punning with different access sizes */
    union {
        int32_t words[4];
        int8_t bytes[16];
    } buffer;
    
    buffer.words[0] = volatile_seed;
    buffer.bytes[5] = volatile_char;  /* SUBREG-like access */
    
    /* Pattern 4: Pointer arithmetic with different types */
    char *base = (char*)array;
    int offset = volatile_index * 2;
    int16_t *alias = (int16_t*)(base + offset);
    *alias = volatile_short;
    
    /* External use */
    use_ptr(short_ptr);
    use_ptr(ptr);
    use_int(buffer.words[0]);
    use_short(*alias);
    
    return array[0] + *ptr + buffer.words[0] + *alias;
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
static int test_combined_patterns(void) {
    int result = 0;
    volatile int control = volatile_seed;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        union {
            uint32_t full;
            struct {
                uint32_t a: 10;
                uint32_t b: 10;
                uint32_t c: 12;
            } parts;
        } data;
        
        data.full = volatile_seed + i;
        
        /* Conditional based on extracted bits */
        if (data.parts.b & 0x100) {
            /* ZERO_EXTRACT in condition */
            int buffer[8];
            
            /* STRICT_LOW_PART store */
            char *low = (char*)&buffer[i % 4];
            *low = data.parts.c & 0xFF;
            
            /* MEM with SUBREG */
            short *mid = (short*)((char*)buffer + 2);
            *mid = data.parts.a;
            
            result += buffer[0];
        } else {
            /* Different MEM pattern */
            int temp[4];
            int idx = data.parts.c % 4;
            temp[idx] = data.parts.b;  /* Complex addressing */
            result += temp[idx];
        }
    }
    
    /* Switch statement with different patterns */
    switch (control & 0x3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            uint32_t val = volatile_seed;
            struct { uint16_t low, high; } split;
            split.low = val & 0xFFFF;
            split.high = (val >> 16) & 0xFFFF;
            result += split.high;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            int x = volatile_seed;
            char *p = (char*)&x;
            p[1] = volatile_char;  /* Modify specific byte */
            result += x;
            break;
        }
        case 2: {
            /* SUBREG/MEM pattern */
            long arr[4];
            short *sptr = (short*)arr;
            sptr[volatile_index & 3] = volatile_short;
            result += arr[0];
            break;
        }
        default: {
            /* Combined pattern */
            union {
                uint64_t full;
                uint32_t halves[2];
            } u;
            u.full = volatile_seed;
            u.halves[0] &= 0x0000FFFF;  /* STRICT_LOW_PART-like */
            result += u.halves[0];
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Call each test function */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg_mem();
    checksum += test_combined_patterns();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result != 0 ? 0 : 1;
}

/* External function definitions (in separate file normally) */
#ifdef COMPILE_WITH_STUBS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
#endif
