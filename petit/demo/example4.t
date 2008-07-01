auto real a(100)
inout real b(100,100)
!
! In this example, no flow dependences are carried by the outer
! loop (although there are flow dependences on a carried from the 
! entry node)
!
integer n, m
for L1 = 1 to n do
  for L2 = m-L1 to m do
    a(L2) = a(L2-1) + b(L1,L2)
  endfor
endfor
