// Copyright (c) 2018,  Zhirnov Andrey. For more information see 'LICENSE'

#pragma once

#include "VDescriptorSetLayout.h"
#include "VPipelineLayout.h"
#include "VGraphicsPipeline.h"
#include "VComputePipeline.h"
#include "VMeshPipeline.h"
#include "VRayTracingPipeline.h"

namespace FG
{

	//
	// Pipeline Cache
	//

	class VPipelineCache final
	{
	// types
	private:
		struct FragmentOutputHash {
			ND_ size_t operator () (const VGraphicsPipeline::FragmentOutputInstance &value) const {
				return size_t(value.GetHash());
			}
		};

		using PipelineCompilers_t		= HashSet< IPipelineCompilerPtr >;
		using VkShaderPtr				= PipelineDescription::VkShaderPtr;
		using ShaderModules_t			= Array< VkShaderPtr >;
		using FragmentOutputCache_t		= HashSet< VGraphicsPipeline::FragmentOutputInstance, FragmentOutputHash >;
		using FragmentOutputPtr			= const VGraphicsPipeline::FragmentOutputInstance *;
		using ShaderModule_t			= VGraphicsPipeline::ShaderModule;

		static constexpr uint			MaxDescriptorPoolSize	= 1024;
		static constexpr uint			MaxDescriptorSets		= 512;
		using DescriptorPoolArray_t		= Array< VkDescriptorPool >;

		template <typename T>
		struct PipelineInstancePairHash {
			ND_ size_t operator () (const Pair<T const*, typename T::PipelineInstance> &value) const {
				return size_t(HashOf(value.first) + value.second._hash);
			}
		};

		template <typename T>
		using PipelineInstanceCacheTempl = std::unordered_map< Pair<T const*, typename T::PipelineInstance>, VkPipeline,
															   PipelineInstancePairHash<T>, std::equal_to<Pair<T const*, typename T::PipelineInstance>>,
															   StdLinearAllocator<Pair< const Pair<T const*, typename T::PipelineInstance>, VkPipeline >> >;

		using CPipelineInstanceMap_t	= Storage< PipelineInstanceCacheTempl< VComputePipeline > >;
		using GPipelineInstanceMap_t	= Storage< PipelineInstanceCacheTempl< VGraphicsPipeline > >;
		using MPipelineInstanceMap_t	= Storage< PipelineInstanceCacheTempl< VMeshPipeline > >;
		using RTPipelineInstanceMap_t	= Storage< PipelineInstanceCacheTempl< VRayTracingPipeline > >;
		
		static constexpr uint	MaxStages = 8;
		using DynamicStates_t			= FixedArray< VkDynamicState, 32 >;
		using Viewports_t				= FixedArray< VkViewport, 16 >;
		using Scissors_t				= FixedArray< VkRect2D, 16 >;
		using ShaderStages_t			= FixedArray< VkPipelineShaderStageCreateInfo, MaxStages >;
		using VertexInputAttributes_t	= FixedArray< VkVertexInputAttributeDescription, FG_MaxAttribs >;
		using VertexInputBindings_t		= FixedArray< VkVertexInputBindingDescription, FG_MaxVertexBuffers >;
		using ColorAttachments_t		= FixedArray< VkPipelineColorBlendAttachmentState, FG_MaxColorBuffers >;
		using Specializations_t			= FixedArray< VkSpecializationInfo, MaxStages >;
		using SpecializationEntries_t	= FixedArray< VkSpecializationMapEntry, FG_MaxSpecConstants * MaxStages >;
		using SpecializationData_t		= FixedArray< uint, FG_MaxSpecConstants * MaxStages >;


	// variables
	private:
		VkPipelineCache				_pipelinesCache;
		FragmentOutputCache_t		_fragmentOutputCache;
		
		ShaderModules_t				_shaderCache;
		PipelineCompilers_t			_compilers;
		DescriptorPoolArray_t		_descriptorPools;
		
		// pipeline cache
		GPipelineInstanceMap_t		_graphicsPipelines;
		CPipelineInstanceMap_t		_computePipelines;
		MPipelineInstanceMap_t		_meshPipelines;
		RTPipelineInstanceMap_t		_rayTracingPipelines;

		// temporary arrays
		ShaderStages_t				_tempStages;			// TODO: use custom allocator?
		Specializations_t			_tempSpecialization;
		DynamicStates_t				_tempDynamicStates;
		Viewports_t					_tempViewports;
		Scissors_t					_tempScissors;
		VertexInputAttributes_t		_tempVertexAttribs;
		VertexInputBindings_t		_tempVertexBinding;
		ColorAttachments_t			_tempAttachments;
		SpecializationEntries_t		_tempSpecEntries;
		SpecializationData_t		_tempSpecData;


	// methods
	public:
		VPipelineCache ();
		~VPipelineCache ();
		
		bool Initialize (const VDevice &dev);
		void Deinitialize (const VDevice &dev);

		void OnBegin (LinearAllocator<> &);
		bool MergeCache (VPipelineCache &);
		void MergePipelines (OUT AppendableVkResources_t);
		
		void AddCompiler (const IPipelineCompilerPtr &comp);

