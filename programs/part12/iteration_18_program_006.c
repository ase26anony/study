/* type_zoo.h */
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
    int x;
    float y;
};

struct ComplexStruct {
    int id;
    char name[32];
    struct SimpleStruct nested;
    void *data;
};

/* User struct types (typedef'd) */
typedef struct {
    int a;
    double b;
    char c;
} UserStruct;

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

/* Union types */
union SimpleUnion {
    int i;
    float f;
    char c;
};

typedef union {
    long long l;
    double d;
    void *p;
} TypedefUnion;

/* Callback types */
typedef int (*BinaryOp)(int, int);
typedef void (*VoidCallback)(void);

/* Language-specific struct (GCC extension) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Array types */
extern int global_int_array[10];
extern struct SimpleStruct struct_array[5];
extern void *pointer_array[8];

/* Multi-level pointer */
extern int ***triple_pointer;

/* Function declarations */
int process_scalars(int a, float b, double c);
void handle_structs(struct ComplexStruct *cs, UserStruct us);
union SimpleUnion process_unions(union SimpleUnion u1, TypedefUnion u2);
int *process_pointers(int *p, char **str_arr);
int process_arrays(int arr[], int size);
BinaryOp get_callback(void);
void use_lang_structs(struct PackedStruct *ps, TransparentUnion tu);

#endif /* TYPE_ZOO_H */
