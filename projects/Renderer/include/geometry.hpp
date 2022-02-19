#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <random>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "FastNoiseLite.h"

/*
    noiseSet: Computes noise for some xy point.
    terrainGenerator: Generates a terrain buffer using a noiseSet object.
*/


/**
*   @brief Computes the coherent noise (FBM) value for specific 2D coordinates (x,y). Range = [0, maxHeight]
*   Parameters used:
*   <ul>
*    <li>Noise type</li>
*    <li>Number of octaves</li>
*    <li>Lacunarity</li>
*    <li>Persistance</li>
*    <li>Scale</li>
*    <li>Multiplier</li>
*    <li>Offsets (one for X, other for Y)</li>
*    <li>Seed (for adding random values to the offsets)</li>
*   </ul>
*/
class noiseSet
{
    FastNoiseLite noise;

    FastNoiseLite::NoiseType noiseType;
    unsigned int numOctaves;
    float lacunarity;
    float persistance;
    float scale;
    float multiplier;
    unsigned curveDegree;
    float offsetX, offsetY;
    unsigned int seed;
    float (*octaveOffsets)[2];

    float maxHeight;

public:
    /*  @brief Constructor. Configure your noise generator:
    *   @param NumOctaves Number of octaves.
    *   @param Lacunarity Range [1, inf]. Determines the frequency.
    *   @param Persistance Range [0, 1]. Determines the amplitude.
    *   @param Scale Scaling magnitude.
    *   @param Multiplier The resulting noise value is multiplied by this.
    *   @param curveDegree Degree of the monomial (makes height difference progressive)
    *   @param OffsetX Offset for the X coordinate.
    *   @param OffsetY Offset for the Y coordinate.
    *   @param NoiseType Noise type (example: FastnoiseLite::NoiseType_Perlin). Options: _OpenSimplex2, _OpenSimplex2S, _Cellular, _Perlin, _ValueCubic, _Value.
    *   @param addRandomOffset Set to true if you want to add different random values (determined by seed) to each coordinate (x, y) in each octave.
    *   @param Seed Determines the random value added to the offsets. Used only if addRandomOffset is true.
    */
    noiseSet(unsigned int NumOctaves            = 5,
             float Lacunarity                   = 1.5,
             float Persistance                  = 0.28f,
             float Scale                        = 1,
             float Multiplier                   = 150,
             unsigned CurveDegree               = 2,
             float OffsetX                      = 0,
             float OffsetY                      = 0,
             FastNoiseLite::NoiseType NoiseType = FastNoiseLite::NoiseType_Perlin,
             bool addRandomOffset               = false,
             unsigned int Seed                  = 0);
    ~noiseSet();                                                                ///< Destructor
    noiseSet(const noiseSet& obj);                                              ///< Copy constructor
    noiseSet& operator = (const noiseSet& obj);                                 ///< Operator =  overloading (copy assignment)
    bool operator != (const noiseSet& obj);                                     ///< Operator != overloading
    friend std::ostream& operator << (std::ostream& os, const noiseSet& obj);   ///< Operator << overloading

    /*
    *   @brief Given xy coordinates, get the noise value
    *   @param x X coordinate of noise
    *   @param y Y coordinate of noise
    *   @return Noise value
    */
    float GetNoise(float x, float y);

    float           getMaxHeight() const;   ///< Get the maximum value that this noise can get. Noise range: [0, maxHeight]

    unsigned        getNoiseType() const;   ///< Get noise type
    unsigned        getNumOctaves() const;  ///< Get number of octaves
    float           getLacunarity() const;  ///< Get lacunarity
    float           getPersistance() const; ///< Get persistance
    float           getScale() const;       ///< Get scale
    float           getMultiplier() const;  ///< Get multiplier
    float           getCurveDegree() const; ///< Get the degree of the monomial used
    float           getOffsetX() const;     ///< Get the X offset
    float           getOffsetY() const;     ///< Get the Y offset
    unsigned int    getSeed() const;        ///< Get the seed
    float*          getOffsets() const;     ///< Get an array with the offsets for each x and y coordinate of each octave

