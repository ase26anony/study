#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) base_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    struct base_struct* GTY((skip)) next;
};

/* Nested anonymous struct */
struct GTY(()) complex_struct {
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    
    /* Bit-fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
    
    /* TYPE_ARRAY within struct */
    struct base_struct* GTY(()) items[10];
    
    /* Multi-dimensional array */
    int GTY(()) matrix[3][3];
};

/* TYPE_UNION: Regular union */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
    struct base_struct* GTY(()) struct_ptr;
};

/* Tagged union within struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } GTY(()) tag;
    union {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY(()) s;
    } GTY(()) value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* GTY((skip)) user_data;
    int GTY(()) user_id;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* GTY(()) base_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) callback_func)(int, const char*);

/* Pointer to pointer */
typedef struct base_struct** GTY(()) base_ptr_ptr;

/* TYPE_STRING: String types */
typedef const char* GTY((length("strlen(%h)"))) counted_string;
typedef char* GTY((string)) simple_string;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    compare_func GTY(()) comparator;
    void* GTY(()) data;
    int GTY(()) size;
};

/* Chain of structures using chain_next/chain_prev */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_struct {
    int GTY(()) value;
    struct chain_struct* GTY(()) next;
    struct chain_struct* GTY(()) prev;
};

/* Array of pointers with length field */
struct GTY(()) array_container {
    int GTY(()) count;
    struct base_struct* GTY((length("%h.count"))) items[];
};

/* For TYPE_LANG_STRUCT - this needs C++ */
#ifdef __cplusplus
class GTY(()) cpp_base {
public:
    virtual ~cpp_base() {}
    virtual void method() = 0;
    int GTY(()) base_value;
};

class GTY(()) cpp_derived : public cpp_base {
public:
    void method() override {}
    double GTY(()) derived_value;
};
#endif

#endif /* TEST_TYPES_H */
