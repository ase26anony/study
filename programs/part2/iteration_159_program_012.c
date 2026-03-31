#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int i;
    double d;
    void* p;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) ptr_to_struct;  /* TYPE_POINTER */
    union my_union* GTY((skip)) ptr_to_union;     /* TYPE_POINTER */
    void* GTY((skip)) opaque_ptr;                 /* TYPE_POINTER */
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    int GTY((length("10"))) fixed_arr[10];        /* TYPE_ARRAY - fixed size */
    struct my_struct* GTY((length("len"))) dyn_arr[0]; /* TYPE_ARRAY - dynamic */
    int len;
};

/* TYPE_SCALAR: Direct scalar types with GTY */
typedef long GTY((skip)) my_long_t;
typedef unsigned GTY((skip)) my_unsigned_t;

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;                 /* TYPE_STRING */
    char* GTY((skip)) mutable_str;                /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, const char*) GTY((callback));

/* Struct using callback */
struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;              /* TYPE_CALLBACK */
};

/* Complex nested type for recursive processing */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;                /* Self-referential pointer */
    struct node* GTY((skip)) children[5];         /* Array of pointers */
};

/* Template-like macro generating multiple structs */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {
    int decl_type;
    void* GTY((skip)) lang_info;
};

/* Forward declaration for user struct */
struct user_defined_type;

#endif /* TEST_GTY_H */
