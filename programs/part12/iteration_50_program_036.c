/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

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
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int* IntPtr;
typedef void* GenericPtr;
typedef const_string_t* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct Point point_array[5];
typedef union Data data_array[3];
typedef char char_matrix[4][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, void*);
typedef char* (*string_generator)(void);
typedef int (*binary_op)(int, int);

/* TYPE_LANG_STRUCT: GCC-specific structure with attributes */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    double precision;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* TYPE_USER_STRUCT: User-defined structure types */
typedef struct Point PointType;
typedef struct Rectangle RectType;

/* Complex nested type to ensure thorough analysis */
struct ComplexType {
    int id;
    char* name;
    struct Point position;
    union Data value;
    int (*operation)(struct ComplexType*);
    struct ComplexType* next;
    int scores[10];
};

/* Function prototypes that use various types */
void process_point(struct Point* p);
union Data create_data(int type);
int compare_points(const void* a, const void* b);
void complex_operation(struct ComplexType* ct, callback_func cb);
