/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* Nested union within struct for more complexity */
struct GTY(()) container {
    union GTY((desc("1"))) nested_union {
        int x;
        double y;
    } u;
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((chain_next("tree"), chain_prev("tree"))) tree_chain;

/* Pointer with callback */
typedef void (*callback_func)(int);
callback_func GTY((user)) func_pointer;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY(()) struct_array[5];

/* Variable length array */
struct GTY(()) varray {
    int length;
    int GTY((length("%0.length"))) data[];
};

/* ========== TYPE_CALLBACK ========== */
/* Function pointer type */
typedef void (*GTY((user)) my_callback_fn)(int);

/* Struct with callback field */
struct GTY(()) callback_container {
    my_callback_fn GTY((user)) handler;
    void (*GTY((user)) direct_handler)(void*);
};

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure pattern */
struct GTY((special("lang_struct"))) lang_specific {
    int lang_code;
    union GTY((desc("%0.lang_code"))) lang_data {
        tree c_tree;
        rtx c_rtx;
        void* other;
    } u;
};

/* Another lang struct variant */
struct GTY(()) tree_common {
    tree chain;
    tree type;
    int code;
};

struct GTY((special("tree_ssa_name"))) tree_ssa_name {
    struct tree_common common;
    unsigned int version;
};

/* ========== Complex nested types ========== */
/* Struct containing pointer to array of pointers */
struct GTY(()) complex_type {
    struct my_struct *** GTY((skip)) triple_ptr;
    union my_union GTY(()) data;
    int GTY((length)) dyn_array[20];
};

/* Template-like pattern */
struct GTY((for_user)) user_facing {
    my_user_struct_t user_data;
    my_callback_fn callback;
};

/* ========== Root declarations ========== */
/* Variables that will be in GC roots */
extern struct my_struct GTY((root)) global_struct;
extern tree GTY((root)) global_tree;
extern struct lang_specific GTY((root)) global_lang_struct;

/* Union root variable */
union my_union GTY((root)) global_union;

#endif /* TEST_GTY_H */
