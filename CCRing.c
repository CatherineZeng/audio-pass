#include <stdlib.h>
#include <string.h> // memset
#include <stdio.h>
#include <math.h>
#include "CCRing.h"
#define PI 3.14159265

CCRing* createRing(unsigned long length) {

  CCRing* ring = (CCRing*) malloc(sizeof(CCRing));
  if (ring == NULL) {
    return NULL;
  }

  // size of the data in bytes
  size_t dataSize = length * sizeof(ccAudioDataType);

  // allocate data storage
  ring->data = (ccAudioDataType*) malloc(dataSize);
  if (ring->data == NULL) {
    free(ring);
    return NULL;
  }

  // initialize length
  ring->length = length;

  // initialize index to last spot in data array
  ring->index = length - 1;

  // initialize all data to zero
  memset(ring->data, 0, dataSize);

  return ring;
}

CCError freeRing(CCRing* pRing) {
  free(pRing->data);
  free(pRing);
  return ccNoError;
}

CCError ccAppend(CCRing* pRing, ccAudioDataType arr[], unsigned long length) {
  for (unsigned long i = 0; i < length; i++) {
    (pRing->index)++;
    if ((pRing->index) >= (pRing->length)){
      pRing->index = 0;
    }
    *(pRing->data + pRing->index) = *(arr + i);
  }
  return ccNoError;
}

CCError ccValidLen(CCRing* ring, unsigned long tap) {
  if (tap > ring->length) return ccError;

  if (tap < ring->index){
    return ring->index - tap + 1;
  }
  else if (tap > ring->index){
    return (ring->length - tap + ring->index + 1);
  }
  else if (tap == ring->index){
    return 1;
  }

  return ccNoError;
}

CCError getSamples(CCRing* source, CCRing* target, unsigned long targetLen, unsigned long tap){
  if (tap > source->length) return ccError;

  unsigned long space = ccValidLen(source, tap);

  if (space >= targetLen){

    // create temp array to hold values from source to transfer to target
    size_t dataSize = space * sizeof(ccAudioDataType);
    ccAudioDataType* temp_array = (ccAudioDataType*) malloc(dataSize);

    // appends temp_array with elements of source array that begin at 
    if(tap <= source->index){
      for (unsigned long i = 0; i < space; ++i){
        temp_array[i] = source->data[tap + i];
      }
    }

    if(tap > source->index){
      for (unsigned long i = 0; i < space; ++i){
        temp_array[i] = source->data[(tap + i)%source->length];
      }
    }

    // appends temp_array to target
    ccAppend(target, temp_array, targetLen);

    free(temp_array);

    return ccNoError;
  }
  else return ccError;
}

CCError ccGenerateSin(CCRing* sinusoid, double cycles) {
  double distBetweenPoints = (2*PI)/sinusoid->length;
  for (unsigned long i = 0; i < sinusoid->length; ++i) {
    double n = i * distBetweenPoints;
    *(sinusoid->data + i) = sin(cycles * n);
  }
  return ccNoError;
}

CCError ccMultiply(CCRing* target, CCRing* source) {
  if (target->length != source->length) {
    return ccBufferSizeMismatch;
  }
  for (unsigned long i = 0; i < target->length; i++) {
    *(target->data + i) = *(target->data + i) * *(source->data + i);
  }
  return ccNoError;
}
