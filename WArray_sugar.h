#ifndef ARRAY_SUGAR_H_INCLUDED
#define ARRAY_SUGAR_H_INCLUDED

#include "WArray.h"

//------------------------------------------------------------

#define warray_append( array, ... )											\
({																			\
	void* __elements[] = { __VA_ARGS__ };									\
	warray_append_n( array, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define warray_prepend( array, ... )											\
({																			\
	void* __elements[] = { __VA_ARGS__ };									\
	warray_prepend_n( array, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define warray_set( array, position, ... )											\
({																					\
	void* __elements[] = { __VA_ARGS__ };											\
	warray_set_n( array, position, sizeof(__elements)/sizeof(void*), __elements );	\
})

#define warray_insert( array, position, ... )											\
({																						\
	void* __elements[] = { __VA_ARGS__ };												\
	warray_insert_n( array, position, sizeof(__elements)/sizeof(void*), __elements );	\
})

//------------------------------------------------------------

#endif // ARRAY_SUGAR_H_INCLUDED
