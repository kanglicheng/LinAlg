/** \file
 *
 *  \brief            Conversion of storage formats
 *
 *  \date             Created:  Jul 12, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mauro@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_UTILITIES_FORMAT_CONVERT_H_
#define LINALG_UTILITIES_FORMAT_CONVERT_H_

#include "../types.h"
#include "../exceptions.h"
#include "../dense.h"
#include "../sparse.h"
#include "../fills.h"

namespace LinAlg {

namespace Utilities {

/** \brief            Add a subblock of a sparse matrix to a dense matrix
 *
 *
 *  \param[in]        src
 *                    The sparse matrix to extract the subblock from.
 *
 *  \param[in]        first_row
 *                    Row to start extraction (included, c-numbering).
 *
 *  \param[in]        first_col
 *                    Colum to start extraction (included, c-numbering).
 *
 *  \param[in]        last_row
 *                    Row to stop extraction (excluded, c-numbering).
 *
 *  \param[in]        last_col
 *                    Column to stop extraction (excluded, c-numbering).
 *
 *  \param[out]       dst
 *                    Dense matrix to store the subblock.
 *
 *  \todo             If the full matrix is to be converted, mkl_?dnscsr and
 *                    cusparseXdense2csr could be used
 */
template <typename T>
inline void sparse2dense(Sparse<T>& src, I_t first_row, I_t last_row,
                         I_t first_col, I_t last_col, Dense<T>& dst) {

#ifndef LINALG_NO_CHECKS
  if (dst._transposed) {
    throw excUnimplemented("sparse2dense(): assignment to transposed dense "
                           "matrices is not supported.");
  } else if (src._transposed) {
    throw excUnimplemented("sparse2dense(): assignment from transposed sparse "
                           "matrices is not supported.");
  } else if (src._format == Format::CSC) {
    throw excUnimplemented("sparse2dense(): assignment from sparse matrices in "
                           "CSC format is not supported.");
  }

  if ((first_row > src._size - 1) || (last_row > src._size)) {
    throw excBadArgument("sparse2dense(): requested row range is not contained "
                         "in the source matrix.");
  }
#endif

  if (src._location == Location::host && dst._location == Location::host) {

    auto rows = last_row - first_row;
    auto cols = last_col - first_col;

    // Check if size fits. If dst is empty, reallocate accordingly
    if (dst._rows == 0) {

      dst.reallocate(rows, cols);

      Fills::zero(dst);

    }
#ifndef LINALG_NO_CHECKS
    else if (dst._rows != rows || dst._cols != cols) {

      throw excBadArgument("sparse2dense(): matrix dimension mismatch");

    }
#endif

    auto first_index = src._first_index;
    auto edges = src._edges.get();
    auto indices = src._indices.get();
    auto values = src._values.get();

    auto dst_data = dst._begin();
    auto dst_ld = dst._leading_dimension;

    for (I_t row_in = first_row; row_in < last_row; ++row_in) {

      for (I_t index = edges[row_in] - first_index;
           index < edges[row_in + 1] - first_index; ++index) {

        I_t col_in = indices[index] - first_index;

        if (col_in < first_col) {

          continue;

        } else if (col_in < last_col) {

          I_t col_out   = col_in - first_col;
          I_t row_out   = row_in - first_row;

          I_t array_pos;
          if (dst._format == Format::ColMajor) {
            array_pos = col_out * dst_ld + row_out;
          } else {
            array_pos = row_out * dst_ld + col_out;
          }

          dst_data[array_pos] += values[index];

        } else {

          break;

        }
      }
    }

  }

#ifndef LINALG_NO_CHECKS
  else {

    throw excUnimplemented("sparse2dense(): only matrices in main memory are "
                           "supported");

  }
#endif

  // if both on GPU: cusparseXcsr2dense

  // if mixed: transfer csr to GPU, call sparse2dense recursively

}

/*
template <typename T>
inline void dense2sparse(Dense<T> src, I_t first_row, I_t last_row,
                         I_t first_col, I_t last_col, Sparse<T> dst) {

  // if both on host: do it yourself + mkl_xdnscsr

  // if both on GPU: cusparseXdense2csr

  // if mixed: transfer csr to GPU, call dense2sparse recursively

}
*/

} /* namespace LinAlg::Utilities */

} /* namespace LinAlg */


#endif /* LINALG_UTILITIES_FORMAT_CONVERT_H_ */
