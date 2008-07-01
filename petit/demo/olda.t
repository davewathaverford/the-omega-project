
! subroutine olda_

formal integer ndim
formal integer morb
formal real v(ndim,1)
formal real xrspq(1)
formal real xrsij(1)
formal real xijrs(1)
formal real xijkl(1)
formal real xrsiq(morb,1)
formal real xijks(morb,1)
formal real xij(1)
formal real xkl(1)
formal integer norb
formal integer num

    static real zero = 0.

    ! System generated locals
    integer v_offset
    integer xrsiq_offset
    integer xijks_offset;

    ! Local variables
    integer lmin, lmax, mijkl, mleft, mrsij, mijrs, mrspq, mrsij0, mi, mj, mk, ml, mp, np, nq, mq, nr, ns, ms, mr, mij, nij
    real val
    real dum
    integer mrs
in  integer nrs

! 
!     ----- THIS IS THE ORIGINAL -TRFCOR- FROM -HONDO- -----
! 
! 
!     ----- TRANSFORM -P- AND -Q- -----
! 
!   nrs = num * (num + 1) / 2
    np = num
    nq = num
    mrsij0 = 0
    mrspq = 0
! 
    do mrs = 1, nrs {
! 
	do mq = 1, num {
	    do mi = 1, morb {
		xrsiq(mi,mq) = zero
	    }
	}
	do mp = 1, num {
	    do mq = 1, mp {
		mrspq = mrspq + 1
		val = xrspq(mrspq)
		if (val != zero) then
		    do mi = 1, morb {
			xrsiq(mi,mq) = xrsiq(mi,mq) + val * v(mp,mi)
			xrsiq(mi,mp) = xrsiq(mi,mp) + val * v(mq,mi)
		    }
		endif
	    }
	}
! 
	mrsij = mrsij0
	do mi = 1, morb {
	    do mj = 1, mi {
		xij(mj) = zero
	    }
	    do mq = 1, num {
		val = xrsiq(mi,mq)
		if (val != zero) then
		    do mj = 1, mi {
			xij(mj) = xij(mj) + val * v(mq,mj)
		    }
		endif
	    }
	    do mj = 1, mi {
		mrsij = mrsij + 1
		xrsij(mrsij) = xij(mj)
	    }
	}
	mrsij0 = mrsij0 + nrs
    }
! 
!     ----- TRANSPOSE (R,S//I,J) -----
! 
    nij = morb * (morb + 1) / 2
    mrsij = 0
    do mrs = 1, nrs {
	mijrs = 0
	do mij = 1, min(mrs,nij) {
	    dum = xrsij(mrsij + mij)
	    xijrs(mrsij + mij) = xrsij(mijrs + mrs)
	    xijrs(mijrs + mrs) = dum
	    mijrs = mijrs + nrs
	}
	mrsij = mrsij + nrs
    }
! 
!     ----- TRANSFORM -R- AND -S- -----
! 
    nr = num
    ns = num
    mijkl = 0
    mijrs = 0
    mij = 0
    mleft = nrs - nij
! 
    do mi = 1, morb {
	do mj = 1, mi {
	    mij = mij + 1
 
	    do ms = 1, num {
		do mk = mi, morb {
		    xijks(mk,ms) = zero
		}
	    }
	    do mr = 1, num {
		do ms = 1, mr {
		    mijrs = mijrs + 1
		    val = xijrs(mijrs)
		    if (val != zero) then
			do mk = mi, morb {
			    xijks(mk,ms) = xijks(mk,ms) + val * v(mr,mk)
			    xijks(mk,mr) = xijks(mk,mr) + val * v(ms,mk)
			}
		    endif
		}
	    }
! 
	    lmin = mj
	    lmax = mi
	    do mk = mi, morb {
		do ml = lmin, lmax {
		    xkl(ml) = zero
		}
		do ms = 1, num {
		    val = xijks(mk,ms)
		    if (val != zero) then
			do ml = lmin, lmax {
			    xkl(ml) = xkl(ml) + val * v(ms,ml)
			}
		    endif
		}
		do ml = lmin, lmax {
		    mijkl = mijkl + 1
		    xijkl(mijkl) = xkl(ml)
		}
		lmin = 1
		lmax = mk + 1
	    }
	    mijkl = mijkl + mij + mleft
! 
	}
    }
    return 0

