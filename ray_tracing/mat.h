#ifndef MAT_H
#define MAT_H


#include <array>
#include <fstream>
#include <stdio.h>

typedef double mat4[4][4];
typedef double mat3[3][3];

// u = Mv

void create_mat4(mat4 m, std::string file)
{
   std::ifstream matrix;
   matrix.open(file);
   std::string line;
   for(int i = 0; i < 4; i++)
   {
       std::getline(matrix, line);
       sscanf(line.c_str(), "%lf %lf %lf %lf", &m[i][0], &m[i][1], &m[i][2], &m[i][3]);
   }
}

void print_mat4(mat4 m)
{
    std::cerr << "===========================\n";
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
            std::cerr << m[i][j] << " ";
        std::cerr << "\n";
    }
    std::cerr << "===========================\n";
}

void print_mat3(mat3 m)
{
    std::cerr << "===========================\n";
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
            std::cerr << m[i][j] << " ";
        std::cerr << "\n";
    }
    std::cerr << "===========================\n";
}

void create_mat3(mat3 m, std::string file)
{
   std::ifstream matrix;
   matrix.open(file);
   std::string line;
   for(int i = 0; i < 3; i++)
   {
       std::getline(matrix, line);
       sscanf(line.c_str(), "%lf %lf %lf", &m[i][0], &m[i][1], &m[i][2]);
   }
}

void mat4_mul(mat4 m, std::array<double, 4> &v, std::array<double, 4> &u)
{
    for(int i = 0; i < 4; i++)
    {
        double sum = 0;
        for(int j = 0; j < 4; j++)
            sum += m[i][j] * v[j];
        u[i] = sum;
        //std::cerr << sum << " ";
    }
    //std::cerr << "\n";
}
void mat3_mul(mat3 m, std::array<double, 3> &v, std::array<double, 3> &u)
{
    for(int i = 0; i < 3; i++)
    {
        double sum = 0;
        for(int j = 0; j < 3; j++)
            sum += m[i][j] * v[j];
        u[i] = sum;
    }
}

#endif
