/* type_zoo.h - Header file with type declarations */
#ifndef TYPE_ZOO_H
#define TYPE_ZOO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Scalar types - TYPE_SCALAR */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern long long global_longlong;
extern _Bool global_bool;

/* String type - TYPE_STRING */
extern char *global_string;

/* Struct types - TYPE_STRUCT */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

struct ComplexStruct {
    struct SimpleStruct nested;
    double extra;
    long long big;
};

/* User struct types - TYPE_USER_STRUCT */
typedef struct {
    int x;
    int y;
    char name[32];
} Point;

typedef struct {
    Point start;
    Point end;
    double length;
} LineSegment;

/* Union types - TYPE_UNION */
union DataUnion {
    int i;
    float f;
    char str[16];
};

typedef union {
    long l;
    double d;
    void *ptr;
} GenericUnion;

/* Pointer types - TYPE_POINTER */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern union DataUnion *union_ptr;
extern Point *point_ptr;
extern void **void_double_ptr;

/* Array types - TYPE_ARRAY */
extern int int_array[20];
extern struct SimpleStruct struct_array[5];
extern Point point_array[10];
extern char *string_array[8];

/* Callback types - TYPE_CALLBACK */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *message, int severity);

/* Function pointer variables */
extern Comparator global_comparator;
extern Logger global_logger;

/* Language-specific struct - TYPE_LANG_STRUCT */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int i;
    float f;
};

/* Undefined type forward declaration - TYPE_UNDEFINED */
struct UndefinedStruct;  /* Forward declaration */

/* Function declarations */
void use_scalar_types(void);
void use_string_types(void);
void use_struct_types(void);
void use_user_struct_types(void);
void use_union_types(void);
void use_pointer_types(void);
void use_array_types(void);
void use_callback_types(void);
void use_lang_struct_types(void);
void use_undefined_types(void);

#endif /* TYPE_ZOO_H */
