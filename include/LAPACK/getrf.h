/** \file             getrf.h
 *
 *  \date             Created:  Jul 16, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcalderara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_LAPACK_GETRF_H_
#define LINALG_LAPACK_GETRF_H_

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
#include "cuda_helper.h"
#endif

#include "types.h"
#include "exceptions.h"
#include "checks.h"
#include "dense.h"

namespace LinAlg {

namespace LAPACK {

namespace FORTRAN {

#ifndef DOXYGEN_SKIP
extern C {
  void fortran_name(sgetrf, SGETRF)(const I_t* m, const I_t* n, S_t* A,
                                    const I_t* lda, I_t* ipiv, int* info);
  void fortran_name(dgetrf, DGETRF)(const I_t* m, const I_t* n, D_t* A,
                                    const I_t* lda, I_t* ipiv, int* info);
  void fortran_name(cgetrf, CGETRF)(const I_t* m, const I_t* n, C_t* A,
                                    const I_t* lda, I_t* ipiv, int* info);
  void fortran_name(zgetrf, ZGETRF)(const I_t* m, const I_t* n, Z_t* A,
                                    const I_t* lda, I_t* ipiv, int* info);
}
#endif

/** \brief            Compute LU factorization
 *
 *  A <- P * L * U
 *
 *  \param[in]        m
 *
 *  \param[in]        n
 *
 *  \param[in|out]    A
 *
 *  \param[in]        lda
 *
 *  \param[in|out]    ipiv
 *
 *  \param[in|out]    info
 *
 *  See
 *  [DGETRF](http://www.math.utah.edu/software/lapack/lapack-d/dgetrf.html)
 */
inline void xGETRF(I_t m, I_t n, S_t* A, I_t lda, I_t* ipiv, int* info) {
  fortran_name(sgetrf, SGETRF)(&m, &n, A, &lda, ipiv, info);
};
/** \overload
 */
inline void xGETRF(I_t m, I_t n, D_t* A, I_t lda, I_t* ipiv, int* info) {
  fortran_name(dgetrf, DGETRF)(&m, &n, A, &lda, ipiv, info);
};
/** \overload
 */
inline void xGETRF(I_t m, I_t n, C_t* A, I_t lda, I_t* ipiv, int* info) {
  fortran_name(cgetrf, CGETRF)(&m, &n, A, &lda, ipiv, info);
};
/** \overload
 */
inline void xGETRF(I_t m, I_t n, Z_t* A, I_t lda, I_t* ipiv, int* info) {
  fortran_name(zgetrf, ZGETRF)(&m, &n, A, &lda, ipiv, info);
};

} /* namespace FORTRAN */


#ifdef HAVE_CUDA
namespace CUBLAS {

/** \brief            Compute LU factorization
 *
 *  A <- P * L * U
 *
 *  \param[in]        handle
 *
 *  \param[in]        n
 *
 *  \param[in|out]    A
 *
 *  \param[in]        lda
 *
 *  \param[in|out]    ipiv
 *
 *  \param[in|out]    info
 *
 *  See [CUBLAS Documentation](http://docs.nvidia.com/cuda/cublas/)
 */
inline void xGETRF(cublasHandle_t handle, I_t n, S_t* A, I_t lda, int* ipiv,
                   int* info) {
  checkCUBLAS(cublasSgetrfBatched(handle, n, &A, lda, ipiv, info, 1));
};
/** \overload
 */
inline void xGETRF(cublasHandle_t handle, I_t n, D_t* A, I_t lda, int* ipiv,
                   int* info) {
  checkCUBLAS(cublasDgetrfBatched(handle, n, &A, lda, ipiv, info, 1));
};
/** \overload
 */
inline void xGETRF(cublasHandle_t handle, I_t n, C_t* A, I_t lda, int* ipiv,
                   int* info) {
  checkCUBLAS(cublasCgetrfBatched(handle, n, \
                                  reinterpret_cast<cuComplex**>(&A),lda, ipiv, \
                                  info, 1));
};
/** \overload
 */
inline void xGETRF(cublasHandle_t handle, I_t n, Z_t* A, I_t lda, int* ipiv,
                   int* info) {
  checkCUBLAS(cublasZgetrfBatched(handle, n, \
                                  reinterpret_cast<cuDoubleComplex**>(&A), \
                                  lda, ipiv, info, 1));
};

/** \brief            Compute LU factorization for multiple matrices
 *
 *  A <- P * L * U
 *
 *  \param[in]        handle
 *
 *  \param[in]        n
 *
 *  \param[in|out]    Aarray[]
 *
 *  \param[in]        lda
 *
 *  \param[in|out]    PivotArray
 *
 *  \param[in|out]    infoArray
 *
 *  \param[in]        batchSize
 */
inline void xGETRF_batched(cublasHandle_t handle, I_t n, S_t* Aarray[], I_t lda,
                           int* PivotArray, int* infoArray, I_t batchSize) {
  checkCUBLAS(cublasSgetrfBatched(handle, n, Aarray, lda, PivotArray, \
                                  infoArray, batchSize));
};
/** \overload
 */
inline void xGETRF_batched(cublasHandle_t handle, I_t n, D_t* Aarray[], I_t lda,
                           int* PivotArray, int* infoArray, I_t batchSize) {
  checkCUBLAS(cublasDgetrfBatched(handle, n, Aarray, lda, PivotArray, \
                                  infoArray, batchSize));
};
/** \overload
 */
inline void xGETRF_batched(cublasHandle_t handle, I_t n, C_t* Aarray[], I_t lda,
                           int* PivotArray, int* infoArray, I_t batchSize) {
  checkCUBLAS(cublasCgetrfBatched(handle, n, \
                                  reinterpret_cast<cuComplex**>(Aarray), lda, \
                                  PivotArray, infoArray, batchSize));
};
/** \overload
 */
inline void xGETRF_batched(cublasHandle_t handle, I_t n, Z_t* Aarray[], I_t lda,
                           int* PivotArray, int* infoArray, I_t batchSize) {
  checkCUBLAS(cublasZgetrfBatched(handle, n, \
                                  reinterpret_cast<cuDoubleComplex**>(Aarray), \
                                  lda, PivotArray, infoArray, batchSize));
};

} /* namespace LinAlg::LAPACK::CUBLAS */
#endif

