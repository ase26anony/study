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

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfield for ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t middle: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    /* Use volatile to prevent constant propagation */
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    uint32_t result1 = u.bits.high;          /* Should generate ZERO_EXTRACT */
    uint32_t result2 = u.bits.middle;        /* Another extraction */
    
    /* Manual extraction that might also generate ZERO_EXTRACT */
    uint32_t manual = (u.full >> 4) & 0x0FFF;
    
    /* Combine results and use externally */
    int final = result1 + result2 + manual;
    use_int(final);
    
    return final;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        char second_byte;
        short low_word;
        int full;
    } data;
    
    /* Initialize with volatile values */
    int temp = volatile_seed;
    
    /* These assignments should generate STRICT_LOW_PART */
    data.low_byte = temp & 0xFF;            /* Modify only low 8 bits */
    data.second_byte = (temp >> 8) & 0xFF;  /* Modify next 8 bits */
    
    /* Word assignment through pointer */
    short *word_ptr = &data.low_word;
    *word_ptr = (short)(temp & 0xFFFF);     /* Modify low 16 bits */
    
    /* Use union for type punning */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } pun;
    
    pun.full = temp;
    pun.parts.low = volatile_mask;          /* STRICT_LOW_PART on low half */
    
    use_short(data.low_byte);
    use_short(*word_ptr);
    
    return data.low_byte + pun.parts.low;
}

/* Pattern 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    volatile int idx = volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Complex memory addressing with type punning */
    
    /* 1. SUBREG pattern: access through different pointer types */
    int *int_ptr = &array[idx];
    short *short_ptr = (short*)int_ptr;      /* Cast creates SUBREG */
    
    /* Access through short pointer (SUBREG of memory) */
    short half_word = *short_ptr;
    use_short(half_word);
    
    /* 2. MEM_P with complex address computation */
    char *char_base = (char*)array;
    char *char_ptr = char_base + (idx * sizeof(int) + 1);
    int *aliased_int = (int*)char_ptr;       /* Misaligned access */
    
    /* 3. More SUBREG patterns with unions */
    union {
        long long doubleword;
        int words[2];
    } u;
    
    u.doubleword = (long long)volatile_seed * 2;
    
    /* Access individual words (SUBREG access) */
    int first_word = u.words[0];
    int second_word = u.words[1];
    
    /* 4. Pointer arithmetic creating complex MEM addresses */
    int *offset_ptr = array + (idx & 3);
    int *computed_ptr = offset_ptr + (volatile_mask % 4);
    
    /* Use all computed values */
    use_int(*aliased_int);
    use_int(first_word);
    use_int(second_word);
    use_ptr(computed_ptr);
    
    return half_word + *aliased_int + first_word;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed;
    
    /* Loop with pattern generation */
    for (int i = 0; i < 4; i++) {
        /* Conditional based on volatile */
        if ((control >> i) & 1) {
            /* ZERO_EXTRACT in conditional path */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 3;
                    uint32_t b: 5;
                    uint32_t c: 24;
                } fields;
            } extractor;
            
            extractor.val = control + i;
            result += extractor.fields.c;  /* ZERO_EXTRACT */
            
            /* STRICT_LOW_PART in the same block */
            struct {
                unsigned char small;
                int normal;
            } s;
            
            s.small = extractor.fields.b & 0x1F;  /* STRICT_LOW_PART */
            result += s.small;
        } else {
            /* SUBREG and MEM in else path */
            int buffer[8];
            for (int j = 0; j < 8; j++) {
                buffer[j] = control + j;
            }
            
            /* Access through different type pointer */
            short *short_view = (short*)buffer;
            result += short_view[i];  /* SUBREG access */
        }
    }
    
    /* Switch statement with different patterns */
    switch (control & 0x3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            uint32_t val = control;
            uint32_t extracted = (val >> 16) & 0xFFFF;
            result += extracted;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            int temp = control;
            unsigned char *byte_ptr = (unsigned char*)&temp;
            byte_ptr[0] = volatile_mask & 0x7F;  /* Modify low byte */
            result += byte_ptr[0];
            break;
        }
        case 2: {
            /* SUBREG pattern */
            long long big = (long long)control * 3;
            int *half = (int*)&big;
            result += half[0] + half[1];  /* Access halves */
            break;
        }
        case 3: {
            /* MEM with complex address */
            int table[4] = {1, 2, 3, 4};
            int *ptr = table + (volatile_index % 4);
            result += *ptr;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    
    /* Additional mixed pattern in main */
    volatile int mix = volatile_seed;
    
    /* Inline pattern that might generate target RTL */
    {
        union {
            int i;
            struct {
                short low;
                short high;
            } s;
        } u;
        
        u.i = mix;
        u.s.low = mix & 0x7FFF;  /* Potential STRICT_LOW_PART */
        checksum += u.s.high;    /* Potential ZERO_EXTRACT */
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
/* In a real test, these would be in a separate file */
#ifdef COMPILE_STANDALONE
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
#endif
