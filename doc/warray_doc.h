/**	@file

	@mainpage Workhorse Array
	A dynamic growing array of typed void* elements supporting many common operations and automatic
	element cloning and deletion.


	@section hello_world Hello world

	\code
	#include "warray.h"

	void foo() {
		//Create an array with an initial capacity of 10 elements holding char* strings.
		WArray* animals = warray_new( 10, wtypeStr );

		//Insert the allocated copies of three strings into the array. From now on the array
		//is responsible for the memory management of the elements.
		warray_append( animals, "cat" );
		warray_append( animals, "mouse" );
		warray_append( animals, "dog" );

		//Sort the array according to the string ordering rules.
		warray_sort( animals );										//->"cat", "dog", "mouse"

		//Find an element by comparing the strings.
		ssize_t mousePosition = warray_index( animals, "mouse" );	//->2

		//Remove the element from the array and free the string.
		warray_removeFirst( animals );

		//Remove the allocated string from the array and pass ownership to
		//the calling code.
		char* mouse = warray_stealLast( animals );					//->mouse = "mouse"

		//Free the array including the remaining dog.
		warray_delete( &animals );									//->animals = NULL
		
		//We stole the mouse, so we must free it ourselves.
		free( mouse );
	}
	\endcode


	@section features Features

	- The array can only contain elements of the same type.
	- Has many functions to add elements: warray_prepend(), warray_append(), warray_insert(),
	  warray_update().
	- Peek at elements with warray_first(), warray_last() and warray_at().
	- Take elements out of the array with warray_stealFirst(), warray_stealLast() and warray_stealAt().
	- Iterate through all elements with warray_filter(), warray_map() and warray_reduce().
	- The array copies the given elements and leaves the originals untouched.
	- Elements get deleted automatically when necessary.
	- The array takes full ownership of the elements. They can only be manipulated through functions.
	- Supports sparse arrays. The gaps are filled with NULL.


	@section api_design API design

	- Once the elements are added to the array, they belong to the array and may only be
		accessed through the array functions. Don't try to read or write elements directly
		from the WArray.data field, the behaviour in this case is undefined.
    - All functions except warray_new() take an array pointer as first argument. This is the array
		queried or manipulated by the functions. It may never be NULL.
    - Functions taking a const argument, be it an array or some other data guarantee not to modify it.
	- Functions taking a non-const argument probably will modify it.
	- Functions manipulating one or more array elements in general do it in place, except where it is not
		possible or does not make sense. The latter functions take a const WArray* argument.
	- The elements may be of any type, as long as they can be passed as void*. NULL as element is allowed.
	- The functions never return NULL except when a NULL element is returned.
	- When a function returns a pointer to const it means that the result is read-only for the calling
		code. It might be changed by the next array function. So clone it with e.g. array->cloneElement( result )
		if you need it later.
	- When a function returns a pointer to a non-const result, it most certainly will be allocated or
		constructed in some way. The calling code is responsible to free or destroy it properly. This
		can be done with array->deleteElement( result );
    - The calling code can pass two functions to the array, one for copying and one for deleting elements.
		When an element is added to the array, it is copied there with the clone function. Whenever an
		element must be deleted by an array function, it is done with the delete function. There are default
		functions that simply copy the pointer (clone) resp. do nothing (delete). The calling code must
		ensure that either the clone function makes a deep copy of the passed element. Otherwise the array
		holds a reference to the given element. Then the caller must ensure that the reference is valid as
		long as the array is alive.
	- The array is a non-opaque, but read only structure. Don't access the actual elements directly.
	- The fields capacity, size, cloneElement and deleteElement can be read.
	- If an array function has a precondition it is described in the API documentation. The functions check if
		the preconditions are met and abort the program with an error message if they are not met
		(Design by Contract). A failed precondition is a bug in the calling code and must be corrected.
		The behaviour cannot be deactivated.


	@section ownership Ownership

	- Elements get copied to the array. The original data stays untouched --> const void*
	- Can only be manipulated through array functions. --> const void*
	- Elements get automatically copied or deleted if necessary.
	- Read-only access with warray_at() --> returned as const void*
	- Ownership transfer with warray_steal() --> void*


	@section create_delete Creating and deleting arrays

	Before you can use an array you must create one with warray_new(). You can optionally pass
	an initial capacity, if not, a default value is taken. Later on if more elements are inserted,
	the array grows automatically as needed.

	More important: You have to pass the element type. It is a pointer to a WType structure with
	function pointers. The referenced functions are responsible for copying, deleting or comparing
	elements of the respective type. There are some predefined types, or you can easily define your
	own custom types. Once the element type is known to the array, it takes care of all the rest, and the
	calling code is relieved of memory management issues.

	Some examples for arrays with standard elements:
	\code
	//Create some arrays with predefined standard C types.
	WArray* array1 = warray_new( 10, wtypeStr );	//initial capacity 10, char* elements
	WArray* array2 = warray_new( 0, wtypeInt );		//default initial capacity, int elements
	WArray* array3 = warray_new( 0, wtypeDouble );	//double elements
	WArray* array4 = warray_new( 0, NULL );			//raw void* elements. This should be the
													//exception, because it prevents the array
													//from properly handling ownership of the
													//inserted elements.
	\endcode

	And a simple example for an array holding custom elements:

	\code
	typedef struct Person {
		char*		name;
		unsigned	age;
	}Person;

	//Allocate memory for the structure and the contained string and copy the content
	//from the input element to the output element. Error handling omitted.
	void* clonePerson( const void* element ) {
		const Person* person = element;
		Person* clone = calloc( 1, sizeof( Person ));
		clone->name = strdup( person->name );
		clone->age = person->age;
		return clone;
	}

	//Free the structure and string memory.
	void deletePerson( void** element ) {
		Person* person = *element;
		free( person->name );
		free( person );
	}

	//Compare two persons by simply comparing the name strings. NULL handling omitted.
	int comparePerson( const void* element1, const void* element2 ) {
		const Person* person1 = element1;
		const Person* person2 = element2;
		return strcmp( person1->name, person2->name );
	}

	//Here we define our custom element type.
	const WType personType = {
		.clone = clonePerson,	//Used e.g. in warray_clone().
		.delete = deletePerson,	//Used e.g. in warray_clear() and warray_delete().
		.comparePerson			//Used e.g. in warray_sort() or warray_index().
	};

	//And finally we pass the type information to a new array.
	WArray* array4 = warray_new( 0, &personType );
	\endcode

	Copying an array is simple: Just one function call and the array structure, capacity, size
	and type information are copied. The elements are copied into the new array according to the type's
	clone method:

	\code
	WArray* clone = warray_clone( array );
	\endcode

	Clearing or deleting arrays is easy too:
	\code
	//Remove and free the elements according to the type's delete method.
	warray_clear( array );

	//Clear the array, free the array structure and set the pointer to NULL.
	warray_delete( &clone );
	\endcode

	- warray_new()
	- warray_clone()
	- warray_delete()
	- warray_clear()
	- warray_assign()


	@section insert Putting elements in the array

	There are several functions to put elements in arrays, be it at the front or back or at
	some other position. They all have in common, that they leave the given data untouched, but
	rather take a copy with the array type's clone method.

	\code
	//Create an array of char* strings.
	WArray* array = warray_new( 10, wtypeStr );
	//Make a copy of the string literal and put it in the array.
	warray_append( array, "cat" );
	//Dto., this time at the front.
	const char dog[] = "dog";
	warray_prepend( array, dog );
	//Or somewhere in the middle, here an empty string.
	warray_insert( array, 1, "" );
	//NULL elements are allowed.
	warray_insert( array, 2, NULL );
	//Make it a sparse array. The gap is filled with NULL elements.
	warray_set( array, 10, "lion" );
	\endcode

	You can also put several elements in the array at once, if you like. But you must ensure
	that the given element number matches the data:

	\code
	WArray* array = warray_new( 0, wtypeStr );
	warray_append_n( array, 5, (void*[]){ "cat", "dog", "", NULL, "mouse" });
	\endcode

	And last but not least you can append an entire array to another array:
	\code
	//Beware: The arrays must hold the same element type.
	warray_concat( front, back );
	\endcode

	- warray_append()
	- warray_prepend()
	- warray_set()
	- warray_insert()
	- warray_insertSorted()
	- warray_append_n()
	- warray_prepend_n()
	- warray_set_n()
	- warray_insert_n()
	- warray_concat()


	@section get Getting elements from the array

	There is a bunch of options how to get elements from arrays, depending on who keeps
	ownership of the element and if the element is removed from the array.

	You can simply read them, but leave ownership to the array:

	\code
	//Take care not to keep the element pointers too long, the next modifying array
	//function might invalidate them. Dont't cast the constness away and manipulate
	//the elements directly. This is undefined behaviour.
	const char* name0 = warray_first( names );
	const char* name1 = warray_at( names, 3 );
	const char* name2 = warray_last( names );
	\endcode

	You can make a full copy of an element or several elements:

	\code
	char* name0 = warray_cloneFirst( names );
	char* name1 = warray_cloneAt( names, 6 );
	char* name2 = warray_cloneLast( names );
	WArray* copiedNames = warray_slice( names, 3, 8 );
	//...Do some stuff with the strings, then free them yourself.
	free( name0 );
	free( name1 );
	free( name2 );
	warray_delete( &copiedNames );
	\endcode

	Or you can steal an element from the array:

	\code
	//The elements are extracted, the array gets smaller:
	char* name0 = warray_stealFirst( names );
	char* name1 = warray_stealAt( names, 2 );
	char* name2 = warray_stealLast( names );
	//...Do some stuff with the strings, then free them yourself.
	free( name0 );
	free( name1 );
	free( name2 );
	\endcode

	And of course you can simply remove them without even looking at them:

	\code
	warray_removeFirst( array );
	warray_removeAt( array, 3 );
	warray_removeLast( array );
	\endcode
	
	- warray_at()
	- warray_first()
	- warray_last()
	- warray_sample()
	- warray_cloneAt()
	- warray_cloneFirst()
	- warray_cloneLast()
	- warray_stealAt()
	- warray_stealFirst()
	- warray_stealLast()
	- warray_removeAt()
	- warray_removeFirst()
	- warray_removeLast()
	- warray_slice()


	@section manipulate Manipulating array elements

	- warray_reverse()
	- warray_shuffle()
	- warray_compact()
	- warray_sort()
	- warray_sortBy()
	- warray_distinct()


	@section iterate Iterating over array elements

	- warray_foreach()
	- warray_foreachIndex()
	- warray_filter()
	- warray_reject()
	- warray_select()
	- warray_unselect()
	- warray_map()
	- warray_reduce()


	@section search Searching the array

	- warray_min()
	- warray_max()
	- warray_index()
	- warray_rindex()
	- warray_bsearch()
	- warray_contains()
	- warray_count()


	@section convert Converting an array to and from a string

	- warray_toString()
	- warray_fromString()


	@section check Checking properties of the array and elements

	- warray_size()
	- warray_empty()
	- warray_nonEmpty()
	- warray_all()
	- warray_any()
	- warray_none()
	- warray_one()


	@section misc Miscellaneous

	- warray_compare()
	- warray_equal()

*/

