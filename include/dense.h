/** \file
 *
 *  \brief            Dense matrix struct (Dense<T>)
 *
 *  \date             Created:  Jul 11, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mauro@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_DENSE_H_
#define LINALG_DENSE_H_

#include <memory>     // std::shared_ptr
#include <iostream>   // std::cout
#include <iomanip>    // std::setw
#include <tuple>      // std::tie
#include <cassert>    // assert

#include "preprocessor.h"

#ifdef HAVE_CUDA
# include <cuda_runtime.h>       // various CUDA routines
# include <functional>           // std::bind
# include "CUDA/cuda_memory_allocation.h" // CUDA::cuda_make_shared
#endif

#include "types.h"
#include "profiling.h"
#include "matrix.h"
#include "exceptions.h"

// Forward declaration of routines that are used in the constructors and
// templated members of Dense<T>
#include "forward.h"

namespace LinAlg {

/** \brief              Dense matrix struct
 */
template <typename T>
struct Dense : Matrix {

  // Construct empty
  Dense();

  // Destructor.
  ~Dense();

  // This would be the famous four and half. I still find the arguments in
  // the Google C++ Style Guide against copy constructors and assignment
  // operators convincing, in particular for a library that is performance
  // critical. However, memory lifetime is bound to the existence of an 
  // instance. When using lambda functions for the stream tasks it is 
  // neccessary to have a copy in the lambda function and thus we need a copy 
  // constructor ...
  // good thing.
  Dense(Dense<T>&& other);
  Dense(Dense<T>& other);
  Dense(const Dense<T>& other);
#ifndef DOXYGEN_SKIP
  Dense& operator=(Dense<T> other) = delete;
#endif

  // Construct by allocating memory
  Dense(I_t rows, I_t cols, Location location = Location::host,
        int device_id = 0);

  // Construct from pre-allocated/existing (col-major) array, don't copy data.
  Dense(T* in_array, I_t leading_dimension_in, I_t rows, I_t cols, 
        Location location = Location::host, int device_id = 0);

  // Submatrix creation (from dense)
  Dense(const Dense<T>& source, SubBlock sub_block);
  Dense(const Dense<T>& source, I_t first_row, I_t last_row, I_t first_col,
        I_t last_col);

  // Submatrix creation from ()-operator
  inline Dense<T> operator()(SubBlock sub_block) const;
  inline Dense<T> operator()(I_t first_row, I_t last_row, I_t first_col,
                             I_t last_col) const;

  // Common Matrix operations

  // Explicitly clone
  inline void clone_from(const Dense<T>& source);
  // Explicit cloning as submatrix
  inline void clone_from(const Dense<T>& source, SubBlock sub_block);
  inline void clone_from(const Dense<T>& source, I_t first_row, I_t last_row,
                         I_t first_col, I_t last_col);

  // Explicitly move
  inline void move_to(Dense<T>& destination);

  // Allocate new memory (no memory is copied)
  inline void reallocate(I_t rows, I_t cols, Location location = Location::host,
                         int device_id = 0);

  // Create a matrix in another matrix's memory (does not neccessarily create 
  // a submatrix)
  inline void use_memory_from(const Dense<T>& source, I_t rows, I_t cols,
                              I_t offset);
  inline void use_memory_from(const Dense<T>& source, I_t rows, I_t cols);
  inline void use_memory_from(const Dense<T>& source, SubBlock sub_block,
                              I_t offset);
  inline void use_memory_from(const Dense<T>& source, SubBlock sub_block);

  // Allocate new memory according to the size of another matrix (no memory is 
  // copied)
  template <typename U>
  inline void reallocate_like(const U& other, SubBlock sub_block,
                              Location location, int device_id = 0);
  template <typename U>
  inline void reallocate_like(const U& other, Location location,
                              int device_id);
  template <typename U>
  inline void reallocate_like(const U& other);

  // Data copy from one (sub)matrix to the current instance
  inline void copy_from(const Dense<T>& source, SubBlock sub_block);
  inline void copy_from(const Sparse<T>& source, SubBlock sub_block);
  inline void copy_from(const Dense<T>& source, I_t first_row, I_t last_row,
                        I_t first_col, I_t last_col);
  inline void copy_from(const Sparse<T>& source, I_t first_row, I_t last_row,
                        I_t first_col, I_t last_col);
  inline void copy_from(const Dense<T>& source);
  inline void copy_from(const Sparse<T>& source);
  inline void operator<<(const Dense<T>& source);
  inline void operator<<(const Sparse<T>& source);

