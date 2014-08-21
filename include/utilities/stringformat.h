/** \file             stringformat.h
 *
 *  \brief            Contains a routine for string formatting and some regular
 *                    expressions.
 *
 *  \date             Created:  Jul 17, 2014
 *  \date             Modified: $Date$
 *
 *  \authors          mauro <mcaldrara@iis.ee.ethz.ch>
 *
 *  \version          $Revision$
 */
#ifndef LINALG_UTILITIES_STRINGFORMAT_H_
#define LINALG_UTILITIES_STRINGFORMAT_H_

/// Maximal length of the string to format (excess will be truncated)
#define LINALG_STRINGFORMAT_BUFFER 1024

#include <string>     // std::string
#include <memory>     // std::unique_ptr

namespace LinAlg {

/** \brief              Format a std::string in sprintf-like fashion.
 *
 *  \param[in]          formatstring
 *                      printf-like format string.
 *
 *  \param[in]          formatargs
 *                      printf-like format arguments.
 *
 *  \returns            A std::string object that contains the formatted string.
 *
 *  \note               There is a limit of LINALG_STRINGMT_LENGTH characters
 *                      the produced string can have. The routine isn't typesafe
 *                      and generally a bit of a hack. However, at least it
 *                      shouldn't leak memory in any case.
 */
template <typename... Ts>
inline std::string stringformat(const char* formatstring, Ts... formatargs) {

  // Create a buffer, use std::unique_ptr for memory management
  std::unique_ptr<char> buffer(new char[LINALG_STRINGFORMAT_BUFFER]);

  // Use snprintf to avoid buffer overflows
  std::snprintf(buffer.get(), LINALG_STRINGFORMAT_BUFFER, formatstring,
                formatargs...);

  // Return a string object constructed from the buffer
  return std::string(buffer.get());

};

} /* namespace LinAlg */

#endif /* LINALG_UTILITIES_STRINGFORMAT_H_ */