#ifndef LIST_H_
#define LIST_H_

typedef int compareFunc(const void*,const void*);
typedef void *data_ptr_t;

typedef struct info{
	int size;
	struct node *head;
	struct node *tail;	
	compareFunc *compare;
}list_t, *list_ptr_t;

typedef struct node{
	data_ptr_t data;
	struct node *next;
	struct node *prev;
}Tnode, *Tlist;

	
	
extern list_ptr_t list_alloc ( compareFunc*);
// Returns a pointer to a newly-allocated list.

extern void list_free( list_ptr_t list );
// Every element of 'list' needs to be deleted (free memory) and finally the list itself needs to be deleted (free all memory)

extern int list_size( list_ptr_t list );
// Returns the number of elements in 'list'.

extern list_ptr_t list_insert_at_index( list_ptr_t list, data_ptr_t data, int index);
// Inserts a new element containing 'data' in 'list' at position 'index'  and returns a pointer to the new list. If 'index' is 0 or negative, the element is inserted at the start of 'list'. If 'index' is bigger than the number of elements in 'list', the element is inserted at the end of 'list'.

extern list_ptr_t list_insert_at_reference( list_ptr_t list, data_ptr_t data, list_ptr_t reference );
// Inserts a new element containing 'data' in the 'list' at position 'reference'  and returns a pointer to the new list. If 'reference' is NULL, the element is inserted at the end of 'list'.

extern list_ptr_t list_insert_sorted( list_ptr_t list, data_ptr_t data );
// Inserts a new element containing 'data' in the sorted 'list' and returns a pointer to the new list. The 'list' must be sorted before calling this function.

extern list_ptr_t list_free_at_index( list_ptr_t list, int index);
// Deletes the element at index 'index' in 'list'. A free() is called on the data pointer of the element to free any dynamic memory allocated to the data pointer. If 'index' is 0 or negative, the first element is deleted. If 'index' is bigger than the number of elements in 'list', the data of the last element is deleted.

extern list_ptr_t list_free_at_reference( list_ptr_t list, list_ptr_t reference );
// Deletes the element with position 'reference' in 'list'. A free() is called on the data pointer of the element to free any dynamic memory allocated to the data pointer. If 'reference' is NULL, the data of the last element is deleted.

extern list_ptr_t list_free_data( list_ptr_t list, data_ptr_t data );
// Finds the first element in 'list' that contains 'data' and deletes the element from 'list'. A free() is called on the data pointer of the element to free any dynamic memory allocated to the data pointer.

extern list_ptr_t list_remove_at_index( list_ptr_t list, int index);
// Removes the element at index 'index' from 'list'. NO free() is called on the data pointer of the element. If 'index' is 0 or negative, the first element is removed. If 'index' is bigger than the number of elements in 'list', the data of the last element is removed.

extern list_ptr_t list_remove_at_reference( list_ptr_t list, list_ptr_t reference );
// Removes the element with reference 'reference' in 'list'. NO free() is called on the data pointer of the element. If 'reference' is NULL, the data of the last element is removed.

extern list_ptr_t list_remove_data( list_ptr_t list, data_ptr_t data );
// Finds the first element in 'list' that contains 'data' and removes the element from 'list'. NO free() is called on the data pointer of the element.

extern list_ptr_t list_get_first_reference( list_ptr_t list );
// Returns a reference to the first element of 'list'. If the list is empty, NULL is returned.

extern list_ptr_t list_get_last_reference( list_ptr_t list );
// Returns a reference to the last element of 'list'. If the list is empty, NULL is returned.

extern list_ptr_t list_get_next_reference( list_ptr_t list, list_ptr_t reference );
// Returns a reference to the next element of the element with reference 'reference' in 'list'. If the next element doesn't exists, NULL is returned.

extern list_ptr_t list_get_previous_reference( list_ptr_t list, list_ptr_t reference );
// Returns a reference to the previous element of the element with reference 'reference' in 'list'. If the previous element doesn't exists, NULL is returned.

extern int list_get_index_of_reference( list_ptr_t list, list_ptr_t reference );
// Returns the index of the element in the 'list' with reference 'reference'. If 'reference' is NULL, the index of the last element is returned.

extern list_ptr_t list_get_reference_at_index( list_ptr_t list, int index );
// Returns a reference to the element with index 'index' in 'list'. If 'index' is 0 or negative, a reference to the first element is returned. If 'index' is bigger than the number of elements in 'list', a reference to the last element is returned. If the list is empty, NULL is returned.

extern data_ptr_t list_get_data_at_reference( list_ptr_t list, list_ptr_t reference );
// Returns the data pointer contained in the element with reference 'reference' in 'list'. If 'reference' is NULL, the data of the last element is returned.

extern data_ptr_t list_get_data_at_index( list_ptr_t list, int index );
// Returns the data contained in the element with index 'index' in 'list'. If 'index' is 0 or negative, the data of the first element is returned. If 'index' is bigger than the number of elements in 'list', the data of the last element is returned.

extern int list_get_index_of_data( list_ptr_t list, data_ptr_t data );
// Returns an index to the first element in 'list' containing 'data'.  If 'data' is not found in 'list', -1 is returned.

extern list_ptr_t list_get_reference_of_data( list_ptr_t list, data_ptr_t data );
// Returns a reference to the first element in 'list' containing 'data'. If 'data' is not found in 'list', NULL is returned.


#endif
