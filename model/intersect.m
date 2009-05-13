function ans = perp(m)
	ans = m;
	ans(1) = -m(2);
	ans(2) = m(1);
endfunction
function ans = mmat(A,B,C,D)
	ans = dot((A - B), perp(C - D));
endfunction

function ans = calc_WEC(A,B,C,D)
	ans = (A(2) - B(2)) * (C(1) - D(1)) - (A(1) - B(1)) * (C(2) - D(2));
endfunction
function intersect(P1, P2, Q1, Q2)
	WEC_P1 = mmat(P1, Q1, Q2, Q1)
	WEC_P2 = mmat(P2, Q1, Q2, Q1)

	psum = WEC_P1 * WEC_P2
	if (psum <= 0)
		WEC_Q1 = mmat(Q1, P1, P2, P1)
		WEC_Q2 = mmat(Q2, P1, P2, P1)

		qsum = WEC_Q1 * WEC_Q2
		if (qsum <= 0)
			alphaP = WEC_P1/(WEC_P1 - WEC_P2)
			alphaQ = WEC_Q1/(WEC_Q1 - WEC_Q2)
		endif
	endif
endfunction

P1 = [-4;0];
P2 = [4;0];
Q1 = [0;4];
Q2 = [0;-4];
intersect(P1, P2, Q1, Q2);
