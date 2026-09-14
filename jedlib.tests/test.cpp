#include "test_assert.h"
#include "test_buffer.h"
#include "test_edit.h"
#include <ctime>

#if defined(MEMORY_LEAK_TRACKING) && defined(_MSC_VER)
#ifndef NDEBUG
#define MEMORY_LEAK_TRACKING_MSVC
#endif
#endif

void init_debug()
  {
#if defined(MEMORY_LEAK_TRACKING_MSVC)
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  _CrtSetDbgFlag(flags | _CRTDBG_ALLOC_MEM_DF);
#endif
  }

void close_debug()
  {
#if defined(MEMORY_LEAK_TRACKING_MSVC)
  int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  flags &= ~_CRTDBG_DELAY_FREE_MEM_DF;
  flags |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(flags);
#endif
  }

int main(int /*argc*/, const char* /*argv*/[])
  {
  init_debug();

  InitTestEngine();

  auto tic = std::clock();
  run_buffer_tests();  
  run_edit_tests();
  auto toc = std::clock();

  if (!testing_fails) 
    {
    TEST_OUTPUT_LINE("Succes: %d tests passed.", testing_success);
    }
  else 
    {
    TEST_OUTPUT_LINE("FAILURE: %d out of %d tests failed (%d failures).", testing_fails, testing_success+testing_fails, testing_fails);
    }
  TEST_OUTPUT_LINE("Test time: %f seconds.", (double)(toc - tic)/(double)CLOCKS_PER_SEC);
  int res = CloseTestEngine(true);
  close_debug();
  return res;
  }
