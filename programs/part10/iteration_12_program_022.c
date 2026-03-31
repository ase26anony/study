/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void escape(void*);

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_char = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int data:12;
    unsigned int footer:16;
} NOINLINE;

/* Another bitfield with different layout */
struct mixed_bitfield {
    unsigned short low:5;
    unsigned short mid:7;
    unsigned short high:4;
} NOINLINE;

NOINLINE static int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet packet;
    VOLATILE_VAR struct mixed_bitfield mixed;
    VOLATILE_VAR unsigned int raw_value = 0xABCD1234;
    int result = 0;
    
    /* Direct bitfield assignments (may generate ZERO_EXTRACT) */
    packet.header = 0x7;
    packet.data = 0xABC;
    packet.footer = 0xDEAD;
    
    /* Bitfield extraction */
    result += packet.data;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int extracted = (raw_value >> 8) & 0xFFF;  /* 12-bit extraction */
    result += extracted;
    
    /* Multiple extractions with different widths */
    extracted = (raw_value >> 4) & 0x7;   /* 3-bit extraction */
    result += extracted;
    
    extracted = (raw_value >> 16) & 0xFFFF; /* 16-bit extraction */
    result += extracted;
    
    /* Bitfield in union */
    union {
        unsigned int full;
        struct {
            unsigned int a:10;
            unsigned int b:10;
            unsigned int c:12;
        } bits;
    } u;
    
    u.full = 0x98765432;
    result += u.bits.b;
    
    /* Complex extraction pattern */
    mixed.low = (raw_value >> 0) & 0x1F;
    mixed.mid = (raw_value >> 5) & 0x7F;
    mixed.high = (raw_value >> 12) & 0xF;
    
    result += mixed.low + mixed.mid + mixed.high;
    
    return result & 0xFF;  /* Return checksum */
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE static int test_strict_low_part(void) {
    VOLATILE_VAR int int_var = 0x12345678;
    VOLATILE_VAR short short_var = 0;
    VOLATILE_VAR char char_var = 0;
    int result = 0;
    
    /* Byte store into integer (may generate STRICT_LOW_PART) */
    *(VOLATILE_VAR unsigned char*)&int_var = 0xAA;
    result += int_var;
    
    /* Another byte store */
    ((VOLATILE_VAR char*)&int_var)[1] = 0xBB;
    result += int_var;
    
    /* Union for type punning */
    union {
        int full;
        char bytes[4];
    } pun;
    
    pun.full = 0x87654321;
    pun.bytes[0] = 0x11;  /* Low byte store */
    result += pun.full;
    
    /* Truncation preserving high bits context */
    int temp = int_var;
    char_var = temp & 0xFF;  /* Explicit truncation */
    result += char_var;
    
    /* Inline assembly forcing low-part access (x86 specific) */
    #ifdef __x86_64__
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r"(char_var)
        : "r"(int_var)
        : "cc"
    );
    #elif defined(__i386__)
    asm volatile (
        "movb %%al, %0\n\t"
        : "=m"(char_var)
        : "a"(int_var)
        : "cc"
    );
    #endif
    
    result += char_var;
    
    /* Multiple low-part operations */
    short_var = int_var & 0xFFFF;
    result += short_var;
    
    /* Volatile byte access */
    VOLATILE_VAR char* byte_ptr = (VOLATILE_VAR char*)&int_var;
    byte_ptr[2] = 0xCC;
    result += int_var;
    
    return result & 0xFF;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR int scalar = 0;
    VOLATILE_VAR float float_var = 3.14159f;
    int result = 0;
    
    /* Vector element extraction (may generate SUBREG) */
    scalar = vec[0];
    result += scalar;
    
    scalar = vec[global_index % 4];  /* Non-constant index */
    result += scalar;
    
    /* Type punning between int and float */
    union {
        float f;
        int i;
    } float_int;
    
    float_int.f = float_var;
    result += float_int.i;  /* Access same bits as int */
    
    /* Short vector element access */
    short s = short_vec[3];
    result += s;
    
    /* Mixed size operations */
    long long big = 0x123456789ABCDEF0LL;
    int small = (int)big;  /* Truncation */
    result += small;
    
    /* Pointer casting for subregister access */
    int* int_ptr = &scalar;
    short* short_ptr = (short*)int_ptr;
    *short_ptr = 0x1234;
    result += scalar;
    
    /* Complex vector operation */
    v4si vec2 = vec + (v4si){5, 6, 7, 8};
    scalar = vec2[1];
    result += scalar;
    
    /* Extract high part */
    scalar = (int)(big >> 32);
    result += scalar;
    
    return result & 0xFF;
}

/* ==================== Memory operand patterns ==================== */

NOINLINE static int test_memory_operand(void) {
    /* Complex memory buffer */
    VOLATILE_VAR char buffer[256];
    VOLATILE_VAR int* int_buffer = (VOLATILE_VAR int*)buffer;
    VOLATILE_VAR void** ptr_buffer = (VOLATILE_VAR void**)buffer;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        int_buffer[i] = i * 0x01010101;
    }
    
    int result = 0;
    
    /* Multi-level pointer dereferencing */
    VOLATILE_VAR int*** triple_ptr = (VOLATILE_VAR int***)&ptr_buffer;
    VOLATILE_VAR int** double_ptr = (VOLATILE_VAR int**)(buffer + 16);
    VOLATILE_VAR int* single_ptr = (VOLATILE_VAR int*)(buffer + 32);
    
    *single_ptr = 0xDEADBEEF;
    result += **double_ptr;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index;
    result += int_buffer[idx % 64];
    result += int_buffer[(idx + 1) % 64];
    result += int_buffer[(idx + 2) % 64];
    
    /* Structure with nested arrays */
    struct nested {
        int data[4][4];
        char* ptr;
    } NOINLINE;
    
    VOLATILE_VAR struct nested nested_struct;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nested_struct.data[i][j] = i * 4 + j;
        }
    }
    
    /* Complex addressing mode */
    result += nested_struct.data[1][2];
    result += nested_struct.data[idx % 4][(idx + 1) % 4];
    
    /* Pointer chasing */
    VOLATILE_VAR char* ptr = buffer;
    for (int i = 0; i < 3; i++) {
        result += *ptr;
        ptr += 16;
    }
    
    /* Volatile memory operation */
    *(VOLATILE_VAR int*)(buffer + 128) = 0xCAFEBABE;
    result += *(VOLATILE_VAR int*)(buffer + 128);
    
    /* Function pointer array dereference */
    int (*func_array[4])(void) = {
        test_zero_extract,
        test_strict_low_part,
        test_subreg,
        test_memory_operand
    };
    
    /* Call through function pointer (complex address calculation) */
    if (global_index < 4) {
        result += func_array[global_index]();
    }
    
    return result & 0xFF;
}

/* ==================== Main test driver ==================== */

int main(void) {
    int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all tests multiple times with different global_index values */
    for (global_index = 0; global_index < 4; global_index++) {
        total += test_zero_extract();
        total += test_strict_low_part();
        total += test_subreg();
        total += test_memory_operand();
    }
    
    printf("Total checksum: %d\n", total);
    printf("Test completed.\n");
    
    return total & 0xFF;
}

/* Dummy external functions */
int get_index(void) {
    return global_index;
}

void escape(void* ptr) {
    global_ptr = ptr;
}
