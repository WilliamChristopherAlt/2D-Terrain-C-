#include <iostream>

#include "goodies.h"
#include "perlin.h"
#include "vec3.h"
#include "color.h"
#include "shadow.h"

#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <limits>

// Image dimemsion in pixels and in world coordinate
int width = 500;
int height = 500;
double world_width = 5.0;
double world_height = 5.0;

// Geographical customization
vec3 sun_pos = vec3(0.0, 0.0, 1.0);
double shadow_brightness = 0.5;
double water_level = 0.3;
double slope_cutoff = 0.3;
double island_exp_mod=1.0/3.0;

double water_drop_density = 0.01;
double water_drop_volume = 0.2;
double soil_hardness = 0.5;

double get_height(double** height_map, double x, double y)
{
    return height_map[static_cast<int>(y * height)][static_cast<int>(x * width)];
}

vec3 get_normal(double** height_map, int i, int j)
{
    double h = (double)(width + height) / 10;

    double u, d, l, r;
    int u_i, d_i, l_i, r_i;

    u_i = std::max(0, j-1);
    d_i = std::min(height-1, j+1);
    l_i = std::max(0, i-1);
    r_i = std::min(width-1, i+1);

    u = h * height_map[u_i][i];
    d = h * height_map[d_i][i];
    l = h * height_map[j][l_i];
    r = h * height_map[j][r_i];

    return unit_vector(vec3(l-r, u-d, 1.0f));
}

// Raise center to make island (or not)
double** get_height_map()
{
    double dx = world_width / width;
    double dy = world_height / height;

    double x = 0.0f, y = 0.0f;

    // double center_x = world_width / 2;
    // double center_y = world_height / 2;

    // Asssuming center left and down edges of the texture lie right on x and y axis
    // double maxD = pow(center_x*center_x + center_y*center_y, island_exp_mod);

    perlin perlin_perm = perlin();
    double** texture = new double*[height];

    double dis;
    double perlin_height;
    double max_height = 0;

    for (int j = 0; j < height; j++)
    {
        texture[j] = new double[width];
        for (int i = 0; i < width; i++) {
            // dis = pow((center_x - x) * (center_x - x) + (center_y - y) * (center_y - y), island_exp_mod);

            perlin_height = (perlin_perm.octaves(x, y) + 1) / 2 ;
            texture[j][i] = perlin_height;
            // texture[j][i] = perlin_height * (1 - dis/maxD);

            if (texture[j][i] > max_height)
                max_height = texture[j][i];
            x += dx;
        }
        y += dy;
        x = 0.0f;
    }

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++)
            texture[j][i] /= max_height;
    return texture;
}

vec3** get_normal_map(double** height_map)
{
    double h = (double)(width + height) / 10;

    // Allocate memory for the normal texture
    vec3** normal_texture = new vec3*[height];
    for (int j = 0; j < height; j++)
        normal_texture[j] = new vec3[width];


    double u, d, l, r;
    int u_i, d_i, l_i, r_i;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            u_i = std::max(0, j-1);
            d_i = std::min(height-1, j+1);
            l_i = std::max(0, i-1);
            r_i = std::min(width-1, i+1);

            u = h * height_map[u_i][i];
            d = h * height_map[d_i][i];
            l = h * height_map[j][l_i];
            r = h * height_map[j][r_i];

            normal_texture[j][i] = unit_vector(vec3(l-r, u-d, 1.0f));
        }
    }
    return normal_texture;
}

void normalize_map(double** height_map)
{
    double max = -99999999999.9;
    double min = 99999999999.9;
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if (max < height_map[j][i])
                max = height_map[j][i];
            if (min > height_map[j][i])
                min = height_map[j][i];
        }
    }
    std::cout << min << ' ' << max << std::endl;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            height_map[j][i] = (height_map[j][i] - min) / (max - min);
        }
    }
}

