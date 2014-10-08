/** \file
 *
 *  \brief            Type definitions and preprocessor macros.
 *
 *  \date             Created:  Jan 5, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcaldrara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_TYPES_H_
#define LINALG_TYPES_H_

#include "elementary_types.h"

/** \def              fortran_name(x,y)
 *
 *  \brief            Fortran naming convention macro. Change names as per the
 *                    requirement of the linker.
 */
#ifndef fortran_name
#ifdef NoChange
#define fortran_name(x,y) (x)
#elif UpCase
#define fortran_name(x,y) (y)
#else
#define fortran_name(x,y) (x ## _)
#endif
#endif

namespace LinAlg {

/// Enum of datatypes
enum class Type {
  O,      //< Other type
  S,      //< Single precision floating point, real
  D,      //< Double precision floating point, real
  C,      //< Single precision floating point, complex
  Z,      //< Double precision floating point, complex
  I,      //< Integer type
};

/** \brief            Return the LinAlg::Type member corresponding to the
 *                    template instanciation
 *
 *  \returns          Type::O
 */
template <typename T> inline Type type()      { return Type::O; }
/** \overload
 *
 *  \returns          Type::S
 */
template <>           inline Type type<S_t>() { return Type::S; }
/** \overload
 *
 *  \returns          Type::D
 */
template <>           inline Type type<D_t>() { return Type::D; }
/** \overload
 *
 *  \returns          Type::C
 */
template <>           inline Type type<C_t>() { return Type::C; }
/** \overload
 *
 *  \returns          Type::Z
 */
template <>           inline Type type<Z_t>() { return Type::Z; }
/** \overload
 *
 *  \returns          Type::I
 */
template <>           inline Type type<I_t>() { return Type::I; }


/** \brief            Matrix properties
 *
 *  We store matrix properties as bitfield internally. The matrices' setter 
 *  members check for internal consistency.
 */
enum Property {
  General   = 0x01,
  Symmetric = 0x02,
  Hermitian = 0x04,
  Packed    = 0x08,
};

/** \brief            Storage locations
 *
 *  \note             This enum only includes members for which there is
 *                    compiled support. Thus, if you want to check for a 
 *                    certain location in a portable code you either have to 
 *                    use #ifdefs or use the .is_on_X() members of Dense<T> 
 *                    and Sparse<T>
 */
enum class Location {
    host,       //< Main memory
#ifdef HAVE_CUDA
    GPU,        //< GPGPU
#endif
#ifdef HAVE_MIC
    MIC,        //< Intel Xeon Phi / MIC
#endif
};

/** \brief            Storage formats
 */
enum class Format {
    ColMajor,   //< Column major (Fortran layout: [a_00, a_10, a_20, ...])
    RowMajor,   //< Row major (C/C++ layout: [a_00, a_01, a_02, ...])
    CSR,        //< Compressed Sparse Row
    CSC,        //< Compressed Sparse Column
};

/** \brief            Buffer types
 */
enum class BufferType {
  OnePass,      //< One pass in one direction, can start at either end
  TwoPass,      //< One pass in each direction, can start at either end
};

/** \brief            Buffer directions
 */
enum class BufferDirection {
  increasing,   //< Buffer runs in direction of increasing indices
  decreasing,   //< Buffer runs in direction of decreasing indices
};

/** \brief            IJ, a point in a matrix (row/column pair)
 */
struct IJ {

  I_t row;      //< Row
  I_t col;      //< Column

  IJ() : row(0), col(0) {};             //< Empty constructor
  IJ(I_t i, I_t j) : row(i), col(j) {}; //< Constructor from row and column

};


} /* namespace LinAlg */

#endif /* LINALG_TYPES_H_ */
