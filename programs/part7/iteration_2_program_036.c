/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void *);
extern void use_long(long);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline)) 
static int pattern_zero_extract(void) {
    /* Union with bitfields to encourage ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low : 8;
            uint32_t mid : 12;
            uint32_t high : 12;
        } bits;
    } u;
    
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* Multiple extractions to increase chances */
    int result = 0;
    result += u.bits.low;          /* Should generate ZERO_EXTRACT for low 8 bits */
    result += u.bits.mid << 4;     /* Should generate ZERO_EXTRACT for middle 12 bits */
    result += u.bits.high << 8;    /* Should generate ZERO_EXTRACT for high 12 bits */
    
    /* Manual masking that might also generate ZERO_EXTRACT */
    result += (input >> 16) & 0xFFF;
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        unsigned char low_byte;
        unsigned char mid_byte;
        unsigned short word;
        int full;
    } data;
    
    volatile int temp = seed + 2;
    
    /* Assignments to partial registers */
    data.low_byte = temp & 0xFF;           /* Should generate STRICT_LOW_PART */
    data.mid_byte = (temp >> 8) & 0xFF;    /* Another partial assignment */
    
    /* Pointer-based partial assignment */
    unsigned short *ptr = &data.word;
    *ptr = (temp >> 16) & 0xFFFF;          /* Should generate STRICT_LOW_PART for 16-bit */
    
    /* Union-based partial assignment */
    union {
        int32_t full;
        struct {
            int16_t low;
            int16_t high;
        } parts;
    } pun;
    
    pun.full = temp;
    pun.parts.low = (temp >> 4) & 0x7FFF;  /* Should generate STRICT_LOW_PART */
    
    use_short(data.word);
    return data.low_byte + data.mid_byte + pun.parts.low;
}

/* Pattern 3: Generate SUBREG and complex MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array with complex addressing */
    int array[64];
    volatile int idx = seed + 3;
    
    /* Initialize array */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Type punning through different pointer types */
    char *char_ptr = (char *)array;
    short *short_ptr = (short *)(char_ptr + idx);
    
    /* Access through SUBREG-like patterns */
    *short_ptr = idx & 0xFFFF;              /* Should generate SUBREG + MEM */
    result += *short_ptr;
    
    /* More complex addressing mode */
    int *int_ptr = (int *)((char *)array + (idx % 56) * sizeof(int));
    result += *int_ptr;
    
    /* Nested pointer arithmetic */
    long *long_ptr = (long *)(array + (idx % 60));
    result += (int)(*long_ptr & 0xFFFFFFFF);
    
    use_ptr(array);
    return result;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed + 4;
    int result = 0;
    
    /* Control flow to generate different RTL in different blocks */
    for (int i = 0; i < 4; i++) {
        switch ((selector + i) & 3) {
            case 0: {
                /* ZERO_EXTRACT in loop */
                union {
                    uint32_t val;
                    struct {
                        uint32_t a : 3;
                        uint32_t b : 5;
                        uint32_t c : 24;
                    } fields;
                } u;
                u.val = selector + i;
                result += u.fields.b;
                break;
            }
            case 1: {
                /* STRICT_LOW_PART in loop */
                struct {
                    unsigned char bytes[4];
                } s;
                int temp = selector * i;
                s.bytes[0] = temp & 0xFF;
                s.bytes[1] = (temp >> 8) & 0xFF;
                result += s.bytes[0] + s.bytes[1];
                break;
            }
            case 2: {
                /* SUBREG/MEM in loop */
                int buffer[8];
                for (int j = 0; j < 8; j++) buffer[j] = j + i;
                short *sp = (short *)((char *)buffer + (i * 2));
                result += *sp;
                break;
            }
            case 3: {
                /* All combined */
                union {
                    uint32_t full;
                    uint16_t halves[2];
                } u;
                u.full = selector;
                
                struct {
                    uint16_t low;
                } partial;
                partial.low = u.halves[0];  /* STRICT_LOW_PART */
                
                int arr[4] = {1, 2, 3, 4};
                int *p = arr + (partial.low & 3);
                result += *p + u.halves[1];  /* MEM + ZERO_EXTRACT */
                break;
            }
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Complex memory addressing with SUBREG */
__attribute__((noinline))
static int pattern_complex_address(void) {
    /* Multi-dimensional array for complex addressing */
    int matrix[8][8];
    volatile int row = (seed + 5) & 7;
    volatile int col = (seed + 6) & 7;
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    /* Complex address calculation that may generate SUBREG in address */
    int *elem = &matrix[row][col];
    
    /* Access through byte pointer with offset */
    char *byte_base = (char *)matrix;
    int offset = (row * 8 + col) * sizeof(int);
    short *short_view = (short *)(byte_base + offset + 1);
    
    int result = *elem + *short_view;
    
    /* Pointer chain that may generate complex MEM expressions */
    void *ptr1 = matrix;
    void *ptr2 = (char *)ptr1 + offset;
    int *ptr3 = (int *)ptr2;
    result += *ptr3;
    
    use_long((long)result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call all pattern generators */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_complex_address();
    
    /* Use volatile to ensure all computations are kept */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result & 0xFF;
}

/* Dummy definitions for external functions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void *x) { (void)x; }
void use_long(long x) { (void)x; }
