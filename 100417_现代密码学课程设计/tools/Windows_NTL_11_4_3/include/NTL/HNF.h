
#ifndef NTL_HNF__H
#define NTL_HNF__H

#include <NTL/mat_ZZ.h>

NTL_OPEN_NNS

void HNF(mat_ZZ& W, const mat_ZZ& A, const ZZ& D);
// The input matrix A is an n x m matrix of rank m (so n >= m), and
// D is a multiple of the determinant of the lattice L spanned by 
// the rows of A.
// W is computed as the Hermite Normal Form of A;
// that is, W is the unique m x m matrix whose rows span L, such that
//   - W is lower triangular,
//   - the diagonal entries are positive,
//   - any entry below the diagonal is a non-negative number
//     strictly less than the diagonal entry in its column.

// Currently, this is implemented using the algorithm of 
// [P. Domich, R. Kannan and L. Trotter, Math. Oper. Research 12:50-59, 1987].

NTL_CLOSE_NNS

#endif
