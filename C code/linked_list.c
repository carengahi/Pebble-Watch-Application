#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

/* definition of linked list helper functions */


/*
 * this function adds a node to the beginning of the linked list
 * -returns head_node - or NULL if error
 */
list* add_to_top (list* head_node, char* name ) {

	/* allocate memory for the new node */
	list* new_node = NULL ;
	new_node = malloc (sizeof(list)) ;
	/* check if malloc worked, otherwise, populate new node */
	if (new_node == NULL) return NULL ;
	new_node->name = malloc(strlen(name) + 1);
	if (new_node->name == NULL) return NULL ;
	strcpy(new_node->name, name);
	//new_node->prev  = NULL  ;
	new_node->next = head_node ;
	head_node = new_node;
	/* update former head pointer's links */
	//if (head_node) head_node->prev = new_node ;
	
	/* return new head node */
	return new_node ;
}

/*
 * this function adds a node to the end of the linked list
 * -returns head_node - or NULL if error
 */
list* add_to_end (list* head_node, char* name ) {

		/*list* new_node = malloc (sizeof(list));   // malloc    
		if (new_node == NULL) return NULL;                           
		list* current_node = head_node;                     // create temp variables
		new_node->name = name;                                      // assign the name
		if(head_node == NULL){                    //edge case, if the list is empty, create new node just like add_to_top
				new_node->name = name;
				new_node->prev  = NULL;
				new_node->next  = head_node ;
				return new_node;
				}
		while(current_node->next){                //if head is not empty, traverse to the end and create node
			current_node = current_node->next;
		}
			current_node->next  =  new_node;
			new_node->prev = current_node;
			new_node->next  = NULL;*/

	return head_node ;

}


/*
 * deletes the first instance of the "name" from the list
 * -returns head_node - or NULL if node isn't in list
 */
list*	delete_node (list* head_node, char* name ) {

	list* temp = head_node;
	list* current_node;

	if(head_node->name == name){                 //if the name of 1st item matches, start deleting!
		head_node = head_node->next;               //save temp and then free it 
		free(temp);
		return head_node;
	}

	current_node = head_node;                    
	while(current_node->next){                     //traverse the list until the name matches
		if(current_node->next->name == name){    
			temp = current_node->next;             //if found match, free the node just like above
			current_node->next = temp->next;
			free(temp);
			return head_node;
		}
		current_node = current_node->next;        //keep moving the current pointer     
	}

	return NULL;
}

/*
 * searches list for first instance of the "name" passed in
 * -returns node if found - or a NULL if node isn't in list
 */
list* search_list (list* head_node, char* name) {

	list* temp;
	temp = head_node;
	while(temp != NULL){      
	                 //traverse the list until the name matches and then return the pointer
		if(strcmp(temp->name, name) == 0){
			return temp;
		}
		temp = temp->next;         //move the pointer ahead
	}
	return NULL;
}


/*
 * sorts linked list in acending order: 1, 2, 3...
 * -returns new head of linked list after sort, or NULL if list is empty
 */
list* sort_list (list* head_node ) {

	list* current_node;
	list* compare_node;
	current_node = head_node->next;      //the pointer that's ahead 
	compare_node = head_node;        //the pointer that's behind
	char* temp = 0;

while(current_node){	                     //nested while loops, keeping tracking of two pointers

	while(compare_node != current_node){

		if(current_node->name < compare_node->name){

			temp = compare_node->name;                     //swap the two numbers if the current < the compared 
			compare_node->name = current_node->name;
			current_node->name = temp;
		}
		compare_node = compare_node->next;      //move the compared number and jumps into another loop
	}
	compare_node = head_node;                  
	current_node = current_node->next;      //move the current pointer
		
}

	return head_node;
}


/*
 * sorts linked list in decending order: 3, 2, 1...
 * -returns new head of linked list after sort, or NULL if list is empty
 */
list* sort_list_rev (list* head_node ) {

	list* current_node;              //exact same code except the sign of comparison
	list* compare_node;
	current_node = head_node->next;
	compare_node = head_node;
	char* temp = 0;

while(current_node){	

	while(compare_node != current_node){

		if(current_node->name > compare_node->name){

			temp = compare_node->name;
			compare_node->name = current_node->name;
			current_node->name = temp;
		}
		compare_node = compare_node->next;
	}
	compare_node = head_node;
	current_node = current_node->next;
		
}

	return head_node;
}


/*
 * prints entire linked list to look like an array
 */
void print_list (list* head_node ) {
	list* print_node = head_node;
	while (print_node) {	                               //as long as the head not null, keep printing, update i	
		printf ("%s ", print_node->name);
		print_node = print_node->next;
	}
	/* CIT 593 to do: this code only prints the first node,
			  print out the rest of the list! */

}


/*
 * delete every node in the linked list
 * returns NULL if successful, otherwise head node is returned
 */
list* delete_list (list* head_node ) {

	list* current_node;

	while(head_node){                     //while head not null, save head to current                                    //move head ahead, free current(head)
		current_node = head_node;
		head_node = head_node->next;
		free(current_node->name);
		free(current_node);
	}

	return head_node;

}
