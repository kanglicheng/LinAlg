/** \file             dense.h
 *
 *  \brief            Dense matrix struct
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

#ifdef HAVE_CUDA
#include <cuda_runtime.h>       // various CUDA routines
#include <functional>           // std::bind
#include "CUDA/cuda_memory_allocation.h" // CUDA::cuda_make_shared
                                         // CUDA::cuda_deallocate
#endif

#include "types.h"
#include "matrix.h"
#include "exceptions.h"

// Forward declaration of routines that are used in the constructors and
// templated members of Dense<T>
#include "utilities/utilities_forward.h"
#include "BLAS/blas_forward.h"

namespace LinAlg {

/** \brief              Dense matrix struct
 *
 *  \note
 *
 *  \todo               Add these constructors
 *                         Dense from CSR
 *                         Dense from file
 */
template <typename T>
struct Dense : Matrix {

  // Construct empty
  Dense();

  // Destructor.
  ~Dense();

  // This would be the famous four and half. However, I find the arguments in
  // the Google C++ Style Guide against copy constructors and assignment
  // operators convincing, in particular for a library that is performance
  // critical. It makes everything a bit more explicit and clear as well. The
  // user code looks less elegant but is more expressive, which I consider a
  // good thing.
  Dense(Dense<T>&& other);
#ifndef DOXYGEN_SKIP
  Dense(Dense<T>& other) = delete;
  Dense(const Dense<T>& other) = delete;
  Dense& operator=(Dense<T> other) = delete;
#endif

  // Construct by allocating memory
  Dense(I_t rows, I_t cols, Location location = Location::host,
        int device_id = 0);

  // Construct from existing (col-major) array, don't copy data and optionally
  // delete the memory afterwards.
  Dense(T* in_array, I_t leading_dimension_in, I_t rows, I_t cols,
        bool delete_active = false, Location location = Location::host,
        int device_id = 0);

  // Submatrix creation (from dense)
  Dense(Dense<T>& source, I_t first_row, I_t last_row, I_t first_col,
        I_t last_col);

  // Submatrix creation from ()-operator
  Dense<T> operator()(I_t first_row, I_t last_row, I_t first_col, I_t last_col);

  // Explicitly clone
  inline void clone_from(const Dense<T>& source);

  // Explicitly move
  inline void move_to(Dense<T>& destination);

  // Allocate new memory (no memory is copied)
  inline void reallocate(I_t rows, I_t cols, Location location = Location::host,
                         int device_id = 0);

  // Data copy from one (sub)matrix to another
  void operator<<(const Dense<T>& source);

  /// Return the number of rows in the matrix
  inline I_t rows() const { return (_transposed ? _cols : _rows); };
  /// Return the number of columns in the matrix
  inline I_t cols() const { return (_transposed ? _rows : _cols); };
  /// Mark the matrix as transposed
  inline void transpose() { _transposed = !_transposed; };

  /// Return the current location of the matrix
  inline Location location() const { return _location; };
  void location(Location new_location, int device_id = 0);

  void unlink();

  /// Return the storage format
  inline Format format() const { return _format; };

  /// Print the matrix contents
  inline void print() const;


  ///////////////////////////////////////////////////////////////////////////
  // Not private: but the _ indicates that the idea is to not mess with these
  // directly unless neccessary.
#ifndef DOXYGEN_SKIP
  // Shared pointer used for the memory containing the matrix
  std::shared_ptr<T> _memory;

  // Offset of first element of the matrix, leading dimension
  I_t _offset;
  I_t _leading_dimension;
  Format _format;
  inline T* _begin() const { return (_memory.get() + _offset); };

  // Rows and columns
  I_t _rows;
  I_t _cols;

  // Location of the matrix
  Location _location;
  int _device_id;

  // Transposition
  bool _transposed;

  // Whether the matrix is complex or not. Overridden in dense.cc with
  // specializations for C_t and Z_t:
  inline bool _is_complex() const { return false; };
#endif

};

