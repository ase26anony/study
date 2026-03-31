// Create an int type
struct type int_type = {TYPE_SCALAR, NULL};

// Create a pointer to int
struct type int_ptr_type = {TYPE_POINTER, NULL};
int_ptr_type.u.pointer_to = &int_type;

// Create an array of 10 ints
struct type int_array_type = {TYPE_ARRAY, NULL};
int_array_type.u.element_type = &int_type;
// Would likely have another field for array size
