#ifndef DWARF_ATTRIBUTES_H
#define DWARF_ATTRIBUTES_H

#include <optional>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// For DW_AT_prototyped - old K&R style prototype
int old_style_proto();  // Declaration without parameter types

// For DW_AT_picture_string - simulate COBOL-like type
typedef struct {
    char data[100];
    int scale;
    int decimal;
} PICTURE_TYPE;

// For DW_AT_string_length attributes
typedef struct string_with_length {
    size_t length;
    char data[];  // Flexible array member
} string_with_length_t;

// For DW_AT_ordering - enum with non-sequential values
enum reordered_enum {
    ENUM_A = 10,
    ENUM_B = 5,
    ENUM_C = 20,
    ENUM_D = 1
};

#ifdef __cplusplus
}
#endif

#endif // DWARF_ATTRIBUTES_H
