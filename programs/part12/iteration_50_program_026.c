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
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

/* Nested structure */
struct Employee {
    char name[50];
    int age;
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
typedef int* int_ptr_t;
typedef char** string_array_t;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char string_array_3[3][50];
typedef double matrix_4x4[4][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, const char*);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
    double d;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Complex type combining multiple categories */
struct ComplexType {
    /* Scalar fields */
    int id;
    float score;
    
    /* String field */
    char* name;
    
    /* Array field */
    int values[10];
    
    /* Nested struct */
    struct Point position;
    
    /* Union field */
    union Data data;
    
    /* Pointer field */
    struct ComplexType* next;
    
    /* Callback field */
    comparator_t compare_func;
    
    /* Array of structs */
    struct Point points[5];
};

/* Forward declaration for linked list */
struct ListNode;

/* Self-referential structure */
struct ListNode {
    int data;
    struct ListNode* next;
    struct ListNode* prev;
};

/* Opaque pointer type */
typedef struct incomplete* OpaquePtr;

#endif /* TEST_TYPES_H */
