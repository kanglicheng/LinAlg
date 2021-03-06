/** \file
 *
 *  \brief            Inclusion of all BLAS and BLAS-like headers
 *
 * Organization of the namespace:
 *
 *    LinAlg::BLAS
 *        convenience bindings supporting different locations for Dense<T>
 *
 *    LinAlg::BLAS::\<NAME\>
 *        bindings to the \<NAME\> BLAS (or BLAS-like) backend
 *
 *
 *  \date             Created:  Jul 16, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcalderara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_BLAS_H_
#define LINALG_BLAS_H_

// Keep this in alphabetical order
#include "axpy.h"
#include "cusparse/csc2dense.h"
#include "cusparse/csr2dense.h"
#include "copy.h"
#include "geam.h"
#include "gemm.h"
#include "omatcopy.h"
#include "trsm.h"

#endif /* LINALG_BLAS_H_ */
