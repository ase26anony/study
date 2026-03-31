#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile const int volatile_const_int;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char global_char_array[50];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
};

/* TYPE_POINTER: Various pointer types */
typedef int* IntPtr;
typedef struct Point* PointPtr;
typedef void (*VoidFuncPtr)(void);

/* TYPE_ARRAY: Array types */
typedef int IntArray[100];
typedef struct Point PointArray[50];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* Complex nested type for deep traversal */
struct ComplexContainer {
    union Value values[10];          /* Array of unions */
    struct Point* points[20];        /* Array of pointers to structs */
    Comparator compare_func;         /* Callback function pointer */
    MyStruct user_structs[5];        /* Array of user-defined structs */
    volatile const int* volatile* complex_ptr; /* Complex pointer with qualifiers */
};

/* GCC attributes for edge cases */
struct __attribute__((aligned(16), packed)) AlignedPackedStruct {
    char data[15];
    int value;
};

typedef struct __attribute__((deprecated)) DeprecatedStruct {
    int old_field;
} DeprecatedStruct;

/* Function declarations using various types */
void process_point(struct Point* p);
int compare_values(const union Value* a, const union Value* b);
void register_callback(EventHandler handler);

#endif /* TYPES_H */
