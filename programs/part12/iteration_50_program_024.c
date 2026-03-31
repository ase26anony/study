/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long long scalar_longlong;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
    int area;
};

/* TYPE_USER_STRUCT: Typedef'd struct (might be treated as user struct) */
typedef struct {
    int id;
    char name[50];
} UserStruct;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    char str[20];
    double d;
};

union Variant {
    long l;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int* IntPtr;
typedef void* GenericPtr;
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
int int_array[10];
struct Point point_array[5];
union Data data_array[3];
char char_array[256];
int multi_dim_array[3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);
typedef struct Point* (*point_creator)(int, int);
typedef void (*error_handler)(const char*);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (might trigger lang_struct) */
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
typedef struct ComplexType {
    int tag;
    union {
        int int_value;
        double double_value;
        char* string_value;
        struct Point point_value;
    } data;
    comparator compare_func;
    int (*transform)(struct ComplexType*);
    struct ComplexType* next;
} ComplexType;

/* Enumeration type (might be treated as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE,
    ALPHA
} Color;

/* Function types */
typedef int (*(*complex_callback)(int))(void);
