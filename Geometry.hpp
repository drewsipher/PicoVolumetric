
#ifndef Geometry_hpp
#define Geometry_hpp

#include <stdio.h>
#include <math.h>
#include <vector>
#include <cstdlib>



class Vector3D {
public:
    float x;
    float y;
    float z;

    Vector3D(float xPos, float yPos, float zPos) : x(xPos), y(yPos), z(zPos) {}
    
};

class Geometry {

public:
    std::vector<Vector3D> points;
    std::vector<int> indices;

    Geometry(float zScale);
    void Translate(float dx, float dy, float dz);
    void SetAbsoluteRotation(float radians, float u, float v, float w);
    
    Vector3D GetScreenSpaceVectorAt(int index);

    void ComputeModelMatrix();

private:
    Vector3D MultiplyMatrix(float matrix[4][4], Vector3D &point);
    
    
};

#endif /* Geometry_hpp */