/** \brief              Default (empty) constructor
 *
 *  This constructor doesn't allocate any memory. Typically empty matrices
 *  (defined as those that have rows == 0) are initialized suitably by all
 *  operations that have output parameters.
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
                _transposed(false) {
#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.empty_constructor\n";
#endif
};

/*  \brief              Destructor
 */
template <typename T>
Dense<T>::~Dense() {
#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.destructor\n";
#endif
}

/*  \brief              Copy constructor
 *
 *  Creates a new matrix handle to the same content as the given matrix.
 *
 *  \param              src
 *                      Source matrix.
 */
/*
template <typename T>
Dense<T>::Dense(Dense<T>& src)
              : _memory(src._memory),
                _offset(src._offset),
                _leading_dimension(src._rows),
                _format(Format::ColMajor),
                _rows(src._rows),
                _cols(src._cols),
                _location(src._location),
                _device_id(src._device_id),
                _transposed(src._transposed) {

#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.copy_constructor\n";
#endif

};
*/

/* \brief              Copy const constructor
 *
 *  Creates a new matrix with the content of the given matrix.
 *
 *  \param              src
 *                      Matrix from which to copy the elements.
 */
/*
template <typename T>
Dense<T>::Dense(const Dense<T>& other) : Dense(const_cast<Dense<T>&>(other)) {
#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.copy_const_constructor\n";
#endif
};
*/

/** \brief              Move constructor
 */
template <typename T>
Dense<T>::Dense(Dense<T>&& other) : Dense() {

#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.move_constructor\n";
#endif

  // Default initialize using the default initialization and swap
  swap(*this, other);

};

/*  \brief              Assignment operator, creates a copy of the handle
 *                      'other' and assigns it to the left hand side. No data is
 *                      copied, the left hand side's data is potentially
 *                      deleted.
 */
/*
template <typename T>
Dense<T>& Dense<T>::operator=(Dense<T> other) {

  // Since we passed 'other' by value, it is already a copy that can later be
  // thrown away.
  swap(*this, other);
  return *this;

  // Here 'other' (that is: what used to be 'this') gets destroyed.
}
*/


/** \brief              Swap function
 *
 *  Swaps by swapping all data members. Allows for simple move constructor
 *  and assignment operator.
 *
 *  \note               We can't use std::swap to swap to instances directly
 *                      as it uses the assignment operator internally itself.
 *
 *  \param[in|out]      first
 *                      Pointer to 'first', will later point to what was
 *                      'second' before the function call.
 *
 *  \param[in|out]      second
 *                      Pointer to 'second', will later point to what was
 *                      'first' before the function call.
 */
template <typename U>
void swap(Dense<U>& first, Dense<U>& second) {

  using std::swap;
  swap(first._memory, second._memory);
  swap(first._offset, second._offset);
  swap(first._leading_dimension, second._leading_dimension);
  swap(first._rows, second._rows);
  swap(first._cols, second._cols);
  swap(first._location, second._location);
  swap(first._device_id, second._device_id);
  swap(first._transposed, second._transposed);

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

#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.allocating_constructor\n";
#endif

#ifndef LINALG_NO_CHECKS
  if (rows == 0 || cols == 0) {
    throw excBadArgument("Dense.allocating_constructor: rows or cols must not "
                         "be zero for this constructor. Use the empty "
                         "constructor instead");
  }
#endif

  reallocate(rows, cols, location, device_id);

};

/** \brief            Constructor from array
 *
 *  This constructor allows to create a matrix from preexisting data. No memory
 *  is allocated and no data is copied. Memory has to be allocated and
 *  deallocated by the user.
 *
 *  \param[in]        in_array
 *                    Host pointer to the first element of the matrix. Device
 *                    pointers are not supported.
 *
 *  \param[in]        leading_dimension
 *                    Distance between the first elements of two consecutive
 *                    columns if (format == Format::ColMajor) or two
 *                    consecutive rows (format == Format::RowMajor).
 *
 *  \param[in]        rows
 *                    Number of rows in the matrix.
 *
 *  \param[in]        cols
 *                    Number of columns in the matrix.
 *
 *  \param[in]        delete_active
 *                    OPTIONAL: Whether to attempt to delete the pointer when
 *                    this matrix handle is destroyed. Default: false
 *
 *  \param[in]        location
 *                    OPTIONAL: Location of the data. Default: Location::host.
 *
 *  \param[in]        device_id
 *                    OPTIONAL: Device id for the data. Default: 0.
 */
