real a(100,100),b(100)
integer n
!
! For LU decomposition, we can also
! obtain all 6 permutations. Normal
! loop interchange techniques can't
! produce the ijk ordering
!
!
for k = 1 to n do
 for i = k+1 to n do
  a(i,k) = a(i,k) / a(k,k)
  for j = k+1 to n do
   a(i,j) = a(i,j) - a(i,k)*a(k,j)
  endfor
 endfor
endfor
