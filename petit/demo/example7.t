real a(101,102), c(103,104)
integer x,y,n,m

!
!
!
!
!
!
!
!
! example of "Zappable" dependences
!
!

for L1 = x to n do
  for L2 = 1 to m do
    a(L1,L2) = a(L1-x,y) + c(L1,L2)
  endfor
endfor