  // Asynchronous data copy from one (sub)matrix to the current instance
  inline I_t copy_from_async(const Dense<T>& source, SubBlock sub_block,
                             Stream& stream);
  inline I_t copy_from_async(const Sparse<T>& source, SubBlock sub_block,
                             Stream& stream);
  inline I_t copy_from_async(const Dense<T>& source, I_t first_row,
                             I_t last_row, I_t first_col, I_t last_col,
                             Stream& stream);
  inline I_t copy_from_async(const Sparse<T>& source, I_t first_row,
                             I_t last_row, I_t first_col, I_t last_col,
                             Stream& stream);
  inline I_t copy_from_async(const Dense<T>& source, Stream& stream);
  inline I_t copy_from_async(const Sparse<T>& source, Stream& stream);

  /// Mark the matrix as transposed
  inline void transpose() { _transposed = !_transposed; }

  /// Return the number of rows in the matrix
  inline I_t rows() const { return (_transposed ? _cols : _rows); }
  /// Return the number of columns in the matrix
  inline I_t cols() const { return (_transposed ? _rows : _cols); }

  /// Return the current location of the matrix
  inline Location location() const { return _location; }
  void location(Location new_location, int device_id = 0);

  // Free all memory, set the matrix to empty
  void unlink();

  /// Return the storage format
  inline Format format() const { return _format; }

  // Set the format
  inline void format(Format new_format);

  // Print the matrix contents
  inline void print() const;

  // Return true if matrix is on the given location. These are needed to allow 
  // user code to check the location irrespective of the compilation flags of 
  // the LinAlg libraries
  inline bool is_on_host() const;
  // Return true if matrix is on GPU
  inline bool is_on_GPU()  const;
  // Return true if matrix is on MIC
  inline bool is_on_MIC()  const;

  // Return pointer to matrix begin
  inline T* operator&() const { return _begin(); }

  // Get properties
  inline bool is(Property property) const { return (_properties & property); }

  // Set properties
  inline void set(Property property);

  // Unset properties
  inline void unset(Property property);

  // Returns true if matrix is empty
  inline bool is_empty() const { return (_rows == 0 || _cols == 0); }


#ifndef DOXYGEN_SKIP
  ///////////////////////////////////////////////////////////////////////////
  // Not private: but the _ indicates that the idea is to not mess with these
  // directly unless neccessary.

  // Shared pointer used for the memory containing the matrix
  std::shared_ptr<T> _memory;

  // Offset of first element of the matrix, leading dimension
  I_t _offset;
  I_t _leading_dimension;
  Format _format;
  inline T* _begin() const { return (_memory.get() + _offset); }

  // Rows and columns
  I_t _rows;
  I_t _cols;

  // Location of the matrix
  Location _location;
  int _device_id;

  // Transposition
  bool _transposed;

  // Whether the matrix is complex or not, specializations for C_t and Z_t:
  inline bool _is_complex() const;

  // Properties of the matrix
  unsigned char _properties;
#endif

};

/** \brief              Default (empty) constructor
 *
 *  This constructor doesn't allocate any memory. Typically empty matrices
 *  are initialized suitably by all operations that have output parameters.
 */
template <typename T>
Dense<T>::Dense()
              : _memory(nullptr),
                _offset(0),
                _leading_dimension(0),
                _format(Format::ColMajor),
                _rows(0),
                _cols(0),
                _location(Location::host),
                _device_id(0),
                _transposed(false),
                _properties(0x0) {
 
  PROFILING_FUNCTION_HEADER

}

/*  \brief              Destructor
 */
template <typename T>
Dense<T>::~Dense() {
 
  PROFILING_FUNCTION_HEADER

  unlink();
}

/** \brief              Move constructor
 */
template <typename T>
Dense<T>::Dense(Dense<T>&& other) : Dense() {

  PROFILING_FUNCTION_HEADER

  // Default initialize using the default initialization and swap
  swap(*this, other);

}

/** \brief              Copy constructor
 */
template <typename T>
Dense<T>::Dense(Dense<T>& other) : Dense() {

  PROFILING_FUNCTION_HEADER

  clone_from(other);

}

/** \brief              Copy const constructor
 */
template <typename T>
Dense<T>::Dense(const Dense<T>& other) : Dense() {

  PROFILING_FUNCTION_HEADER

  clone_from(other);

}

/** \brief              Swap function
 *
 *  Swaps by swapping all data members. Allows for simple move constructor
 *  and assignment operator.
 *
 *  \note               We can't use std::swap to swap to instances directly
 *                      as it uses the assignment operator internally itself.
 *
 *  \param[in,out]      first
 *                      Pointer to 'first', will later point to what was
 *                      'second' before the function call.
 *
 *  \param[in,out]      second
 *                      Pointer to 'second', will later point to what was
 *                      'first' before the function call.
 */
