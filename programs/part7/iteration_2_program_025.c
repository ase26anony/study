/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    volatile int input = seed + 1;
    
    /* Method 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    u.full = input;
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual masking and shifting with volatile */
    volatile uint32_t v = input;
    int result2 = (v >> 16) & 0xFFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Method 3: In conditional context */
    if (input & 1) {
        result2 = (v >> 8) & 0xFF;  /* Different width extraction */
    }
    
    /* Force usage */
    use_int(result1);
    use_int(result2);
    
    return result1 + result2;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed + 2;
    
    /* Method 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } s;
    
    int temp = input;
    s.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Method 2: Pointer to char for low part access */
    int value = input;
    char *low_ptr = (char*)&value;
    *low_ptr = (input >> 8) & 0xFF;  /* Another low-part modification */
    
    /* Method 3: Union for type punning */
    union {
        int full;
        struct {
            char b0, b1, b2, b3;
        } bytes;
    } u;
    
    u.full = input;
    u.bytes.b1 = (input >> 16) & 0xFF;  /* Modify specific byte */
    
    /* Force usage */
    use_char(s.low_byte);
    use_int(u.full);
    
    return s.low_byte + *low_ptr + u.bytes.b1;
}

/* Pattern 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = seed & 0xF;
    volatile int value = seed + 3;
    
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100;
    }
    
    /* SUBREG pattern: Access through different pointer types */
    int result = 0;
    
    /* Access as short (SUBREG likely) */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;  /* MEM with SUBREG access */
    result += *short_ptr;
    
    /* Complex addressing mode */
    int *int_ptr = &array[index ^ 1];
    *int_ptr = value;  /* MEM with index computation */
    result += *int_ptr;
    
    /* Pointer arithmetic with different types */
    char *char_base = (char*)array;
    int *aliased_int = (int*)(char_base + (index * 2) % 16);
    *aliased_int = *aliased_int + 1;  /* MEM with non-simple address */
    result += *aliased_int;
    
    /* Force usage */
    use_short(*short_ptr);
    use_ptr(int_ptr);
    
    return result;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int input = seed + 4;
    volatile int selector = seed & 3;
    
    int result = 0;
    
    /* Switch with different pattern combinations */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT + MEM */
            union {
                uint32_t full;
                struct {
                    uint32_t a: 4;
                    uint32_t b: 12;
                    uint32_t c: 16;
                } fields;
            } u;
            
            u.full = input;
            int array[4] = {0};
            array[u.fields.a] = u.fields.b;  /* MEM with extracted index */
            result = array[1];
            break;
        }
            
        case 1: {
            /* STRICT_LOW_PART + SUBREG */
            struct {
                short low;
                short high;
            } s;
            
            s.low = input & 0xFFFF;  /* STRICT_LOW_PART */
            
            /* Access through different pointer type */
            int *as_int = (int*)&s;
            result = (*as_int) & 0xFFFF;  /* SUBREG access */
            break;
        }
            
        case 2: {
            /* All three patterns */
            volatile uint32_t v = input;
            
            /* ZERO_EXTRACT */
            uint32_t extracted = (v >> 8) & 0xFF;
            
            /* Store with STRICT_LOW_PART */
            char buffer[8];
            buffer[3] = extracted;  /* Could be STRICT_LOW_PART */
            
            /* Access with SUBREG */
            short *as_short = (short*)buffer;
            result = as_short[1];  /* SUBREG MEM access */
            break;
        }
            
        default: {
            /* Complex memory addressing */
            int matrix[4][4];
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix[i][j] = i * 4 + j;
                }
            }
            
            /* Multi-dimensional array access with computation */
            int *row = matrix[selector];
            result = row[input & 3];  /* MEM with address computation */
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Loop with pattern variations */
__attribute__((noinline))
static int pattern_loop(void) {
    volatile int iterations = (seed & 7) + 1;
    int accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        volatile int variant = seed + i;
        
        /* Alternate between patterns based on loop index */
        if (i & 1) {
            /* ZERO_EXTRACT in loop */
            uint32_t mask = 0xFF << (i * 4);
            uint32_t extracted = (variant & mask) >> (i * 4);
            accumulator += extracted;
        } else {
            /* STRICT_LOW_PART in loop */
            struct {
                char data[4];
            } s;
            s.data[i % 4] = variant & 0xFF;
            accumulator += s.data[i % 4];
        }
        
        /* MEM access with changing index */
        int buffer[8];
        buffer[i % 8] = accumulator;
        accumulator = buffer[(i + 1) % 8];
    }
    
    use_int(accumulator);
    return accumulator;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_loop();
    
    /* Mix in some volatile operations */
    volatile int final_mix = checksum;
    for (int i = 0; i < 4; i++) {
        final_mix = (final_mix << 3) | (final_mix >> 29);
        final_mix ^= seed;
    }
    
    printf("Final checksum: %d\n", final_mix);
    return final_mix & 0xFF;
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void* x) { (void)x; }
#endif
