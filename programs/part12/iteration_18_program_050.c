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
extern char* global_string;

/* Struct types */
struct SimpleStruct {
    int x;
    float y;
    char z;
};

struct ComplexStruct {
    int id;
    double values[10];
    char* name;
    struct SimpleStruct nested;
};

/* User struct types (typedef'd) */
typedef struct {
    int a;
    int b;
    int c;
} UserStruct;

typedef struct Node {
    int data;
    struct Node* next;
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
    void* data;
} TimestampUnion;

/* Pointer types */
extern int* int_ptr;
extern struct SimpleStruct* struct_ptr;
extern union SimpleUnion* union_ptr;
extern void** void_double_ptr;

/* Array types */
extern int int_array[100];
extern struct SimpleStruct struct_array[20];
extern char* string_array[50];

/* Callback types */
typedef void (*SimpleCallback)(int);
typedef int (*ComplexCallback)(struct ComplexStruct*, UserStruct*);

/* Language-specific structs */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
};

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
