/* test-gty.h - Header file with various GTY-annotated types */

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

/* TYPE_POINTER: Will be used within another struct */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) ptr_field;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];  /* TYPE_ARRAY */
};

/* TYPE_SCALAR: Scalar type with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;  /* TYPE_SCALAR */
};

/* TYPE_STRING: String field */
struct string_container GTY(()) {
    const char* GTY((skip)) name;  /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn handler;
};

/* Complex nested type for recursive processing */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;  /* Self-referential pointer */
    struct node* GTY((skip)) prev;
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
    struct my_struct s;
    struct node* GTY((skip)) n;
    int data[5];
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct node*);

/* Language-specific structure (simulating Tree node) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int decl_uid;
    const char* GTY((skip)) name;
    struct lang_struct* GTY((skip)) chain;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
