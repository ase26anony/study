/* test_types.h - Comprehensive type definitions for gengtype coverage testing */

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
    scalar_int x;
    scalar_int y;
    scalar_float z;
};

struct ComplexStruct {
    struct Point position;
    string_ptr name;
    scalar_double value;
    int flags;
};

/* TYPE_UNION: Union types */
union Data {
    scalar_int i;
    scalar_float f;
    scalar_double d;
    char str[50];
    struct Point point;
};

union TaggedUnion {
    int type;
    struct {
        int x, y;
    } coords;
    struct {
        float min, max;
    } range;
};

/* TYPE_POINTER: Pointer types to various entities */
typedef struct Point* PointPtr;
typedef union Data* DataPtr;
typedef scalar_int* IntPtr;
typedef string_ptr* StringPtrPtr;
typedef void (*FuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[10];
typedef struct Point PointArray[5];
typedef union Data DataArray[3];
typedef char* StringArray[20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);
typedef scalar_double (*MathFunc)(scalar_double, scalar_double);
typedef struct Point* (*PointFactory)(int, int);

/* TYPE_LANG_STRUCT: Struct with GCC attributes (potential lang_struct) */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    char metadata[32];
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* TYPE_USER_STRUCT: May be triggered by typedef'd structs */
typedef struct {
    int id;
    char name[50];
    float score;
} UserDefined;

/* Additional complex nested types for thorough testing */
typedef struct Node {
    int value;
    struct Node* next;
    struct Node* prev;
} Node;

typedef union {
    struct {
        unsigned char r, g, b, a;
    } rgba;
    unsigned int value;
} Color;

/* Function prototypes using various types */
struct Point* create_point(int x, int y);
void process_data(union Data* data, int count);
int compare_points(const void* a, const void* b);
void string_operations(const_string_ptr str, string_ptr buffer);
