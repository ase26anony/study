/* test_types.h - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_longlong_t;
typedef unsigned int scalar_uint_t;
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
typedef union Variant Variant_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void* generic_ptr_t;
typedef const struct Rectangle* ConstRectPtr;
typedef volatile char* volatile_char_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10_t[10];
typedef struct Point point_array_5_t[5];
typedef char string_array_3_t[3][64];
typedef int* pointer_array_8_t[8];
typedef const double const_double_array_4_t[4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, char*);
typedef struct Point* (*point_creator_t)(int, int);
typedef int (*binary_op_t)(int, int);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
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
typedef struct {
    int id;
    char name[64];
    union {
        int int_value;
        double double_value;
        void* pointer_value;
    } data;
    int (*processor)(struct Point*);
    struct Point points[4];
} ComplexType;

/* Enumeration type (should be treated as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE
} Color;

typedef enum ErrorCode {
    ERR_NONE = 0,
    ERR_INVALID = 1,
    ERR_OVERFLOW = 2
} ErrorCode_t;
