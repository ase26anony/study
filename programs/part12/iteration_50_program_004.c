/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr_t;

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

typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated as user struct) */
typedef struct {
    int id;
    char name[50];
    float salary;
} Employee;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    struct Point point_val;
};

typedef union Data Data_t;
typedef union Variant Variant_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void* generic_ptr_t;
typedef int (*func_ptr_t)(void);
typedef const struct Rectangle* ConstRectPtr;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char string_array[3][50];
typedef int* pointer_array[20];
typedef int (*func_ptr_array[5])(int, int);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, void*);
typedef char* (*string_processor)(const char*);
typedef int (*binary_op)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (might trigger lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
struct ComplexType {
    int scalar_field;
    char* string_field;
    struct Point point_field;
    union Data data_field;
    int (*callback_field)(int, int);
    struct ComplexType* next;
    int array_field[10];
    struct {
        int nested_scalar;
        char nested_string[20];
    } anonymous_field;
};

/* Enumeration type (might be classified as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color;

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
char* process_string(const char* input);
void register_callback(callback_func func);

#endif /* TEST_TYPES_H */
