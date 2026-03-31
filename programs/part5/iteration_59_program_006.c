#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type, len_field) type * GTY((length(#len_field))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []

/* Complex attribute with string literals and parentheses */
#define DESC_ATTR(tag_field) desc("%0." #tag_field)
#define PARAM_IS(type) param_is(struct type)

/* Recursive tree node structure with all delimiter types */
typedef struct node node_t;

struct GTY((
  chain_next = "next",
  chain_prev = "prev",
  desc(DESC_ATTR(tag)),
  param_is(node_t)
)) node {
  int tag;
  int value;
  
  /* Parentheses: function pointer with skip attribute */
  int (* GTY((skip)) callback)(
    struct node * GTY((skip)) child, 
    int depth
  );
  
  /* Brackets: array with variable length */
  struct node * GTY((length("child_count"))) children[];
  
  /* Nested brackets via macro expansion */
  PTR_ARRAY(struct node, grandchild_count) grandchildren;
  
  /* Double pointer array */
  NESTED_PTR_ARRAY(struct node) matrix;
  
  /* Braces: nested union within structure */
  union {
    int int_val;
    char * GTY((tag("1"))) str_data;
    void * GTY((tag("2"))) ptr_data;
    struct {
      int x;
      int y;
      int (* GTY((skip)) compute)(int, int); /* More parentheses */
    } GTY((tag("3"))) coord;
  } variant;
  
  struct node *next;
  struct node *prev;
  
  /* Array of function pointers (parentheses in type) */
  void (* GTY((length("func_count"), skip)) handlers[])(
    struct node *,
    int * GTY((skip)) result
  );
};

/* Union type with conditional attributes */
union GTY((
  desc("%0.tag"),
  param_is(variant_data)
)) variant_data {
  int GTY((tag("0"))) tag;
  double GTY((tag("1"))) number;
  struct node * GTY((tag("2"))) node_ptr;
  
  /* Nested structure with array */
  struct {
    int count;
    char * GTY((length("count"))) strings[];
  } GTY((tag("3"))) string_list;
};

/* Global variable declarations */
extern node_t * GTY(()) global_tree_root;
extern union variant_data * GTY(()) global_variants[];

#endif /* TEST_GTY_H */
