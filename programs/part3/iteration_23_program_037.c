/* complex_types.h */
#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

#ifdef __GNUC__

/* For DW_AT_picture_string - decimal type (COBOL/Ada style) */
typedef int __attribute__((decimal(9,2))) decimal_type;

/* For DW_AT_small - packed structure */
struct __attribute__((packed)) small_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
};

/* For DW_AT_segment - pointer with address space */
typedef int __attribute__((address_space(1))) *segment_ptr;

/* For string length attributes - Pascal-style string */
struct pascal_string {
    int length;
    char data[];
};

/* For DW_AT_ordering - Fortran-style array */
#ifdef __GNUC__
typedef int fortran_array __attribute__((fortran_array));
#else
typedef int fortran_array;
#endif

#endif /* __GNUC__ */

#endif /* COMPLEX_TYPES_H */
