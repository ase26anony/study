/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
int test_zero_extract(void) {
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
    
    /* Pattern 2: Manual bitfield extraction with volatile */
    unsigned int val = volatile_seed;
    int result2 = (val >> 8) & 0xFFFF;  /* Alternative ZERO_EXTRACT pattern */
    
    /* Pattern 3: Nested extraction */
    struct {
        union {
            unsigned int word;
            struct {
                unsigned int a:4;
                unsigned int b:12;
                unsigned int c:16;
            } fields;
        } inner;
    } s;
    
    s.inner.word = volatile_seed;
    int result3 = s.inner.fields.b;
    
    /* Use results to prevent elimination */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
int test_strict_low_part(void) {
    /* Pattern 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } data;
    
    int temp = volatile_seed;
    data.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Pattern 2: Pointer to low part */
    int value = volatile_seed;
    char *low_ptr = (char*)&value;
    *low_ptr = volatile_char;  /* Modify only low byte */
    
    /* Pattern 3: Union assignment to partial register */
    union {
        int full;
        struct {
            short low;
            short high;
        } halves;
    } reg;
    
    reg.full = volatile_seed;
    reg.halves.low = volatile_short;  /* Modify low half */
    
    /* Use results */
    use_char(data.low_byte);
    use_int(value);
    use_short(reg.halves.low);
    
    return data.low_byte + value + reg.halves.low;
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
int test_subreg(void) {
    /* Pattern 1: Array access with type punning */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Access through different type pointer */
    short *short_ptr = (short*)array;
    short_ptr[volatile_index] = volatile_short;  /* SUBREG in address calculation */
    
    /* Pattern 2: Nested structure with different sized members */
    struct container {
        long long big;
        int medium;
        short small;
        char tiny;
    } container;
    
    container.big = volatile_seed;
    container.medium = volatile_seed >> 8;
    container.small = volatile_short;
    container.tiny = volatile_char;
    
    /* Access through different sized pointers */
    int *medium_ptr = &container.medium;
    short *small_ptr = (short*)medium_ptr;  /* SUBREG conversion */
    
    /* Pattern 3: Complex pointer arithmetic */
    char *base = (char*)array;
    int offset = volatile_index * sizeof(int) / 2;
    short *computed_ptr = (short*)(base + offset);  /* MEM with complex address */
    *computed_ptr = volatile_short;
    
    /* Use results */
    use_short(short_ptr[volatile_index]);
    use_short(*small_ptr);
    use_short(*computed_ptr);
    
    return short_ptr[volatile_index] + *small_ptr + *computed_ptr;
}

/* Function 4: Combine all patterns with control flow */
__attribute__((noinline))
int test_combined(void) {
    int result = 0;
    
    /* Conditional based on volatile */
    if (volatile_seed & 1) {
        /* ZERO_EXTRACT in conditional path */
        union {
            unsigned int val;
            struct {
                unsigned int a:10;
                unsigned int b:10;
                unsigned int c:12;
            } parts;
        } u;
        
        u.val = volatile_seed;
        result += u.parts.b;  /* ZERO_EXTRACT */
        
        /* Followed by STRICT_LOW_PART */
        struct {
            short low;
            short high;
        } s;
        s.low = result & 0xFFFF;  /* STRICT_LOW_PART */
        result = s.low;
    } else {
        /* SUBREG and MEM in else path */
        int buffer[8];
        for (int i = 0; i < 8; i++) {
            buffer[i] = volatile_seed + i * 2;
        }
        
        /* Complex memory access */
        short *ptr = (short*)((char*)buffer + volatile_index);
        *ptr = volatile_short;  /* MEM with SUBREG address */
        result = *ptr;
    }
    
    /* Loop with pattern generation */
    for (int i = 0; i < 3; i++) {
        /* Mix patterns in loop */
        unsigned int val = volatile_seed + i;
        
        /* ZERO_EXTRACT in loop */
        int extracted = (val >> 4) & 0xFFF;
        
        /* Convert to STRICT_LOW_PART store */
        char *dest = (char*)&result + i;
        *dest = extracted & 0xFF;  /* STRICT_LOW_PART */
    }
    
    use_int(result);
    return result;
}

/* Function 5: Complex memory addressing patterns */
__attribute__((noinline))
int test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    int matrix[4][4];
    volatile int idx1 = volatile_index;
    volatile int idx2 = volatile_index + 1;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = volatile_seed + i * 4 + j;
        }
    }
    
    /* Complex address calculation */
    int *row = matrix[idx1 & 3];
    short *half_ptr = (short*)&row[idx2 & 3];  /* SUBREG conversion */
    
    /* MEM with complex address computation */
    *half_ptr = volatile_short;
    
    /* Pointer chain */
    char *byte_ptr = (char*)matrix;
    byte_ptr += idx1 * 16 + idx2 * 4;  /* Complex offset */
    int *final_ptr = (int*)byte_ptr;    /* MEM with address computation */
    
    /* ZERO_EXTRACT from memory */
    int val = *final_ptr;
    int extracted = (val >> 12) & 0xFFF;
    
    /* Store back with STRICT_LOW_PART */
    char *low_byte = (char*)final_ptr;
    *low_byte = extracted & 0xFF;
    
    use_int(*final_ptr);
    return *final_ptr + *half_ptr;
}

/* Main function to execute all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Run all test functions */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_combined();
    checksum += test_complex_mem();
    
    printf("Checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    return 0;
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void* x) { (void)x; }
#endif
