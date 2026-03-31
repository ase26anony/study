+------------+
| field1     |  (4 bytes typically)
+------------+
| u.ptr      |  (or u.arr[0])
| ...        |  (union members overlap)
| u.arr[9]   |
+------------+
| callback   |  (function pointer)
+------------+
