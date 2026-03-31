/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long long scalar_llong_t;
typedef unsigned long scalar_ulong_t;

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

/* TYPE_USER_STRUCT: Typedef'd struct (might be classified differently) */
typedef struct {
    int data;
    char name[32];
} user_struct_t;

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[32];
    void *ptr;
};

typedef union Data DataUnion_t;

/* TYPE_POINTER: Pointer types to various entities */
typedef int* int_ptr_t;
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef void (*func_ptr_t)(void);
typedef const char* const* string_ptr_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10_t[10];
typedef struct Point point_array_5_t[5];
typedef char string_array_3_t[3][64];
typedef int* pointer_array_8_t[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, char*);
typedef int (*binary_op_t)(int, int);
typedef char* (*string_transform_t)(const char*);

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
    int i;
    float f;
};

/* Complex nested type to ensure thorough analysis */
typedef struct ComplexType {
    int id;
    char name[64];
    union Data value;
    struct Point location;
    int (*compare)(struct ComplexType*, struct ComplexType*);
    struct ComplexType* next;
    int array[8];
} ComplexType_t;

/* Enumeration type (might be TYPE_SCALAR) */
typedef enum {
    RED,
    GREEN,
    BLUE,
    ALPHA
} Color_t;

/* Bitfield structure */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int reserved : 26;
};

/* Forward declared struct that will be completed later */
struct forward_declared;
struct forward_declared* global_fwd_ptr;

/* Opaque pointer type */
typedef void* opaque_handle_t;

/* Self-referential structure */
struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
};

/* Array of function pointers */
typedef int (*math_ops_t[4])(int, int);

/* Structure with flexible array member */
struct FlexArray {
    int count;
    int data[];
};

/* Complete the forward declaration */
struct incomplete {
    int magic;
    struct incomplete* self_ptr;
};

/* Macro to generate type usage */
#define USE_TYPE(type, var) type var##_inst
