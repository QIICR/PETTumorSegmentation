/*==============================================================================

 Program: PETTumorSegmentation

 (c) Copyright University of Iowa All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ==============================================================================*/
#ifndef _itkWorkers_h
#define _itkWorkers_h

#include <itkPlatformMultiThreader.h>

namespace itk
{
// TODO: write class description with usage examples
/**\class Workers
 * \brief Runs functions in a multithreaded manner.
 * \date	12/9/2014
 * \author	Christian Bauer
 * Runs functions in a multithreaded manner. CONTINUE
 */
class Workers
{
public:
  Workers(int numWorkers=1024);

  int GetNumberOfWorkers() const;

  // run non-const method on object without parameters
  template <class T>
  void RunMethod(T* object, void(T::*method)(int, int) );

  // run non-const method on object with one parameter
  template <class T, typename T1>
  void RunMethod(T* object, void(T::*method)(int, int, T1), T1 p1 );

  // run const method on object without parameters
  template <class T>
  void RunMethod(T* object, void(T::*method)(int, int) const );

  // run static method or function without parameters
  void RunFunction(void(*function)(int, int));

  // run static method or function with one parameter
  template <typename T1>
  void RunFunction(void(*function)(int, int, T1), T1 p1);

  // run non-const method on object without parameters for a specified range of values
  template <class T, typename RangeType>
  void RunMethodForRange(T* object, void(T::*method)(RangeType), RangeType min, RangeType max );

  // run non-const method on object with one parameter for a specified range of values
  template <class T, typename RangeType, typename T1>
  void RunMethodForRange(T* object, void(T::*method)(RangeType, T1), RangeType min, RangeType max, T1 p1 );

  // run static method or function without parameters for a specified range of values
  template <typename RangeType>
  void RunFunctionForRange(void(*function)(RangeType), RangeType min, RangeType max);

  // run static method or function with one parameter for a specified range of values
  template <typename RangeType, typename T1>
  void RunFunctionForRange(void(*function)(RangeType, T1), RangeType min, RangeType max, T1 p1);

  // run static method or function with two parameter for a specified range of values
  // TODO: implement

  // run static method or function with three parameter for a specified range of values
  template <typename RangeType, typename T1, typename T2, typename T3>
  void RunFunctionForRange(void(*function)(RangeType, T1, T2, T3), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3);

  // run static method or function with five parameter for a specified range of values
  template <typename RangeType, typename T1, typename T2, typename T3, typename T4, typename T5>
  void RunFunctionForRange(void(*function)(RangeType, T1, T2, T3, T4, T5), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5);

  // run static method or function with six parameter for a specified range of values
  template <typename RangeType, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
  void RunFunctionForRange(void(*function)(RangeType, T1, T2, T3, T4, T5, T6), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6);

protected:
  int m_NumWorkers;
  typedef itk::PlatformMultiThreader MultiThreader;
  typename MultiThreader::Pointer m_MultiThreader;

  // callback to run non-const method on object without parameters
  template <class T>
  static ITK_THREAD_RETURN_TYPE RunMethodCB(void* arg);

  // callback to run non-const method on object with one parameter
  template <class T, typename T1>
  static ITK_THREAD_RETURN_TYPE RunMethodCB(void* arg);

  // callback to run const method on object without parameters
  template <class T>
  static ITK_THREAD_RETURN_TYPE RunConstMethodCB(void* arg);

  // callback to run static method or function without parameters
  static ITK_THREAD_RETURN_TYPE RunFunctionCB(void* arg);

  // callback to run static method or function with one parameter
  template <typename T1>
  static ITK_THREAD_RETURN_TYPE RunFunctionCB(void* arg);

  // callback to run method without parameters for a specified range of values
  template <class T, typename RangeType>
  static ITK_THREAD_RETURN_TYPE RunMethodForRangeCB(void* arg);

  // callback to run method with one parameter for a specified range of values
  template <class T, typename RangeType, typename T1>
  static ITK_THREAD_RETURN_TYPE RunMethodForRangeCB(void* arg);

  // callback to run static method or function without parameters for a specified range of values
  template <typename RangeType>
  static ITK_THREAD_RETURN_TYPE RunFunctionForRangeCB(void* arg);

  // callback to run static method or function with one parameters for a specified range of values
  template <typename RangeType, typename T1>
  static ITK_THREAD_RETURN_TYPE RunFunctionForRangeCB(void* arg);

  // callback to run static method or function with two parameters for a specified range of values
  // TODO: implement

  // callback to run static method or function with three parameters for a specified range of values
  template <typename RangeType, typename T1,  typename T2,  typename T3>
  static ITK_THREAD_RETURN_TYPE RunFunctionForRangeCB(void* arg);

  // callback to run static method or function with five parameters for a specified range of values
  template <typename RangeType, typename T1,  typename T2,  typename T3, typename T4, typename T5>
  static ITK_THREAD_RETURN_TYPE RunFunctionForRangeCB(void* arg);

  // callback to run static method or function with six parameters for a specified range of values
  template <typename RangeType, typename T1,  typename T2,  typename T3, typename T4, typename T5, typename T6>
  static ITK_THREAD_RETURN_TYPE RunFunctionForRangeCB(void* arg);


};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkWorkers.txx"
#endif

#endif
