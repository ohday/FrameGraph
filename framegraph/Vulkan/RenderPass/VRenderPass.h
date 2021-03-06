// Copyright (c) 2018-2019,  Zhirnov Andrey. For more information see 'LICENSE'

#pragma once

#include "framegraph/Public/Pipeline.h"
#include "VLogicalRenderPass.h"

namespace FG
{

	//
	// Vulkan Render Pass
	//

	class VRenderPass final
	{
	// types
	private:
		static constexpr uint	maxColorAttachments	= FG_MaxColorBuffers;
		static constexpr uint	maxAttachments		= FG_MaxColorBuffers + 1;
		static constexpr uint	maxSubpasses		= FG_MaxRenderPassSubpasses;
		static constexpr uint	maxDependencies		= maxSubpasses * 2;

		using Attachments_t			= FixedArray< VkAttachmentDescription, maxAttachments >;
		using AttachmentsRef_t		= FixedArray< VkAttachmentReference, maxAttachments * maxSubpasses >;
		using AttachmentsRef2_t		= FixedArray< VkAttachmentReference, maxSubpasses >;
		using Subpasses_t			= FixedArray< VkSubpassDescription, maxSubpasses >;
		using Dependencies_t		= FixedArray< VkSubpassDependency, maxDependencies >;
		using Preserves_t			= FixedArray< uint, maxColorAttachments * maxSubpasses >;
		using SubpassesHash_t		= FixedArray< HashVal, maxSubpasses >;


	// variables
	private:
		VkRenderPass			_renderPass		= VK_NULL_HANDLE;

		HashVal					_hash;
		HashVal					_attachmentHash;
		SubpassesHash_t			_subpassesHash;
		
		VkRenderPassCreateInfo	_createInfo		= {};
		Attachments_t			_attachments;
		AttachmentsRef_t		_attachmentRef;
		AttachmentsRef_t		_inputAttachRef;
		AttachmentsRef2_t		_resolveAttachRef;
		Subpasses_t				_subpasses;
		Dependencies_t			_dependencies;
		Preserves_t				_preserves;
		
		DebugName_t				_debugName;
		
		RWDataRaceCheck			_drCheck;


	// methods
	public:
		VRenderPass () {}
		VRenderPass (VRenderPass &&) = default;
		explicit VRenderPass (ArrayView<VLogicalRenderPass*> logicalPasses);
		~VRenderPass ();

		bool Create (const VDevice &dev, StringView dbgName);
		void Destroy (VResourceManager &);

		ND_ bool operator == (const VRenderPass &rhs) const;

		ND_ VkRenderPass					Handle ()			const	{ SHAREDLOCK( _drCheck );  return _renderPass; }
		ND_ VkRenderPassCreateInfo const&	GetCreateInfo ()	const	{ SHAREDLOCK( _drCheck );  return _createInfo; }
		ND_ HashVal							GetHash ()			const	{ SHAREDLOCK( _drCheck );  return _hash; }


	private:
		bool _Initialize (ArrayView<VLogicalRenderPass*> logicalPasses);

		static void  _CalcHash (const VkRenderPassCreateInfo &ci, OUT HashVal &hash, OUT HashVal &attachmentHash,
								OUT SubpassesHash_t &subpassesHash);
	};


}	// FG


namespace std
{
	template <>
	struct hash< FG::VRenderPass > {
		ND_ size_t  operator () (const FG::VRenderPass &value) const {
			return size_t(value.GetHash());
		}
	};

}	// std
