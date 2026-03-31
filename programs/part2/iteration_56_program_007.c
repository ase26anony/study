/* test_types.h - Header file for multi-file type testing */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

/* TYPE_SCALAR: Basic scalar types */
extern int global_int;
extern float global_float;
extern double global_double;
extern char global_char;
extern _Bool global_bool;

/* TYPE_STRING: String types */
extern const char *global_string;
extern char global_string_array[];

/* TYPE_STRUCT: Plain C structures */
struct GlobalStruct {
    int id;
    float value;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int counter;
    char name[32];
} UserStruct;

/* TYPE_UNION: Union declarations */
union GlobalUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern int *global_int_ptr;
extern void **global_void_ptr_ptr;
extern struct GlobalStruct *global_struct_ptr;

/* TYPE_ARRAY: Array declarations */
extern int global_int_array[100];
extern float global_matrix[10][10];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*BinaryFunc)(int, int);
extern void (*global_void_callback)(void);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed, aligned(1))) PackedGlobalStruct {
    char flag;
    int data;
};

/* Complex nested type for thorough traversal */
typedef struct ComplexNode {
    void *data;
    struct ComplexNode **children;  /* Pointer to array of pointers */
    int (*compare)(struct ComplexNode *, struct ComplexNode *);
    union {
        int int_val;
        double dbl_val;
    } value;
} ComplexNode;

/* Function declarations using various types */
ComplexNode *create_node(void);
int process_structure(struct GlobalStruct *gs, UserStruct *us);

#endif /* TEST_TYPES_H */