template <typename U>
void swap(Dense<U>& first, Dense<U>& second) {

  PROFILING_FUNCTION_HEADER

  using std::swap;
  swap(first._memory, second._memory);
  swap(first._offset, second._offset);
  swap(first._leading_dimension, second._leading_dimension);
  swap(first._format, second._format);
  swap(first._rows, second._rows);
  swap(first._cols, second._cols);
  swap(first._location, second._location);
  swap(first._device_id, second._device_id);
  swap(first._transposed, second._transposed);
  swap(first._properties, second._properties);

}


/** \brief            Allocating-only constructor
 *
 *  This constructor allocates some memory area
 *
 *  \param[in]        rows
 *                    Number of rows.
 *
 *  \param[in]        cols
 *                    Number of columns.
 *
 *  \param[in]        location
 *                    OPTIONAL: Memory / device on which to allocate the
 *                    memory. Default: Location::host.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: The number of the device on which to
 *                    allocate the memory. Ignored for Location::host.
 *                    Default: 0.
 */
template <typename T>
Dense<T>::Dense(I_t rows, I_t cols, Location location, int device_id)
              : Dense() {

  PROFILING_FUNCTION_HEADER

#ifndef LINALG_NO_CHECKS
  if (rows == 0 || cols == 0) {
    throw excBadArgument("Dense.allocating_constructor: rows or cols must not "
                         "be zero for this constructor. Use the empty "
                         "constructor instead");
  }
#endif

  reallocate(rows, cols, location, device_id);

}

/** \brief            Constructor from array
 *
 *  This constructor allows to create a matrix from pre-allocated/existing 
 *  data in ColMajor format.  No memory is allocated and no data is copied.  
 *  Memory has to be allocated and deallocated by the user.
 *
 *  \param[in]        in_array
 *                    Host pointer to the first element of the matrix. Device
 *                    pointers are not supported.
 *
 *  \param[in]        leading_dimension
 *                    Distance between the first elements of two consecutive
 *                    columns
 *
 *  \param[in]        rows
 *                    Number of rows in the matrix.
 *
 *  \param[in]        cols
 *                    Number of columns in the matrix.
 *
 *  \param[in]        location
 *                    OPTIONAL: Location of the data. Default: Location::host.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: Device id for the data. Default: 0.
 */
template <typename T>
Dense<T>::Dense(T* in_array, I_t leading_dimension, I_t rows, I_t cols,
                Location location, int device_id)
              : _offset(0),
                _leading_dimension(leading_dimension),
                _format(Format::ColMajor),
                _rows(rows),
                _cols(cols),
                _location(location),
                _device_id(device_id),
                _transposed(false),
                _properties(0x0) {

  PROFILING_FUNCTION_HEADER

  // Create a shared_ptr that will not deallocate upon destruction.
  _memory = std::shared_ptr<T>(in_array, [](T* in_array){});

}

/** \brief            Submatrix constructor
 *
 *  Create a submatrix from an existing matrix.
 *  For construction from dense matrices no memory is copied and the ownership
 *  of the memory of the source matrix is shared with the source and all other
 *  submatrices.
 *
 *  \param[in]        source
 *                    The matrix from which to construct the new dense
 *                    matrix.
 *
 *  \param[in]        sub_block
 *                    Submatrix specification.
 */
template <typename T>
Dense<T>::Dense(const Dense<T>& source, SubBlock sub_block)
              : _memory(source._memory),
                _leading_dimension(source._leading_dimension),
                _format(source._format),
                _rows(sub_block.last_row - sub_block.first_row),
                _cols(sub_block.last_col - sub_block.first_col),
                _location(source._location),
                _device_id(source._device_id),
                _transposed(source._transposed) {

  PROFILING_FUNCTION_HEADER

  if (_format == Format::ColMajor) {
    _offset = source._offset + sub_block.first_col * 
              _leading_dimension + sub_block.first_row;
  } else {
    _offset = source._offset + sub_block.first_row * 
              _leading_dimension + sub_block.first_col;
  }

  if ((sub_block.first_row == sub_block.first_col) &&
      (sub_block.last_row  == sub_block.last_col )   ) {

    if (source.is(Property::symmetric)) {
      set(Property::symmetric);
    }
    if (source.is(Property::hermitian)) {
      set(Property::hermitian);
    }

  }

}

/** \brief            Submatrix constructor
 *
 *  Create a submatrix from an existing matrix.
 *  For construction from dense matrices no memory is copied and the ownership
 *  of the memory of the source matrix is shared with the source and all other
 *  submatrices.
 *
 *  \param[in]        source
 *                    The matrix from which to construct the new dense
 *                    matrix.
 *
 *  \param[in]        first_row
 *                    The first row of the source matrix which is part of
 *                    the submatrix (i.e. inclusive).
 *
 *  \param[in]        last_row
 *                    The first row of the source matrix which is not part
 *                    of the submatrix (i.e. exclusive).
 *
 *  \param[in]        first_col
 *                    The first column of the source matrix which is part of
 *                    the submatrix (i.e. inclusive).
 *
 *  \param[in]        last_col
 *                    The first column of the source matrix which is not
 *                    part of the submatrix (i.e. exclusive).
 */
