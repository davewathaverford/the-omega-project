real A(1:100,1:100)
real B(1:100,1:100)
real C(1:100,1:100)
real alpha
!
! As it stands, this program has poor cache performance, because none
! of the references to A and B are stride-1 in the inner-most loop.
! This can not be fixed by performing loop interchange.
! In order to make all of the array references stride 1 in the innermost
! loop, apply a uniform transformation and enable normalization.
!
integer n,b
for i = 1 to n do
  for j = i to min(i+2*b-2,n) do
    for k = max(i-b+1,j-b+1,1) to min(i+b-1,j+b-1,n) do
	C(i,j-i+1) = C(i,j-i+1) + alpha*A(k,i-k+b)*B(k,j-k+b) + alpha*A(k,j-k+b)*B(k,i-k+b)
    endfor
  endfor
endfor

