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

/* TYPE_STRING: String types */
typedef char* string_t;
typedef const char* const_string_t;
typedef char* mutable_string;

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
    char str[32];
};

union Variant {
    long long ll;
    void* ptr;
    struct Point point;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int* IntPtr;
typedef string_t* StringPtrPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct Point point_array[5];
typedef union Data data_array[8];
typedef char char_matrix[3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, char*);
typedef struct Point* (*point_creator)(int, int);
typedef union Data (*data_processor)(union Data);

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

/* TYPE_USER_STRUCT: User-defined type structures */
typedef struct Point MyPoint;
typedef union Data MyData;
typedef struct {
    int tag;
    union {
        int i;
        float f;
        char* s;
    } value;
} TaggedUnion;

/* Additional complex nested types for thorough testing */
struct ComplexType {
    int id;
    char* name;
    struct Point location;
    union Data payload;
    int_array scores;
    comparator compare_func;
    struct ComplexType* next;
};

/* Function prototypes using various types */
void process_point(struct Point* p);
union Data create_data(int type, void* value);
int compare_points(const void* a, const void* b);
void string_operations(string_t str, const_string_t cstr);
