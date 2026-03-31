/* test_types.h - Comprehensive type definitions to cover all gengtype classifications */

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct incomplete;
typedef struct incomplete incomplete_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long long scalar_longlong;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types */
typedef char* string_ptr;
typedef const char* const_string_ptr;
typedef char string_array[100];

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

/* TYPE_UNION: Union types */
union Data {
    int i;
    float f;
    double d;
    char str[20];
    void* ptr;
};

union Variant {
    long long int_val;
    double float_val;
    struct Point point_val;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int* IntPtr;
typedef void (*VoidFuncPtr)(void);
typedef const_string_ptr* StringPtrPtr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct Point point_array[5];
typedef union Data data_array[3];
typedef char* string_ptr_array[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, const char*);
typedef struct Point* (*point_creator)(int, int, double);
typedef void (*error_handler)(const char*, int);

/* TYPE_LANG_STRUCT: GCC-specific attributed structures */
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
    char name[50];
    struct Point location;
    union Data data;
    int (*process)(struct ComplexType*);
    struct ComplexType* next;
    int_array scores;
};

/* Function prototypes using various types */
void process_point(struct Point* p);
union Data create_data(int type, void* value);
int compare_points(const void* a, const void* b);
void handle_error(const char* msg, int code);
