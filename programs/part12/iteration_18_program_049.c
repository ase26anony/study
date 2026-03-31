/* type_zoo.h */
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
    struct SimpleStruct nested;
    double extra;
    char name[32];
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
    char label[16];
} Point2D;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union DataUnion {
    int as_int;
    float as_float;
    char as_str[8];
};

typedef union {
    long long timestamp;
    double precision;
} TimeUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern char **string_ptr_ptr;

/* Array types */
extern int int_array[10];
extern struct SimpleStruct struct_array[5];
extern Point2D point_array[8];
extern int *pointer_array[4];

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(const char *, size_t);

extern void (*callback_var)(void);
extern SimpleCallback user_callback;

/* Language-specific struct (GCC extensions) */
#ifdef __GNUC__
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int *ptr;
    long value;
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
void use_lang_structs(void);

#endif /* TYPE_ZOO_H */
