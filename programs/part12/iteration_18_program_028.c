/* types.h - Header file to declare types */
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
    int x;
    float y;
};

struct ComplexStruct {
    int id;
    char name[32];
    struct SimpleStruct nested;
    void *data;
};

/* User struct type (typedef'd) */
typedef struct {
    int counter;
    double values[4];
} UserStruct;

/* Union types */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long as_llong;
    double as_double;
    struct SimpleStruct as_struct;
} TypedefUnion;

/* Pointer types */
extern int *global_int_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern void **global_void_ptr_ptr;

/* Array types */
extern int global_int_array[10];
extern struct ComplexStruct global_struct_array[5];
extern char *global_ptr_array[8];

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct ComplexStruct*, UserStruct*);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(1)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations */
void use_scalars(void);
void use_strings(void);
void use_structs(void);
void use_unions(void);
void use_pointers(void);
void use_arrays(void);
void use_callbacks(void);
void use_lang_structs(void);

#endif /* TYPES_H */
