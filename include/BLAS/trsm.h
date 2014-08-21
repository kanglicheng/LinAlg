/** \file             gemm.h
 *
 *  \date             Created:  Jul 16, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcalderara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_BLAS_TRSM_H_
#define LINALG_BLAS_TRSM_H_

/* Organization of the namespace:
 *
 *    LinAlg::BLAS
 *        convenience bindings supporting different locations for Dense<T>
 *
 *        'Abstract' functions like 'solve' and 'invert'
 *
 *    LinAlg::BLAS::<NAME>
 *        bindings to the <NAME> BLAS backend
 */

#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include "cuda_helper.h"
#endif

#include "types.h"

namespace LinAlg {

namespace BLAS {

namespace FORTRAN {

#ifndef DOXYGEN_SKIP
extern C {
  void fortran_name(strsm, STRSM)(char* side, char* uplo, char* transa,
                                  char* diag, I_t* m, I_t* n, S_t* alpha,
                                  S_t* A, I_t* lda, S_t* B, I_t* ldb);
  void fortran_name(dtrsm, DTRSM)(char* side, char* uplo, char* transa,
                                  char* diag, I_t* m, I_t* n, D_t* alpha,
                                  D_t* A, I_t* lda, D_t* B, I_t* ldb);
  void fortran_name(ctrsm, CTRSM)(char* side, char* uplo, char* transa,
                                  char* diag, I_t* m, I_t* n, C_t* alpha,
                                  C_t* A, I_t* lda, C_t* B, I_t* ldb);
  void fortran_name(ztrsm, ZTRSM)(char* side, char* uplo, char* transa,
                                  char* diag, I_t* m, I_t* n, Z_t* alpha,
                                  Z_t* A, I_t* lda, Z_t* B, I_t* ldb);
}
#endif

/** \brief            Triangular matrix solve
 *
 *  op(A) * X = alpha * B
 *  X * op(A) = alpha * B
 *
 *  \param[in]        side
 *
 *  \param[in]        uplo
 *
 *  \param[in]        transa
 *
 *  \param[in]        diag
 *
 *  \param[in]        m
 *
 *  \param[in]        n
 *
 *  \param[in]        alpha
 *
 *  \param[in]        A
 *
 *  \param[in|out]    B
 *
 *  \param[in]        ldb
 *
 *
 *  See [DTRSM](http://www.mathkeisan.com/usersguide/man/dtrsm.html)
 */
inline void xTRSM(char side, char uplo, char transa, char diag, int m,
                  int n, S_t alpha, S_t* A, int lda, S_t* B, int ldb) {
  fortran_name(strsm, STRSM)(&side, &uplo, &transa, &diag, &m, &n, &alpha, A,
                             &lda, B, &ldb);
}
/** \overload
 */
inline void xTRSM(char side, char uplo, char transa, char diag, int m,
                  int n, D_t alpha, D_t* A, int lda, D_t* B, int ldb) {
  fortran_name(dtrsm, DTRSM)(&side, &uplo, &transa, &diag, &m, &n, &alpha, A,
                             &lda, B, &ldb);
}
/** \overload
 */
inline void xTRSM(char side, char uplo, char transa, char diag, int m,
                  int n, C_t alpha, C_t* A, int lda, C_t* B, int ldb) {
  fortran_name(ctrsm, CTRSM)(&side, &uplo, &transa, &diag, &m, &n, &alpha, A,
                             &lda, B, &ldb);
}
/** \overload
 */
inline void xTRSM(char side, char uplo, char transa, char diag, int m,
                  int n, Z_t alpha, Z_t* A, int lda, Z_t* B, int ldb) {
  fortran_name(ztrsm, ZTRSM)(&side, &uplo, &transa, &diag, &m, &n, &alpha, A,
                             &lda, B, &ldb);
}

} /* namespace LinAlg::BLAS::FORTRAN */

#ifdef HAVE_CUDA
namespace CUBLAS {

/** \brief            Triangular matrix solve
 *
 *  op(A) * X = alpha * B
 *  X * op(A) = alpha * B
 *
 *  \param[in]        handle
 *
 *  \param[in]        side
 *
 *  \param[in]        uplo
 *
 *  \param[in]        transa
 *
 *  \param[in]        diag
 *
 *  \param[in]        m
 *
 *  \param[in]        n
 *
 *  \param[in]        alpha
 *
 *  \param[in]        A
 *
 *  \param[in|out]    B
 *
 *  \param[in]        ldb
 *
 *  See [CUBLAS Documentation](http://docs.nvidia.com/cuda/cublas/)
 */
inline void xTRSM(cublasHandle_t handle, cublasSideMode_t side,
                  cublasFillMode_t uplo, cublasOperation_t trans,
                  cublasDiagType_t diag, I_t m, I_t n, const S_t alpha,
                  const S_t* A, I_t lda, S_t* B, I_t ldb) {
  checkCUBLAS(cublasStrsm(handle, side, uplo, trans, diag, m, n, &alpha, A, \
                          lda, B, ldb));
}
/** \overload
 */
inline void xTRSM(cublasHandle_t handle, cublasSideMode_t side,
                  cublasFillMode_t uplo, cublasOperation_t trans,
                  cublasDiagType_t diag, I_t m, I_t n, const D_t alpha,
                  const D_t* A, I_t lda, D_t* B, I_t ldb) {
  checkCUBLAS(cublasDtrsm(handle, side, uplo, trans, diag, m, n, &alpha, A, \
                          lda, B, ldb));
}
/** \overload
 */
inline void xTRSM(cublasHandle_t handle, cublasSideMode_t side,
                  cublasFillMode_t uplo, cublasOperation_t trans,
                  cublasDiagType_t diag, I_t m, I_t n, const C_t alpha,
                  const C_t* A, I_t lda, C_t* B, I_t ldb) {
  checkCUBLAS(cublasCtrsm(handle, side, uplo, trans, diag, m, n, \
                          (const cuComplex*)&alpha, (const cuComplex*)A, lda, \
                          (cuComplex*)B, ldb));
}
/** \overload
 */
inline void xTRSM(cublasHandle_t handle, cublasSideMode_t side,
                  cublasFillMode_t uplo, cublasOperation_t trans,
                  cublasDiagType_t diag, I_t m, I_t n, const Z_t alpha,
                  const Z_t* A, I_t lda, Z_t* B, I_t ldb) {
  checkCUBLAS(cublasZtrsm(handle, side, uplo, trans, diag, m, n, \
                          (const cuDoubleComplex*)&alpha, \
                          (const cuDoubleComplex*)A, lda, \
                          (cuDoubleComplex*)B, ldb));
}


} /* namespace LinAlg::BLAS::CUBLAS */
#endif

} /* namespace LinAlg::BLAS */

} /* namespace LinAlg */

#endif /* LINALG_BLAS_TRSM_H_ */