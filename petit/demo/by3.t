real a(100,100)
!
! This examples shows some subtleties to the definition
! of dependence distance. Falling prey to these subtleties,
! earlier versions of Parascope, Parafrase and KAP all
! reported that no dependence existed in this loop.
!
for i = 1 to 10 do
  for j = i to 10 by 3 do
    a(i,j) = a(i-1,j+2)
  endfor
endfor
