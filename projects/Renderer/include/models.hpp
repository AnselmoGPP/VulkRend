#ifndef MODELS_HPP
#define MODELS_HPP

#include <iostream>
#include <array>
#include <functional>						// std::function (function wrapper that stores a callable object)
#include <fstream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE			// GLM uses OpenGL depth range [-1.0, 1.0]. This macro forces GLM to use Vulkan range [0.0, 1.0].
#define GLM_ENABLE_EXPERIMENTAL				// Required for using std::hash functions for the GLM types (since gtx folder contains experimental extensions)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>		// Generate transformations matrices with glm::rotate (model), glm::lookAt (view), glm::perspective (projection).
#include <glm/gtx/hash.hpp>

#include "models_2.hpp"

/*
	Basic ModelData interface:

		- ModelData obj(...)		// Create basic model object
		- obj.fullConstruction()	// Load model data into the object (useful in a second thread)
		- setUBO(...)				// Used by final user for updating some UBO

		- dynamicOffsets			// For command buffer creation
		- MM						// For command buffer creation, UBO update, and setRenders
		- numMM						// For command buffer creation, UBO update, and setRenders (MM.size() and numMM may be different)
		- getUBORange()				// For updating command buffer
		- isDataFromFile()
		- numTextures()

		- resizeUBOset(...)			// For modifying the number of renders

		- cleanupSwapChain()		// For recreating swapchain (for window resize)
		- recreateSwapChain()		// For recreating swapchain (for window resize)
*/

#define LINE_WIDTH 1.0f

extern std::vector<Texture> noTextures;
extern std::vector<uint32_t> noIndices;


//template <typename Vertex>
class ModelData
{
	VulkanEnvironment& e;

	const char* modelPath;

	const char* VSpath;
	const char* FSpath;

	VkPrimitiveTopology primitiveTopology;	///< Primitive topology (VK_PRIMITIVE_TOPOLOGY_ ... POINT_LIST, LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP). Used when creating the graphics pipeline.
	//VkDeviceSize usefulUBOsize;
	bool dataFromFile;						///< Flags if vertex-color-texture_index comes from file or from code
	bool fullyConstructed;					///< Flags if this object has been fully constructed (i.e. has a model loaded)
	bool hasTransparencies;					///< Flags if textures contain transparencies (alpha channel)

	// Main methods:

	void createDescriptorSetLayout();		///< Layout for the descriptor set (descriptor: handle or pointer into a resource (buffer, sampler, texture...))
	void createGraphicsPipeline(const char* VSpath, const char* FSpath);///< Create the graphics pipeline.

	//void createTextureImage(const char* path);///< Load an image and upload it into a Vulkan object.
	//void createTextureImageView();			///< Create an image view for the texture (images are accessed through image views rather than directly).
	//void createTextureSampler();			///< Create a sampler for the textures (it applies filtering and transformations).
	void loadModel(const char* obj_file);	///< Populate the vertices and indices members with the vertex data from the mesh (OBJ file).
	void createVertexBuffer();				///< Vertex buffer creation.
	void createIndexBuffer();				///< Index buffer creation
	void createUniformBuffers();			///< Uniform buffer creation (type of descriptors that can be bound), one for each swap chain image.
	void createDescriptorPool();			///< Descriptor pool creation (a descriptor set for each VkBuffer resource to bind it to the uniform buffer descriptor).
	void createDescriptorSets();			///< Descriptor sets creation.

	void cleanup();

	// Helper methods:

	static std::vector<char>	readFile(/*const std::string& filename*/ const char* filename);	///< Read all of the bytes from the specified file and return them in a byte array managed by a std::vector.
	VkShaderModule				createShaderModule(const std::vector<char>& code);				///< Take a buffer with the bytecode as parameter and create a VkShaderModule from it.
	//void						createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);	///< Helper function for creating a buffer (VkBuffer and VkDeviceMemory).
	//void						copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	//void						generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void						copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void						copyCString(const char*& destination, const char* source);

	friend void createBuffer(VulkanEnvironment& e, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

public:
	/// Data from file 
	ModelData(VulkanEnvironment& environment, size_t numRenderings, VkPrimitiveTopology primitiveTopology, const UBOtype& vsUboType, const UBOtype& fsUboType, const char* modelPath, std::vector<Texture>& textures, const char* VSpath, const char* FSpath, VertexType vertexType, bool transparency);
	/// Data passed as argument
	ModelData(VulkanEnvironment& environment, size_t numRenderings, VkPrimitiveTopology primitiveTopology, const UBOtype& vsUboType, const UBOtype& fsUboType, const VertexType& vertexType, size_t numVertex, const void* vertexData, std::vector<uint32_t>& indices, std::vector<Texture>& textures, const char* VSpath, const char* FSpath, bool transparency);
	virtual ~ModelData();

	ModelData& fullConstruction();
	void recreateSwapChain();							///< Called by Renderer for window resizing
	void cleanupSwapChain();

	VkDescriptorSetLayout		 descriptorSetLayout;	///< Opaque handle to a descriptor set layout object (combines all of the descriptor bindings).
	VkPipelineLayout			 pipelineLayout;		///< Pipeline layout. Allows to use uniform values in shaders (globals similar to dynamic state variables that can be changed at drawing at drawing time to alter the behavior of your shaders without having to recreate them).
	VkPipeline					 graphicsPipeline;		///< Opaque handle to a pipeline object.

	std::vector<Texture>		 textures;

	VertexSet					 vertices;				///< Vertices of our model (position, color, texture coordinates, ...). This data is copied into Vulkan
	std::vector<uint32_t>		 indices;				///< Indices of our model (indices(
	VkBuffer					 vertexBuffer;			///< Opaque handle to a buffer object (here, vertex buffer).
	VkDeviceMemory				 vertexBufferMemory;	///< Opaque handle to a device memory object (here, memory for the vertex buffer).
	VkBuffer					 indexBuffer;			///< Opaque handle to a buffer object (here, index buffer).
	VkDeviceMemory				 indexBufferMemory;		///< Opaque handle to a device memory object (here, memory for the index buffer).

	UBO							 vsDynUBO;				///< Stores the set of dynamic UBOs that will be passed to the vertex shader
	UBO							 fsUBO;					///< Stores the UBO that will be passed to the fragment shader
	//std::vector<VkBuffer>		 uniformBuffers;		///< Opaque handle to a buffer object (here, uniform buffer). One for each swap chain image.
	//std::vector<VkDeviceMemory>uniformBuffersMemory;	///< Opaque handle to a device memory object (here, memory for the uniform buffer). One for each swap chain image.
	VkDescriptorPool			 descriptorPool;		///< Opaque handle to a descriptor pool object.
	std::vector<VkDescriptorSet> descriptorSets;		///< List. Opaque handle to a descriptor set object. One for each swap chain image.

	//std::vector <std::function<glm::mat4(float)>> getModelMatrix;	///< Callbacks required in loopManager::updateUniformBuffer() for each model to render.
	//glm::mat4(*getModelMatrix) (float time);

	void resizeUBOset(size_t newSize);		///< Set up dynamic offsets and number of MM (model matrices)
	void setMM(size_t posDynUbo, size_t attrib, glm::mat4& newValue);

	bool isDataFromFile();
};


#endif