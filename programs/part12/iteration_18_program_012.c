/* type_zoo.h - Header file with declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

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
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long big;
    double precise;
    void *generic;
} GenericUnion;

/* Pointer types */
extern int *int_ptr;
extern int **int_dbl_ptr;
extern struct SimpleStruct *struct_ptr;
extern union SimpleUnion *union_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*SimpleCallback)(int, char *);

/* Language-specific struct (GCC extensions) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    void *generic;
};
#endif

/* Function declarations */
void use_scalars(void);
void use_strings(void);
void use_structs(void);
void use_unions(void);
void use_pointers(void);
void use_arrays(void);
void use_callbacks(void);
int compare_ints(const void *a, const void *b);
void sample_callback(int x, char *msg);

#endif /* TYPE_ZOO_H */