template <typename T>
Dense<T>::Dense(const Dense<T>& source, I_t first_row, I_t last_row,
                I_t first_col, I_t last_col)
              : Dense(source, SubBlock(first_row, last_row, first_col, 
                      last_col)) {
}

/** \brief            Submatrix creation
 *
 *  \param[in]        sub_block
 *                    Submatrix specification.
 *
 *  \returns          A submatrix with the given coordinates
 */
template <typename T>
inline Dense<T> Dense<T>::operator()(SubBlock sub_block) const {

  return Dense<T>(*this, sub_block);

}

/** \brief            Submatrix creation
 *
 *  \param[in]        first_row
 *                    The first row of the source matrix which is part of
 *                    the submatrix (i.e. inclusive)
 *
 *  \param[in]        last_row
 *                    The first row of the source matrix which is not part
 *                    of the submatrix (i.e. exclusive)
 *
 *  \param[in]        first_col
 *                    The first column of the source matrix which is part of
 *                    the submatrix (i.e. inclusive)
 *
 *  \param[in]        last_col
 *                    The first column of the source matrix which is not
 *                    part of the submatrix (i.e. exclusive).
 *
 *  \returns          A submatrix with the given coordinates
 */
template <typename T>
inline Dense<T> Dense<T>::operator()(I_t first_row, I_t last_row, I_t first_col,
                                     I_t last_col) const {

  return Dense<T>(*this, SubBlock(first_row, last_row, first_col, last_col));

}

/** \brief              Cloning from an existing matrix
 *
 *  Applies the parameters of another instance \<source\> to the left hand 
 *  instance. No memory is copied.
 *
 *  \param[in]          source
 *                      The matrix to clone from
 */
template <typename T>
inline void Dense<T>::clone_from(const Dense<T>& source) {

  PROFILING_FUNCTION_HEADER

  _memory            = source._memory;
  _offset            = source._offset;
  _format            = source._format;
  _leading_dimension = source._leading_dimension;
  _rows              = source._rows;
  _cols              = source._cols;
  _location          = source._location;
  _device_id         = source._device_id;
  _transposed        = source._transposed;
  _properties        = source._properties;

}

/** \brief            Cloning from an existing matrix
 *
 *  Applies the parameters of another instance \<source\> to the left hand 
 *  instance. No memory is copied.
 *
 *  \param[in]        source
 *                    The matrix to clone from.
 *
 *  \param[in]        sub_block
 *                    Submatrix specification.
 *
 */
template <typename T>
inline void Dense<T>::clone_from(const Dense<T>& source, SubBlock sub_block) {

  PROFILING_FUNCTION_HEADER

#ifndef LINALG_NO_CHECKS
  auto first_row = sub_block.first_row;
  auto last_row  = sub_block.last_row;
  auto first_col = sub_block.first_col;
  auto last_col  = sub_block.last_col;
  if ((first_row < 0) || (first_row > source.rows()) ||
      (last_row < 0)  || (last_row > source.rows())  ||
      (first_col < 0) || (first_col > source.cols()) ||
      (last_col < 0)  || (last_col > source.cols())    ) {
    throw excBadArgument("Dense.clone_from(source, sub_block), sub_block: "
                         "sub_block (%d:%d,%d:%d) not contained in matrix "
                         "of size %dx%d", first_row, last_row, first_col, 
                         last_col, source.rows(), source.cols());
  }
#endif

  clone_from(source);

  if (_format == Format::ColMajor) {
    _offset = source._offset + sub_block.first_col * 
              _leading_dimension + sub_block.first_row;
  } else {
    _offset = source._offset + sub_block.first_col *
              _leading_dimension + sub_block.first_col;
  }

  _rows = sub_block.last_row - sub_block.first_row;
  _cols = sub_block.last_col - sub_block.first_col;

}


/** \brief            Cloning from an existing matrix
 *
 *  Applies the parameters of another instance \<source\> to the left hand 
 *  instance. No memory is copied.
 *
 *  \param[in]        source
 *                    The matrix to clone from
 *
 *  \param[in]        first_row
 *                    The first row of the source matrix which is part of
 *                    the submatrix (i.e. inclusive)
 *
 *  \param[in]        last_row
 *                    The first row of the source matrix which is not part
 *                    of the submatrix (i.e. exclusive)
 *
 *  \param[in]        first_col
 *                    The first column of the source matrix which is part of
 *                    the submatrix (i.e. inclusive)
 *
 *  \param[in]        last_col
 *                    The first column of the source matrix which is not
 *                    part of the submatrix (i.e. exclusive).
 */