template <typename T>
Dense<T>::Dense(T* in_array, I_t leading_dimension, I_t rows, I_t cols,
                bool delete_active, Location location, int device_id)
              : _offset(0),
                _leading_dimension(leading_dimension),
                _rows(rows),
                _cols(cols),
                _location(location),
                _device_id(device_id),
                _transposed(false) {

#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.constructor_from_array\n";
#endif

  if (!delete_active) {

    // Create a shared_ptr that will not deallocate upon destruction.
    _memory = std::shared_ptr<T>(in_array, [](T* in_array){});

  } else {
    if (_location == Location::host) {

      // Create a shared_ptr that will deallocate using delete[]
      _memory = std::shared_ptr<T>(in_array, [](T* p){ delete[] p; });

    }
#ifdef HAVE_CUDA
    else if (_location == Location::GPU) {

      using CUDA::cuda_deallocate;
      // Create a deleter for memory on cuda and a shared pointer using it
      auto deleter = std::bind(cuda_deallocate<T>, std::placeholders::_1,
                               _device_id);
      _memory = std::shared_ptr<T>(in_array, deleter);

    }
#endif
  }

};

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
Dense<T>::Dense(Dense<T>& source, I_t first_row, I_t last_row, I_t first_col,
                I_t last_col)
              : _memory(source._memory),
                _leading_dimension(source._leading_dimension),
                _format(source._format),
                _rows(last_row - first_row),
                _cols(last_col - first_col),
                _location(source._location),
                _device_id(source._device_id),
                _transposed(source._transposed) {

#ifdef CONSTRUCTOR_VERBOSE
  std::cout << "Dense.submatrix_constructor\n";
#endif

  if (_format == Format::ColMajor) {
    _offset = source._offset + first_col * _leading_dimension + first_row;
  } else {
    _offset = source._offset + first_row * _leading_dimension + first_col;
  }

};

/** \brief            Submatrix creation
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
Dense<T> Dense<T>::operator()(I_t first_row, I_t last_row, I_t first_col,
                          I_t last_col) {

  return Dense<T>(this, first_row, last_row, first_col, last_col);

}

/** \brief              Cloning from an existing matrix
 *
 *  Creates another instance of Dense<T> with the exact same parameters. No
 *  memory is copied.
 *
 *  \param[in]          source
 *                      The matrix to clone from
 */
template <typename T>
inline void Dense<T>::clone_from(const Dense<T>& source) {

  _memory            = source._memory;
  _offset            = source._offset;
  _format            = source._format;
  _leading_dimension = source._leading_dimension;
  _rows              = source._rows;
  _cols              = source._cols;
  _location          = source._location;
  _device_id         = source._device_id;
  _transposed        = source._transposed;

};

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

  destination.clone_from(*this);
  unlink();

};

/** \brief              Data copy operator, copies values from one (sub)matrix
 *                      to another (sub)matrix.
 *
 *  \param[in]          source
 *                      Right hand side of the operator, used as source of the
 *                      copy. The left hand side is the destination of the
 *                      copy.
 *
 *  \note               Usage:    A << B      // assign's B's values to A
 */
