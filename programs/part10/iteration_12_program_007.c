/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ===== ZERO_EXTRACT patterns ===== */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

struct bitfield_large {
    unsigned long long high:32;
    unsigned long long low:32;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_pack bf1 = {0};
    struct bitfield_large bf2 = {0};
    VOLATILE_VAR unsigned int temp = 0x12345678;
    unsigned int result = 0;
    
    /* Direct bitfield assignments (may generate ZERO_EXTRACT) */
    bf1.flag = (temp >> 3) & 0x7;
    bf1.value = (temp >> 8) & 0x1F;
    bf1.mode = (temp >> 16) & 0xF;
    
    /* Bitfield extraction */
    result |= bf1.flag;
    result |= (bf1.value << 3);
    result |= (bf1.mode << 8);
    
    /* Large bitfield operations */
    bf2.high = (temp >> 16);
    bf2.low = temp & 0xFFFF;
    
    /* Explicit bit extraction that may generate ZERO_EXTRACT */
    unsigned int extracted = (temp >> 4) & 0xFFF;  /* 12-bit extraction */
    result ^= extracted;
    
    /* Multiple extractions */
    unsigned int ext1 = (temp >> 0) & 0x3;   /* 2 bits */
    unsigned int ext2 = (temp >> 10) & 0x3F; /* 6 bits */
    unsigned int ext3 = (temp >> 20) & 0xFF; /* 8 bits */
    
    result += ext1 + ext2 + ext3;
    
    /* Compound extraction */
    unsigned int compound = ((temp & 0xFF00) >> 8) | ((temp & 0xFF) << 8);
    result ^= compound;
    
    return result;
}

/* ===== STRICT_LOW_PART patterns ===== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned short half_reg = 0;
    VOLATILE_VAR unsigned char byte_reg = 0;
    unsigned int result = 0;
    
    /* Byte store into integer (may generate STRICT_LOW_PART) */
    unsigned char* byte_ptr = (unsigned char*)&wide_reg;
    byte_ptr[0] = 0xAA;  /* Modify low byte only */
    byte_ptr[1] = 0xBB;  /* Modify second byte */
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x12345678;
    pun.bytes[0] = 0xFF;  /* Low byte store */
    result = pun.full;
    
    /* Explicit truncation that must preserve high bits */
    unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* Explicit low part extraction */
    result += low_byte;
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %b0\n\t"          /* Copy low byte */
        "movw %w1, %w0\n\t"          /* Copy low word */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    result ^= asm_out;
    
    /* Multiple low-part operations */
    for (int i = 0; i < 4; i++) {
        unsigned char* ptr = (unsigned char*)&wide_reg + i;
        *ptr = (unsigned char)(result + i);  /* Individual byte stores */
    }
    
    /* Arithmetic with explicit low-part preservation */
    wide_reg = (wide_reg & ~0xFF) | (result & 0xFF);
    
    return result ^ wide_reg;
}

/* ===== SUBREG patterns ===== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR v4si vec_int = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    unsigned int result = 0;
    
    /* Vector element extraction (generates SUBREG) */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    result += elem0 + elem2;
    
    /* Type punning through unions */
    union {
        float f;
        int i;
        short s[2];
    } converter;
    
    converter.f = 3.14159f;
    result ^= converter.i;  /* SUBREG for the integer view */
    
    /* Mixed-size operations */
    short short_val = 0x1234;
    int int_val = short_val;  /* Sign extension may use SUBREG */
    result += int_val;
    
    /* Explicit casts between different sizes */
    long long big_val = 0x123456789ABCDEF0LL;
    int small_part = (int)big_val;  /* Truncation */
    result ^= small_part;
    
    /* Vector lane operations */
    vec_short[3] = (short)result;  /* SUBREG store */
    result += vec_short[3];
    
    /* Float/int reinterpretation */
    float f = (float)result;
    int* int_ptr = (int*)&f;
    result ^= *int_ptr;
    
    /* Complex vector extraction */
    for (int i = 0; i < 4; i++) {
        result += vec_int[i] * i;
    }
    
    return result;
}

/* ===== Complex memory operand patterns ===== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE_VAR int array[256];
    VOLATILE_VAR int* ptr_array[16];
    VOLATILE_VAR struct nested nodes[8];
    
    /* Initialize structures */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = &array[i * 8];
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        nodes[i].next = (i < 7) ? &nodes[i + 1] : NULL;
    }
    
    unsigned int result = 0;
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    int** double_ptr = (int**)malloc(sizeof(int*));
    *double_ptr = &array[64];
    *triple_ptr = double_ptr;
    
    result += ***triple_ptr;  /* Complex memory address */
    
    /* Non-constant array indexing */
    VOLATILE_VAR int idx = get_index() % 256;
    result += array[idx];           /* Variable index */
    result += array[idx + 1];       /* Offset from variable index */
    result += array[idx * 2];       /* Scaled index */
    
    /* Structure field chasing */
    struct nested* current = &nodes[0];
    for (int i = 0; i < 4 && current != NULL; i++) {
        result += current->data[i % 4];  /* Structure field access */
        current = current->next;          /* Pointer chasing */
    }
    
    /* Pointer arithmetic with multiple dereferences */
    int* volatile_ptr = ptr_array[global_index % 16];
    result += volatile_ptr[0];
    result += volatile_ptr[1];
    result += *(volatile_ptr + 2);
    
    /* Complex address calculation */
    result += *(array + (idx & 0x7F) + 32);  /* Combined offset */
    
    /* Memory operations that must be preserved */
    VOLATILE_VAR int* mem_ptr = &array[128];
    asm volatile ("" : "+m" (*mem_ptr));  /* Memory barrier */
    
    /* Cleanup */
    free(double_ptr);
    free(triple_ptr);
    
    return result;
}

/* ===== Main test driver ===== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Test each pattern */
    checksum += test_zero_extract();
    printf("Zero-extract test completed\n");
    
    checksum += test_strict_low_part();
    printf("Strict-low-part test completed\n");
    
    checksum += test_subreg();
    printf("Subreg test completed\n");
    
    checksum += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}
