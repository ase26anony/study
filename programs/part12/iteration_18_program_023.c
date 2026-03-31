/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_llong;
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
} ListNode;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long l;
    double d;
    void *p;
} GenericUnion;

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(const char *, void *);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Array types */
typedef int IntArray[10];
typedef struct SimpleStruct StructArray[5];

/* Function declarations using various types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_complex_struct(void);
Point *create_points(int count);
void register_callback(SimpleCallback cb);

#endif /* TYPE_ZOO_H */
