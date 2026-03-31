/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile char volatile_char;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfield to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:8;
            unsigned int high:16;
        } bits;
    } u;
    
    /* Volatile assignment prevents constant folding */
    u.full = volatile_seed;
    
    /* Multiple extractions to increase chances */
    unsigned int result1 = u.bits.low;
    unsigned int result2 = u.bits.middle;
    unsigned int result3 = u.bits.high;
    
    /* Complex extraction with masking */
    unsigned int masked = (u.full >> 4) & 0xFFF;
    
    /* Use results to prevent elimination */
    use_int(result1 + result2 + result3 + masked);
    return result1 ^ result2 ^ result3 ^ masked;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        short low_word;
        int full;
    } data;
    
    /* Initialize with volatile to prevent constant propagation */
    int temp = volatile_seed;
    
    /* These assignments should generate STRICT_LOW_PART */
    data.low_byte = temp & 0xFF;          /* Modifies only low 8 bits */
    data.low_word = temp & 0xFFFF;        /* Modifies only low 16 bits */
    data.full = temp;                     /* Full assignment */
    
    /* Pointer-based low-part modification */
    short* ptr = &data.low_word;
    *ptr = volatile_short;                /* Another low-part store */
    
    /* Use results */
    use_int(data.low_byte + data.low_word + data.full);
    return data.low_byte ^ data.low_word ^ data.full;
}

/* Pattern 3: Generate SUBREG and MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for complex memory addressing */
    int array[64];
    char char_array[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        array[i] = i + volatile_seed;
    }
    
    /* Complex pointer arithmetic generating SUBREG/MEM */
    volatile int idx = volatile_index & 63;
    
    /* Type punning through different pointer types */
    int* int_ptr = array + idx;
    short* short_ptr = (short*)int_ptr;           /* SUBREG generation */
    char* char_ptr = (char*)int_ptr;
    
    /* Memory accesses with different widths */
    int int_val = *int_ptr;                       /* MEM access */
    short short_val = *short_ptr;                 /* SUBREG + MEM */
    char char_val = char_ptr[2];                  /* Another MEM */
    
    /* More complex addressing mode */
    long* long_ptr = (long*)(char_array + (idx * 4));
    *long_ptr = volatile_seed;                    /* MEM store */
    
    /* Use results */
    use_int(int_val);
    use_short(short_val);
    use_int(char_val);
    use_long(*long_ptr);
    
    return int_val + short_val + char_val + *long_ptr;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    
    /* Control flow based on volatile input */
    switch (volatile_seed & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            union {
                uint32_t dword;
                struct {
                    uint16_t low;
                    uint16_t high;
                } words;
            } u;
            u.dword = volatile_seed;
            result = u.words.high - u.words.low;  /* ZERO_EXTRACT */
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            struct {
                int32_t full;
                int16_t half;
            } s;
            s.full = volatile_seed;
            s.half = volatile_short;              /* STRICT_LOW_PART */
            result = s.full + s.half;
            break;
        }
        case 2: {
            /* SUBREG + MEM pattern */
            int buffer[4];
            for (int i = 0; i < 4; i++) {
                buffer[i] = volatile_seed + i;
            }
            int16_t* half_ptr = (int16_t*)&buffer[1];
            result = *half_ptr;                   /* SUBREG + MEM */
            break;
        }
        default: {
            /* Mixed pattern */
            volatile int v = volatile_seed;
            result = (v >> 8) & 0xFF;             /* ZERO_EXTRACT */
            struct { char c; } s;
            s.c = result;                         /* STRICT_LOW_PART */
            result = s.c;
            break;
        }
    }
    
    return result;
}

/* Pattern 5: Loop with pattern generation */
__attribute__((noinline))
static int pattern_loop(void) {
    int sum = 0;
    
    /* Loop prevents some optimizations */
    for (volatile int i = 0; i < 4; i++) {
        /* Different pattern each iteration */
        if (i & 1) {
            /* ZERO_EXTRACT in loop */
            uint32_t val = volatile_seed + i;
            uint8_t byte = (val >> (i * 4)) & 0xF;
            sum += byte;
        } else {
            /* STRICT_LOW_PART in loop */
            struct {
                int32_t val;
                int8_t byte;
            } s;
            s.val = volatile_seed;
            s.byte = i;                          /* STRICT_LOW_PART */
            sum += s.byte;
        }
        
        /* MEM access with index */
        int array[8];
        array[i] = volatile_seed;
        sum += array[i];                         /* MEM access */
    }
    
    return sum;
}

/* Main function that exercises all patterns */
int main(void) {
    /* Initialize volatile values */
    volatile_seed = 0x12345678;
    volatile_index = 16;
    volatile_short = 0xABCD;
    volatile_char = 'X';
    
    int checksum = 0;
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_loop();
    
    /* Additional complex pattern mixing all three */
    {
        /* Create a ZERO_EXTRACT */
        union {
            unsigned int val;
            struct {
                unsigned int a:3;
                unsigned int b:5;
                unsigned int c:24;
            } bits;
        } u;
        u.val = volatile_seed;
        
        /* Use it in STRICT_LOW_PART */
        struct {
            unsigned int full;
            unsigned char low;
        } s;
        s.full = u.bits.c;                     /* From ZERO_EXTRACT */
        s.low = u.bits.b;                      /* STRICT_LOW_PART */
        
        /* Access via SUBREG/MEM */
        unsigned char* ptr = (unsigned char*)&s.full;
        ptr[1] = u.bits.a;                     /* MEM + possible SUBREG */
        
        checksum ^= s.full ^ s.low ^ ptr[0];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
