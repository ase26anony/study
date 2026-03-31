/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stddef.h>

/* Scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_long_long;
extern _Bool global_bool;

/* String type */
extern char *global_string;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

struct ComplexStruct {
    int id;
    double data[4];
    char *name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int counter;
    float value;
    char label[32];
} UserStruct;

typedef struct Node {
    int data;
    struct Node *next;
} ListNode;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    double precision;
    void *pointer;
} TypedefUnion;

/* Pointer types */
extern int *int_ptr;
extern int **int_double_ptr;
extern struct SimpleStruct *struct_ptr;
extern union SimpleUnion *union_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern char *string_array[8];

/* Callback types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_type, void *data);

/* Language-specific struct (GCC extensions) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

#ifdef __GNUC__
struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long value;
};
#endif

/* Function declarations */
void process_scalars(void);
void process_structs_and_unions(void);
void process_pointers_and_arrays(void);
void process_callbacks(void);

#endif /* TYPE_ZOO_H */
