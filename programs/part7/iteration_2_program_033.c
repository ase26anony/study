/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
volatile int volatile_input = 42;
volatile int volatile_index = 3;
volatile short volatile_short = 0xABCD;
volatile char volatile_char = 'X';

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
int generate_zero_extract(void) {
    /* Pattern 1: Union with bitfields for ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = volatile_input * 7 + 13;
    
    /* This should generate ZERO_EXTRACT for the bitfield access */
    unsigned int extracted = u.bits.high;
    
    /* Pattern 2: Manual bit extraction that might become ZERO_EXTRACT */
    unsigned int value = volatile_input * 11;
    unsigned int mask = 0xFF00;
    unsigned int shift = 8;
    unsigned int extracted2 = (value & mask) >> shift;
    
    /* Combine with control flow */
    if (extracted > 100) {
        extracted2 = (value & 0x00FF0000) >> 16;
    }
    
    use_int(extracted + extracted2);
    return extracted + extracted2;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
int generate_strict_low_part(void) {
    /* Pattern 1: Structure with small member for STRICT_LOW_PART */
    struct {
        char low_byte;
        int rest;
    } s1;
    
    int temp = volatile_input * 17;
    
    /* This assignment to low_byte should generate STRICT_LOW_PART */
    s1.low_byte = temp & 0xFF;
    s1.rest = temp >> 8;
    
    /* Pattern 2: Pointer to char for low part modification */
    int value2 = volatile_input * 23;
    char *low_ptr = (char*)&value2;
    
    /* Modify only low byte */
    *low_ptr = volatile_char;
    
    /* Pattern 3: Union for type punning */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u2;
    
    u2.full = value2;
    /* This might generate STRICT_LOW_PART for the short assignment */
    u2.parts.low = volatile_short;
    
    use_int(s1.low_byte + *low_ptr + u2.parts.low);
    return s1.low_byte + u2.full;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
int generate_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_input + i * 3;
    }
    
    /* Pattern 1: SUBREG through pointer casting */
    volatile int idx = volatile_index & 0xF;
    
    /* Access as different types - should generate SUBREG */
    short *short_ptr = (short*)((char*)array + idx * sizeof(int));
    *short_ptr = volatile_short;
    
    /* Pattern 2: Complex MEM address with computation */
    int *ptr = &array[idx + 2];
    ptr = (int*)((char*)ptr + 1);  /* Misaligned access */
    
    /* This should generate MEM with complex address */
    int value = *ptr;
    
    /* Pattern 3: Nested pointer dereference */
    int **ptr_ptr = &ptr;
    int value2 = **ptr_ptr;
    
    /* Pattern 4: Array access with variable index in loop */
    int sum = 0;
    for (volatile int i = 0; i < 4; i++) {
        /* Each iteration has different MEM address */
        sum += array[(idx + i) & 0xF];
    }
    
    use_int(value + value2 + sum);
    return *short_ptr + value + sum;
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
int generate_combined_patterns(void) {
    int result = 0;
    
    /* Switch with volatile control */
    switch (volatile_input & 0x3) {
        case 0: {
            /* ZERO_EXTRACT in case 0 */
            union {
                unsigned long full;
                struct {
                    unsigned int low;
                    unsigned int high;
                } words;
            } u;
            u.full = (unsigned long)volatile_input * 0x12345678;
            result = u.words.high & 0xFFFF;  /* ZERO_EXTRACT */
            break;
        }
        case 1: {
            /* STRICT_LOW_PART in case 1 */
            struct {
                unsigned char a;
                unsigned char b;
                unsigned short c;
            } s;
            int temp = volatile_input * 31;
            s.a = temp & 0xFF;      /* STRICT_LOW_PART */
            s.b = (temp >> 8) & 0xFF;
            s.c = volatile_short;
            result = s.a + s.b + s.c;
            break;
        }
        case 2: {
            /* SUBREG and MEM in case 2 */
            int buffer[8];
            for (int i = 0; i < 8; i++) {
                buffer[i] = volatile_input + i * 5;
            }
            
            /* Access through different type pointer */
            char *char_ptr = (char*)buffer;
            char_ptr += volatile_index * sizeof(int);
            
            /* SUBREG access */
            short *short_in_char = (short*)char_ptr;
            result = *short_in_char;
            break;
        }
        default: {
            /* All patterns combined in default */
            union {
                int i;
                short s[2];
            } u;
            u.i = volatile_input * 37;
            
            /* STRICT_LOW_PART for short assignment */
            u.s[0] = volatile_short;
            
            /* MEM with complex address */
            int *ptr = &u.i;
            ptr = (int*)((char*)ptr + 1);
            result = u.s[0] + (*ptr & 0xFF);
            break;
        }
    }
    
    /* Loop with combined operations */
    for (volatile int i = 0; i < 3; i++) {
        /* Mix patterns in loop body */
        if (i == 0) {
            /* ZERO_EXTRACT in loop */
            unsigned int val = volatile_input * (i + 2);
            result += (val & 0xFF00) >> 8;
        } else if (i == 1) {
            /* STRICT_LOW_PART in loop */
            int temp = result;
            char *low = (char*)&temp;
            *low = volatile_char;
            result = temp;
        } else {
            /* MEM with SUBREG in loop */
            int local[4] = {1, 2, 3, 4};
            short *sp = (short*)&local[volatile_index & 0x3];
            result += *sp;
        }
    }
    
    use_int(result);
    return result;
}

/* Function 5: Deeply nested patterns */
__attribute__((noinline))
int generate_deep_patterns(void) {
    /* Multi-dimensional array for complex addressing */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = volatile_input + i * 4 + j;
        }
    }
    
    /* Complex pointer chain */
    int (*row_ptr)[4] = &matrix[volatile_index & 0x3];
    int *elem_ptr = (int*)((char*)row_ptr + sizeof(int) * (volatile_index & 0x3));
    
    /* Access through multiple indirections */
    int value = *elem_ptr;
    
    /* Type punning with the value */
    union {
        int i;
        struct {
            short low;
            short high;
        } s;
    } u;
    u.i = value;
    
    /* STRICT_LOW_PART assignment */
    u.s.low = volatile_short;
    
    /* ZERO_EXTRACT from the modified value */
    unsigned int extracted = (u.i & 0x00FF0000) >> 16;
    
    /* More complex MEM access */
    int *alias_ptr = (int*)&u.s.high;  /* Points to high short */
    *alias_ptr = extracted;           /* Actually modifies u.i */
    
    use_int(u.i + value);
    return u.i + extracted;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call each pattern generator */
    checksum += generate_zero_extract();
    checksum += generate_strict_low_part();
    checksum += generate_subreg_mem();
    checksum += generate_combined_patterns();
    checksum += generate_deep_patterns();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Final result (volatile): %d\n", final_result);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}

/* External function definitions (in separate file normally) */
void use_int(int x) {
    /* Empty - just to prevent optimization */
    volatile static int sink;
    sink = x;
}

void use_short(short x) {
    volatile static short sink;
    sink = x;
}

void use_ptr(void* x) {
    volatile static void* sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}
