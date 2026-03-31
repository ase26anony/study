/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int_t;
typedef float my_float_t;
typedef double my_double_t;
typedef char my_char_t;
typedef long long my_longlong_t;
typedef unsigned int my_uint_t;
typedef _Bool my_bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* const const_string_ptr;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

/* TYPE_USER_STRUCT: Typedef'd struct */
typedef struct {
    int data;
    char name[32];
} NamedData;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long ll;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int* IntPtr;
typedef void* GenericPtr;
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][4];
typedef void* PtrArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (may trigger lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    union Data data;
    struct Point position;
    int (*operation)(struct ComplexType*);
    void* extra_data;
    char name[64];
} ComplexType;

/* Function declarations using various types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
string_t generate_string(void);
void handle_error(const char* message, int code);

#endif /* TEST_TYPES_H */
