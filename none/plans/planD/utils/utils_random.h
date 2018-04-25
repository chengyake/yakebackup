#ifndef random_h
#define random_h

#include <stdint.h>


int32_t init_random();
int32_t get_random(int8_t *data, uint32_t num);
void close_random ();


#endif
