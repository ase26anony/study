val_0 = 0
for i = 0 to n:
  val_1 = Φ(val_0, val_3)  // Phi node at loop header
  if (some_condition(i)):
    val_2 = 1
  else:
    val_3 = 0
  val_4 = Φ(val_2, val_3)  // Phi node after if-else
  if (val_4 == 1):
    // do work
  val_0 = val_4  // loop carried dependency
