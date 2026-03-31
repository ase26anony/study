val_0 = 0  // initial value
for i = 0 to n:
  val_phi = Φ(val_0, val_prev)  // Phi node at loop header
  if (some_condition(i)):
    val_1 = 1
  else:
    val_2 = 0
  val_prev = Φ(val_1, val_2)  // Phi node after conditional
  if (val_prev == 1):
    // do work
  val_0 = val_prev  // for next iteration
