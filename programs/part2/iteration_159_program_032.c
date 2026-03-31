/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) struct_ptr;  /* TYPE_POINTER */
    void* GTY((skip)) opaque_ptr;              /* TYPE_POINTER */
    int* GTY((skip)) int_ptr;                  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) fixed_array[10];  /* TYPE_ARRAY */
    char GTY((length("256"))) buffer[256];    /* TYPE_ARRAY */
};

/* TYPE_SCALAR: Direct scalar types with GTY */
long GTY((skip)) global_counter;              /* TYPE_SCALAR */
unsigned GTY((skip)) global_flags;            /* TYPE_SCALAR */

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;             /* TYPE_STRING */
    char* GTY((skip)) dynamic_string;         /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;          /* TYPE_CALLBACK */
};

/* Complex nested type graph */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;           /* Creates type graph */
    struct node* GTY((skip)) prev;
};

/* Union with mixed members */
union mixed_union GTY(()) {
    struct my_struct s;
    struct pointer_container* GTY((skip)) pc;
    int array[5];
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

/* Struct referencing user-defined type */
struct user_ref_container GTY(()) {
    struct user_defined_type* GTY((skip)) user_ptr;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
