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

#ifndef _itkWorkers_txx
#define _itkWorkers_txx

#include "itkWorkers.h"
 
namespace itk
{

//----------------------------------------------------------------------------
Workers::Workers(int numWorkers) :
m_MultiThreader(itk::PlatformMultiThreader::New())
{
  m_NumWorkers = std::max(1, std::min(numWorkers, int(m_MultiThreader->GetGlobalMaximumNumberOfThreads())));
  m_MultiThreader->SetNumberOfThreads(m_NumWorkers);
}

//----------------------------------------------------------------------------
int Workers::GetNumberOfWorkers() const
{
  return m_NumWorkers;
};

//----------------------------------------------------------------------------
template <class T>
void Workers::RunMethod(T* object, void(T::*method)(int, int) )
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int);};
  UserData data = {this, object, method};
  m_MultiThreader->SetSingleMethod(this->RunMethodCB<T>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <class T>
itk::ITK_THREAD_RETURN_TYPE Workers::RunMethodCB(void* arg)
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int);};    
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);    
  ((data->object)->*(data->method))(workerId,numWorkers);    
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <class T, typename T1>
void Workers::RunMethod(T* object, void(T::*method)(int, int, T1), T1 p1 )
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int, T1); T1 p1;};
  UserData data = {this,object,method,p1};
  m_MultiThreader->SetSingleMethod(this->RunMethodCB<T, T1>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <class T, typename T1>
itk::ITK_THREAD_RETURN_TYPE Workers::RunMethodCB(void* arg)
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int, T1); T1 p1;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);    
  ((data->object)->*(data->method))(workerId,numWorkers, data->p1);
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <class T>
void Workers::RunMethod(T* object, void(T::*method)(int, int) const )
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int) const;};
  UserData data = {this, object, method};
  m_MultiThreader->SetSingleMethod(this->RunConstMethodCB<T>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <class T>
itk::ITK_THREAD_RETURN_TYPE Workers::RunConstMethodCB(void* arg)
{
  struct UserData{Workers* workers; T* object; void(T::*method)(int, int) const;};    
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);    
  ((data->object)->*(data->method))(workerId,numWorkers);    
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
void Workers::RunFunction(void(*function)(int, int))
{
  struct UserData{Workers* workers; void(*function)(int, int);};
  UserData data = {this, function};
  m_MultiThreader->SetSingleMethod(RunFunctionCB, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(int, int);};    
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);    
  (data->method)(workerId,numWorkers);
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}
  
