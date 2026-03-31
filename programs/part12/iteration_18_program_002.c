/* type_zoo.h - Header file with type declarations */
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
    int id;
    double values[5];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd structs) */
typedef struct {
    int x, y;
} Point2D;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[20];
};

typedef union {
    long long ll;
    double d;
} NumericUnion;

/* Pointer types */
extern int *int_ptr;
extern int **int_dbl_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[3];

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(int, void*);

/* Language-specific struct (GCC extension) */
struct TransparentUnion {
    int a;
    double b;
} __attribute__((transparent_union));

/* Packed struct (another GCC extension) */
struct PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Function declarations using these types */
void process_scalars(int a, float b, double c);
struct ComplexStruct create_complex_struct(int id);
Point2D* create_point_array(int count);
void register_callback(SimpleCallback cb);
union DataUnion process_union(union DataUnion u);

#endif /* TYPE_ZOO_H */
