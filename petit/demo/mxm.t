integer m,n,p
real a(1:100,1:100), b(1:100,1:100), c(1:100,1:100)
!
! We can use our unified reordering transformation
! framework to consider all 6 permutations of the ijk
! loops
!
!
for i = 1,n do
  for j = 1,p do
    for k = 1,m do
      c(i,j) = c(i,j)+a(i,k)*b(k,j)
    endfor
  endfor
endfor
