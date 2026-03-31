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
    double float_val;
    void* ptr_val;
    char char_val;
};

typedef union Data Data_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void (*func_ptr_t)(void);
typedef const char* const* string_ptr_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef float float_matrix[3][3];
typedef struct Point point_array[5];
typedef char string_buffer[256];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef char* (*string_generator_t)(void);
typedef int (*binary_op_t)(int, int);

/* TYPE_LANG_STRUCT: GCC-specific structures with attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
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

/* TYPE_USER_STRUCT: User-defined type structures */
typedef struct {
    int id;
    char name[64];
    float score;
} UserRecord;

typedef struct Node {
    int value;
    struct Node* next;
    struct Node* prev;
} Node_t;

/* Complex nested type for thorough testing */
typedef struct ComplexType {
    int scalar_field;
    char* string_field;
    struct Point point_field;
    union Data data_field;
    int (*callback_field)(int);
    struct ComplexType* self_ptr;
    int array_field[8];
    struct {
        int nested_id;
        char nested_name[32];
    } anonymous_struct;
} ComplexType_t;

/* Enumeration types */
typedef enum Color {
    RED,
    GREEN,
    BLUE,
    ALPHA = 255
} Color_t;

typedef enum __attribute__((packed)) SmallEnum {
    ONE = 1,
    TWO = 2,
    THREE = 3
} SmallEnum_t;

/* Function declarations using the types */
void process_point(struct Point* p);
int compare_data(const union Data* a, const union Data* b);
char* generate_string(void);
void register_callback(callback_t cb);

/* Macro for type tagging */
#define TYPE_TAG(t) __typeof__(t)

#endif /* TEST_TYPES_H */
