/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

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
    double area;
};

/* Nested structure */
struct Employee {
    char name[50];
    int id;
    struct Point location;
    double salary;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    char char_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* IntPtr;
typedef void (*VoidFuncPtr)(void);
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef char CharMatrix[3][3];
typedef union Data DataArray[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, char*);
typedef int (*BinaryOp)(int, int);
typedef char* (*StringProcessor)(const char*);

/* TYPE_LANG_STRUCT: Structure with GCC attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated differently) */
typedef struct {
    int counter;
    char* name;
} NamedCounter;

/* Complex type combinations */
struct ComplexType {
    /* Contains multiple type categories */
    int scalar_field;           /* TYPE_SCALAR */
    char* string_field;         /* TYPE_STRING */
    struct Point struct_field;  /* TYPE_STRUCT */
    union Data union_field;     /* TYPE_UNION */
    int* pointer_field;         /* TYPE_POINTER */
    int array_field[5];         /* TYPE_ARRAY */
    Comparator callback_field;  /* TYPE_CALLBACK */
};

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
char* process_string(const char* input);
void register_callback(Callback cb);

#endif /* TEST_TYPES_H */
