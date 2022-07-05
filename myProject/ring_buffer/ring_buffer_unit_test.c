#include "ring_buffer.h"
#include "ring_buffer_unit_test.h"
#include <stdint.h> 
#include <stdio.h> 
#include <string.h>


#ifdef _WIN32
#include <inttypes.h>
#define PASSED "PASSED"
#define FAILED "FAILED"

#ifdef _WIN64
#define PRI_SIZET PRIu64
#else
#define PRI_SIZET PRIu32
#endif
#else
#define PRI_SIZET "zu"
#define PASSED "\x1b[32mPASSED\x1b[0m"
#define FAILED "\x1b[31mFAILED\x1b[0m"

#endif


#define test(expr)(!(expr)) ? (printf(FAILED" at line %u\n", __LINE__), false) : (printf(PASSED"\n"), true)
#define test_quiet(expr)(!(expr)) ? (printf(FAILED" at line %u\n", __LINE__), false) : true

enum {
  capacity1 = 10,
  capacity2 = 100,
  capacity3 = 500,
};

const char guardValue = 0xAB;

static bool RingBuffer_Init_ut();
static bool RingBuffer_PutChar_ut();

static bool RingBuffer_ut(RingBuffer* buffer) {
  printf("Test 0: Checking RingBuffer structure...\n");
  printf("   - Check if size of ring buffer structure is reasonable: ");
  bool result = test(sizeof(*buffer) <= 15 * sizeof(int));
  printf("Test 0: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}

static bool RingBuffer_Init_ut(RingBuffer* buffer, char* data, size_t dataSize) {
  printf("Test 1: Checking RingBuffer_Init...\n");
  printf("   - Initialize ring buffer with memory region with size %"PRI_SIZET": ", dataSize);
  bool result = test(true == RingBuffer_Init(buffer, data, dataSize));
  if(true == result) {
    printf("     Is empty: ");
    result &= test(true == RingBuffer_IsEmpty(buffer));
    printf("     Length is 0: ");
    result &= test(0 == RingBuffer_GetLen(buffer));
    printf("     Capacity is %"PRI_SIZET": ", dataSize);
    result &= test(dataSize == RingBuffer_GetCapacity(buffer));
  }
  printf("Test 1: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}

static bool RingBuffer_PutChar_ut(RingBuffer* buffer, char* data, size_t dataSize) {
  printf("Test 2: Checking RingBuffer_PutChar...\n");
  printf("   - Initialize ring buffer with memory region with size %"PRI_SIZET": ", dataSize);
  bool result = test(true == RingBuffer_Init(buffer, data, dataSize));
  if(true == result) {
    printf("     Write data into the buffer...\n");
    for(size_t j = 0; true == result && j < dataSize; j++) {
      result &= test_quiet(j == RingBuffer_GetLen(buffer));
      result &= test_quiet(true == RingBuffer_PutChar(buffer, j % 0x7f));
      result &= test_quiet(false == RingBuffer_IsEmpty(buffer));
      if(false == result) {
        printf("     Writing data no %"PRI_SIZET" produced invalid state (IsEmpty: %c, Length: %"PRI_SIZET", Capacity: %"PRI_SIZET")\n", j, RingBuffer_IsEmpty(buffer) ? 'T' : 'F', RingBuffer_GetLen(buffer), RingBuffer_GetCapacity(buffer));
      }
    }
    if(true == result) {
      printf("     Cannot hold more data: ");
      result &= test(false == RingBuffer_PutChar(buffer, '0'));
      printf("     Capacity is %"PRI_SIZET": ", dataSize);
      result &= test(dataSize == RingBuffer_GetCapacity(buffer));
      printf("     Length is %"PRI_SIZET": ", dataSize);
      result &= test(dataSize == RingBuffer_GetLen(buffer));
      printf("     Is not empty: ");
      result &= test(false == RingBuffer_IsEmpty(buffer));
    }
  }
  printf("Test 2: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}

static bool RingBuffer_GetChar_ut(RingBuffer* buffer, char* data, size_t dataSize) {
  printf("Test 3: Checking RingBuffer_GetChar...\n");
  printf("   - Initialize ring buffer with memory region with size %"PRI_SIZET": ", dataSize);
  bool result = test(true == RingBuffer_Init(buffer, data, dataSize));
  if(true == result) {
    printf("     Write data into the buffer...\n");
    for(size_t j = 0; true == result && j < dataSize; j++) {
      result &= test_quiet(true == RingBuffer_PutChar(buffer, j % 0x7f));
      if(false == result) {
        printf("     Writing data no %"PRI_SIZET" produced invalid state (IsEmpty: %c, Length: %"PRI_SIZET", Capacity: %"PRI_SIZET")\n", j, RingBuffer_IsEmpty(buffer) ? 'T' : 'F', RingBuffer_GetLen(buffer), RingBuffer_GetCapacity(buffer));
      }
    }
    if(true == result) {
      printf("     Read data from the buffer...\n");
      char in ;
      for(size_t j = 0; true == result && j < dataSize; ++j) {
        result &= test_quiet(true == RingBuffer_GetChar(buffer, &in));
        if(true == result) {
          result &= test_quiet(j % 0x7f == in);
          if(false == result) {
            printf("     Data no %"PRI_SIZET" mismatch (%x, but expected %x)\n", j, (int)in, (int)j % 0x7f);
          }
        } else {
          printf("     Failed to read data no %"PRI_SIZET"\n", j);
        }
      }
      if(true == result) {
        printf("     Is empty: ");
        result &= test(true == RingBuffer_IsEmpty(buffer));
        printf("     Length is 0: ");
        result &= test(0 == RingBuffer_GetLen(buffer));
        printf("     Capacity is %"PRI_SIZET": ", dataSize);
        result &= test(dataSize == RingBuffer_GetCapacity(buffer));
        printf("     Can be written: ");
        result &= test(true == RingBuffer_PutChar(buffer, '0'));
      }
    }
  }
  printf("Test 3: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}

static bool RingBuffer_Clear_ut(RingBuffer* buffer, char* data, size_t dataSize) {
  printf("Test 4: Checking RingBuffer_Clear...\n");
  printf("   - Initialize ring buffer with memory region with size %"PRI_SIZET": ", dataSize);
  bool result = test(true == RingBuffer_Init(buffer, data, dataSize));
  if(true == result) {
    printf("     Write data into the buffer...\n");
    for(size_t j = 0; true == result && j < dataSize; j++) {
      result &= test_quiet(true == RingBuffer_PutChar(buffer, j % 0x7f));
      if(false == result) {
        printf("     Writing data no %"PRI_SIZET" produced invalid state (IsEmpty: %c, Length: %"PRI_SIZET", Capacity: %"PRI_SIZET")\n", j, RingBuffer_IsEmpty(buffer) ? 'T' : 'F', RingBuffer_GetLen(buffer), RingBuffer_GetCapacity(buffer));
      }
    }
    if(true == result) {
      printf("     Successful Clear: ");
      result &= test(true == RingBuffer_Clear(buffer));
      printf("     Is empty: ");
      result &= test(true == RingBuffer_IsEmpty(buffer));
      printf("     Length is 0: ");
      result &= test(0 == RingBuffer_GetLen(buffer));
      printf("     Capacity is %"PRI_SIZET": ", dataSize);
      result &= test(dataSize == RingBuffer_GetCapacity(buffer));
      printf("     Can be written: ");
      result &= test(true == RingBuffer_PutChar(buffer, '0'));
    }
  }
  printf("Test 4: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}

static bool RingBuffer_Interleaved_ut(RingBuffer* buffer, char* data, size_t dataSize) {
  printf("Test 5: Checking RingBuffer interleaved read/write...\n");
  printf("   - Initialize guard memory region...\n");
  data[0] = data[dataSize - 1] = guardValue;
  size_t dataSizeWithoutGuard = dataSize -2;
  printf("   - Initialize ring buffer with memory region with size %"PRI_SIZET": ", dataSizeWithoutGuard);
  bool result = test(true == RingBuffer_Init(buffer, data + 1, dataSizeWithoutGuard));
 
  if(true == result) {
    printf("     Write data into the buffer...\n");
    for(size_t j = 0; true == result && j < dataSizeWithoutGuard; ++j) {
      result &= test_quiet(j == RingBuffer_GetLen(buffer));
      result &= test_quiet(true == RingBuffer_PutChar(buffer, 0xFF));
    }
    if(true == result) {
      for(size_t j = 0; true == result && j < 10; ++j) {
        printf("     Read data from the buffer...\n");
        char in;
        for(size_t j = 0; true == result && j < dataSizeWithoutGuard/2; ++j) {
          result &= test_quiet(true == RingBuffer_GetChar(buffer, &in));
          if(true == result) {
            result &= test_quiet((char)0xFF == in);
          }
        }
        if(true == result) {
          printf("     Write data into the buffer...\n");
          for(size_t j = 0; true == result && j < dataSizeWithoutGuard/2; ++j) {
            if(result &= test_quiet(true == RingBuffer_PutChar(buffer, 0xFF))) {
                if(false == (result &= test_quiet(guardValue == data[0] && guardValue == data[dataSize - 1]))) {
                    printf("     Writing data resulted in Buffer Overflow\n");
                }
            }
          }
        }
      }
    }
  }
  printf("Test 5: %s\n\n", (true == result) ? PASSED : FAILED);
  return result;
}








bool RingBuffer_UnitTest(void) {
  RingBuffer buffer;
  char data1[capacity1], data2[capacity2], data3[capacity3];
    
  char* data[] = {data1, data2, data3};
  size_t dataSize[] = { sizeof(data1), sizeof(data2), sizeof(data3)};
  bool result = true;
  for(int i = 0; true == result && i < sizeof(data)/ sizeof(*data); ++i) {
    printf("========== PERFORMING TEST FOR DATA BUFFER WITH SIZE %"PRI_SIZET" ==========\n", dataSize[i]);
    do {
    if(false == (result &= RingBuffer_ut(&buffer))) break;
    if(false == (result &= RingBuffer_Init_ut(&buffer, data[i], dataSize[i]))) break;
    if(false == (result &= RingBuffer_PutChar_ut(&buffer, data[i], dataSize[i]))) break;
    if(false == (result &= RingBuffer_GetChar_ut(&buffer, data[i], dataSize[i]))) break;
    if(false == (result &= RingBuffer_Clear_ut(&buffer, data[i], dataSize[i]))) break;
    if(false == (result &= RingBuffer_Interleaved_ut(&buffer, data[i], dataSize[i]))) break;
    } while(0);
    printf("========== TEST FOR DATA BUFFER WITH SIZE %"PRI_SIZET" FINISHED WITH RESULT %s ==========\n", dataSize[i], (true == result) ? PASSED : FAILED);
  }
  return result;
}