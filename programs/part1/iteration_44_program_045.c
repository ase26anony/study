+------------+
| field1     |  (4 bytes, typically)
+------------+
| u.ptr      |  (8 bytes on 64-bit systems)
| or         |
| u.arr[10]  |  (40 bytes = 10 * 4 bytes)
+------------+
| callback   |  (8 bytes on 64-bit systems)
+------------+