double** get_erosion_map(double** height_map, int steps=100)
{
    // Copying height map and initialize water drops
    double** erosion_map = new double*[height];
    double** water_drop_map = new double*[height];
    for (int j = 0; j < height; j++)
    {
        erosion_map[j] = new double[width];
        water_drop_map[j] = new double[width];
        for (int i = 0; i < width; i++)
        {   
            erosion_map[j][i] = height_map[j][i];
            if (random_double() < water_drop_density)
                water_drop_map[j][i] = water_drop_volume;
            else
                water_drop_map[j][i] = 0.0;
        }
    }

    int up, down, left, right, minX, minY, i_, j_;
    double min, h_val, diff, erosion;

    for (int k = 0; k < steps; k++)
    {
        for (int j = 0; j < height; j++)
            for (int i = 0; i < width; i++)
                if (random_double() < water_drop_density)
                        water_drop_map[j][i] = water_drop_volume;
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {  
                if (water_drop_map[j][i] < 0.0)
                    continue;

                left = std::max(0, i-1);
                right = std::min(width-1, i+1);
                up = std::max(0, j-1);
                down = std::min(height-1, j+1);

                min = 999999999999.0;
                minX = 0;
                minY = 0;

                for (int i_ = left; i_ <= right; i_++)
                {
                    for (int j_ = up; j_ <= down; j_++)
                    {
                        if (i_ != i || j_ != j)
                        {
                            h_val = erosion_map[j_][i_] + water_drop_map[j_][i_];
                            if (h_val < min)
                            {
                                min = h_val;
                                minX = i_;
                                minY = j_;
                            }
                        }
                    }
                }

                diff = 0.5 * (erosion_map[j][i] + water_drop_map[j][i] - min);

                if (diff > 0)
                {
                    erosion = diff * (1.0 - soil_hardness);

                    erosion_map[j][i] -= erosion;

                    // if (erosion_map[minY][minX] + erosion > 1.0)
                    //     break;

                    erosion_map[minY][minX] += erosion * 0.9;

                    diff = 0.5 * (erosion_map[j][i] + water_drop_map[j][i] - erosion_map[minX][minY] - water_drop_map[minX][minY]);
                    water_drop_map[j][i] -= diff;
                    water_drop_map[minY][minX] += diff;
                }
            } 
        }
    }

    return erosion_map;
}


int main() {

    // Color customization
    std::map<std::string, terrain_color> color_dicts;
    color_dicts.emplace("water", terrain_color(0.0, water_level, vec3(98, 166, 169)));
    color_dicts.emplace("sand", terrain_color(water_level, 0.4, vec3(241, 182, 158)));
    color_dicts.emplace("grass", terrain_color(0.4, 0.5, vec3(152, 173, 90)));
    color_dicts.emplace("dark_grass", terrain_color(0.5, 0.6, vec3(101, 133, 65)));
    color_dicts.emplace("forest", terrain_color(0.6, 0.7, vec3(71, 118, 69)));
    color_dicts.emplace("stone", terrain_color(0.7, 0.8, vec3(109, 118, 135)));
    color_dicts.emplace("slate", terrain_color(0.8, 0.9, vec3(132, 141, 154)));
    color_dicts.emplace("snow", terrain_color(0.9, 1.0, vec3(210, 224, 222)));

    // Maps
    double** height_map = get_height_map();
    double** erosion_map = get_erosion_map(height_map);
    normalize_map(erosion_map);
    vec3** normal_map = get_normal_map(erosion_map);

    Environment scene_params;
    scene_params.height_map = erosion_map;
    scene_params.width = width;
    scene_params.height = height;
    scene_params.sun_pos = sun_pos;
    scene_params.water_level = water_level;
    scene_params.shadow_brightness = shadow_brightness;
    scene_params.min_step = std::min(static_cast<double>(1.0f/scene_params.width), static_cast<double>(1.0f/scene_params.height));

    vec3 color_tile, shadow_tile;
    int brightness;
    double height_val;

    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    double max = 0.0;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++) {
            // if (max < erosion_map[j][i])
            //     max = erosion_map[j][i];
            // color_tile = get_color_by_height(color_dicts, erosion_map[j][i], normal_map[j][i], water_level, slope_cutoff);
            // shadow_tile = get_shadow_onthorgonal(color_tile, i, j, scene_params);

            color_tile = (normal_map[j][i] + 1) / 2;
            // color_tile = (get_normal(erosion_map, i, j) + 1) / 2;

            // std::cout << static_cast<int> (256 * color_tile[0]) << ' '
            //           << static_cast<int> (256 * color_tile[1]) << ' '
            //           << static_cast<int> (256 * color_tile[2]) << '\n';
        }
    }

    // std::cout << "max of erosion map: " << max << std::endl;

    return 0;
}