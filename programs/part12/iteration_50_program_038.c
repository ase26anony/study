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
typedef long long long_long_t;
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

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long long as_int;
    double as_float;
    void* as_pointer;
    struct Point as_point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef int_t* int_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const_string_t* string_ptr_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef union Data data_array_3[3];
typedef char char_matrix[4][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef string_t (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* TYPE_USER_STRUCT: Might be triggered by typedef struct */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Complex nested type for thorough testing */
typedef struct ComplexType {
    int id;
    union Data data;
    struct Point position;
    int_array_10 scores;
    comparator_t compare_func;
    struct ComplexType* next;
} ComplexType;

/* Function declarations using these types */
void process_point(struct Point* p);
union Data create_data(int type);
int compare_points(const void* a, const void* b);
ComplexType* create_complex_type(int id);

#endif /* TEST_TYPES_H */
