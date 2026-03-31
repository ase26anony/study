+------------+
| field1     |  (4 bytes typically)
+------------+
| union u    |  (40 bytes typically - size of int[10])
|   ptr      |
|   OR       |
|   arr[10]  |
+------------+
| callback   |  (8 bytes typically on 64-bit systems)
+------------+
