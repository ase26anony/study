/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    volatile int input = seed + 1;
    int result = 0;
    
    /* Method 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = input;
    result = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual masking and shifting */
    volatile int mask = 0xFF00;
    result |= (input & mask) >> 8;
    
    /* Method 3: In conditional context */
    if (input & 0x80000000) {
        result = (input >> 24) & 0xFF;
    } else {
        result = (input >> 16) & 0xFF;
    }
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed + 2;
    int result = 0;
    
    /* Method 1: Structure with small member */
    struct {
        char low_byte;
        char pad[3];
        int full;
    } s;
    
    s.full = input;
    s.low_byte = (input & 0xFF);  /* Should generate STRICT_LOW_PART */
    
    /* Method 2: Pointer to low part */
    int temp = input;
    char *low_ptr = (char*)&temp;
    *low_ptr = (input >> 8) & 0xFF;
    
    /* Method 3: In loop context */
    for (int i = 0; i < 3; i++) {
        struct {
            short low_word;
            short high_word;
        } w;
        w.low_word = (input >> (i * 8)) & 0xFFFF;
        result += w.low_word;
    }
    
    use_int(temp);
    return result + s.low_byte;
}

/* Pattern 3: Generate SUBREG and MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = seed & 0xF;
    volatile int value = seed + 3;
    int result = 0;
    
    /* Array with type-punning access */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100;
    }
    
    /* SUBREG pattern: access through different type */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;  /* Should generate SUBREG */
    
    /* Complex MEM_P pattern with address computation */
    int *ptr = &array[5];
    ptr += (index & 3);
    
    /* Nested addressing */
    int **ptr_to_ptr = &ptr;
    result = **ptr_to_ptr;
    
    /* Memory access in switch statement */
    switch (index & 3) {
        case 0:
            *(volatile char*)&array[0] = value;
            break;
        case 1:
            *(volatile short*)&array[1] = value;
            break;
        case 2:
            *(volatile int*)&array[2] = value;
            break;
        default:
            *(volatile long long*)&array[3] = value;
            break;
    }
    
    use_ptr(array);
    return result + *short_ptr;
}

/* Pattern 4: Combined patterns */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int input = seed + 4;
    int result = 0;
    
    /* Combined structure for multiple patterns */
    union {
        struct {
            unsigned int field1: 10;
            unsigned int field2: 6;
            unsigned int field3: 16;
        } bits;
        unsigned int full;
        unsigned char bytes[4];
    } data;
    
    data.full = input;
    
    /* ZERO_EXTRACT from bitfield */
    int extracted = data.bits.field2;
    
    /* STRICT_LOW_PART assignment */
    data.bytes[0] = extracted & 0xFF;
    
    /* SUBREG/MEM access */
    volatile int buffer[8];
    short *buf_short = (short*)buffer;
    buf_short[(input & 3) * 2] = data.bytes[0];
    
    /* Complex control flow */
    for (int i = 0; i < 4; i++) {
        if (input & (1 << i)) {
            /* Nested pattern generation */
            union {
                int word;
                struct {
                    short low;
                    short high;
                } halves;
            } u;
            u.word = buffer[i];
            u.halves.low = data.bytes[i];
            buffer[i] = u.word;
        }
    }
    
    /* Compute result */
    for (int i = 0; i < 8; i++) {
        result += buffer[i];
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Complex addressing modes */
__attribute__((noinline))
static int pattern_complex_address(void) {
    volatile int base = seed + 5;
    volatile int offset = seed & 7;
    int result = 0;
    
    /* Multi-dimensional array with computed indices */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Complex address computation */
    int *row = matrix[base & 3];
    int *elem = &row[offset];
    
    /* Access through multiple pointer indirections */
    int **elem_ptr = &elem;
    int ***elem_ptr_ptr = &elem_ptr;
    
    result = ***elem_ptr_ptr;
    
    /* Pointer arithmetic with different types */
    char *char_base = (char*)matrix;
    char_base += (offset * sizeof(int) * 2);
    
    /* Access with SUBREG */
    short *short_view = (short*)char_base;
    for (int i = 0; i < 4; i++) {
        short_view[i] = (base >> (i * 4)) & 0xFFFF;
    }
    
    use_ptr(matrix);
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_complex_address();
    
    /* Use volatile to ensure execution */
    volatile int final_result = checksum;
    
    printf("Checksum: 0x%08x\n", final_result);
    
    return final_result & 0xFF;
}

/* External function definitions (weak linkage) */
void __attribute__((weak)) use_int(int x) {
    /* Do nothing, just prevent optimization */
    asm volatile("" : : "r"(x));
}

void __attribute__((weak)) use_short(short x) {
    asm volatile("" : : "r"(x));
}

void __attribute__((weak)) use_ptr(void* x) {
    asm volatile("" : : "r"(x));
}

void __attribute__((weak)) use_long(long x) {
    asm volatile("" : : "r"(x));
}
