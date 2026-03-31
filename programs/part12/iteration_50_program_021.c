/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;  /* This should trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_ll_t;
typedef _Bool scalar_bool_t;

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
    long long ll;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void (*func_ptr_t)(void);
typedef const char* const* string_ptr_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10_t[10];
typedef struct Point point_array_5_t[5];
typedef char char_matrix_3x3_t[3][3];
typedef union Data data_array_8_t[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, char*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes that might trigger special handling */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int id;
    char* name;
    struct Point* points;
    int point_count;
    union Data* data;
    comparator_t compare_func;
    int_array_10_t buffer;
    struct ComplexType* next;
} ComplexType_t;

/* Function declarations using these types */
void process_point(struct Point* p);
int compare_points(const void* a, const void* b);
union Data create_data(int type, void* value);
ComplexType_t* create_complex_type(const char* name, int count);

#endif /* TEST_TYPES_H */
