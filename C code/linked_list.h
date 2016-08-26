#ifndef LINKED_LIST_H
#define LINKED_LIST_H

/* linked list node definition */

typedef struct list_name {
	char* name ; 
	struct list_name *next ;  /* pointer to next node in list */
} list ;


/* linked list helper function declarations */
list* add_to_top    (list* head_node, char* value ) ;
list* add_to_end    (list* head_node, char* value ) ;
list*	delete_node   (list* head_node, char* value ) ;
list* search_list   (list* head_node, char* name) ;
list* sort_list     (list* head_node ) ;
list* sort_list_rev (list* head_node ) ;
void	print_list    (list* head_node ) ;
list* delete_list   (list* head_node ) ;


#endif
