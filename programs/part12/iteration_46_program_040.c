struct type {
  enum type_kind kind;      // What kind of type this is
  struct type *next;        // For creating linked lists of types
  
  union {
    struct type *pointer_to;    // For TYPE_POINTER: what it points to
    struct type *element_type;  // For TYPE_ARRAY: element type
    // Other fields would be here for other type kinds
  } u;
};
