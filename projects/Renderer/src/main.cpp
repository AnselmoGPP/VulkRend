/*
	Renderer	< VulkanEnvironment
				< ModelData		< VulkanEnvironment
								< modelConfig	< VulkanEnvironment
												< getModelMatrix callbacks
								< Input			< Camera

	set of modelConfig (callbacks + paths) > Renderer > set of ModelData
*/

/*
	TODO:
		- Axis
		> Sun billboard (transparencies)
		> Terrain
		Make the renderer a static library
		Add ProcessInput() maybe
		Dynamic states (graphics pipeline)
		Push constants
		Deferred rendering (https://gamedevelopment.tutsplus.com/articles/forward-rendering-vs-deferred-rendering--gamedev-12342)
		Move createDescriptorSetLayout() to Environment
		Why is FPS limited by default?

		UBO of each renders should be stored in a vector-like structure, so there are UBO available for new renders (generated with setRender())
		Destroy Vulkan buffers (UBO) outside semaphores

	Rendering:
		- Vertex struct has pos, color, text coord. Different vertex structs are required.
		- Points, lines, triangles
		2D graphics
		Transparencies
		Draw in front of some rendering (used for weapons)
		Shading stuff (lights, diffuse, ...)
		Make classes more secure (hide sensitive variables)
		Parallel loading (many threads)
		- When passing vertex data directly, should I copy it or pass by reference?
		> VkDrawIndex instanceCount -> check this way of multiple renderings
		> In a single draw, draw skybox from one mesh and many textures.
		> Generalize VertexPCT in loadModel()
		> Check that different operations work (add/remove renders, add/erase model, 0 renders, ... do it with different primitives)
	
		- Allow to update MM immediately after addModel() or addRender()
		- Only dynamic UBOs
		- Start thread since run() (objectAlreadyConstructed)
		- Improve modelData object destruction (call stuff from destructor, and take code out from Renderer)
		Can we take stuff out from thread 2?
		Optimization: Parallel commandBuffer creation (2 or more commandBuffers exist)
		model&commandBuffer mutex, think about it
		Usar numMM o MM.size()?
		Profiling
		Skybox borders (could be fixed with VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT)

	BUGS:
		Sometimes camera continue moving backwards/left indefinetely

	Model & Data system:
		Each ModelData could have: Vertices, Color buffers, textures, texture coords, Indices, UBO class, shaders, vertex struct
		Unique elements (always): Vertices, indices, shaders
		Unique elements (sometimes): Color buffer, texture coords,
		Shared elements (sometimes): UBO class, Textures, vertex struct(Vertices, color, textCoords)
*/

/*
	Render same model with different descriptors
		- You technically don't have multiple uniform buffers; you just have one. But you can use the offset(s) provided to vkCmdBindDescriptorSets
		to shift where in that buffer the next rendering command(s) will get their data from. Basically, you rebind your descriptor sets, but with
		different pDynamicOffset array values.
		- Your pipeline layout has to explicitly declare those descriptors as being dynamic descriptors. And every time you bind the set, you'll need
		to provide the offset into the buffer used by that descriptor.
		- More: https://stackoverflow.com/questions/45425603/vulkan-is-there-a-way-to-draw-multiple-objects-in-different-locations-like-in-d
*/

#include <iostream>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE
#include <iomanip>
#include <map>

#include "renderer.hpp"
#include "toolkit.hpp"
#include "data.hpp"

void update(Renderer& r);
std::map<std::string, modelIterator> assets;
int gridStep = 10;

void setPoints	(Renderer& app);
void setAxis	(Renderer& app);
void setGrid	(Renderer& app);
void setSkybox	(Renderer& app);
void setCottage	(Renderer& app);
void setRoom	(Renderer& app);
void setFloor	(Renderer& app);

bool roomVisible = false;
bool cottageLoaded = false;
bool check1 = false, check2 = false;


int main(int argc, char* argv[])
{
	// Create a renderer object. Pass a callback that will be called for each frame (useful for updating model view matrices).
	Renderer app(update);		std::cout << "-----------------------------------------------------------------------------" << std::endl;

	setPoints(app);
	setAxis(app);
	setGrid(app);
	setSkybox(app);
	setCottage(app);
	setRoom(app);
	//setFloor(app);

	app.run();		// Start rendering
	
	return EXIT_SUCCESS;
}


