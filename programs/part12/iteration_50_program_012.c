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
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int id;
};

typedef struct Point Point_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
};

union Variant {
    long long int_val;
    double double_val;
    void* ptr_val;
};

typedef union Data Data_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void* generic_ptr_t;
typedef const struct Point* ConstPointPtr;
typedef int (*func_ptr_t)(void);  /* Also a callback type */

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct Point point_array_5[5];
typedef char string_array[3][64];
typedef int* pointer_array[8];
typedef int multi_dim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data;
    double precision;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* TYPE_USER_STRUCT: Typedef'd struct (might be classified differently) */
typedef struct {
    int id;
    char name[32];
} UserStruct_t;

/* Complex nested type to ensure thorough analysis */
struct ComplexType {
    int scalar_field;
    char* string_field;
    struct Point point_field;
    union Data data_field;
    int (*callback_field)(int);
    int array_field[8];
    struct ComplexType* next;
};

/* Enumeration type (should be TYPE_SCALAR) */
typedef enum {
    RED,
    GREEN,
    BLUE,
    ALPHA
} Color_t;

/* Function prototypes using various types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
char* generate_string(void);
void register_callback(callback_t cb);

#endif /* TEST_TYPES_H */
