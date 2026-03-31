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
    int id;
    char name[32];
    float score;
} UserStruct;

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
} TypedefUnion;

/* Pointer types */
extern int *global_int_ptr;
extern int **global_int_dbl_ptr;
extern struct SimpleStruct *global_struct_ptr;
extern UserStruct *global_userstruct_ptr;

/* Array types */
extern int global_int_array[10];
extern struct ComplexStruct global_struct_array[5];
extern char *global_ptr_array[8];

/* Callback types */
typedef int (*BinaryOp)(int, int);
typedef void (*VoidCallback)(void);

extern void (*global_callback)(int, float);
extern BinaryOp global_binary_op;

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    short c;
};

/* Transparent union (another GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Function declarations using various types */
struct ComplexStruct process_struct(struct SimpleStruct s);
UserStruct *create_user_struct(int id, const char *name);
int use_callback(BinaryOp op, int x, int y);
void process_array(int arr[], int size);

#endif /* TYPE_ZOO_H */
