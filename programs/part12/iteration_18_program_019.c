/* type_zoo.h - Header file with declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* String type */
extern char *global_string;

/* Struct types */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    int *ptr;
    double data[4];
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char name[32];
} Point;

typedef struct Node {
    int value;
    struct Node *next;
} Node;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long long ll;
    double d;
} NumberUnion;

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(const char*, void*);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct global_struct_array[5];
extern Point *global_pointer_array[8];

/* Function pointer array */
typedef int (*MathFunc)(int, int);
extern MathFunc func_array[3];

#endif /* TYPE_ZOO_H */
