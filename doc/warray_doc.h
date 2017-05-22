/**	@file
	The Workhorse Array documentation main page

	@mainpage Workhorse Array

	A dynamic growing array of typed void* elements supporting many common operations and automatic
	management of the elements' memory.

	@section Content

	- \ref features
	- \ref hello_world
	- \ref function_overview
		- \ref create_delete
		- \ref putting
		- \ref getting
		- \ref basic_data
		- \ref manipulating
		- \ref iterating
		- \ref checking
		- \ref searching
		- \ref converting
		- \ref misc
	- \ref namespace_api
	- \ref auto_destructor


	@section features Features

	- Has many functions to add elements, e.g. warray_prepend(), warray_append(), warray_insert(),
	  warray_set().
	- Get elements with functions like warray_at(), warray_stealFirst() or warray_cloneLast().
	- Iterate through all elements with warray_filter(), warray_map() and warray_reduce().
	- Many manipulating functions like warray_sort(), warray_reverse() or warray_compact()
	- Serialize arrays with warray_toString() and warray_fromString().
	- ... and many more functions.
	- The array takes full ownership of the elements and automatically manages their memory. No
	  manual copying or deleting necessary.
	- Supports sparse arrays with the gaps being filled with NULL.


	@section hello_world Hello world

	\code
	#include "warray.h"

	void foo() {
		//Create an array with an initial capacity of 10 elements holding elements of the type
		wtypeStr (char* strings).
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


	@section function_overview Function overview

	@subsection create_delete Creating and deleting arrays

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


	@subsection putting Putting elements in the array

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


	@subsection getting Getting elements from the array

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


	@subsection basic_data Getting basic array data

	- warray_size()
	- warray_empty()
	- warray_nonEmpty()


	@subsection manipulating Manipulating array elements

	- warray_reverse()
	- warray_shuffle()
	- warray_compact()
	- warray_sort()
	- warray_sortBy()
	- warray_distinct()


	@subsection iterating Iterating over array elements

	There are several functions to operate on all elements at once. They all have in common
	to take a callback function as argument which is executed for each element in the array.

	Most basic is the warray_foreach() function. It allows you to read each element and do
	something with it:

	\code
	//Define a simple callback function printing one element. We could optionally pass an
	//additional data argument here. But we don't use it in the example.

	void printAnimals( const void* element, const void* data ) {
		const char* animal = element;
		puts( animal );
	}

	//...
	//Create an array and add some strings to it.

	WArray* animals = warray_new( 0, wtypeStr );
	warray_append_n( animals, 3, (void*[]){ "cat", "dog", "elephant" });

	//Call printAnimals() for each element. Pass NULL for the unused data argument.
	//Prints "cat", "dog" and "elephant", each in a separate line.

	warray_foreach( animals, printAnimals, NULL );
	\endcode

	Then there are functions to remove array elements based on a condition. The warray_filter()
	and warray_reject() functions create new arrays with the elements omitted that meet resp.
	don't meet a condition:

	\code
	//Define a callback function checking if the string element is longer than a
	//given length.

	bool animalLongerThan( const void* element, const void* data ) {
		const char* animal = element;
		size_t length = (size_t)data;
		return strlen( animal ) > length;
	}

	//...

	//Create an array and add some strings to it.

	WArray* animals = warray_new( 0, wtypeStr );
	warray_append_n( animals, 3, (void*[]){ "cat", "dog", "elephant" });

	//Create two new arrays: One with animals longer than 3, one with animals shorter than or
	//equal to 3. The elements are copied according to the element type's clone() method.
	//longAnimals = ["elephant"] and shortAnimals = ["cat", "dog"].

	WArray* longAnimals = warray_filter( animals, animalLongerThan, (void*)3 );
	WArray* shortAnimals = warray_reject( animals, animalLongerThan, (void*)3 );

	//...Do something with the new animal arrays.

	//Then delete them all.

	warray_delete( &animals );
	warray_delete( &longAnimals );
	warray_delete( &shortAnimals );
	\endcode

	The two functions warray_select() and warray_unselect() basically provide the same functionality. But
	they remove the elements in place instead of creating a new array.

	With warray_map() you can transform a given array with a callback function to a new array
	with the same size and warray_reduce() lets you create a single result value from an array.

	- warray_foreach()
	- warray_foreachIndex()
	- warray_filter()
	- warray_reject()
	- warray_select()
	- warray_unselect()
	- warray_map()
	- warray_reduce()


	@subsection checking Checking properties of the array elements

	With the following functions you can check how many of the elements meet a given condition:

	\code
	//Define a callback function checking if the string element is longer than a
	//given length.

	bool animalLongerThan( const void* element, const void* data ) {
		const char* animal = element;
		size_t length = (size_t)data;
		return strlen( animal ) > length;
	}

	//Create an array and add some strings to it.

	WArray* animals = warray_new( 0, wtypeStr );
	warray_append_n( animals, 3, (void*[]){ "cat", "dog", "elephant" });

	//Check if the array elements meet a condition.

	assert( warray_count( animals, animalLongerThan, (void*)2 ) == 3 );
	assert( warray_all( animals, animalLongerThan, (void*)2 ) == true );
	assert( warray_any( animals, animalLongerThan, (void*)7 ) == true );
	assert( warray_none( animals, animalLongerThan, (void*)100 ) == true );
	assert( warray_one( animals, animalLongerThan, (void*)7 ) == true );
	\endcode

	- warray_count()
	- warray_all()
	- warray_any()
	- warray_none()
	- warray_one()


	@subsection searching Searching the array

	- warray_min()
	- warray_max()
	- warray_index()
	- warray_rindex()
	- warray_bsearch()
	- warray_contains()


	@subsection converting Converting an array to and from a string

	- warray_toString()
	- warray_fromString()


	@subsection misc Miscellaneous

	- warray_compare()
	- warray_equal()


	@section namespace_api Namespace API

	If you include "warray_sugar.h" instead of "warray.h" you can define a namespace for workhorse
	arrays and use shorter function calls:

	\code
	//Include this to get the Namespace capability.
	#include "warray_sugar.h"

	//Define the namespace with a variable name of your liking. Internally a struct of function
	//pointers is set to the warray function calls here.
	WArrayNamespace a = warrayNamespace;

	int main() {
		WArray* array = a.new( 0, wtypeStr );

		//Equivalent to warray_append()
		a.append( array, "cat" );

        //You can mix namespace calls and conventional calls.
		warray_append( array, "dog" );
		a.append( array, "mouse" );

		a.toString( array, ", " );

        a.delete( &array );
	}
	\endcode


	\section auto_destructor Automatic destructor
	If you include "warray_sugar.h" instead of "warray.h" you can define a workhorse array
	so, that it gets automatically deleted when leaving scope. Thus you can avoid memory
	leaks or dangling pointers much easier:

	\code
	void foo()
	{
		//Define a normal array.
		WArray* array1 = warray_new( 0, wtypeStr );
		//Define an auto destructive array.
		autoWArray* array2 = warray_new( 0, wtypeStr );

		//Do some stuff with the arrays.
		warray_append( array1, "cat" );
		warray_append( array2, "dog" );
		//...

		//Delete array1 manually.
		warray_delete( &array1 );

		//array2 gets automatically deleted here when leaving scope.
	}
	\endcode

	The autoWArray feature needs GCC's non-standard cleanup() attribute to work. So if you
	want to stay portable, you'd better do not use it. Clang on the other hand should
	support it as well, so you might reach a broad base of platforms with it anyway.


*/

