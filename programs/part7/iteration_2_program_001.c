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
static volatile char volatile_char = 0xEF;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
static int generate_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:16;
            unsigned int high:8;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.middle;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual masking and shifting */
    unsigned int val = volatile_seed;
    int result2 = (val >> 8) & 0xFFFF;  /* Alternative ZERO_EXTRACT pattern */
    
    /* Pattern 3: Nested bitfield extraction */
    struct {
        struct {
            unsigned int a:4;
            unsigned int b:12;
            unsigned int c:16;
        } inner;
    } s;
    
    *(unsigned int*)&s = volatile_seed;
    int result3 = s.inner.b;
    
    /* Force use of results */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static int generate_strict_low_part(void) {
    int result = 0;
    
    /* Pattern 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } data;
    
    data.rest = volatile_seed;
    data.low_byte = volatile_char;  /* Should generate STRICT_LOW_PART */
    result += data.low_byte;
    
    /* Pattern 2: Pointer to low part */
    int temp = volatile_seed;
    char *low_ptr = (char*)&temp;
    *low_ptr = volatile_char;  /* Modifies only low byte */
    result += temp & 0xFF;
    
    /* Pattern 3: Union modification */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u;
    
    u.full = volatile_seed;
    u.parts.low = volatile_short;  /* STRICT_LOW_PART of 16 bits */
    result += u.full;
    
    use_int(result);
    return result;
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
static int generate_subreg(void) {
    int result = 0;
    
    /* Pattern 1: Type punning through pointers */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Access through different type pointer */
    short *short_ptr = (short*)array;
    short_ptr[volatile_index] = volatile_short;  /* SUBREG store */
    result += short_ptr[volatile_index];         /* SUBREG load */
    
    /* Pattern 2: Complex pointer arithmetic */
    char *base = (char*)array;
    int *int_ptr = (int*)(base + volatile_index * sizeof(int));
    *int_ptr = volatile_seed;  /* MEM with address computation */
    result += *int_ptr;
    
    /* Pattern 3: Nested SUBREG accesses */
    struct {
        int a;
        int b;
        short c;
        char d;
    } compound;
    
    compound.a = volatile_seed;
    short *c_ptr = &compound.c;
    *c_ptr = volatile_short;  /* SUBREG access to struct member */
    result += compound.c;
    
    use_int(result);
    use_ptr(array);
    return result;
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
static int generate_mem_complex(void) {
    int result = 0;
    volatile int volatile_offset = 5;
    
    /* Pattern 1: Array with computed index */
    int buffer[20];
    for (int i = 0; i < 20; i++) {
        buffer[i] = volatile_seed * i;
    }
    
    /* Complex addressing mode */
    int idx = volatile_index + volatile_offset;
    buffer[idx * 2] = volatile_seed;  /* MEM with scaled index */
    result += buffer[idx * 2];
    
    /* Pattern 2: Pointer chain */
    int *ptr1 = buffer + volatile_index;
    int *ptr2 = ptr1 + volatile_offset;
    *ptr2 = volatile_seed >> 4;  /* MEM with pointer arithmetic */
    result += *ptr2;
    
    /* Pattern 3: Mixed types in memory access */
    struct mixed {
        int a;
        short b;
        char c[3];
    } m;
    
    m.a = volatile_seed;
    m.b = volatile_short;
    m.c[0] = volatile_char;
    
    /* Access through byte pointer */
    char *byte_ptr = (char*)&m;
    byte_ptr[sizeof(int) + sizeof(short)] = volatile_char + 1;
    result += m.c[0];
    
    use_int(result);
    use_ptr(buffer);
    return result;
}

/* Function 5: Combined patterns to trigger recursive calls */
__attribute__((noinline))
static int generate_combined_pattern(void) {
    /* This function combines multiple patterns to potentially trigger
       the recursive mark_referenced_resources calls */
    
    union extractor {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
        struct {
            uint8_t b0, b1, b2, b3;
        } bytes;
    } data;
    
    data.full = volatile_seed;
    
    /* ZERO_EXTRACT pattern */
    uint16_t extracted = data.words.high;  /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART pattern */
    data.bytes.b1 = volatile_char;  /* STRICT_LOW_PART */
    
    /* SUBREG and MEM patterns */
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = volatile_seed + i * 100;
    }
    
    /* Complex memory access */
    short *short_view = (short*)array;
    int idx = volatile_index & 0x3;
    short_view[idx * 2 + 1] = extracted;  /* SUBREG + MEM */
    
    /* Use all results */
    int result = data.full + array[idx] + short_view[idx * 2];
    
    use_int(result);
    use_short(extracted);
    use_ptr(array);
    
    return result;
}

/* Function with control flow variation */
__attribute__((noinline))
static int generate_with_control_flow(void) {
    int result = 0;
    
    /* Loop with pattern generation */
    for (volatile int i = 0; i < 4; i++) {
        union {
            int val;
            struct {
                short low;
                short high;
            } parts;
        } u;
        
        u.val = volatile_seed + i;
        
        /* Conditional ZERO_EXTRACT/STRICT_LOW_PART */
        if (i & 1) {
            result += u.parts.high;  /* ZERO_EXTRACT in loop */
        } else {
            u.parts.low = volatile_short + i;  /* STRICT_LOW_PART in loop */
            result += u.val;
        }
        
        /* Memory access in loop */
        int temp[4];
        temp[i] = u.val;
        result += temp[i];  /* MEM access */
    }
    
    /* Switch statement with different patterns */
    switch (volatile_index & 3) {
        case 0: {
            /* ZERO_EXTRACT pattern */
            unsigned int x = volatile_seed;
            result += (x >> 16) & 0xFFFF;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART pattern */
            int y = volatile_seed;
            *(char*)&y = volatile_char;
            result += y;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            int z[2] = {volatile_seed, volatile_seed + 1};
            short *sz = (short*)z;
            result += sz[volatile_index % 4];
            break;
        }
        default: {
            /* Complex MEM pattern */
            int *ptr = &result;
            for (int j = 0; j < 2; j++) {
                ptr[j] = volatile_seed + j;
            }
            result += ptr[1];
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Main function that calls all pattern generators */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation...\n");
    
    checksum += generate_zero_extract();
    checksum += generate_strict_low_part();
    checksum += generate_subreg();
    checksum += generate_mem_complex();
    checksum += generate_combined_pattern();
    checksum += generate_with_control_flow();
    
    printf("Final checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return 0;
}

/* Dummy external function definitions to satisfy linker */
/* These would normally be in a separate file */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
