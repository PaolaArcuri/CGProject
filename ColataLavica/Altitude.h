#ifndef ALTITUDE_H
#define ALTITUDE_H

#include "Matrix.h"
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Altitude : public Matrix
{

public:

    Altitude (const char * path) :Matrix (path)
    {
        coordsAltitude.nCols = coords.nCols+1;
        coordsAltitude.nRows = coords.nRows+1;
        VBOVertices = NULL;

        allocMatrix();
    }

    ~Altitude()
    {
        for (int i = 0; i < coordsAltitude.nRows; ++i) {
            delete [] vertices[i];
            delete [] normals[i];
            delete [] texCoords[i];
        }
        delete [] vertices;
        delete [] normals;
        delete [] texCoords;

        if (VBOVertices != NULL)
            delete [] VBOVertices;
    }


    friend std::ostream & operator <<( std::ostream &os, const Altitude &altitude )
    {

        os<<"Matrix di size: "<<altitude.coordsAltitude.nRows<<" X "<<altitude.coordsAltitude.nCols<<"\n";

        for (int i = 0; i < altitude.coordsAltitude.nRows; ++i) {
            for (int j = 0; j < altitude.coordsAltitude.nCols; ++j) {
                os<<altitude.vertices[i][j].y<<" ";


            }
            os<<"\n";
        }
        return os;
    }


    float* getVBOVertices()
    {
        if(VBOVertices == NULL)
        {
            createMatrixAltitude();
            calculateNormals();
            calculateTexCoords();
            calcuteEBO();
            generateVBO();
        }
        return VBOVertices;
    }


    Coordinates getCoordinates()
    {
        return coordsAltitude;
    }

protected:
    glm::vec3 ** vertices;
    glm::vec3 ** normals;
    glm::vec2 ** texCoords;

    int * indicesEBO;

    Coordinates coordsAltitude;

    float * VBOVertices;
    int sizeVBO;



    const int neighborhoodPattern[4][2]=
    {{-1,0},{0,1}, {1,0},{0,-1}};


    glm::vec3 getNormalToFace(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3)
    {
        glm::vec3 v= p2-p1;
        glm::vec3 u= p3-p1;

        glm::vec3 normal= glm::cross (v,u);

        return normal;
    }

    glm::vec3 getNormalOfVertex (unsigned int i, unsigned int j)
    {
        std::vector <glm::vec3> normalFaces;
        glm::vec3 p1 = vertices[i][j];
        for (int n = 0; n < 4; ++n)
        {

            glm::vec3* p2= getVertex(i+neighborhoodPattern[n][0],j+neighborhoodPattern[n][1]);
            glm::vec3* p3= getVertex(i+neighborhoodPattern[(n+1)%4][0],j+neighborhoodPattern[(n+1)%4][1]);
            if(p2 != NULL && p3!= NULL)
                normalFaces.push_back(getNormalToFace(p1,*p2,*p3));

        }

        glm::vec3 normal(0.0f,0.0f,0.0f);

        for (int i = 0; i < normalFaces.size(); ++i) {
            normal = normal+normalFaces[i];
        }

        return glm::normalize(normal);

    }

    glm::vec3* getVertex(unsigned int i, unsigned int j)
    {
        if(i>=0 && i<coordsAltitude.nRows && j>=0 && j<coordsAltitude.nCols)
            return &vertices[i][j];

        return NULL;
    }


    int linearIndex (unsigned int i, unsigned j)
    {
        return i*coordsAltitude.nCols+j;
    }



    float averageNeighbourhood(const int i, const int j)
    {

        //        cout<<"prova "<<data[0];
        float sum = 0.0f;
        int cout_cells = 0;
        for (int _i = -1; _i <= 0; ++_i) {
            for (int _j = -1; _j <= 0; ++_j) {
                //                printf ("Sono cella (%d, %d) e sto sommando (%d,%d) \n", i,j,i+_i,j+_j);
                if(i+_i >=0 && i+_i <coords.nRows && j+_j >=0 && j+_j <coords.nCols )
                {

//                    printf ("Sono cella (%d, %d) e sto sommando %f della cella (%d,%d)\n", i,j,data[i+_i][j+_j], i+_i,j+_j);
                    sum += data[i+_i][j+_j];
                    cout_cells++;
                }
            }
        }

        return sum /cout_cells;
        //        printf ("count_cell %d \n", cout_cells );

    }


    void allocMatrix()
    {
        vertices = new glm::vec3* [coordsAltitude.nRows];
        for (int i = 0; i < coordsAltitude.nRows; ++i) {
            vertices[i] = new glm::vec3 [coordsAltitude.nCols];
        }

        normals = new glm::vec3* [coordsAltitude.nRows];
        for (int i = 0; i < coordsAltitude.nRows; ++i) {
            normals[i] = new glm::vec3 [coordsAltitude.nCols];
        }

        texCoords = new glm::vec2* [coordsAltitude.nRows];
        for (int i = 0; i < coordsAltitude.nRows; ++i) {
            texCoords[i] = new glm::vec2 [coordsAltitude.nCols];
        }

        indicesEBO = new int [coordsAltitude.nRows * coordsAltitude.nCols * 2 * 3];
    }


    void createMatrixAltitude()
    {
        for(int i = 0; i< coordsAltitude.nRows; i++)
        {
            for (int j = 0; j < coordsAltitude.nCols; ++j)
            {
                vertices[i][j].x = j*cellsize;
                vertices[i][j].y = averageNeighbourhood(i,j);
                vertices[i][j].z = i*cellsize;

            }

        }
    }






    void calculateNormals ()
    {
        for(int i = 0; i< coordsAltitude.nRows; i++)
        {
            for (int j = 0; j < coordsAltitude.nCols; ++j)
            {
                normals[i][j] = getNormalOfVertex(i,j);

            }

        }


    }

    void calculateTexCoords ()
    {
        //j:xTex = nCol : 1
        //i:xTex = nRow : 1

        for(int i = 0; i< coordsAltitude.nRows; i++)
        {
            for (int j = 0; j < coordsAltitude.nCols; ++j)
            {
                texCoords[i][j] = glm::vec2(i/coordsAltitude.nRows, j/coordsAltitude.nCols);

            }

        }
    }

    void calcuteEBO ()
    {
        unsigned int globalIndex = 0;
        for (int i = 0; i < coordsAltitude.nRows-1; ++i) {
            for (int j = 0; j < coordsAltitude.nCols-1; ++j) {

                indicesEBO[globalIndex++] = linearIndex(i,j);
                indicesEBO[globalIndex++] = linearIndex(i,j+1);
                indicesEBO[globalIndex++] = linearIndex(i+1,j);
            }
        }

        for (int i = 1; i < coordsAltitude.nRows; ++i) {
            for (int j = 0; j < coordsAltitude.nCols-1; ++j) {

                indicesEBO[globalIndex++] = linearIndex(i,j);
                indicesEBO[globalIndex++] = linearIndex(i-1,j+1);
                indicesEBO[globalIndex++] = linearIndex(i,j+1);
            }
        }



    }

    void generateVBO ()
    {
        sizeVBO = coordsAltitude.nRows * coordsAltitude.nCols * 8;
        VBOVertices = new float [sizeVBO];

        int globalIndex = 0;
        for (int i = 0; i < coordsAltitude.nRows; ++i) {
            for (int j = 0; j < coordsAltitude.nCols; ++j) {
                VBOVertices[globalIndex++] = vertices[i][j].x;
                VBOVertices[globalIndex++] = vertices[i][j].y;
                VBOVertices[globalIndex++] = vertices[i][j].z;

                VBOVertices[globalIndex++] = normals[i][j].x;
                VBOVertices[globalIndex++] = normals[i][j].y;
                VBOVertices[globalIndex++] = normals[i][j].z;

                VBOVertices[globalIndex++] = texCoords[i][j].x;
                VBOVertices[globalIndex++] = texCoords[i][j].y;


            }
        }

    }






};

#endif