using LinAlg::Utilities::check_device;
using LinAlg::Utilities::check_input_transposed;

/** \brief            Compute the LU decomposition of a matrix
 *
 *  A = P * L * U     A is overwritten with L and U
 *
 *  \param[in]        A
 *
 *  \param[in]        ipiv
 *                    ipiv is A.rows()*1 matrix (a vector). When using the
 *                    CUBLAS backend and specifying an empty matrix for ipiv,
 *                    the routine performs a non-pivoting LU decomposition.
 *
 *  \note             The return value of the routine is checked and a
 *                    corresponding exception is thrown if the matrix is
 *                    singular.
 *
 *  \todo             MAGMA's GETRF function also supports non-square matrices.
 */
template <typename T>
inline void xGETRF(Dense<T>& A, Dense<int>& ipiv) {

#ifndef LINALG_NO_CHECKS
  check_device(A, ipiv, "xGETRF()");
  check_input_transposed(A, "xGETRF()");
#ifndef HAVE_CUDA
  check_input_transposed(ipiv, "xGETRF()");
#else
  bool ipiv_empty = (ipiv._rows == 0) ? true : false;
  if (!ipiv_empty) {
    check_input_transposed(ipiv, "xGETRF()");
  }
#endif

#ifndef HAVE_CUDA
  if (A.rows() != ipiv.rows()) {
    throw excBadArgument("xGETRF(): argument matrix size mismatch");
  }
#else
  if (A.rows() != A.cols() && A._location == Location::GPU) {
    throw excBadArgument("xGETRF(): matrix A must be a square matrix (CUBLAS "
                         "restriction)");
  }
  if (!ipiv_empty) {
    if (A.rows() != ipiv.rows()) {
      throw excBadArgument("xGETRF(): argument matrix size mismatch");
    }
  }
#endif
#endif /* LINALG_NO_CHECKS */

  auto device_id = A._device_id;
  auto n = A.cols();
  auto A_ptr = A._begin();
  auto lda = A._leading_dimension;
  auto ipiv_ptr = ipiv._begin();
  int  info = 0;

  if (A._location == Location::host) {
    auto m = A.cols();
    FORTRAN::xGETRF(m, n, A_ptr, lda, ipiv_ptr, &info);
  }
#ifdef HAVE_CUDA
  else if (A._location == Location::GPU) {
    using LinAlg::CUDA::CUBLAS::handles;
    CUBLAS::xGETRF(handles[device_id], n, A_ptr, lda, ipiv_ptr, &info);
  }
#ifdef HAVE_MAGMA
  // check if MAGMA's or CUBLAS' GETRF is faster and use that one.
#endif
#endif

#ifndef LINALG_NO_CHECKS
  else {
    throw excUnimplemented("xGETRF(): LAPACK GETRF not supported on selected "
                           "location");
  }

  if (info != 0) {
    throw excMath("xGETRF(): error: info = %d", info);
  }
#endif

};

} /* namespace LinAlg::LAPACK */

} /* namespace LinAlg */

#endif /* LINALG_LAPACK_GETRF_H_ */