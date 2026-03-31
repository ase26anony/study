/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile globals to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile short g_volatile_short = 7;
volatile long g_volatile_long = 123456;
volatile int g_index = 3;
volatile int g_mask = 0xFF;

/* Function 1: Generate ZERO_EXTRACT pattern through bitfield union */
__attribute__((noinline))
int pattern_zero_extract(void) {
    /* Union with bitfield to force ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t middle: 12;
            uint32_t high: 12;
        } bits;
    } u;
    
    /* Volatile input prevents constant folding */
    uint32_t input = g_volatile_int;
    u.full = input;
    
    /* Multiple extractions to increase chances */
    int result = u.bits.middle;          /* Should generate ZERO_EXTRACT */
    result += u.bits.high << 12;         /* Another extraction with shift */
    
    /* Use result to keep it alive */
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
short pattern_strict_low_part(void) {
    struct {
        char low_byte;
        char high_byte;
        short full_word;
    } s;
    
    /* Initialize with volatile */
    int temp = g_volatile_int;
    
    /* Modify only low part through pointer */
    char *ptr = &s.low_byte;
    *ptr = temp & 0xFF;                  /* Should generate STRICT_LOW_PART */
    
    /* Another low-part modification */
    s.full_word = g_volatile_short;      /* Assignment to short might use STRICT_LOW_PART */
    
    /* Complex expression with low part */
    s.high_byte = (temp >> 8) & 0xFF;
    
    use_short(s.full_word);
    return s.full_word;
}

/* Function 3: Generate SUBREG and MEM patterns */
__attribute__((noinline))
int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    volatile int idx = g_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * g_volatile_int;
    }
    
    /* SUBREG pattern: access through different type */
    short *short_ptr = (short*)array;
    short_ptr[idx] = g_volatile_short;   /* MEM with SUBREG access */
    
    /* More complex addressing mode */
    char *char_ptr = (char*)array;
    int *int_ptr = (int*)(char_ptr + idx * sizeof(int) + 1);
    *int_ptr = g_volatile_int;           /* Unaligned MEM access */
    
    /* Pointer arithmetic creating complex MEM */
    long *long_ptr = (long*)&array[4];
    long_ptr[g_index & 3] = g_volatile_long;
    
    use_ptr(array);
    return array[0] + array[g_index & 15];
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
int pattern_combined(void) {
    volatile int selector = g_volatile_int;
    int result = 0;
    
    /* Switch with different patterns in each case */
    switch (selector & 3) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 4;
                    uint32_t b: 10;
                    uint32_t c: 18;
                } fields;
            } u;
            
            for (int i = 0; i < 4; i++) {
                u.val = g_volatile_int + i;
                result += u.fields.b;    /* ZERO_EXTRACT in loop */
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with conditionals */
            struct {
                unsigned char bytes[4];
                unsigned int word;
            } data;
            
            if (selector > 100) {
                data.bytes[0] = selector & 0xFF;  /* STRICT_LOW_PART */
                data.word = selector;
            } else {
                data.bytes[2] = (selector >> 8) & 0xFF;
            }
            result = data.word;
            break;
        }
        
        case 2: {
            /* SUBREG and MEM in nested loops */
            int matrix[4][4];
            volatile int row = g_index & 3;
            volatile int col = (g_index >> 2) & 3;
            
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    /* Access through byte pointer creates SUBREG */
                    unsigned char *ptr = (unsigned char*)&matrix[i][j];
                    ptr[(i + j) & 3] = (i * j + g_volatile_int) & 0xFF;
                }
            }
            result = matrix[row][col];
            break;
        }
        
        default: {
            /* Mixed patterns */
            union bitfield {
                unsigned int full;
                struct {
                    unsigned int low: 16;
                    unsigned int high: 16;
                } half;
            } b;
            
            b.full = g_volatile_int;
            /* ZERO_EXTRACT */
            unsigned short low = b.half.low;
            
            /* STRICT_LOW_PART through pointer */
            unsigned short *ptr = &b.half.low;
            *ptr = g_volatile_short;
            
            result = b.full;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Function 5: Complex memory addressing with SUBREG */
__attribute__((noinline))
int pattern_complex_address(void) {
    struct buffer {
        char data[64];
        int checksum;
    } buf;
    
    volatile int offset = g_index;
    
    /* Complex addressing computation */
    int *int_ptr = (int*)(buf.data + offset * 2 + 1);
    *int_ptr = g_volatile_int;           /* MEM with complex address */
    
    /* Access through different-sized types */
    short *short_ptr = (short*)(buf.data + 8);
    for (int i = 0; i < 4; i++) {
        short_ptr[i] = (g_volatile_int + i) & 0xFFFF;  /* SUBREG stores */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += buf.data[i];
    }
    buf.checksum = sum;
    
    return buf.checksum;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Execute each pattern function */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_complex_address();
    
    /* Use volatile to prevent dead code elimination */
    g_volatile_int = checksum;
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* External function definitions (could be in separate file) */
void __attribute__((weak)) use_int(int x) {
    /* Prevent optimization */
    asm volatile("" : "+r" (x));
}

void __attribute__((weak)) use_short(short x) {
    asm volatile("" : "+r" (x));
}

void __attribute__((weak)) use_ptr(void* x) {
    asm volatile("" : "+r" (x));
}

void __attribute__((weak)) use_long(long x) {
    asm volatile("" : "+r" (x));
}
