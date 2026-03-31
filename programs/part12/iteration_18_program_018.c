/* types.h - Header file with declarations */
#ifndef TYPES_H
#define TYPES_H

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
    char name[32];
    struct SimpleStruct nested;
    void *data;
};

/* User struct types (typedef'd) */
typedef struct {
    int x, y;
} Point;

typedef struct {
    Point start;
    Point end;
    double thickness;
} Line;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    double value;
    void *ptr;
} DataUnion;

/* Pointer types */
extern int *int_ptr;
extern struct SimpleStruct *struct_ptr;
extern void **void_double_ptr;
extern char ***char_triple_ptr;

/* Array types */
extern int int_array[10];
extern struct ComplexStruct struct_array[5];
extern Point *pointer_array[8];

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

/* Function declarations */
void process_scalars(int seed);
void process_structs_and_unions(void);
void process_arrays_and_pointers(void);
void use_callbacks(void);
void process_lang_structs(void);

#endif /* TYPES_H */
