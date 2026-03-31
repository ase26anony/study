// dwarf_test.c - Test program to trigger specific DWARF attribute generation

// First, let's create a header with complex types
#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#ifdef __GNUC__
// For DW_AT_picture_string (decimal types)
typedef int __attribute__((mode(SD))) _Decimal32;
typedef int __attribute__((mode(DD))) _Decimal64;
typedef int __attribute__((mode(TD))) _Decimal128;

// For DW_AT_segment (address space pointers)
typedef int __attribute__((address_space(1))) *far_ptr_t;

// For DW_AT_small (packed structures)
struct __attribute__((packed)) small_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
};

// For DW_AT_string_length (Pascal-style strings)
struct pascal_string {
    int length;
    char data[];
};

// For DW_AT_string_length_bit_size/byte_size
typedef struct {
    unsigned int length_bits : 16;
    unsigned int length_bytes : 16;
    char data[256];
} sized_string_t;

// Function prototype for DW_AT_prototyped
int prototyped_function(int a, double b, char c);

// Variadic function that might trigger optional parameters
int variadic_func(int required, ...);

#endif // __GNUC__

#endif // DWARF_TEST_H
