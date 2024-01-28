
#include <iostream>

#include "ubo.hpp"


Sizes size;


// (Set of) Uniform Buffer Objects -----------------------------------------------------------------

/// Constructor. Computes sizes (range, totalBytes) and allocates buffers (ubo, offsets).
UBO::UBO(VulkanEnvironment* e, UBOinfo uboInfo)
	:e(e),
	maxNumDescriptors(uboInfo.maxNumDescriptors),
	descriptorSize(uboInfo.minDescriptorSize ? e->c.deviceData.minUniformBufferOffsetAlignment * (1 + uboInfo.minDescriptorSize / e->c.deviceData.minUniformBufferOffsetAlignment) : 0),
	totalBytes(descriptorSize* maxNumDescriptors),
	ubo(totalBytes)
{
	setNumActiveDescriptors(uboInfo.numActiveDescriptors);
}

uint8_t* UBO::getDescriptorPtr(size_t descriptorIndex) { return ubo.data() + descriptorIndex * descriptorSize; }

bool UBO::setNumActiveDescriptors(size_t count)
{
	if (count > maxNumDescriptors)
	{
		numActiveDescriptors = maxNumDescriptors;
		return false;
	}

	numActiveDescriptors = count;
	return true;
}

// (21)
void UBO::createUBObuffers()
{
	uboBuffers.resize(e->swapChain.images.size());
	uboMemories.resize(e->swapChain.images.size());
	
	//destroyUniformBuffers();		// Not required since Renderer calls this first

	if (descriptorSize)
		for (size_t i = 0; i < e->swapChain.images.size(); i++)
			createBuffer(
				e,
				maxNumDescriptors == 0 ? descriptorSize : totalBytes,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uboBuffers[i],
				uboMemories[i]);
}

void UBO::destroyUBOs()
{
	if (descriptorSize)
	{
		for (size_t i = 0; i < e->swapChain.images.size(); i++)
		{
			vkDestroyBuffer(e->c.device, uboBuffers[i], nullptr);
			vkFreeMemory(e->c.device, uboMemories[i], nullptr);
			e->c.memAllocObjects--;
		}
	}
}

Material::Material(glm::vec3& diffuse, glm::vec3& specular, float shininess)
	: diffuse(diffuse), specular(specular), shininess(shininess) { }


// LightSet -------------------------------------------------------------

LightSet::LightSet(unsigned numLights)
	: numLights(numLights), posDirBytes(numLights * sizeof(LightPosDir)), propsBytes(numLights * sizeof(LightProps))
{
	this->posDir = new LightPosDir[numLights];
	this->props = new LightProps[numLights];

	for (size_t i = 0; i < numLights; i++)
		props[i].type = 0;

	//if (numLights < 0) numLights = 0;
}

LightSet::~LightSet()
{
	delete[] posDir;
	delete[] props;
}

void LightSet::turnOff(size_t index) { props[index].type = 0; }

void LightSet::setDirectional(size_t index, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
	props[index].type = 1;
	posDir[index].direction = direction;

	props[index].ambient = ambient;
	props[index].diffuse = diffuse;
	props[index].specular = specular;
}

void LightSet::setPoint(size_t index, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
{
	posDir[index].position = position;

	props[index].type = 2;

	props[index].ambient = ambient;
	props[index].diffuse = diffuse;
	props[index].specular = specular;

	props[index].degree.x = constant;
	props[index].degree.y = linear;
	props[index].degree.z = quadratic;
}

void LightSet::setSpot(size_t index, glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff)
{
	posDir[index].position = position;
	posDir[index].direction = direction;

	props[index].type = 3;

	props[index].ambient = ambient;
	props[index].diffuse = diffuse;
	props[index].specular = specular;

	props[index].degree.x = constant;
	props[index].degree.y = linear;
	props[index].degree.z = quadratic;

	props[index].cutOff.x = cutOff;
	props[index].cutOff.y = outerCutOff;
}


