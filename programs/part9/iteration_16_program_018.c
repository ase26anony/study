/* types.h - Header file with declarations for gengtype coverage test */

#ifndef TYPES_H
#define TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;

/* TYPE_STRING: String type */
extern const char* global_string;

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for struct */
typedef struct {
    int data;
    char tag;
} MyStruct;

/* TYPE_UNION: Union type */
union Value {
    int i;
    float f;
    double d;
    char c;
};

/* TYPE_POINTER: Various pointer types */
extern int* int_ptr;
extern struct Point* point_ptr;
extern void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
extern int int_array[10];
extern char char_array[20];

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*comparator)(const void*, const void*);

/* Complex nested type combinations */
struct ComplexStruct {
    /* Array of pointers to unions */
    union Value* value_ptrs[5];
    
    /* Callback member */
    comparator compare_func;
    
    /* Nested struct */
    struct {
        int depth;
        char label[32];
    } inner;
    
    /* Pointer to array */
    int (*matrix_ptr)[3][3];
};

/* GCC attributes for edge cases */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
};

typedef __attribute__((deprecated)) int DeprecatedInt;

/* Function declarations using various types */
void process_struct(struct Point* p);
union Value create_value(int type);
int compare_values(const void* a, const void* b);

#endif /* TYPES_H */
