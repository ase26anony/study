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
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Explicit masking and shifting with volatile */
    volatile unsigned int mask = 0xFFFF0000;
    int result2 = (input & mask) >> 16;
    
    /* Method 3: Nested extractions */
    union {
        uint32_t dword;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
    } v;
    
    v.dword = input;
    int result3 = (v.words.high >> 4) & 0xFFF;  /* Complex extraction */
    
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int base = seed + 2;
    
    /* Method 1: Structure assignment to low part */
    struct {
        char low_byte;
        char pad[3];
    } s1;
    
    s1.low_byte = base & 0xFF;  /* Should generate STRICT_LOW_PART */
    int r1 = s1.low_byte;
    
    /* Method 2: Pointer to low part */
    int temp = base;
    unsigned char *low_ptr = (unsigned char*)&temp;
    *low_ptr = (base >> 8) & 0xFF;  /* Modify only low byte */
    int r2 = temp;
    
    /* Method 3: Union modification */
    union {
        int full;
        struct {
            short low;
            short high;
        } halves;
    } u;
    
    u.full = base;
    u.halves.low = (base >> 16) & 0xFFFF;  /* Modify low 16 bits */
    int r3 = u.full;
    
    /* Method 4: In conditional context */
    if (base & 1) {
        struct {
            signed char low;
            int rest;
        } s2;
        s2.low = base & 0x7F;
        r3 += s2.low;
    }
    
    use_int(r1);
    use_int(r2);
    use_int(r3);
    
    return r1 + r2 + r3;
}

/* Pattern 3: Generate SUBREG and MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = (seed + 3) & 7;  /* 0-7 range */
    volatile int value = seed + 4;
    
    /* Array with complex access patterns */
    int array[16] = {0};
    
    /* Method 1: SUBREG through pointer casting */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;  /* MEM with SUBREG access */
    
    /* Method 2: Nested SUBREG access */
    char *char_ptr = (char*)array + index;
    for (int i = 0; i < 4; i++) {
        char_ptr[i] = (value >> (i * 8)) & 0xFF;
    }
    
    /* Method 3: Complex addressing mode */
    int *ptr = &array[index];
    struct {
        int a;
        int b;
    } *struct_ptr = (void*)ptr;
    
    struct_ptr->a = value;
    struct_ptr->b = value * 2;
    
    /* Method 4: Loop with SUBREG memory access */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char *byte_ptr = (unsigned char*)&array[i];
        sum += byte_ptr[index & 3];  /* MEM with address calculation */
    }
    
    /* Method 5: Switch with different access patterns */
    switch (index) {
        case 0:
            *(int*)((char*)array + 4) = value;  /* MEM with offset */
            break;
        case 1:
            *(short*)(array + 2) = value;  /* SUBREG access */
            break;
        case 2:
            array[3] = *(int*)((char*)array + index);  /* Load and store */
            break;
        default:
            ((char*)array)[index * 4] = value;
    }
    
    use_int(sum);
    use_ptr(array);
    
    return sum + array[0] + array[1];
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int control = seed + 5;
    int result = 0;
    
    /* Mixed operations in loop */
    for (int i = 0; i < 4; i++) {
        volatile int iter = control + i;
        
        /* ZERO_EXTRACT pattern */
        union {
            uint32_t val;
            struct {
                uint32_t low: 12;
                uint32_t high: 20;
            } f;
        } u;
        u.val = iter;
        int extracted = u.f.high;  /* ZERO_EXTRACT */
        
        /* STRICT_LOW_PART pattern */
        struct {
            short low;
            short high;
        } s;
        s.low = extracted & 0xFFF;  /* STRICT_LOW_PART */
        
        /* MEM with SUBREG pattern */
        int buffer[8];
        short *ptr = (short*)((char*)buffer + (iter & 7));
        *ptr = s.low;  /* MEM with address calculation */
        
        result += buffer[0] + extracted;
        
        /* Conditional with different patterns */
        if (iter & 1) {
            /* Another ZERO_EXTRACT */
            result += (iter & 0xF000) >> 12;
        } else {
            /* Another STRICT_LOW_PART */
            unsigned char *cptr = (unsigned char*)&result;
            cptr[0] = iter & 0xFF;
        }
    }
    
    /* Nested conditionals */
    if (control & 0x100) {
        /* Complex memory access chain */
        int data[4];
        for (int i = 0; i < 4; i++) {
            char *byte_ptr = (char*)data + i;
            *byte_ptr = (control >> (i * 2)) & 3;
        }
        result += data[0] + data[3];
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Recursive-like patterns with function pointers */
__attribute__((noinline))
static int pattern_complex_mem(void) {
    volatile int offset = (seed + 6) & 3;
    
    /* Multi-dimensional array with complex indexing */
    int matrix[4][4];
    
    /* Fill matrix */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (i << 4) | j;
        }
    }
    
    /* Access through multiple pointer indirections */
    int *row_ptr = matrix[offset];
    short *half_ptr = (short*)row_ptr;
    
    /* Modify through different type views */
    for (int i = 0; i < 4; i++) {
        /* Alternate between int and short access */
        if (i & 1) {
            row_ptr[i] = seed + i;
        } else {
            half_ptr[i] = (seed + i) & 0xFFFF;
        }
    }
    
    /* Compute checksum with mixed access */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        /* Access as bytes */
        unsigned char *byte_ptr = (unsigned char*)&matrix[i][0];
        for (int j = 0; j < 4 * sizeof(int); j++) {
            sum += byte_ptr[j];
        }
    }
    
    /* Pointer arithmetic with different scales */
    char *base = (char*)matrix;
    for (int i = 0; i < 16; i++) {
        int *int_ptr = (int*)(base + i * sizeof(int));
        short *short_ptr = (short*)(base + i * sizeof(int) + offset);
        
        *int_ptr += sum;
        *short_ptr = *short_ptr & 0xFF;
    }
    
    use_int(sum);
    return sum;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_complex_mem();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result & 0xFF;
}

/* External function definitions (would normally be in separate file) */
void use_int(int x) {
    /* Prevent optimization */
    asm volatile("" : : "r"(x));
}

void use_short(short x) {
    asm volatile("" : : "r"(x));
}

void use_ptr(void* x) {
    asm volatile("" : : "r"(x));
}

void use_long(long x) {
    asm volatile("" : : "r"(x));
}
