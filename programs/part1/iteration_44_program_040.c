struct my_struct {
  int field1;                    // Regular integer field
  union {                        // Anonymous union
    char *ptr;                   // Either a character pointer
    int arr[10];                 // OR an integer array of 10 elements
  } u;                           // Union variable named 'u'
  void (*callback)(int, char);   // Function pointer
};
