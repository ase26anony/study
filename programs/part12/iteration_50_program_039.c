/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete incomplete_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int int_t;
typedef float float_t;
typedef double double_t;
typedef char char_t;
typedef long long longlong_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string_t;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
    float z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

typedef struct {
    char name[50];
    int id;
    float score;
} Student;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
};

union Variant {
    long long int_val;
    double double_val;
    void* ptr_val;
    struct Point point_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*FuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char char_matrix[3][3];
typedef union Data data_array[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential for lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int id;
    char* name;
    struct ComplexType* next;
    union {
        int int_data;
        double double_data;
        char* string_data;
    } value;
    int (*process)(struct ComplexType*);
    int array[5];
} ComplexType;

#endif /* TEST_TYPES_H */
