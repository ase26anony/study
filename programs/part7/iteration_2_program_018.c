/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int g_volatile_int = 42;
static volatile short g_volatile_short = 7;
static volatile char g_volatile_char = 3;
static volatile long g_volatile_long = 123456;

/* Global arrays for memory access patterns */
static int g_array[256];
static short g_short_array[512];
static char g_char_array[1024];

/* Function 1: ZERO_EXTRACT patterns using bitfields and unions */
__attribute__((noinline)) 
static int test_zero_extract(void) {
    int result = 0;
    
    /* Pattern 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low : 8;
            uint32_t mid : 12;
            uint32_t high : 12;
        } bits;
    } u;
    
    u.full = g_volatile_int;
    result += u.bits.mid;  /* Should generate ZERO_EXTRACT for mid field */
    result += u.bits.high; /* Should generate ZERO_EXTRACT for high field */
    
    /* Pattern 2: Manual bit extraction with volatile */
    volatile uint32_t v = g_volatile_int;
    uint32_t extracted = (v >> 8) & 0xFFF;  /* Extract bits 8-19 */
    result += extracted;
    
    /* Pattern 3: Nested bitfield in struct */
    struct {
        struct {
            unsigned a : 4;
            unsigned b : 4;
            unsigned c : 8;
        } inner;
        unsigned d : 16;
    } s;
    
    s.inner.a = g_volatile_char & 0xF;
    s.inner.c = g_volatile_char;
    result += s.inner.c;
    
    use_int(result);
    return result;
}

/* Function 2: STRICT_LOW_PART patterns */
__attribute__((noinline))
static int test_strict_low_part(void) {
    int result = 0;
    
    /* Pattern 1: Modify low part through pointer */
    int temp = g_volatile_int;
    char *byte_ptr = (char*)&temp;
    *byte_ptr = g_volatile_char;  /* Modify only low 8 bits */
    result += temp;
    
    /* Pattern 2: Struct with small member assignment */
    struct {
        short low;
        int high;
    } data;
    
    data.high = g_volatile_int;
    data.low = g_volatile_short;  /* Should generate STRICT_LOW_PART */
    result += data.low;
    
    /* Pattern 3: Union for partial modification */
    union {
        uint32_t full;
        uint16_t parts[2];
    } u2;
    
    u2.full = g_volatile_int;
    u2.parts[0] = g_volatile_short;  /* Modify low 16 bits only */
    result += u2.full;
    
    use_int(result);
    return result;
}

/* Function 3: SUBREG and MEM_P patterns */
__attribute__((noinline))
static int test_subreg_mem(void) {
    int result = 0;
    volatile int idx = g_volatile_int % 128;
    
    /* Pattern 1: Type punning with pointer arithmetic */
    int *int_ptr = &g_array[idx];
    short *short_ptr = (short*)((char*)int_ptr + 2);
    *short_ptr = g_volatile_short;  /* MEM with SUBREG addressing */
    result += *short_ptr;
    
    /* Pattern 2: Array access with different types */
    char *base = (char*)g_short_array;
    int offset = g_volatile_char * 2;
    short *ptr = (short*)(base + offset);
    result += *ptr;
    
    /* Pattern 3: Complex addressing mode */
    long *long_ptr = (long*)(g_char_array + g_volatile_char * 8);
    *long_ptr = g_volatile_long;
    result += (int)(*long_ptr);
    
    /* Pattern 4: Nested pointer dereference */
    void **ptr_array = (void**)g_array;
    ptr_array[0] = &g_volatile_int;
    int **int_ptr_ptr = (int**)ptr_array;
    result += **int_ptr_ptr;
    
    use_int(result);
    return result;
}

/* Function 4: Combined patterns in control flow */
__attribute__((noinline))
static int test_combined_patterns(void) {
    int result = 0;
    volatile int selector = g_volatile_int;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        switch ((selector + i) % 3) {
            case 0: {
                /* ZERO_EXTRACT in loop */
                union {
                    uint32_t val;
                    struct {
                        uint32_t a : 5;
                        uint32_t b : 10;
                        uint32_t c : 17;
                    } bits;
                } u;
                u.val = g_volatile_int + i;
                result += u.bits.b;  /* ZERO_EXTRACT */
                break;
            }
            case 1: {
                /* STRICT_LOW_PART in loop */
                short *sptr = (short*)&g_array[i];
                *sptr = g_volatile_short + i;  /* May generate STRICT_LOW_PART */
                result += *sptr;
                break;
            }
            case 2: {
                /* SUBREG/MEM in loop */
                char *base = (char*)g_array;
                int *ptr = (int*)(base + i * sizeof(int) + 1);
                result += *ptr & 0xFF;  /* Complex MEM access */
                break;
            }
        }
    }
    
    /* Conditional with memory pattern */
    if (selector > 100) {
        /* Create SUBREG pattern */
        struct {
            int a;
            short b;
        } s;
        int *alias = (int*)&s.b;
        *alias = g_volatile_int;  /* Aliasing through different type */
        result += s.b;
    } else {
        /* Create ZERO_EXTRACT pattern */
        uint32_t mask = 0x00FF00FF;
        uint32_t val = g_volatile_int;
        result += (val & mask) | ((val >> 8) & mask);
    }
    
    use_int(result);
    return result;
}

/* Function 5: Complex nested patterns */
__attribute__((noinline))
static int test_complex_nested(void) {
    volatile int idx1 = g_volatile_int % 64;
    volatile int idx2 = g_volatile_char % 32;
    
    /* Nested union/struct for multiple patterns */
    union {
        struct {
            uint32_t field1 : 12;
            uint32_t field2 : 8;
            uint32_t field3 : 12;
        } bits;
        uint32_t words[2];
        uint16_t halves[4];
    } complex;
    
    complex.bits.field1 = g_volatile_int & 0xFFF;
    complex.bits.field2 = g_volatile_char;
    
    /* Access through different views */
    uint16_t *half_ptr = &complex.halves[idx1 % 4];
    *half_ptr = g_volatile_short;  /* Could be STRICT_LOW_PART or SUBREG */
    
    /* Pointer chain for MEM_P */
    void **ptr1 = (void**)&g_array[idx1];
    void **ptr2 = (void**)ptr1;
    int *final = (int*)*ptr2;
    
    int result = complex.bits.field1 + complex.bits.field2 + *half_ptr;
    if (final) result += *final;
    
    use_int(result);
    return result;
}

/* Main function to execute all tests */
int main(void) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 512; i++) {
        g_short_array[i] = i * 2;
    }
    for (int i = 0; i < 1024; i++) {
        g_char_array[i] = i % 256;
    }
    
    /* Run all pattern tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg_mem();
    checksum += test_combined_patterns();
    checksum += test_complex_nested();
    
    /* Print checksum to ensure execution */
    printf("Resource pattern test checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy definitions for external functions (in same file for testing,
   but should be in separate file for proper compilation) */
void use_int(int x) { g_array[0] += x; }
void use_short(short x) { g_short_array[0] += x; }
void use_ptr(void* x) { if (x) g_array[1] = 1; }
void use_long(long x) { g_array[2] += (int)x; }
