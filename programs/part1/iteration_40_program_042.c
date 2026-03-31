/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define dummy GCC internal types if not present */
#ifndef tree
typedef int tree;
#endif

#ifndef rtx
typedef void* rtx;
#endif

/* Define GTY macro if not defined by system.h */
#ifndef GTY
#define GTY(x) __attribute__((gty))
#endif

/* 1. TYPE_UNDEFINED: Forward declared struct without definition */
struct GTY(()) undefined_struct;

/* 2. TYPE_SCALAR: Basic scalar type with user annotation */
typedef int GTY((user)) my_scalar_t;

/* 3. TYPE_STRING: String pointer with length attribute */
const char * GTY((length("strlen(%h.my_string)"))) my_string;

/* 4. TYPE_STRUCT: Regular struct with tag */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* 5. TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* 6. TYPE_UNION: Union with descriminator */
union GTY((desc("(%1.a != 0) ? 1 : 0"))) my_union {
    int a;
    char * GTY((skip)) b;
    struct my_struct * GTY((skip)) c;
};

/* 7. TYPE_POINTER: Pointer to struct with skip attribute */
struct my_struct * GTY((skip)) my_pointer;

/* 8. TYPE_ARRAY: Array with length attribute */
int GTY((length("sizeof(%h.my_array)/sizeof(%h.my_array[0])"))) my_array[10];

/* 9. TYPE_CALLBACK: Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int);

/* 10. TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_code;
    union my_union GTY((desc("0"))) data;
    rtx insn;  /* Use dummy GCC type */
};

/* Additional complex types to ensure full traversal */

/* Nested struct with pointer chain */
struct GTY(()) container {
    struct my_struct * GTY((skip)) first;
    struct container * GTY((skip)) next;
    my_callback_fn callback;
};

/* Variable length array in struct */
struct GTY(()) varray_struct {
    int count;
    int GTY((length("%h.count"))) elements[1];
};

/* Union with nested struct */
union GTY((desc("(%1.type == 0) ? 0 : 1"))) nested_union {
    struct {
        int type;
        char * GTY((skip)) name;
    } GTY((tag("0"))) s;
    struct {
        int type;
        int GTY((length("10"))) values[10];
    } GTY((tag("1"))) a;
};

/* Global variables with various GTY annotations */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct lang_specific * GTY((skip)) lang_ptr;

#endif /* TEST_GTY_H */
