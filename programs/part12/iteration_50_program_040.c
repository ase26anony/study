/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

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

/* Nested structure */
struct Container {
    struct Point point;
    struct Rectangle rect;
    scalar_int_t count;
};

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
    void* ptr;
};

union Variant {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    struct Point as_point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int_t* IntPtr;
typedef void* GenericPtr;
typedef const struct Rectangle* ConstRectPtr;
typedef volatile char* VolatileCharPtr;

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef struct Point point_array_t[5];
typedef union Data data_array_t[3];
typedef char char_matrix_t[4][4];
typedef const char* string_array_t[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef scalar_int_t (*transform_t)(scalar_float_t);
typedef string_t (*formatter_t)(const struct Point*);
typedef void (*error_handler_t)(const char*, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    union {
        int i;
        float f;
    } u;
};

/* Complex type combinations */
typedef struct {
    PointPtr points;
    int count;
    comparator_t compare;
} Collection;

/* Function type with complex signature */
typedef union Data* (*processor_t)(int_array_t, callback_t);

/* Forward declarations for testing TYPE_UNDEFINED */
struct forward_declared;
extern struct forward_declared* global_forward_ptr;

/* Enumeration type (should be treated as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE,
    ALPHA
} color_t;

/* Bitfield structure */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int reserved : 26;
};

/* Anonymous struct/union */
struct AnonymousContainer {
    struct {
        int x;
        int y;
    } position;
    union {
        int int_val;
        float float_val;
    } value;
};