//----------------------------------------------------------------------------
template <typename T1>
void Workers::RunFunction(void(*function)(int, int, T1), T1 p1)
{
  struct UserData{Workers* workers; void(*function)(int, int, T1); T1 p1;};
  UserData data = {this, function, p1};
  m_MultiThreader->SetSingleMethod(this->RunFunctionCB<T1>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename T1>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(int, int, T1); T1 p1;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);    
  (data->method)(workerId,numWorkers, data->p1);
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <typename RangeType>
void Workers::RunFunctionForRange(void(*function)(RangeType), RangeType min, RangeType max)
{
  struct UserData{Workers* workers; void(*method)(RangeType); RangeType min; RangeType max;};
  UserData data = {this, function, min, max};
  m_MultiThreader->SetSingleMethod(this->RunFunctionForRangeCB<RangeType>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename RangeType>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionForRangeCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(RangeType); RangeType min; RangeType max;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {   
    (data->method)(currentValue);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1>
void Workers::RunFunctionForRange(void(*function)(RangeType, T1), RangeType min, RangeType max, T1 p1)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1); RangeType min; RangeType max; T1 p1;};
  UserData data = {this, function, min, max, p1};
  m_MultiThreader->SetSingleMethod(this->RunFunctionForRangeCB<RangeType, T1>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionForRangeCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1); RangeType min; RangeType max; T1 p1;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {   
    (data->method)(currentValue, data->p1);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1, typename T2, typename T3>
void Workers::RunFunctionForRange(void(*function)(RangeType, T1, T2, T3), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3;};
  UserData data = {this, function, min, max, p1, p2, p3};
  m_MultiThreader->SetSingleMethod(this->RunFunctionForRangeCB<RangeType, T1, T2, T3>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1, typename T2, typename T3>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionForRangeCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {   
    (data->method)(currentValue, data->p1, data->p2, data->p3);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1, typename T2, typename T3, typename T4, typename T5>
void Workers::RunFunctionForRange(void(*function)(RangeType, T1, T2, T3, T4, T5), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3, T4, T5); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3; T4 p4; T5 p5;};
  UserData data = {this, function, min, max, p1, p2, p3, p4, p5};
  m_MultiThreader->SetSingleMethod(this->RunFunctionForRangeCB<RangeType, T1, T2, T3, T4, T5>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1, typename T2,  typename T3, typename T4, typename T5>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionForRangeCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3, T4 p4, T5 p5); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3; T4 p4; T5 p5;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {
    (data->method)(currentValue, data->p1, data->p2, data->p3, data->p4, data->p5);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void Workers::RunFunctionForRange(void(*function)(RangeType, T1, T2, T3, T4, T5, T6), RangeType min, RangeType max, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3, T4, T5, T6); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3; T4 p4; T5 p5; T6 p6;};
  UserData data = {this, function, min, max, p1, p2, p3, p4, p5, p6};
  m_MultiThreader->SetSingleMethod(this->RunFunctionForRangeCB<RangeType, T1, T2, T3, T4, T5, T6>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <typename RangeType, typename T1,  typename T2,  typename T3, typename T4, typename T5, typename T6>
itk::ITK_THREAD_RETURN_TYPE Workers::RunFunctionForRangeCB(void* arg)
{
  struct UserData{Workers* workers; void(*method)(RangeType, T1, T2, T3, T4 p4, T5 p5, T6 p6); RangeType min; RangeType max; T1 p1; T2 p2; T3 p3; T4 p4; T5 p5; T6 p6;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {
    (data->method)(currentValue, data->p1, data->p2, data->p3, data->p4, data->p5, data->p6);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <class T, typename RangeType>
void Workers::RunMethodForRange(T* object, void(T::*method)(RangeType), RangeType min, RangeType max )
{
  struct UserData{Workers* workers; T* object; void(*method)(RangeType); RangeType min; RangeType max;};
  UserData data = {this, object, method, min, max};
  m_MultiThreader->SetSingleMethod(this->RunMethodForRangeCB<T,RangeType>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <class T, typename RangeType>
itk::ITK_THREAD_RETURN_TYPE Workers::RunMethodForRangeCB(void* arg)
{
  struct UserData{Workers* workers; T* object; void(*method)(RangeType); RangeType min; RangeType max;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {   
     ((data->object)->*(data->method))(currentValue);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

//----------------------------------------------------------------------------
template <class T, typename RangeType, typename T1>
void Workers::RunMethodForRange(T* object, void(T::*method)(RangeType, T1), RangeType min, RangeType max, T1 p1 )
{
  struct UserData{Workers* workers; T* object; void(*method)(RangeType); RangeType min; RangeType max; T1 p1;};
  UserData data = {this, object, method, min, max, p1};
  m_MultiThreader->SetSingleMethod(this->RunMethodForRangeCB<T,RangeType,T1>, (void*)(&data));
  m_MultiThreader->SingleMethodExecute();
}

//----------------------------------------------------------------------------
template <class T, typename RangeType, typename T1>
itk::ITK_THREAD_RETURN_TYPE Workers::RunMethodForRangeCB(void* arg)
{
  struct UserData{Workers* workers; T* object; void(*method)(RangeType); RangeType min; RangeType max; T1 p1;};
  itk::PlatformMultiThreader::ThreadInfoStruct* threadInfo = (itk::PlatformMultiThreader::ThreadInfoStruct *)( arg );
  int workerId = threadInfo->ThreadID;
  int numWorkers = threadInfo->NumberOfThreads;    
  UserData* data = (UserData*)(threadInfo->UserData);
  RangeType currentValue = (data->min)+RangeType(workerId);
  while (currentValue<=data->max)
  {   
     ((data->object)->*(data->method))(currentValue, data->p1);
    currentValue+=RangeType(numWorkers);
  }
  return itk::ITK_THREAD_RETURN_DEFAULT_VALUE;
}

} // namespace

#endif