// Update model's model matrix each frame
void update(Renderer& r)
{
	long double time	= r.getTimer().getTime();
	size_t fps			= r.getTimer().getFPS();
	size_t maxfps		= r.getTimer().getMaxPossibleFPS();
	float xpos			= r.getCamera().Position.x;
	float ypos			= r.getCamera().Position.y;
	float zpos			= r.getCamera().Position.z;

	if (assets.find("grid") != assets.end())
		assets["grid"]->setUBO(0, modelMatrix(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(gridStep*((int)xpos/gridStep), gridStep*((int)ypos/gridStep), 0.0f)));

	if (assets.find("skyBoxX") != assets.end())
	{
		assets["skyBoxX"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
		assets["skyBoxY"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
		assets["skyBoxZ"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
		assets["skyBox-X"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
		assets["skyBox-Y"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
		assets["skyBox-Z"]->setUBO(0, modelMatrix(glm::vec3(2048.0f, 2048.0f, 2048.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(xpos, ypos, zpos)));
	}

	if (assets.find("cottage") != assets.end())
		assets["cottage"]->setUBO(0, modelMatrix(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(90.0f, time * 45.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)));

	//if (time > 5 && cottageLoaded)
	//{
	//	r.setRenders(assets["cottage"], 0);
	//	cottageLoaded = false;
	//}
/*
	if (time > 5 && !check1)
	{
		r.setRenders(assets["room"], 4);
		assets["room"]->setUBO(0, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, -90.0f), glm::vec3( 0.0f, -50.0f, 3.0f)));
		assets["room"]->setUBO(1, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f,   0.0f), glm::vec3( 0.0f, -80.0f, 3.0f)));
		assets["room"]->setUBO(2, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f,  90.0f), glm::vec3(30.0f, -80.0f, 3.0f)));
		assets["room"]->setUBO(3, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(30.0f, -50.0f, 3.0f)));
		check1 = true;
	}
	else if (time > 10 && !check2)
	{
		 r.deleteModel(assets["room"]);
		//r.setRenders(assets["room"], 3);
		check2 = true;
	}
	
	 if (time > 5)
	 {
		 //assets["room"]->setUBO(0, room1_MM(0));
		 //assets["room"]->setUBO(1, room2_MM(0));
		 //assets["room"]->setUBO(2, room3_MM(0));
		 //assets["room"]->setUBO(3, room4_MM(0));
		 //assets["room"]->setUBO(4, room5_MM(0));
	 }
*/		 



}


void setPoints(Renderer& app)
{
	assets["points"] = app.newModel(1,
		VertexType(sizeof(VertexPC), 1, 1, 0), 9, v_points.data(),
		nullptr,
		"",
		(SHADERS_DIR + "pointV.spv").c_str(),
		(SHADERS_DIR + "pointF.spv").c_str(),
		primitiveTopology::point);

	//assets["points"]->setUBO(0, modelMatrix());
}

void setAxis(Renderer& app)
{
	std::vector<VertexPC> v_axis;
	std::vector<uint32_t> i_axis;
	size_t numVertex = getAxis(v_axis, i_axis, 500, 0.8);

	assets["axis"] = app.newModel(1,
		VertexType(sizeof(VertexPC), 1, 1, 0), numVertex, v_axis.data(),
		&i_axis,
		"",
		(SHADERS_DIR + "lineV.spv").c_str(),
		(SHADERS_DIR + "lineF.spv").c_str(),
		primitiveTopology::line);

	//assets["axis"]->setUBO(0, modelMatrix());
}

void setGrid(Renderer& app)
{
	std::vector<VertexPC> v_grid;
	std::vector<uint32_t> i_grid;
	size_t numVertex = getGrid(v_grid, i_grid, gridStep, 100, glm::vec3(0.1, 0.1, 0.6));

	assets["grid"] = app.newModel(1,
		VertexType(sizeof(VertexPC), 1, 1, 0), numVertex, v_grid.data(),
		&i_grid,
		"",
		(SHADERS_DIR + "lineV.spv").c_str(),
		(SHADERS_DIR + "lineF.spv").c_str(),
		primitiveTopology::line);

	assets["grid"]->setUBO(0, modelMatrix());
}

void setSkybox(Renderer& app)
{
	assets["skyBoxX"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_posx.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_posx.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);

	assets["skyBoxY"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_posy.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_posy.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);

	assets["skyBoxZ"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_posz.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_posz.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);

	assets["skyBox-X"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_negx.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_negx.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);

	assets["skyBox-Y"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_negy.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_negy.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);

	assets["skyBox-Z"] = app.newModel(1,
		VertexType(sizeof(VertexPT), 1, 0, 1), 4, v_negz.data(),
		&i_square,
		(TEXTURES_DIR + "sky_box/space1_negz.jpg").c_str(),
		(SHADERS_DIR + "triangleTV.spv").c_str(),
		(SHADERS_DIR + "triangleTF.spv").c_str(),
		primitiveTopology::triangle);
}

void setCottage(Renderer& app)
{
	// Add a model to render. An iterator is returned (modelIterator). Save it for updating model data later.
	assets["cottage"] = app.newModel( 0,
		(MODELS_DIR   + "cottage_obj.obj").c_str(),
		(TEXTURES_DIR + "cottage/cottage_diffuse.png").c_str(),
		(SHADERS_DIR  + "triangleV.spv").c_str(),
		(SHADERS_DIR  + "triangleF.spv").c_str(),
		VertexType(sizeof(VertexPCT), 1, 1, 1),
		primitiveTopology::triangle);

	cottageLoaded = true;

	// Delete a model you passed previously.
	app.deleteModel(assets["cottage"]);
	cottageLoaded = false;

	// Add the same model again.
	assets["cottage"] = app.newModel( 1,
		(MODELS_DIR + "cottage_obj.obj").c_str(),
		(TEXTURES_DIR + "cottage/cottage_diffuse.png").c_str(),
		(SHADERS_DIR + "triangleV.spv").c_str(),
		(SHADERS_DIR + "triangleF.spv").c_str(),
		VertexType(sizeof(VertexPCT), 1, 1, 1),
		primitiveTopology::triangle);
}

void setRoom(Renderer& app)
{
	assets["room"] = app.newModel(2,
		(MODELS_DIR + "viking_room.obj").c_str(),
		(TEXTURES_DIR + "viking_room.png").c_str(),
		(SHADERS_DIR + "triangleV.spv").c_str(),
		(SHADERS_DIR + "triangleF.spv").c_str(),
		VertexType(sizeof(VertexPCT), 1, 1, 1),
		primitiveTopology::triangle);

	assets["room"]->setUBO(0, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, -90.0f), glm::vec3(0.0f, -50.0f, 3.0f)));
	assets["room"]->setUBO(1, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -80.0f, 3.0f)));
	//assets["room"]->setUBO(2, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f,  90.0f), glm::vec3(30.0f, -80.0f, 3.0f)));
	//assets["room"]->setUBO(3, modelMatrix(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(30.0f, -50.0f, 3.0f)));
}

void setFloor(Renderer& app)
{
	assets["floor"] = app.newModel( 1,
		VertexType(sizeof(VertexPCT), 1, 1, 1), 4, v_floor.data(),
		&i_floor,
		(TEXTURES_DIR + "grass.png").c_str(),
		(SHADERS_DIR + "triangleV.spv").c_str(),
		(SHADERS_DIR + "triangleF.spv").c_str(),
		primitiveTopology::triangle );

	//assets["floor"]->setUBO(0, modelMatrix());
}

/*
void update2(Renderer& r)
{
	long double time = r.getTimer().getTime();
	size_t fps = r.getTimer().getFPS();
	std::cout << fps << " - " << time << std::endl;

	// Update model's model matrix each frame
	if (cottageLoaded)
		assets["cottage"]->MM[0] = cottage_MM(time);

	if (time < 5)	// LOOK when setRenders(), the first frame uses default MMs
	{
		assets["room"]->MM[0] = room1_MM(time);
		assets["room"]->MM[1] = room2_MM(time);
		assets["room"]->MM[2] = room3_MM(time);
		assets["room"]->MM[3] = room4_MM(time);
	}
	else if (time > 5 && !check1)
	{
		r.setRenders(&assets["room"], 2);
		check1 = true;
	}
	else if(time < 10)
	{
		assets["room"]->MM[0] = room1_MM(time);
		assets["room"]->MM[1] = room2_MM(time);
	}
	else if (time > 10 && !check2)
	{
		r.setRenders(&assets["room"], 3);
		check2 = true;
	}
	else
	{
		assets["room"]->MM[0] = room1_MM(time);
		assets["room"]->MM[1] = room2_MM(time);
		assets["room"]->MM[2] = room3_MM(time);
	}
}

void update1(Renderer &r)
{
	long double time = r.getTimer().getTime();
	size_t fps		 = r.getTimer().getFPS();
	std::cout << fps << " - " << time << std::endl;

	// Model loaded before run(), and deleted after run()
	if (time < 5 && cottageLoaded)
		assets["cottage"]->MM[0] = cottage_MM(time);
	else if (cottageLoaded)
	{
		r.deleteModel(assets["cottage"]);
		cottageLoaded = false;
	}

	// Model loaded after run()
	if (time > 10 && !roomVisible)
	{
		assets["room"] = r.newModel(4,
			(MODELS_DIR + "viking_room.obj").c_str(),
			(TEXTURES_DIR + "viking_room.png").c_str(),
			(SHADERS_DIR + "triangleV.spv").c_str(),
			(SHADERS_DIR + "triangleF.spv").c_str());

		roomVisible = true;
	}
	else if (roomVisible)
	{
		assets["room"]->MM[0] = room1_MM(time);
		assets["room"]->MM[1] = room2_MM(time);
		assets["room"]->MM[2] = room3_MM(time);
		assets["room"]->MM[3] = room4_MM(time);
	}
}
*/