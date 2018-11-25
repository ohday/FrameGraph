// Copyright (c) 2018,  Zhirnov Andrey. For more information see 'LICENSE'

#pragma once

#include "framegraph/Public/EResourceState.h"
#include "framegraph/Shared/ResourceDataRange.h"
#include "VBuffer.h"

namespace FG
{

	//
	// Vulkan Buffer thread local
	//

	class VLocalBuffer final
	{
		friend class VBufferUnitTest;

	// types
	public:
		using BufferRange = ResourceDataRange< VkDeviceSize >;
		
		struct BufferState
		{
		// variables
			EResourceState	state	= Default;
			BufferRange		range;
			Task			task;

		// methods
			BufferState () {}

			BufferState (EResourceState state, VkDeviceSize begin, VkDeviceSize end, Task task) :
				state{state}, range{begin, end}, task{task} {}
		};

	private:
		struct BufferAccess
		{
		// variables
			BufferRange				range;
			VkPipelineStageFlags	stages		= 0;
			VkAccessFlags			access		= 0;
			ExeOrderIndex			index		= ExeOrderIndex::Initial;
			bool					isReadable : 1;
			bool					isWritable : 1;

		// methods
			BufferAccess () : isReadable{false}, isWritable{false} {}
		};

		using AccessRecords_t	= Array< BufferAccess >;	// TODO: fixed size array or custom allocator
		using AccessIter_t		= AccessRecords_t::iterator;


	// variables
	private:
		VBuffer const*				_bufferData		= null;		// readonly access is thread safe

		mutable AccessRecords_t		_pendingAccesses;
		mutable AccessRecords_t		_accessForWrite;
		mutable AccessRecords_t		_accessForRead;
		mutable bool				_isFirstBarrier	= false;
		
		RWRaceConditionCheck		_rcCheck;


	// methods
	public:
		VLocalBuffer () {}
		VLocalBuffer (VLocalBuffer &&) = delete;
		~VLocalBuffer ();

		bool Create (const VBuffer *);
		void Destroy (OUT AppendableVkResources_t, OUT AppendableResourceIDs_t);

		void AddPendingState (const BufferState &state) const;
		void ResetState (ExeOrderIndex index, VBarrierManager &barrierMngr, VFrameGraphDebugger *debugger) const;
		void CommitBarrier (VBarrierManager &barrierMngr, VFrameGraphDebugger *debugger) const;

		ND_ bool				IsCreated ()	const	{ SHAREDLOCK( _rcCheck );  return _bufferData != null; }
		ND_ VkBuffer			Handle ()		const	{ SHAREDLOCK( _rcCheck );  return _bufferData->Handle(); }
		ND_ VBuffer const*		ToGlobal ()		const	{ SHAREDLOCK( _rcCheck );  return _bufferData; }

		ND_ BufferDesc const&	Description ()	const	{ SHAREDLOCK( _rcCheck );  return _bufferData->Description(); }
		ND_ BytesU				Size ()			const	{ SHAREDLOCK( _rcCheck );  return Description().size; }


	private:
		ND_ static AccessIter_t	_FindFirstAccess (AccessRecords_t &arr, const BufferRange &range);
			static void			_ReplaceAccessRecords (INOUT AccessRecords_t &arr, AccessIter_t iter, const BufferAccess &barrier);
			static AccessIter_t	_EraseAccessRecords (INOUT AccessRecords_t &arr, AccessIter_t iter, const BufferRange &range);
	};


}	// FG