template <typename T>
inline void Dense<T>::clone_from(const Dense<T>& source, I_t first_row, 
                                 I_t last_row, I_t first_col, I_t last_col) {

  clone_from(source, SubBlock(first_row, last_row, first_col, last_col));

}

/** \brief              Move matrix to another matrix
 *
 *  'Moves' an instance of Dense<T> to another instance. No memory is copied,
 *  the instance which is moved to destination is left empty (unlinked) after
 *  the operation.
 *
 *  \param[in]          destination
 *                      The destination matrix to move the instance to
 */
template <typename T>
inline void Dense<T>::move_to(Dense<T>& destination) {

  PROFILING_FUNCTION_HEADER

  destination.clone_from(*this);
  unlink();

}

/** \brief            Allocates new empty memory for an already constructed
 *                    matrix.
 *
 *  \param[in]        rows
 *                    Number of rows.
 *
 *  \param[in]        cols
 *                    Number of columns.
 *
 *  \param[in]        location
 *                    OPTIONAL: Memory / device on which to allocate the
 *                    memory. Default: Location::host.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: The number of the device on which to
 *                    allocate the memory. Ignored for Location::host.
 *                    Default: 0.
 *
 */
template <typename T>
inline void Dense<T>::reallocate(I_t rows, I_t cols, Location location,
                                 int device_id) {

  PROFILING_FUNCTION_HEADER

#ifndef LINALG_NO_CHECKS
  if (rows == 0 || cols == 0) {
    throw excBadArgument("Dense.reallocate(): rows or cols must not be zero, "
                         "use unlink() instead");
  }
#endif

  // Allocation for main memory
  if (location == Location::host) {

    _memory            = Utilities::host_make_shared<T>(rows * cols);
    _leading_dimension = (_format == Format::ColMajor) ? rows : cols;
    device_id          = 0;

  }

#ifdef HAVE_CUDA
  // Custom allocator and destructor for GPU memory. Even if cuda_allocate
  // throws we won't leak memory.
  else if (location == Location::GPU) {

    // OLD:
    // Linear allocation
    using CUDA::cuda_make_shared;
    _memory            = cuda_make_shared<T>(rows * cols, device_id);
    _leading_dimension = (_format == Format::ColMajor) ? rows : cols;

    // NEW:
    // Optimized allocation, potentially with a leading dimension (which is 
    // automatically set by this call via the reference to _leading_dimension)
    // Doesn't yet work though
    //using CUDA::cuda_make_shared_2D;
    //_memory    = cuda_make_shared_2D<T>(rows, cols, device_id, _format, 
    //                                    _leading_dimension);
    //_device_id = device_id;

  }
#endif

#ifdef HAVE_MIC
  else if (location == Location::MIC) {

    using Utilities::MIC::mic_make_shared;

    // NOTE: the MIC seems not to support 'MIC only' memory such that there must
    // always be a corresponding block of memory on the host itself.
    _memory            = mic_make_shared<T>(rows * cols, device_id);
    _leading_dimension = (_format == Format::ColMajor) ? rows : cols;

  }
#endif

  _offset    = 0;
  _device_id = device_id;
  _location  = location;
  _cols      = cols;
  _rows      = rows;

}

/** \brief            Uses the memory of a matrix to create another matrix 
 *                    (not neccessarily a submatrix of the donor matrix)
 *
 *  \param[in]        donor
 *
 *  \param[in]        rows
 *                    Number of rows in the new matrix
 *
 *  \param[in]        cols
 *                    Number of columns in the new matrix
 *
 *  \param[in]        offset
 *                    OPTIONAL: Offset from the beginning of the donor memory.
 *                    Default: 0.
 */
template <typename T>
inline void Dense<T>::use_memory_from(const Dense<T>& donor, I_t rows,
                                      I_t columns, I_t offset) {

#ifndef LINALG_NO_CHECKS
  // Donor can't have an offset
  if (donor._offset != 0) {
    throw excBadArgument("Dense.use_memory_from(donor, sub_block, offset), "
                         "donor: donor has an offset");
  }
  // Donor must be continuous in memory
  if (((donor._format == Format::ColMajor) &&
       (donor._leading_dimension != donor._rows)) ||
      ((donor._format == Format::RowMajor) &&
       (donor._leading_dimension != donor._cols))   ) {
    throw excBadArgument("Dense.use_memory_from(donor, sub_block), "
                         "donor: donor not continuous in memory");
  }
  // It also must have enough space
  if (donor._rows * donor._cols < offset + rows * columns) {
    throw excBadArgument("Dense.use_memory_from(donor, sub_block), "
                         "donor: donor not large enough (%dx%d) for "
                         "requested alias (%d + %dx%d)", offset, donor.rows(),
                         donor.cols(), rows, columns);
  }
#endif

  Dense<T> tmp(donor._begin() + offset, rows, rows, columns, donor._location, 
               donor._device_id);

  swap(*this, tmp);

}
/** \overload
 */
