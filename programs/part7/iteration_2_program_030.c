/* test_resource.c - Generate specific RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Use union with bitfields to force ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    unsigned int extracted1 = u.bits.high;          /* Should generate ZERO_EXTRACT */
    unsigned int extracted2 = u.bits.middle << 4;   /* Shifted extraction */
    unsigned int extracted3 = (u.full >> 12) & 0xFF; /* Manual extraction */
    
    /* Use results to prevent elimination */
    use_int(extracted1);
    use_int(extracted2);
    use_int(extracted3);
    
    return extracted1 + extracted2 + extracted3;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        int full_word;
    } s;
    
    int temp = volatile_seed;
    
    /* Modify only low part through structure */
    s.low_byte = temp & 0xFF;           /* Should generate STRICT_LOW_PART */
    
    /* Another pattern using pointer to char */
    int value = volatile_seed + 1;
    char *byte_ptr = (char*)&value;
    byte_ptr[0] = volatile_char;        /* Modify low byte */
    
    /* Use results */
    use_int(s.full_word);
    use_int(value);
    
    return s.low_byte + value;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    volatile int idx = volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100 + volatile_seed;
    }
    
    /* Complex memory addressing with SUBREG */
    short *short_ptr = (short*)((char*)array + idx * sizeof(int) + 1);
    *short_ptr = volatile_short;        /* Should generate SUBREG + MEM */
    
    /* Another SUBREG pattern through union */
    union {
        long long doubleword;
        int words[2];
    } u;
    u.doubleword = (long long)volatile_seed << 32 | volatile_seed;
    int *word_ptr = &u.words[1];        /* Access high word */
    *word_ptr = volatile_index;         /* Modify through pointer */
    
    /* Use results */
    use_short(*short_ptr);
    use_int(*word_ptr);
    use_ptr(array);
    
    return array[idx] + *short_ptr + *word_ptr;
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_seed & 0xF;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        switch ((control + i) & 3) {
            case 0: {
                /* ZERO_EXTRACT in loop */
                union {
                    uint32_t val;
                    struct {
                        uint32_t a:3;
                        uint32_t b:5;
                        uint32_t c:24;
                    } fields;
                } u;
                u.val = volatile_seed + i;
                result += u.fields.b;  /* ZERO_EXTRACT */
                break;
            }
            case 1: {
                /* STRICT_LOW_PART in loop */
                struct {
                    unsigned char low;
                    unsigned char high;
                } s;
                int temp = volatile_seed - i;
                s.low = temp;          /* STRICT_LOW_PART */
                result += s.low;
                break;
            }
            case 2: {
                /* SUBREG/MEM in loop */
                int buffer[8];
                for (int j = 0; j < 8; j++) {
                    buffer[j] = volatile_seed + j;
                }
                char *byte_ptr = (char*)buffer + i * 2;
                short *short_ptr = (short*)byte_ptr;
                *short_ptr = volatile_short + i;  /* SUBREG + MEM */
                result += buffer[i % 8];
                break;
            }
            case 3: {
                /* Nested patterns */
                union {
                    long long combined;
                    struct {
                        int first;
                        short second;
                        short third;
                    } parts;
                } u2;
                u2.combined = (long long)volatile_seed << 32;
                u2.parts.second = volatile_short;  /* STRICT_LOW_PART on sub-register */
                result += u2.parts.first;
                break;
            }
        }
    }
    
    use_int(result);
    return result;
}

/* Function 5: Complex addressing modes */
__attribute__((noinline))
static int pattern_complex_address(void) {
    struct {
        int data[4][4];
        short metadata[8];
    } block;
    
    volatile int row = volatile_index & 3;
    volatile int col = (volatile_index >> 2) & 3;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            block.data[i][j] = i * 1000 + j * 100 + volatile_seed;
        }
    }
    
    /* Complex memory address calculation */
    int *elem_ptr = &block.data[row][col];
    short *meta_ptr = (short*)((char*)elem_ptr + sizeof(int) - 2);
    
    /* Access through computed pointer */
    *meta_ptr = volatile_short;  /* MEM with complex address + SUBREG */
    
    /* Another level of indirection */
    void **ptr_array[2];
    ptr_array[0] = (void*)elem_ptr;
    ptr_array[1] = (void*)meta_ptr;
    
    use_ptr(ptr_array[0]);
    use_ptr(ptr_array[1]);
    use_short(*meta_ptr);
    
    return *elem_ptr + *meta_ptr;
}

/* Main function */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation...\n");
    
    /* Call all pattern functions */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_complex_address();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Final result (volatile): %d\n", final_result);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

/* External function definitions (weak) to satisfy linker */
#ifdef __GNUC__
__attribute__((weak))
#endif
void use_int(int x) {
    /* Empty - just to prevent optimization */
    volatile int dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_short(short x) {
    volatile short dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_ptr(void* x) {
    volatile void* dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_long(long x) {
    volatile long dummy = x;
    (void)dummy;
}
