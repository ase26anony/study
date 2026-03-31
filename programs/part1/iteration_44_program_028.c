+----------------+
| field1 (int)   |
+----------------+
| u.ptr          |  <-- Union: either ptr OR arr[10]
| or u.arr[0]    |
| ...            |
| u.arr[9]       |
+----------------+
| callback       |
+----------------+