template <typename T>
inline void Dense<T>::use_memory_from(const Dense<T>& donor, I_t rows,
                                      I_t columns) {
  use_memory_from(donor, rows, columns, 0);
}

/** \brief            Uses the memory of a matrix to create another matrix 
 *                    (not neccessarily a submatrix of the donor matrix)
 *
 *  \param[in]        donor
 *
 *  \param[in]        sub_block
 *                    Specification of the matrix size to be created (only the 
 *                    size of sub_block is extracted, not its position, in 
 *                    that sense it is not interpreted as a sub_block)
 *
 *  \param[in]        offset
 *                    OPTIONAL: Offset from the beginning of the donor memory.  
 *                    Default: 0.
 */
template <typename T>
inline void Dense<T>::use_memory_from(const Dense<T>& donor,
                                      SubBlock sub_block, I_t offset) {
  use_memory_from(donor, sub_block.rows(), sub_block.cols(), offset);
}
/** \overload
 */
template <typename T>
inline void Dense<T>::use_memory_from(const Dense<T>& donor,
                                      SubBlock sub_block) {
  use_memory_from(donor, sub_block.rows(), sub_block.cols(), 0);
}


/** \brief            Allocates new empty memory with the same dimensions,
 *                    transposition status and optionally the same location as 
 *                    a given matrix
 *
 *  \param[in]        other
 *                    Other matrix whose size and transposition status will be 
 *                    used for this allocation.
 *
 *  \param[in]        sub_block
 *                    OPTIONAL: Sub matrix specification for 'other'
 *
 *  \param[in]        location
 *                    OPTIONAL: Memory / device on which to allocate the
 *                    memory. Default: Location::host.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: The number of the device on which to
 *                    allocate the memory. Ignored for Location::host.
 *                    Default: 0.
 */
template <typename T>
template <typename U>
inline void Dense<T>::reallocate_like(const U& other, SubBlock sub_block, 
                                      Location location, int device_id) {

  Utilities::reallocate_like(*this, other, sub_block, location, device_id);

}
/** \overload
 */
template <typename T>
template <typename U>
inline void Dense<T>::reallocate_like(const U& other, Location location,
                                      int device_id) {

  Utilities::reallocate_like(*this, other, location, device_id);

}
/** \overload
 */
template <typename T>
template <typename U>
inline void Dense<T>::reallocate_like(const U& other) {

  Utilities::reallocate_like(*this, other);

}


/** \brief            Copies data from a (sub)matrix into the current matrix
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 *
 *  \param[in]        sub_block
 *                    Submatrix specification (C-style indexing).
 *
 *  \note             This function copies memory
 */
template <typename T>
inline void Dense<T>::copy_from(const Dense<T>& source, SubBlock sub_block) {

  PROFILING_FUNCTION_HEADER

  copy(source, sub_block, *this);

}
/** \overload
 */
template <typename T>
inline void Dense<T>::copy_from(const Sparse<T>& source, SubBlock sub_block) {

  PROFILING_FUNCTION_HEADER

  copy(source, sub_block, *this);

}

/** \brief            Copies data from a (sub)matrix into the current matrix
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 *
 *  \param[in]        first_row
 *                    The first row of the source matrix which is part of
 *                    the submatrix (i.e. inclusive, C-style indexing).
 *
 *  \param[in]        last_row
 *                    The first row of the source matrix which is not part
 *                    of the submatrix (i.e. exclusive, C-style indexing).
 *
 *  \param[in]        first_col
 *                    The first column of the source matrix which is part of
 *                    the submatrix (i.e. inclusive, C-style indexing).
 *
 *  \param[in]        last_col
 *                    The first column of the source matrix which is not
 *                    part of the submatrix (i.e. exclusive, C-style indexing).
 */
template <typename T>
inline void Dense<T>::copy_from(const Dense<T>& source, I_t first_row, 
                                I_t last_row, I_t first_col, I_t last_col) {
  copy_from(source, SubBlock(first_row, last_row, first_col, last_col));
}
/** \overload
 */
template <typename T>
inline void Dense<T>::copy_from(const Sparse<T>& source, I_t first_row, 
                                I_t last_row, I_t first_col, I_t last_col) {
  copy_from(source, SubBlock(first_row, last_row, first_col, last_col));
}

