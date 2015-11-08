#ifndef _BUFFER_H_DEFINED_
#define _BUFFER_H_DEFINED_

typedef int buffer_item;

#define BUFFER_SIZE 5

bool buffer_insert_item( buffer_item item );

bool buffer_remove_item( buffer_item *item );

#endif // _BUFFER_H_DEFINED_
