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

/* ========== ZERO_EXTRACT patterns ========== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int payload:12;
    unsigned int footer:8;
    unsigned int checksum:8;
} NOINLINE;

/* Another bitfield with non-byte boundaries */
struct weird_bitfield {
    unsigned int a:3;
    unsigned int b:5;
    unsigned int c:7;
    unsigned int d:9;
    unsigned int e:8;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_packet packet;
    struct weird_bitfield weird;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize with volatile to prevent constant folding */
    packet.header = global_index & 0xF;
    packet.payload = (global_index * 37) & 0xFFF;
    packet.footer = (global_index + 123) & 0xFF;
    packet.checksum = (packet.header + packet.payload + packet.footer) & 0xFF;
    
    weird.a = global_index & 0x7;
    weird.b = (global_index >> 3) & 0x1F;
    weird.c = (global_index * 13) & 0x7F;
    weird.d = (global_index * 71) & 0x1FF;
    weird.e = (global_index * 211) & 0xFF;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    unsigned int combined = 0;
    combined |= (packet.header << 28);
    combined |= (packet.payload << 16);
    combined |= (packet.footer << 8);
    combined |= packet.checksum;
    
    /* Explicit bit extraction that may generate ZERO_EXTRACT */
    unsigned int mask = 0xFF;
    unsigned int shift = 8;
    unsigned int extracted = (combined >> shift) & mask;
    
    /* Multiple extractions with varying widths */
    unsigned int wide_extract = (combined >> 4) & 0x3FF;  /* 10-bit extract */
    unsigned int narrow_extract = (combined >> 20) & 0xF; /* 4-bit extract */
    
    /* Bitfield copy */
    struct bitfield_packet packet2;
    packet2 = packet;  /* This may generate ZERO_EXTRACT for field copies */
    
    result = extracted + wide_extract + narrow_extract + 
             weird.c + packet2.payload;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */

NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int value = 0x12345678;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    unsigned char* byte_ptr = (unsigned char*)&value;
    byte_ptr[0] = 0xAA;  /* Modify low byte */
    byte_ptr[1] = 0xBB;  /* Modify second byte */
    
    /* Union for type punning - may generate low-part accesses */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0xDEADBEEF;
    pun.bytes[0] = global_index & 0xFF;  /* Low byte store */
    
    /* Truncation operations that preserve high bits */
    unsigned int temp = value;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    temp = (temp & ~0xFF) | (low_byte + 1);  /* Modify only low byte */
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %b0\n\t"          /* Move low byte */
        "movw %w1, %w0\n\t"          /* Move low word */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* Arithmetic that only affects low part */
    unsigned int x = 0xFFFFFFFF;
    x = (x & ~0xFF) | ((x + 1) & 0xFF);  /* Increment only low byte */
    
    result = value + pun.full + temp + asm_out + x;
    return result;
}

/* ========== SUBREG patterns ========== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR unsigned int result = 0;
    
    /* Vector operations with element extraction */
    v4si vec = {1, 2, 3, 4};
    v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Extract individual elements - may generate SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    short selem = short_vec[3];
    
    /* Type punning between different sizes */
    float f = 3.14159f;
    unsigned int ifloat;
    memcpy(&ifloat, &f, sizeof(f));  /* Type punning via memcpy */
    
    /* Explicit casts between different integer sizes */
    unsigned long long big = 0x123456789ABCDEF0ULL;
    unsigned int lower = (unsigned int)big;          /* Truncation */
    unsigned short shorter = (unsigned short)big;    /* Further truncation */
    
    /* Mixed-size arithmetic */
    unsigned char c = 100;
    unsigned int i = c * 256;  /* Promotion and multiplication */
    
    /* Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } mix;
    mix.c = 'A';
    mix.s = 12345;
    mix.i = 0xDEADBEEF;
    mix.ll = 0x123456789ABCDEF0ULL;
    
    /* Access different parts */
    short s_from_mix = mix.s;  /* May involve SUBREG */
    int i_from_mix = mix.i;
    
    result = elem0 + elem2 + selem + ifloat + lower + 
             shorter + i + s_from_mix + i_from_mix;
    return result;
}

/* ========== Complex Memory Operand patterns ========== */

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE_VAR unsigned int result = 0;
    
    /* Create complex memory addressing structures */
    int array1[100];
    int array2[100];
    int* ptr_array[10];
    
    /* Initialize with volatile indices */
    for (VOLATILE_VAR int i = 0; i < 100; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
    }
    
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &array1[i * 10];
    }
    
    /* Complex pointer chasing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    int** double_ptr = (int**)malloc(sizeof(int*));
    *double_ptr = array2;
    *triple_ptr = double_ptr;
    
    /* Multi-level dereferencing */
    int val1 = ***triple_ptr;  /* Triple pointer dereference */
    int val2 = **(ptr_array[global_index % 10] + 5);  /* Array + offset */
    
    /* Structure with nested arrays */
    struct nested {
        int data[5][5];
        int* ptrs[3];
    } nested_struct;
    
    /* Initialize nested structure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            nested_struct.data[i][j] = i * 10 + j;
        }
    }
    
    nested_struct.ptrs[0] = array1;
    nested_struct.ptrs[1] = array2;
    nested_struct.ptrs[2] = (int*)&nested_struct;
    
    /* Complex addressing with structure fields */
    int val3 = nested_struct.data[2][3];
    int val4 = nested_struct.ptrs[1][global_index % 50];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = array1;
    int val5 = volatile_ptr[10];  /* Volatile pointer dereference */
    
    /* Pointer arithmetic with non-constant offset */
    int offset = get_index() % 20;
    int* base_ptr = array2;
    int val6 = *(base_ptr + offset);  /* Non-constant offset */
    int val7 = *(base_ptr + offset + 5);  /* More complex offset */
    
    /* Cleanup */
    free(double_ptr);
    free(triple_ptr);
    
    result = val1 + val2 + val3 + val4 + val5 + val6 + val7;
    return result;
}

/* ========== Main function ========== */

int main(void) {
    unsigned int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Initialize global variables */
    global_index = 42;
    global_ptr = malloc(100);
    
    /* Run all tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    /* Cleanup */
    free(global_ptr);
    
    printf("Total checksum: %u\n", total);
    printf("All tests completed successfully\n");
    
    return 0;
}

/* Dummy external functions */
int get_index(void) {
    return global_index * 3 + 7;
}

void* get_ptr(void) {
    return global_ptr;
}