/** \brief            Copies a matrix into the current matrix
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 */
template <typename T>
inline void Dense<T>::copy_from(const Dense<T>& source) {

  PROFILING_FUNCTION_HEADER

  copy(source, *this);

}
/** \overload
 */
template <typename T>
inline void Dense<T>::copy_from(const Sparse<T>& source) {

  PROFILING_FUNCTION_HEADER

  copy(source, *this);

}

/** \brief              Data copy operator, copies values of one (sub)matrix
 *                      to another matrix.
 *
 *  \param[in]          source
 *                      Right hand side of the operator, used as source of the
 *                      copy. The left hand side is the destination of the
 *                      copy.
 *
 *  \note               Usage:    A << B      // assign's B's values to A
 */
template <typename T>
inline void Dense<T>::operator<<(const Dense<T>& source) {

  PROFILING_FUNCTION_HEADER

  copy(source, *this);

}
/** \overload
 */
template <typename T>
inline void Dense<T>::operator<<(const Sparse<T>& source) {

  PROFILING_FUNCTION_HEADER

  copy(source, *this);

}

/** \brief            Copies data from a (sub)matrix into the current matrix
 *                    asynchronously
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 *
 *  \param[in]        sub_block
 *                    Submatrix specification (C-style indexing).
 *
 *  \param[in,out]    stream
 *                    Stream to use for the transfer
 *
 *  \returns          The ticket number on the stream for the transfer
 *
 *  \note             This function copies memory
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Dense<T>& source,
                                     SubBlock sub_block, Stream& stream) {

  PROFILING_FUNCTION_HEADER

  return copy_async(source, sub_block, *this, stream);

}
/** \overload
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Sparse<T>& source,
                                     SubBlock sub_block, Stream& stream) {

  PROFILING_FUNCTION_HEADER

  return copy_async(source, sub_block, *this, stream);

}

/** \brief            Copies data from a (sub)matrix into the current matrix
 *                    asynchronously
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 *
 *  \param[in]        first_row
 *                    The first row of the source matrix which is part of
 *                    the submatrix (i.e. inclusive, C-style indexing).
 *
 *  \param[in]        last_row
 *                    The first row of the source matrix which is not part
 *                    of the submatrix (i.e. exclusive, C-style indexing).
 *
 *  \param[in]        first_col
 *                    The first column of the source matrix which is part of
 *                    the submatrix (i.e. inclusive, C-style indexing).
 *
 *  \param[in]        last_col
 *                    The first column of the source matrix which is not
 *                    part of the submatrix (i.e. exclusive, C-style indexing).
 *
 *  \param[in,out]    stream
 *                    Stream to use for the transfer
 *
 *  \returns          The ticket number on the stream for the transfer
 *
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Dense<T>& source, I_t first_row,
                               I_t last_row, I_t first_col, I_t last_col,
                               Stream& stream) {
  return copy_from_async(source,
                         SubBlock(first_row, last_row, first_col, last_col), 
                         stream);
}
/** \overload
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Sparse<T>& source, I_t first_row, 
                                     I_t last_row, I_t first_col,
                                     I_t last_col, Stream& stream) {
  return copy_from_async(source,
                         SubBlock(first_row, last_row, first_col, last_col),
                         stream);
}

/** \brief            Copies a matrix into the current matrix
 *
 *  \param[in]        source
 *                    The matrix from which to copy the data.
 *
 *  \param[in,out]    stream
 *                    Stream to use for the transfer
 *
 *  \returns          The ticket number on the stream for the transfer
 *
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Dense<T>& source, Stream& stream) {

  PROFILING_FUNCTION_HEADER

  return copy_async(source, *this, stream);

}
/** \overload
 */
template <typename T>
inline I_t Dense<T>::copy_from_async(const Sparse<T>& source, Stream& stream) {

  PROFILING_FUNCTION_HEADER

  return copy_async(source, *this, stream);

}

/** \brief            Changes the matrix' location
 *
 *  \param[in]        new_location
 *                    The new matrix location.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: the device id of the new location. DEFAULT: 0
 */
template <typename T>
void Dense<T>::location(Location new_location, int device_id) {

  PROFILING_FUNCTION_HEADER

  if (new_location == _location) {

    if (new_location == Location::host) device_id = 0;
    if (device_id == _device_id) return;

  }

  if (is_empty()) {

    _location  = new_location;
    _device_id = device_id;

  } else {

    Dense<T> tmp;
    tmp._format = _format;
    tmp.reallocate_like(*this, new_location, device_id);

#ifdef HAVE_CUDA
    // Transfers out of the GPU don't support transposing on the fly so we 
    // unset the transpose flags prior to transfering and set them back 
    // afterwards
    bool was_transposed = _transposed;
    if (_location == Location::GPU && was_transposed) _transposed = false;
#endif
    copy(*this, tmp);
    clone_from(tmp);
#ifdef HAVE_CUDA
    if (was_transposed) _transposed = true;
#endif

  }

}