		bool CompileShaders (INOUT GraphicsPipelineDesc &desc, const VDevice &dev);
		bool CompileShaders (INOUT MeshPipelineDesc &desc, const VDevice &dev);
		bool CompileShader (INOUT ComputePipelineDesc &desc, const VDevice &dev);
		bool CompileShaders (INOUT RayTracingPipelineDesc &desc, const VDevice &dev);

		bool AllocDescriptorSet (const VDevice &dev, VkDescriptorSetLayout layout, OUT VkDescriptorSet &ds);

		ND_ FragmentOutputPtr  CreateFramentOutput (ArrayView<GraphicsPipelineDesc::FragmentOutput> values);

		ND_ VkPipeline	CreatePipelineInstance (VResourceManagerThread	&resMngr,
												RawGPipelineID			 ppln,
												RawRenderPassID			 renderPass,
												uint					 subpassIndex,
												const RenderState		&renderState,
												const VertexInputState	&vertexInput,
												EPipelineDynamicState	 dynamicState,
												VkPipelineCreateFlags	 pipelineFlags,
												uint					 viewportCount);
		
		ND_ VkPipeline	CreatePipelineInstance (VResourceManagerThread	&resMngr,
												RawMPipelineID			 ppln,
												RawRenderPassID			 renderPass,
												uint					 subpassIndex,
												const RenderState		&renderState,
												EPipelineDynamicState	 dynamicState,
												VkPipelineCreateFlags	 pipelineFlags,
												uint					 viewportCount);

		ND_ VkPipeline	CreatePipelineInstance (VResourceManagerThread	&resMngr,
												RawCPipelineID			 ppln,
												const Optional<uint3>	&localGroupSize,
												VkPipelineCreateFlags	 pipelineFlags);

		ND_ VkPipeline	CreatePipelineInstance (VResourceManagerThread		&resMngr,
												RawRTPipelineID				 ppln,
												VkPipelineCreateFlags		 pipelineFlags);


	private:
		ND_ FixedArray<EShaderLangFormat,16>	_GetBuiltinFormats (const VDevice &dev) const;
		
		template <typename T>
		void _MergePipelines (INOUT Storage<PipelineInstanceCacheTempl<T>> &, OUT AppendableVkResources_t) const;

		template <typename DescT>
		bool _CompileShaders (INOUT DescT &desc, const VDevice &dev);
		bool _CompileSPIRVShader (const VDevice &dev, const PipelineDescription::ShaderDataUnion_t &shaderData, OUT VkShaderPtr &module);

		bool _CreatePipelineCache (const VDevice &dev);
		bool _CreateDescriptorPool (const VDevice &dev);

		void _ClearTemp ();

		void _SetColorBlendState (OUT VkPipelineColorBlendStateCreateInfo &outState,
								  OUT ColorAttachments_t &attachments,
								  const RenderState::ColorBuffersState &inState,
								  const VRenderPass &renderPass,
								  uint subpassIndex) const;

		void _SetShaderStages (OUT ShaderStages_t &stages,
							   INOUT Specializations_t &specialization,
							   INOUT SpecializationEntries_t &specEntries,
							   ArrayView<ShaderModule_t> shaders) const;

		void _SetDynamicState (OUT VkPipelineDynamicStateCreateInfo &outState,
							   OUT DynamicStates_t &states,
							   EPipelineDynamicState inState) const;

		void _SetMultisampleState (OUT VkPipelineMultisampleStateCreateInfo &outState,
								   const RenderState::MultisampleState &inState) const;

		void _SetTessellationState (OUT VkPipelineTessellationStateCreateInfo &outState,
									uint patchSize) const;

		void _SetDepthStencilState (OUT VkPipelineDepthStencilStateCreateInfo &outState,
									const RenderState::DepthBufferState &depth,
									const RenderState::StencilBufferState &stencil) const;

		void _SetRasterizationState (OUT VkPipelineRasterizationStateCreateInfo &outState,
									 const RenderState::RasterizationState &inState) const;

		void _SetupPipelineInputAssemblyState (OUT VkPipelineInputAssemblyStateCreateInfo &outState,
											   const RenderState::InputAssemblyState &inState) const;

		void _SetVertexInputState (OUT VkPipelineVertexInputStateCreateInfo &outState,
								   OUT VertexInputAttributes_t &vertexAttribs,
								   OUT VertexInputBindings_t &vertexBinding,
								   const VertexInputState &inState) const;

		void _SetViewportState (OUT VkPipelineViewportStateCreateInfo &outState,
								OUT Viewports_t &tmpViewports,
								OUT Scissors_t &tmpScissors,
								const uint2 &viewportSize,
								uint viewportCount,
								EPipelineDynamicState dynamicStates) const;

		void _AddLocalGroupSizeSpecialization (INOUT SpecializationEntries_t &outEntries,
											   INOUT SpecializationData_t &outEntryData,
											   const uint3 &localSizeSpec,
											   const uint3 &localGroupSize) const;

		void _ValidateRenderState (const VDevice &dev, INOUT RenderState &renderState, INOUT EPipelineDynamicState &dynamicStates) const;
	};


}	// FG