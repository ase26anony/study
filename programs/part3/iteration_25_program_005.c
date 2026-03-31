   x_initial = 0
   for i = 0 to N-1:
       x_phi = φ(x_initial, x_prev)  // Phi node
       if i % 2 == 0:
           x_current = 1
       else:
           x_current = 0
       x_prev = x_current
       
       if x_phi == 0:  // NOT constant - depends on previous iteration's x!
           // ...
