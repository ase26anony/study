/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
volatile int g_volatile_int = 42;
volatile short g_volatile_short = 1234;
volatile char g_volatile_char = 'A';
volatile int g_volatile_index = 3;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
int zero_extract_pattern(void) {
    /* Union with bitfields to encourage ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low8 : 8;
            uint32_t mid8 : 8;
            uint32_t high16 : 16;
        } bits;
    } u;
    
    /* Volatile read to prevent constant propagation */
    u.full = g_volatile_int;
    
    /* Extract different bitfields - should generate ZERO_EXTRACT */
    int result = 0;
    result += u.bits.low8;      /* Low 8 bits */
    result += u.bits.mid8;      /* Next 8 bits */
    result += u.bits.high16;    /* High 16 bits */
    
    /* Manual extraction with masking/shifting */
    uint32_t temp = g_volatile_int;
    result += (temp >> 4) & 0xF;    /* Extract bits 4-7 */
    result += (temp >> 16) & 0xFF;  /* Extract bits 16-23 */
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
int strict_low_part_pattern(void) {
    /* Structure with small integer members */
    struct {
        char low_byte;
        short low_word;
        int full;
    } s;
    
    /* Initialize with volatile values */
    s.full = g_volatile_int;
    
    /* Modify only low parts - should generate STRICT_LOW_PART */
    int temp = g_volatile_int;
    s.low_byte = temp & 0xFF;           /* Modify only low byte */
    s.low_word = temp & 0xFFFF;         /* Modify only low word */
    
    /* Pointer-based low part modification */
    int* ptr = &s.full;
    char* byte_ptr = (char*)ptr;
    byte_ptr[0] = g_volatile_char;      /* Modify first byte only */
    
    /* Union for type punning */
    union {
        int i;
        struct {
            char b0, b1, b2, b3;
        } bytes;
    } u;
    u.i = g_volatile_int;
    u.bytes.b1 = g_volatile_char;       /* Modify only byte 1 */
    
    use_int(s.full + u.i);
    return s.full + u.i;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
int subreg_mem_pattern(void) {
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * g_volatile_int;
    }
    
    /* Complex pointer arithmetic - should generate SUBREG and MEM */
    volatile int idx = g_volatile_index;
    
    /* Access through different pointer types */
    char* char_ptr = (char*)array;
    short* short_ptr = (short*)(char_ptr + idx * sizeof(int));
    int* int_ptr = (int*)((char*)array + idx * 2);
    
    /* Perform accesses that should generate SUBREG */
    short half_word = *short_ptr;           /* Read as short */
    *short_ptr = g_volatile_short;          /* Write as short */
    
    /* Access with offset calculation */
    int offset = idx * 3;
    int* offset_ptr = (int*)((char*)array + offset);
    int value = *offset_ptr;
    *offset_ptr = g_volatile_int;
    
    /* Nested pointer dereference */
    int** ptr_to_ptr = &offset_ptr;
    int deref_value = **ptr_to_ptr;
    
    use_int(value + deref_value + half_word);
    return value + deref_value + half_word;
}

/* Function 4: Combined pattern with control flow */
__attribute__((noinline))
int combined_pattern_with_flow(void) {
    int result = 0;
    volatile int limit = g_volatile_int & 0x7;  /* Limit 0-7 */
    
    /* Loop with varying patterns */
    for (int i = 0; i < limit; i++) {
        /* ZERO_EXTRACT in loop */
        uint32_t val = g_volatile_int + i;
        result += (val >> (i * 2)) & 0x3;  /* Extract 2 bits at varying positions */
        
        /* STRICT_LOW_PART in loop */
        struct {
            int data;
            char low;
        } s;
        s.data = val;
        s.low = (val + i) & 0xFF;  /* Modify only low byte */
        result += s.data;
        
        /* Conditional MEM access */
        if (i & 1) {
            /* Access memory with pointer arithmetic */
            int buffer[8];
            for (int j = 0; j < 8; j++) {
                buffer[j] = j * g_volatile_int;
            }
            
            short* bp = (short*)buffer;
            bp[i] = g_volatile_short;  /* Write as short */
            result += bp[i];
        }
    }
    
    /* Switch statement with different patterns */
    switch (g_volatile_int & 0x3) {
        case 0: {
            /* ZERO_EXTRACT case */
            union {
                uint64_t full;
                uint32_t halves[2];
            } u;
            u.full = g_volatile_int * 100LL;
            result += u.halves[0] & 0xFFFF;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART case */
            int temp = g_volatile_int;
            char* cp = (char*)&temp;
            cp[1] = g_volatile_char;  /* Modify byte 1 only */
            result += temp;
            break;
        }
        case 2: {
            /* SUBREG/MEM case */
            int arr[4] = {1, 2, 3, 4};
            int idx = g_volatile_index & 0x3;
            short* sp = (short*)arr;
            result += sp[idx * 2];  /* Access as short */
            break;
        }
        default:
            result += g_volatile_int;
    }
    
    use_int(result);
    return result;
}

/* Function 5: Complex addressing modes */
__attribute__((noinline))
int complex_addressing(void) {
    /* Multi-dimensional array with complex indexing */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 10 + j + g_volatile_int;
        }
    }
    
    /* Complex address calculation */
    volatile int row = g_volatile_index & 0x3;
    volatile int col = (g_volatile_int >> 2) & 0x3;
    
    /* Should generate MEM with complex address */
    int* elem_ptr = &matrix[row][col];
    int value = *elem_ptr;
    
    /* Pointer chain */
    int** ptr1 = &elem_ptr;
    int*** ptr2 = &ptr1;
    value += ***ptr2;
    
    /* Access through byte pointer with offset */
    char* base = (char*)matrix;
    int offset = (row * 4 + col) * sizeof(int);
    int* offset_ptr = (int*)(base + offset);
    value += *offset_ptr;
    
    /* Type punning with different sizes */
    struct {
        int a;
        short b;
        char c;
    } packed;
    
    packed.a = g_volatile_int;
    packed.b = g_volatile_short;
    packed.c = g_volatile_char;
    
    /* Access individual members - should generate SUBREG */
    short* b_ptr = &packed.b;
    value += *b_ptr;
    
    use_int(value);
    return value;
}

/* Main function to execute all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    checksum += zero_extract_pattern();
    checksum += strict_low_part_pattern();
    checksum += subreg_mem_pattern();
    checksum += combined_pattern_with_flow();
    checksum += complex_addressing();
    
    printf("Checksum: %d\n", checksum);
    printf("All patterns executed.\n");
    
    return 0;
}

/* Dummy definitions for external functions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
