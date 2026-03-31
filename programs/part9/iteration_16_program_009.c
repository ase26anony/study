#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern volatile int volatile_int;
extern const double const_double;

/* TYPE_STRING: String types */
extern const char* global_string;
extern char mutable_string[50];

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char tag;
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char* s;
} __attribute__((aligned(16)));

/* TYPE_POINTER: Various pointer types */
extern int* int_ptr;
extern struct Point* point_ptr;
extern void* void_ptr;
extern volatile const int* volatile* complex_ptr;

/* TYPE_ARRAY: Array types */
extern int int_array[10];
extern char char_array[20][30];
extern struct Point point_array[5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator)(const void*, const void*);
typedef void (*simple_callback)(void);
extern comparator global_comparator;

/* TYPE_LANG_STRUCT: GCC-specific attributes for language-specific handling */
struct __attribute__((visibility("hidden"))) HiddenStruct {
    int secret;
};

/* Complex nested type combinations */
struct Container {
    /* Nested struct */
    struct {
        int id;
        char name[32];
    } nested;
    
    /* Array of pointers to unions */
    union Value* value_ptrs[8];
    
    /* Callback member */
    simple_callback callback;
    
    /* Pointer to array of structs */
    MyStruct (*struct_array_ptr)[4];
} __attribute__((packed, aligned(8)));

/* Another complex union with struct containing callback */
union ComplexUnion {
    struct {
        int type;
        union Value value;
        comparator compare_func;
    } tagged;
    double raw_data[4];
};

/* Typedef for pointer to array of structs */
typedef struct Container (*ContainerArrayPtr)[3];

/* Deprecated type with attribute */
typedef int OldInt __attribute__((deprecated));

/* Packed struct with pragma */
#pragma pack(push, 1)
struct PackedData {
    char flag;
    int count;
    short checksum;
};
#pragma pack(pop)

#endif /* TEST_TYPES_H */
