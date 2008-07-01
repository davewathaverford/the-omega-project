real a(100), c(100)
integer n,m
!
! This example shows refinement of a coupled dependence
!
for L1 = 1 to n do
  for L2 = 1 to m do
    a(L1-L2) = a(L1-L2) + c(L1)
  endfor
endfor