template <typename T>
void Dense<T>::operator<<(const Dense<T>& source) {

#ifdef WARN_COPY
  std::cout << "Warning: matrix data copy\n";
#endif

#ifndef LINALG_NO_CHECKS
  // Can't assign to transposed matrices
  if (this->_transposed) {
    throw excBadArgument("DenseA^t << DenseB: can't assign to transposed "
                         "matrices");
  } else if (source._rows == 0) {
    throw excBadArgument("DenseA << DenseB: can't assign from empty matrix");
  }
#endif


  // If 'this' is empty, allocate memory accordingly
  if (this->_rows == 0) {
    this->reallocate(source.rows(), source.cols(), source._location,
                      source._device_id);
  }

#ifndef LINALG_NO_CHECKS
  else if (source.rows() != this->rows() || source.cols() != this->cols()) {
    throw excBadArgument("DenseA%s << DenseB%s: dimension mismatch",
                         this->_transposed ? "^t" : " ",
                         source._transposed ? "^t" : " ");
  }

  try {

#endif
    Utilities::copy_2Darray(source._transposed, source._format, source._begin(),
                            source._leading_dimension, source._location,
                            source._device_id, source._rows, source._cols,
                            this->_format, this->_begin(),
                            this->_leading_dimension, this->_location,
                            this->_device_id);
#ifndef LINALG_NO_CHECKS
  } catch (excUnimplemented e) {

    throw excUnimplemented("Dense.operator<<(): exception from copy_2Darray:  "
                           " %s", e.what());

  }
#endif

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

  if (new_location == Location::host) {
    device_id = 0;
  }

  if (new_location == _location) {
    return;
  }

#ifndef LINALG_NO_CHECKS
  else if ((new_location != Location::host) && (_location != Location::host)){

    throw excUnimplemented("Dense.location(): Moves only to and from main "
                           "memory supported. Try moving to main memory first "
                           "and move to target from there.");
    // The reason for this is that it is tedious to guarantee a consistent state
    // in all cases. It is easier if the user handles potential failures.
  }
#endif

  // The matrix is empty, we just update the meta data
  if (_rows == 0) {

    _location = new_location;
    _device_id = device_id;

    return;

  };

  // Create a temporary (just increases the pointer count in ._memory so we
  // don't deallocate yet)
  Dense<T> tmp;
  tmp.clone_from(*this);

  // Reallocate the memory on the target
  this->reallocate(_rows, _cols, new_location, device_id);

  // Copy the data over
  *this << tmp;

  // At exit, tmp will be destroyed and the memory is released unless there

}

/** \brief              Resets the matrix/submatrix to empty state.
 *
 *  \note               If the matrix has submatrices, those will not be
 *                      unlinked and the shared memory will not be deleted.
 */
template <typename T>
inline void Dense<T>::unlink() {

  _offset = 0;
  _leading_dimension = 0;
  _rows = 0;
  _cols = 0;
  _location = Location::host;
  _device_id = 0;
  _transposed = false;

  // This potentially frees the memory
  _memory = nullptr;

};

/** \brief            Prints the contents of the matrix to std::cout
 */
template <typename T>
inline void Dense<T>::print() const {

  if (_rows == 0) {
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

};


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

#ifndef LINALG_NO_CHECKS
  if (rows == 0 || cols == 0) {
    throw excBadArgument("Dense.reallocate(): rows or cols must not be zero.");
  }
#endif

  // Allocation for main memory
  if (location == Location::host) {

    _memory = Utilities::host_make_shared<T>(rows * cols);

  }

#ifdef HAVE_CUDA
  // Custom allocator and destructor for GPU memory. Even if cuda_allocate
  // throws we won't leak memory.
  else if (location == Location::GPU) {

    using CUDA::cuda_make_shared;
    _memory = cuda_make_shared<T>(_rows * _cols, device_id);

  }
#endif

#ifdef HAVE_MIC
  else if (location == Location::MIC) {

    using Utilities::MIC::mic_make_shared;

    // NOTE: the MIC seems not to support 'MIC only' memory such that there must
    // always be a corresponding block of memory on the host itself.
    _memory = mic_make_shared<T>(_rows * _cols, device_id);

  }
#endif

  _offset = 0;
  _leading_dimension = (_format == Format::ColMajor) ? rows : cols;
  _device_id = 0;
  _location = location;
  _cols = cols;
  _rows = rows;

}


} /* namespace LinAlg */


#endif /* LINALG_DENSE_H_ */