    /*
     *  @brief Used for testing purposes. Checks the noise values for a size x size terrain and outputs the absolute maximum and minimum
     *  @param size Size of one side of the square that will be tested
     */
    void noiseTester(size_t size);
};

std::ostream& operator << (std::ostream& os, const noiseSet& obj);      ///< Operator << overloading for class noiseSet

// -----------------------------------------------------------------------------------

/// Given a noiseSet object, and the xy dimensions, generates a terrain buffer
class terrainGenerator
{
    size_t    getPos(size_t x, size_t y) const;
    glm::vec3 getVertex(size_t position) const;
    void      computeGridNormals(float (*vertex)[8], unsigned numVertexX, unsigned numVertexY, float stride, noiseSet &noise);

    unsigned numVertexX;
    unsigned numVertexY;
    unsigned numVertex;         // example: a square has 4 vertex
    unsigned numIndices;        // example: a square has 6 indices

public:
    terrainGenerator();                                         ///< Default constructor
    ~terrainGenerator();                                        ///< Destructor
    terrainGenerator& operator = (const terrainGenerator& obj); ///< Operator =  overloading (copy assignment)

    float        (*vertex)[8];      ///< VBO (vertex position, texture coordinates, normals)
    std::vector<uint32_t> indices;  ///< EBO
    //unsigned int (*indices)[3];     ///< EBO

    /*
    *   @brief Compute VBO and EBO (creates some terrain specified by the user)
    *   @param noise Noise generator
    *   @param x0 Coordinate X of the first square's corner
    *   @param y0 Coordinate Y of the first square's corner
    *   @param stride Separation between vertex
    *   @param numVertex_X Number of vertex along the X axis
    *   @param numVertex_Y Number of vertex along the Y axis
    *   @param textureFactor How much of the texture surface will fit in a square of 4 contiguous vertex
    */
    void computeTerrain(noiseSet &noise, float x0, float y0, float stride, unsigned numVertexX, unsigned numVertexY, float textureFactor = 1.f);

    unsigned getXside() const;      ///< Get number of vertex along X axis
    unsigned getYside() const;      ///< Get number of vertex along Y axis
    unsigned getNumVertex() const;  ///< Amount of vertex in VBO (example: two triangles = 4)
    unsigned getNumIndices() const; ///< Amount of indices in the EBO (example: two triangles = 2*3)
};

std::ostream& operator << (std::ostream& os, const terrainGenerator& obj);
std::istream& operator >> (std::istream& is, terrainGenerator& obj);

// -----------------------------------------------------------------------------------

/*
*   @brief Makes a vertex buffer containing the vertex and colors for a 3D axis system in the origin
*   @param array[12][3] Pointer to the array where data will be stored (float array[12][3])
*   @param sizeOfAxis Desired length for each axis
*/
void fillAxis(float array[6][6], float sizeOfAxis);

/*
*   @brief Makes a vertex buffer containing the vertex and color for an horizontal sea surface
*   @param array[12][3] Pointer to the array where data will be stored (float array[12][7])
*   @param height Sea level
*   @param transparency Alpha channel (range: [0, 1])
*   @param x0 Origin X coordinate of the square surface
*   @param y0 Origin Y coordinate of the square surface
*   @param x1 Ending X coordinate of the square surface
*   @param y1 Ending Y coordinate of the square surface
*/
void fillSea(float array[6][10], float height, float transparency, float x0, float y0, float x1, float y1);

/*
*   @brief Makes a vertex buffer containing a sphere generated out of a cube, with a specified color
*   @param array Pointer to the array where data will be stored ( float array[6][numVertexPerSide * numVertexPerSide][6] )
*   @param indices Pointer to the array where indices will be stored ( unsigned indices[6][(numVertexPerSide-1)*(numVertexPerSide-1)*2][3] )
*   @param radius Sphere's radius
*   @param numVertexPerSide Number of vertex per side in each square side of the cube
*   @param R Red value
*   @param G Green value
*   @param B Blue value
*/
void fillCubicSphere(float*** array, unsigned*** indices, float radius, unsigned numVertexPerSide, float R, float G, float B);



#endif
