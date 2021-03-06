#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "util.h"
#include "queue.h"
#include "list.h"

void queue_element_print(element_ptr_t element);
void queue_element_copy(element_ptr_t* dest_element, element_ptr_t src_element);
void queue_element_free(element_ptr_t* element);
int  queue_element_compare(element_ptr_t x, element_ptr_t y) {
    return *(int*)x > *(int*)y ? 1 :
           *(int*)x < *(int*)y ? -1 : 0;
}

const unsigned int QUEUE_ELEMENT_SIZE = sizeof(int);


int main( void )
{
   int* a = (int*)malloc(sizeof(int));
   int* b = (int*)malloc(sizeof(int));
   int* c = (int*)malloc(sizeof(int));
   int* d = (int*)malloc(sizeof(int));
   *a = 1;
   *b = 2;
   *c = 3;
   *d = 4;

   queue_ptr_t queue = NULL;
   queue = queue_create();
   printf("queue top: %p\n", queue_top(queue));
   queue_enqueue(queue, b);
   queue_enqueue(queue, c);
   queue_enqueue(queue, d);
   queue_print(queue);

   queue_enqueue(queue, a);
   queue_enqueue(queue, b);
   queue_print(queue);

   queue_dequeue(queue);
   queue_print(queue);

   queue_enqueue(queue, a);
   queue_enqueue(queue, b);
   queue_enqueue(queue, c);
   queue_enqueue(queue, d);
   queue_enqueue(queue, b);
   queue_enqueue(queue, c);
   queue_enqueue(queue, d);
   queue_print(queue);

   printf("%d %d %d\n",*a,*b, queue_element_compare(a,b));

//   list_ptr_t list = list_create( &queue_element_copy, &queue_element_free,
//                                  &queue_element_compare, &queue_element_print);
//   list_insert_at_index(list, a, 3);
//   list_print(list);
//   list_insert_at_index(list, b, -1);
//   list_print(list);
//   list_insert_at_index(list, c, 1);
//   list_print(list);
//   list_insert_at_index(list, d, 3);
//   list_print(list);

//   printf("%d %d\n", list_get_index_of_element(list,a),
//                     list_get_index_of_element(list,c));

//   queue_free(&queue);
//   list_fFree(&list);

   queue_free(&queue);

   free(a);
   free(b);
   free(c);
   free(d);
   return 0;
}



/*
 * Implement here private functions to copy, to print and to destroy an element. Do you understand why you need these functions?
 * Later you will learn how you could avoid this by using funtion pointers.
 *
 */

/*
 * Print 1 element to stdout.
 * If the defition of element_ptr_t changes, then this code needs to change as well.
 */
void queue_element_print(element_ptr_t element)
{
    if(element) printf("%d ", *(int*)element);
}


/*
 * Copy the content (e.g. all fields of a struct) of src_element to dst_element.
 * dest_element alredy points to allocated memory - no memory allocation will be done in this function
 * If the defition of element_ptr_t changes, then this code needs to change as well.
 */
void queue_element_copy(element_ptr_t* dest_element, element_ptr_t src_element)
{
    assert(src_element && dest_element && *dest_element);
    memcpy(*dest_element, src_element, sizeof(int));
}

/*
 * if the element contains pointers inside its definition,
 * then free those memory, but don't free the element in the end
 * because it's just part of the array of the circular queue
 * Otherwise do nothing
 */
void queue_element_free(element_ptr_t* element)
{
//    free(&element);
}
