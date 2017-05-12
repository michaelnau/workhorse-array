/**	@file
	Macro and non-portable extensions to WArray.h:
	- autoWArray
	- ArrayNamespace
*/
#ifndef WARRAY_SUGAR_H_INCLUDED
#define WARRAY_SUGAR_H_INCLUDED

/* Copyright (c) 2017 Michael Nau
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "warray.h"

//------------------------------------------------------------
//	Type declaration with auto destruction
//------------------------------------------------------------

/**	When declared as autoArray, an array gets automatically destroyed when
	leaving scope.
*/
#define autoWArray __attribute__(( cleanup( warray_delete ))) WArray

//------------------------------------------------------------
//	Array namespace
//------------------------------------------------------------

/**	Structure containing pointers to all Array functions. Can be used to simulate
	a module namespace.

	Example:
	\code
    ArrayNamespace a = arrayNamespace;

    ...

    Array* array = warray_new( 15, elementStr );
    a.append( array, "cat" );
    a.prepend( array, "dog" );
    printf( "%s", a.toString( array ));		//->"dog", "cat"
	\endcode
*/
typedef struct WArrayNamespace {
	WArray* 	(*new)		(size_t capacity, const WType* type);
    WArray* 	(*clone)	(const WArray* array);
	void		(*delete)	(WArray** array);
	WArray*		(*clear)	(WArray* array);

	WArray* 	(*append)	(WArray* array, const void* element);
	WArray* 	(*prepend)	(WArray* array, const void* element);
	WArray* 	(*set)		(WArray* array, size_t, const void* element);
	WArray* 	(*insert)	(WArray* array, size_t, const void* element);
	WArray*		(*insertSorted)(WArray* array, const void* element);
	WArray*		(*addToSet)	(WArray* array, const void* element);

	WArray*		(*append_n)	(WArray* array, size_t n, void* const elements[n]);
	WArray*		(*prepend_n)(WArray* array, size_t n, void* const elements[n]);
	WArray*		(*set_n)	(WArray* array, size_t position, size_t n, void* const elements[n]);
	WArray*		(*insert_n)	(WArray* array, size_t position, size_t n, void* const elements[n]);

	const void* (*at)		(const WArray* array, size_t position);
	const void* (*first)	(const WArray* array);
	const void* (*last)		(const WArray* array);

	void* 		(*stealAt)	(WArray* array, size_t position);
	void* 		(*stealFirst)(WArray* array);
	void* 		(*stealLast)(WArray* array);
	void* 		(*stealSample)(WArray* array);

	WArray*		(*removeAt)	(WArray* array, size_t position);
	WArray*		(*removeFirst)(WArray* array);
	WArray*		(*removeLast)(WArray* array);

	size_t		(*size)		(const WArray* array);
	bool		(*empty)	(const WArray* array);
	bool		(*nonEmpty)	(const WArray* array);

	bool		(*all)		(const WArray* array, WElementCondition* condition, const void* conditionData);
	bool		(*any)		(const WArray* array, WElementCondition* condition, const void* conditionData);
	bool		(*none)		(const WArray* array, WElementCondition* condition, const void* conditionData);
	bool		(*one)		(const WArray* array, WElementCondition* condition, const void* conditionData);

	WArray* 	(*filter)	(const WArray* array, WElementCondition* filter, const void* filterData );
	WArray* 	(*reject)	(const WArray* array, WElementCondition* filter, const void* filterData );
	WArray* 	(*map)		(const WArray* array, WElementMap*, const void*, const WType* type );
	void*		(*reduce)	(const WArray* array, WElementReduce*, const void*, const WType* type );

	void		(*foreach)	(const WArray* array, WElementForeach* foreach, void* foreachData);
	void		(*foreachIndex)(const WArray* array, WElementForeachIndex* foreach, void* foreachData);

//TODO: a.min() and a.max() disturbed by min() and max() macros
//	const void*	(*min)		(const Array*);
//	const void*	(*max)		(const Array*);
	ssize_t		(*index)	(const WArray* array, const void* element);
	ssize_t		(*rindex)	(const WArray* array, const void* element);
	ssize_t		(*bsearch)	(const WArray* array, WElementCompare* compare, const void* key);
	bool		(*contains)	(const WArray* array, const void* element);
	size_t		(*count)	(const WArray* array, WElementCondition*, const void* conditionData);

	WArray*		(*reverse)	(WArray* array);
	WArray*		(*compact)	(WArray* array);
	WArray*		(*sort)		(WArray* array);
	WArray*		(*sortBy)	(WArray* array, WElementCompare* compare);
	WArray*		(*distinct)	(WArray* array);
	WArray*		(*shuffle)	(WArray* array);

	char*		(*toString)	(const WArray* array, const char delimiters[]);
	WArray*		(*fromString)(const char string[], const char delimiters[]);
	int			(*compare) 	(const WArray* array1, const WArray* array2);
	bool		(*equal)	(const WArray* array1, const WArray* array2);
}WArrayNamespace;

/**	Predefined value for ArrayNamespace variables
*/
#define warrayNamespace					\
{										\
	.new = warray_new,					\
	.clone = warray_clone,				\
	.delete = warray_delete,			\
	.clear = warray_clear,				\
\
	.append = warray_append,			\
	.prepend = warray_prepend,			\
	.set = warray_set,					\
	.insert = warray_insert,			\
	.insertSorted = warray_insertSorted,\
\
	.append_n = warray_append_n,		\
	.prepend_n = warray_prepend_n,		\
	.set_n = warray_set_n,				\
	.insert_n = warray_insert_n,		\
\
	.at = warray_at,					\
	.first = warray_first,				\
	.last = warray_last,				\
\
	.stealAt = warray_stealAt,			\
	.stealFirst = warray_stealFirst,	\
	.stealLast = warray_stealLast,		\
	.stealSample = warray_stealSample,	\
\
	.removeAt = warray_removeAt,		\
	.removeFirst = warray_removeFirst,	\
	.removeLast = warray_removeLast,	\
\
	.size = warray_size,				\
	.empty = warray_empty,				\
	.nonEmpty = warray_nonEmpty,		\
\
	.all = warray_all,					\
	.any = warray_any,					\
	.none = warray_none,				\
	.one = warray_one,					\
\
	.filter = warray_filter,			\
	.reject = warray_reject,			\
	.map = warray_map,					\
	.reduce = warray_reduce,			\
\
	.foreach = warray_foreach,			\
	.foreachIndex = warray_foreachIndex,\
\
	.index = warray_index,				\
	.rindex = warray_rindex,			\
	.bsearch = warray_bsearch,			\
	.contains = warray_contains,		\
	.count = warray_count,				\
\
	.reverse = warray_reverse,			\
	.compact = warray_compact,			\
	.sort = warray_sort,				\
	.sortBy = warray_sortBy,			\
	.distinct = warray_distinct,		\
	.shuffle = warray_shuffle,			\
\
	.toString = warray_toString,		\
	.fromString = warray_fromString,	\
	.compare = warray_compare,			\
	.equal = warray_equal,				\
}

//------------------------------------------------------------

#endif // WARRAY_SUGAR_H_INCLUDED
