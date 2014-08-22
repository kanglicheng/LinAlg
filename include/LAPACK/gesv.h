/** \file
 *
 *  \brief            xGESV
 *
 *  \date             Created:  Jul 16, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcalderara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_LAPACK_GESV_H_
#define LINALG_LAPACK_GESV_H_

/* Organization of the namespace:
 *
 *    LinAlg::LAPACK
 *        convenience bindings supporting different locations for Dense<T>
 *
 *    LinAlg::LAPACK::<NAME>
 *        bindings to the <NAME> LAPACK backend
 */

#ifdef HAVE_CUDA

#include <cuda_runtime.h>
#include <cublas_v2.h>
#include "../CUDA/cuda_checks.h"

#ifdef HAVE_MAGMA
#include <magma.h>
#include <magma_lapack.h>
#endif

#endif


#include "../types.h"
#include "../exceptions.h"
#include "../utilities/checks.h"
#include "../dense.h"
#include "getrf.h"

namespace LinAlg {

namespace LAPACK {

namespace FORTRAN {

#ifndef DOXYGEN_SKIP
extern "C" {
  void fortran_name(sgesv, SGESV)(const I_t* n, const I_t* nrhs, S_t* A,
                                  const I_t* lda, I_t* ipiv, S_t* B,
                                  const I_t* ldb, int* info);
  void fortran_name(dgesv, DGESV)(const I_t* n, const I_t* nrhs, D_t* A,
                                  const I_t* lda, I_t* ipiv, D_t* B,
                                  const I_t* ldb, int* info);
  void fortran_name(cgesv, CGESV)(const I_t* n, const I_t* nrhs, C_t* A,
                                  const I_t* lda, I_t* ipiv, C_t* B,
                                  const I_t* ldb, int* info);
  void fortran_name(zgesv, ZGESV)(const I_t* n, const I_t* nrhs, Z_t* A,
                                  const I_t* lda, I_t* ipiv, Z_t* B,
                                  const I_t* ldb, int* info);
}
#endif

/** \brief            GESV
 *
 *  X = A^(-1) * B
 *
 *  \param[in]        n
 *
 *  \param[in]        nrhs
 *
 *  \param[in]        A
 *
 *  \param[in]        lda
 *
 *  \param[in]        ipiv
 *
 *  \param[in,out]    B
 *
 *  \param[in]        ldb
 *
 *  \param[in,out]    info
 *
 *  See [DGESV](http://www.mathkeisan.com/UsersGuide/man/dgesv.html)
 */
inline void xGESV(I_t n, I_t nrhs, S_t* A, I_t lda, int* ipiv, S_t* B, int ldb,
                  int* info) {
  fortran_name(sgesv, SGESV)(&n, &nrhs, A, &lda, ipiv, B, &ldb, info);
};
/** \overload
 */
inline void xGESV(I_t n, I_t nrhs, D_t* A, I_t lda, int* ipiv, D_t* B, int ldb,
                  int* info) {
  fortran_name(dgesv, DGESV)(&n, &nrhs, A, &lda, ipiv, B, &ldb, info);
};
/** \overload
 */
inline void xGESV(I_t n, I_t nrhs, C_t* A, I_t lda, int* ipiv, C_t* B, int ldb,
                  int* info) {
  fortran_name(cgesv, CGESV)(&n, &nrhs, A, &lda, ipiv, B, &ldb, info);
};
/** \overload
 */
inline void xGESV(I_t n, I_t nrhs, Z_t* A, I_t lda, int* ipiv, Z_t* B, int ldb,
                  int* info) {
  fortran_name(zgesv, ZGESV)(&n, &nrhs, A, &lda, ipiv, B, &ldb, info);
};

} /* namespace LinAlg::LAPACK::FORTRAN */



using LinAlg::Utilities::check_device;
using LinAlg::Utilities::check_format;
using LinAlg::Utilities::check_input_transposed;
using LinAlg::Utilities::check_dimensions;

/** \brief            xGESV
 *
 *  B <- A^(-1) * B
 *
 *  \param[in,out]    A
 *
 *  \param[in,out]    ipiv
 *
 *  \param[in,out]    B
 *
 *  \todo             Actually one should check if B is a vector and call
 *                    xTRSV instead of xTRSM in the CUDA code.
 */
template <typename T>
inline void xGESV(Dense<T>& A, Dense<int>& ipiv, Dense<T>& B) {

#ifndef LINALG_NO_CHECKS
  check_device(A, B, ipiv, "xGESV(A, ipiv, B)");
  check_format(Format::ColMajor, A, "xGESV(A, ipiv, B), A");
  check_format(Format::ColMajor, B, "xGESV(A, ipiv, B), B");
  check_dimensions(A.rows(), B.cols(), B, "xGESV(A, ipiv, B), B");
  check_dimensions(A.rows(), 1, ipiv, "xGESV(A, ipiv, B), ipiv");
  check_input_transposed(A, "xGESV(A, ipiv, B), A (even though A^T could be "
                         "implemented)");
  check_input_transposed(B, "xGESV(A, ipiv, B), B");
#endif /* LINALG_NO_CHECKS */

  auto n             = A.rows();
  auto nrhs          = B.cols();
  auto A_ptr         = A._begin();
  auto lda           = A._leading_dimension;
  auto B_ptr         = B._begin();
  auto ldb           = B._leading_dimension;
  auto ipiv_ptr      = ipiv._begin();
  int  info          = 0;

  if (A._location == Location::host) {

    FORTRAN::xGESV(n, nrhs, A_ptr, lda, ipiv_ptr, B_ptr, ldb, &info);
#ifndef LINALG_NO_CHECKS
    if (info != 0) {
      throw excMath("xGESV(): error = %d)", info);
    }
#endif

  }
#ifdef HAVE_CUDA
  else if (A._location == Location::GPU) {

    using LinAlg::CUDA::CUBLAS::handles;

#ifndef USE_MAGMA_GESV
    // This is basically xGESV done by hand.

#ifndef LINALG_NO_CHECKS
    // Note that the MAGMA GETRF would support non-square matrices
    if (A._rows != A._cols) {
      throw excBadArgument("xGETRF(A, ipiv, B), A: matrix must be square");
    }
#endif

    auto handle        = CUDA::CUBLAS::handles[A._device_id];
    auto ipiv_override = nullptr;

    // LU decompose using xGETRF without pivoting (use ipiv_override == empty
    // vector to enforce no pivoting. cudaXtrsm doesn't support pivoting, 
    // there is no xLASWP in CUBLAS, respectively)
    LAPACK::CUBLAS::xGETRF(handle, n, A_ptr, lda, ipiv_override, &info);

#ifndef LINALG_NO_CHECKS
    if (info != 0) {
      throw excMath("xGESV() (using CUBLAS::GETRF + CUBLAS::TRSM): unable to "
                    "LU decompose A (CUBLAS::xGETRF(): error = %d)", info);
    }
#endif

    // Directly solve using xTRSM (no xLASWP since we didn't pivot):
    // 1: y = L\b
    BLAS::CUBLAS::xTRSM(handle, CUBLAS_SIDE_LEFT, CUBLAS_FILL_MODE_LOWER,
                        CUBLAS_OP_N, CUBLAS_DIAG_UNIT, n, nrhs, T(1.0), A_ptr,
                        lda, B_ptr, ldb);
    // 2: x = U\y
    BLAS::CUBLAS::xTRSM(handle, CUBLAS_SIDE_LEFT, CUBLAS_FILL_MODE_UPPER,
                        CUBLAS_OP_N, CUBLAS_DIAG_NON_UNIT, n, nrhs, T(1.0),
                        A_ptr, lda, B_ptr, ldb);

#endif /* not USE_MAGMA_GESV */

  }
#endif /* HAVE_CUDA */

#ifndef LINALG_NO_CHECKS
  else {
    throw excUnimplemented("xGESV(): LAPACK GESV not supported on selected "
                           "location");
  }
#endif

};

} /* namespace LinAlg::LAPACK */

} /* namespace LinAlg */

#endif /* LINALG_LAPACK_GESV_H_ */
