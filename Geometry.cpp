#include "Geometry.hpp"

float rotationMatrix[4][4];
float transformationMatrix[4][4];
float modelMatrix[4][4];

float ZScale = 0.2f;


Geometry::Geometry(float zScale)
{
    ZScale = zScale;
    points.clear();
    indices.clear();
    rotationMatrix[0][0] = 1.0f;
    rotationMatrix[0][1] = 0.0f;
    rotationMatrix[0][2] = 0.0f;
    rotationMatrix[0][3] = 0.0f;
    
    rotationMatrix[1][0] = 0.0f;
    rotationMatrix[1][1] = 1.0f;
    rotationMatrix[1][2] = 0.0f;
    rotationMatrix[1][3] = 0.0f;
    
    rotationMatrix[2][0] = 0.0f;
    rotationMatrix[2][1] = 0.0f;
    rotationMatrix[2][2] = 1.0f;
    rotationMatrix[2][3] = 0.0f;

    rotationMatrix[3][0] = 0.0f;
    rotationMatrix[3][1] = 0.0f;
    rotationMatrix[3][2] = 0.0f;
    rotationMatrix[3][3] = 1.0f;
}

Vector3D Geometry::GetScreenSpaceVectorAt(int index)
{
    Vector3D p = MultiplyMatrix(modelMatrix, points[index]);
    p.z *= ZScale;
    return p;
    
}

void Geometry::Translate(float dx, float dy, float dz)
{
    transformationMatrix[0][0] = 1.0f;
    transformationMatrix[0][1] = 0.0f;
    transformationMatrix[0][2] = 0.0f;
    transformationMatrix[0][3] += dx;

    transformationMatrix[1][0] = 0.0f;
    transformationMatrix[1][1] = 1.0f;
    transformationMatrix[1][2] = 0.0f;
    transformationMatrix[1][3] += dy;

    transformationMatrix[2][0] = 0.0f;
    transformationMatrix[2][1] = 0.0f;
    transformationMatrix[2][2] = 1.0f;
    transformationMatrix[2][3] += dz;

    transformationMatrix[3][0] = 0.0f;
    transformationMatrix[3][1] = 0.0f;
    transformationMatrix[3][2] = 0.0f;
    transformationMatrix[3][3] = 1.0f;
    
}


float inputMatrix[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float outputMatrix[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 

Vector3D Geometry::MultiplyMatrix(float matrix[4][4], Vector3D &point)
{
    inputMatrix[0] = point.x;
    inputMatrix[1] = point.y;
    inputMatrix[2] = point.z;

    for(int i = 0; i < 4; i++ ){
        outputMatrix[i] = 0;
        for(int k = 0; k < 4; k++){
            outputMatrix[i] += matrix[i][k] * inputMatrix[k];
        }
    }
    return Vector3D(outputMatrix[0]/outputMatrix[3],outputMatrix[1]/outputMatrix[3],outputMatrix[2]/outputMatrix[3]);
}

void Geometry::ComputeModelMatrix()
{

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            modelMatrix[i][j] = rotationMatrix[i][j];
        }
    }

    modelMatrix[0][3] = transformationMatrix[0][3];
    modelMatrix[1][3] = transformationMatrix[1][3];
    modelMatrix[2][3] = transformationMatrix[2][3];

}

void Geometry::SetAbsoluteRotation(float radians, float u, float v, float w)
{
    float L = (u*u + v * v + w * w);
    float u2 = u * u;
    float v2 = v * v;
    float w2 = w * w; 
 
    rotationMatrix[0][0] = (u2 + (v2 + w2) * cos(radians)) / L;
    rotationMatrix[0][1] = (u * v * (1 - cos(radians)) - w * sqrt(L) * sin(radians)) / L;
    rotationMatrix[0][2] = (u * w * (1 - cos(radians)) + v * sqrt(L) * sin(radians)) / L;
    rotationMatrix[0][3] = 0.0; 
 
    rotationMatrix[1][0] = (u * v * (1 - cos(radians)) + w * sqrt(L) * sin(radians)) / L;
    rotationMatrix[1][1] = (v2 + (u2 + w2) * cos(radians)) / L;
    rotationMatrix[1][2] = (v * w * (1 - cos(radians)) - u * sqrt(L) * sin(radians)) / L;
    rotationMatrix[1][3] = 0.0; 
 
    rotationMatrix[2][0] = (u * w * (1 - cos(radians)) - v * sqrt(L) * sin(radians)) / L;
    rotationMatrix[2][1] = (v * w * (1 - cos(radians)) + u * sqrt(L) * sin(radians)) / L;
    rotationMatrix[2][2] = (w2 + (u2 + v2) * cos(radians)) / L;
    rotationMatrix[2][3] = 0.0; 
 
    rotationMatrix[3][0] = 0.0;
    rotationMatrix[3][1] = 0.0;
    rotationMatrix[3][2] = 0.0;
    rotationMatrix[3][3] = 1.0;
} 
    