/** \brief              Resets the matrix/submatrix to empty state.
 *
 *  \note               If the matrix has submatrices, those will not be
 *                      unlinked and the shared memory will not be deleted.
 */
template <typename T>
inline void Dense<T>::unlink() {

  PROFILING_FUNCTION_HEADER

  _offset = 0;
  _leading_dimension = 0;
  _rows = 0;
  _cols = 0;
  _location = Location::host;
  _device_id = 0;
  _transposed = false;
  _properties = 0x0;

  // This potentially frees the memory
  _memory = nullptr;

}


/** \brief            Change the storage format of the matrix (out-of-place)
 *
 *  \param[in]        new_format
 *                    The new storage format
 */
template <typename T>
inline void Dense<T>::format(Format new_format) {

  if (new_format == _format) return;

  Dense<T> new_matrix;
  new_matrix.reallocate_like(*this);
  new_matrix._format = new_format;
  new_matrix._leading_dimension = (new_format == Format::RowMajor) ?
                                  new_matrix._cols : new_matrix._rows;

  new_matrix << *this;

  swap(*this, new_matrix);

}

/** \brief            Prints the contents of the matrix to std::cout
 */
template <typename T>
inline void Dense<T>::print() const {

  PROFILING_FUNCTION_HEADER

  if (is_empty()) {
    return;
  }

  // Make sure we have the data in main memory
  auto source = *this;
  if (_location != Location::host) {
    source.reallocate(rows(), cols());
    source << *this;
  }

  // Set output field width. We use setw which doesn't change the state of
  // std::cout permanently
  auto width = std::cout.precision() + 4;
  if (source._is_complex()) {
    width = std::cout.precision() * 2 + 4;
  }

  auto data = source._begin();
  auto ld   = source._leading_dimension;

  if ((source._format == Format::ColMajor && source._transposed) ||
      (source._format == Format::RowMajor && !source._transposed)  ) {

    for (I_t row = 0; row < source.rows(); ++row) {
      for (I_t col = 0; col < source.cols(); ++col) {

        auto value = data[row * ld + col];
        std::cout << std::setw(width) << std::left << value;

      }
      std::cout << "\n";
    }

  } else {

    for (I_t row = 0; row < source.rows(); ++row) {
      for (I_t col = 0; col < source.cols(); ++col) {

        auto value = data[col * ld + row];
        std::cout << std::setw(width) << std::left << value;

      }
      std::cout << "\n";
    }

  }

}

/** \brief            Return true if matrix is on host
 */
template <typename T>
inline bool Dense<T>::is_on_host() const {

  return _location == Location::host;

}

/** \brief            Return true if matrix is on GPU
 */
template <typename T>
inline bool Dense<T>::is_on_GPU() const {

#ifdef HAVE_CUDA
  return _location == Location::GPU;
#else
  return false;
#endif

}

/** \brief            Return true if matrix is on MIC
 */
template <typename T>
inline bool Dense<T>::is_on_MIC() const {

#ifdef HAVE_MIC
  return _location == Location::MIC;
#else
  return false;
#endif

}

/** \brief            Setter for properties
 *
 *  \param[in]        property
 *                    Property to set on the matrix.
 */
template <typename T>
inline void Dense<T>::set(Property property) {

  PROFILING_FUNCTION_HEADER

  if (property == Property::hermitian) {

    if (!_is_complex()) {

      throw excBadArgument("Dense.set(property): can't set Property::hermitian "
                           "on real matrices");

    }

  }

  _properties = _properties | property;

}

/** \brief            Unset properties
 *
 *  \param[in]        property
 *                    Property to remove from the matrix.
 */
template <typename T>
inline void Dense<T>::unset(Property property) {

  PROFILING_FUNCTION_HEADER

  _properties = _properties & ~(property);

}

#ifndef DOXYGEN_SKIP
/*  \brief            Returns whether the matrix is complex or not
 *
 *  \return           True if the matrix is of complex data type (C_t, Z_t),
 *                    false otherwise.
 */
template <typename T>
inline bool Dense<T>::_is_complex() const { return false; }
/** \overload
 */
template <>
inline bool Dense<C_t>::_is_complex() const { return true; }
/** \overload
 */
template <>
inline bool Dense<Z_t>::_is_complex() const { return true; }
#endif

} /* namespace LinAlg */

#endif /* LINALG_DENSE_H_ */
