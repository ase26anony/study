/* test_state_gty.h - Comprehensive GTY annotations for gengtype state coverage */

#ifndef TEST_STATE_GTY_H
#define TEST_STATE_GTY_H

/* Define GTY macro if not already defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types to avoid dependency */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ============================================
   TYPE_UNDEFINED: Forward declared struct without definition
   ============================================ */
struct GTY(()) my_undefined_struct;  /* TYPE_UNDEFINED */

/* ============================================
   TYPE_STRUCT: Simple struct with tag
   ============================================ */
struct GTY((tag("my_struct"))) my_struct {
  int field1;
  char field2;
  tree gcc_field;  /* Use dummy GCC type */
};  /* TYPE_STRUCT */

/* ============================================
   TYPE_USER_STRUCT: Typedef with user marker
   ============================================ */
typedef struct my_struct GTY((user)) my_user_struct_t;  /* TYPE_USER_STRUCT */

/* ============================================
   TYPE_UNION: Union with desc tag and skip pointer
   ============================================ */
union GTY((desc("0"))) my_union {
  int a;
  char * GTY((skip)) b;  /* Skip pointer field */
  struct my_struct * GTY((skip)) c;
  double d;
};  /* TYPE_UNION */

/* ============================================
   TYPE_POINTER: Pointer to struct with skip
   ============================================ */
struct my_struct * GTY((skip)) my_pointer;  /* TYPE_POINTER */

/* ============================================
   TYPE_ARRAY: Array with length attribute
   ============================================ */
int GTY((length("my_array_length"))) my_array[10];  /* TYPE_ARRAY */

/* Helper for array length */
extern int my_array_length;

/* ============================================
   TYPE_LANG_STRUCT: Language-specific struct
   ============================================ */
struct GTY((special("lang_struct"))) my_lang_struct {
  int lang_specific;
  union {
    int a;
    void * GTY((skip)) p;
    tree t;
  } u;
  rtx insn;  /* Use dummy GCC type */
};  /* TYPE_LANG_STRUCT */

/* ============================================
   TYPE_SCALAR: Scalar typedef with user marker
   ============================================ */
typedef int GTY((user)) my_scalar_t;  /* TYPE_SCALAR */

/* ============================================
   TYPE_STRING: String pointer with length
   ============================================ */
const char * GTY((length("strlen(%h.my_string) + 1"))) my_string;  /* TYPE_STRING */

/* ============================================
   TYPE_CALLBACK: Function pointer typedef
   ============================================ */
typedef void (*GTY((user)) my_callback_fn)(int, void*);  /* TYPE_CALLBACK */

/* ============================================
   Additional complex types to ensure thorough traversal
   ============================================ */

/* Nested struct with pointer chain */
struct GTY((tag("nested_struct"))) nested_struct {
  struct my_struct * GTY((skip)) ptr1;
  union my_union data;
  struct nested_struct * GTY((skip)) next;
};

/* Array of pointers */
struct my_struct * GTY((length("5"))) ptr_array[5];

/* Struct containing callback */
struct GTY((tag("with_callback"))) struct_with_callback {
  my_callback_fn GTY((skip)) callback;
  int data;
};

/* Union with nested struct */
union GTY((desc("1"))) complex_union {
  struct my_struct s;
  struct nested_struct n;
  my_callback_fn GTY((skip)) fn;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct GTY((tag("uses_forward"))) uses_forward {
  struct forward_declared * GTY((skip)) fwd_ptr;
};

/* Definition of forward declared struct */
struct GTY((tag("forward_declared"))) forward_declared {
  int value;
  struct uses_forward * GTY((skip)) back_ptr;
};

#endif /* TEST_STATE_GTY_H */
