/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

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
    double area;
};

typedef struct {
    int id;
    string_t name;
    float score;
} Student;

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated as user struct) */
typedef struct Point Point_t;
typedef struct Rectangle Rect_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long int_val;
    double float_val;
    void* ptr_val;
    struct Point point_val;
};

typedef union Data Data_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void (*func_ptr_t)(void);
typedef int (*int_func_ptr_t)(int, int);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef float matrix_3x3[3][3];
typedef char string_array[4][64];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef string_t (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int id;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
    struct ComplexType* next;
    void (*processor)(struct ComplexType*);
    int values[8];
} ComplexType_t;

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_students(const void* a, const void* b);
Data_t create_data(int type, void* value);
ComplexType_t* create_complex_type(int id);

#endif /* TEST_TYPES_H */
