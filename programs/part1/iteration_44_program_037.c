struct my_struct {
  int field1;
  union {
    char *ptr;
    int arr[10];
  } u;
  void (*callback)(int, char);
};
