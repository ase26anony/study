/* types.h - Header file with declarations */
#ifndef TYPES_H
#define TYPES_H

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
} Node;

/* Union types */
union DataUnion {
    int i;
    float f;
    char str[16];
};

/* Typedef'd union */
typedef union {
    long long l;
    double d;
} NumberUnion;

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct ComplexStruct*, Point*);

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

/* Forward declarations for functions */
void use_scalars(void);
void use_strings(void);
void use_structs(void);
void use_unions(void);
void use_pointers(void);
void use_arrays(void);
void use_callbacks(void);
void use_lang_structs(void);

#endif /* TYPES_H */
