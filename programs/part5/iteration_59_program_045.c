#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [depth]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY attributes with string literals containing special characters */
#define DESC_ATTR(field) GTY((desc("%0." #field), param_is(struct node)))
#define CONDITIONAL_ATTR GTY((desc("%0.tag == 1 ? \"active\" : \"inactive\"")))

/* Primary recursive structure with deeply nested GTY annotations */
struct node GTY((
  chain_next = "next",
  chain_prev = "prev",
  desc("%0.tag == 0 ? \"leaf\" : \"internal\"")
))
{
  int tag;
  int value;
  
  /* Parentheses: Function pointer with complex signature */
  int (* GTY((skip)) callback)(
    struct node *child,
    int depth,
    void (* GTY((skip)) helper)(int, char *)
  );
  
  /* Brackets: Array with variable bounds */
  struct node * GTY((length("child_count"))) children[];
  
  /* Braces: Nested union within structure */
  union {
    int tag;
    void * GTY((tag("0"))) data;
    struct {
      int x;
      int y;
      char * GTY((length("str_len"))) name[];
    } GTY((desc("%0.coords"))) coords;
  } variant;
  
  /* Using macro with nested brackets */
  PTR_ARRAY(struct node) grandchildren;
  
  struct node *next;
  struct node *prev;
};

/* Union type with GTY markers containing all delimiter types */
union complex_union GTY((
  desc("%0.utag == 0 ? \"ptr_array\" : \"callback\"")
))
{
  int utag;
  
  /* Nested array in union */
  struct node * GTY((length("arr_len"))) node_array[10];
  
  /* Function pointer with parentheses */
  void (* GTY((skip)) process)(
    union complex_union *self,
    int (* GTY((skip)) filter)(int),
    char *args[]
  );
  
  /* Structure within union with braces */
  struct {
    int count;
    char * GTY((length("count * 2"))) buffer[];
  } GTY((desc("%0.buf_data"))) buffer_data;
};

/* Template-like structure using macros */
struct tree_container GTY(())
{
  int depth;
  
  /* Multi-dimensional array with GTY annotation */
  struct node * GTY((length("depth"), nested_ptr)) levels[][5];
  
  /* Callback with nested parentheses */
  CALLBACK_TYPE(int, (struct node *, int)) validator;
  
  /* Union array with braces in GTY param */
  union complex_union GTY((desc("items[%0]"))) items[10];
};

/* Global variables with GTY markers */
extern struct node * GTY(()) global_tree_root;
extern union complex_union GTY(()) global_union_array[];

/* Function pointer type with GTY skip */
typedef int (* GTY((skip)) node_processor)(
  struct node *n,
  void * GTY((skip)) context,
  int (* GTY((skip)) callback)(int)
);

#endif /* TEST_GTY_H */
