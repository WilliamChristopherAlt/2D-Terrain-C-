#ifndef SHADOW_H 
#define SHADOW_H

#include "vec3.h"
#include "goodies.h"
#include <cmath>
#include <typeinfo>

vec3 get_shadow_onthorgonal(const vec3& color_tile, int i_idx, int j_idx, const Environment& scene_params, int steps = 500)
{
    vec3 sun_dir = scene_params.sun_pos - vec3(0.5, 0.5, 0.0);
    vec3 step_dir = unit_vector(sun_dir);

    double height_origin = scene_params.height_map[j_idx][i_idx];
    if (height_origin < scene_params.water_level)
        height_origin = scene_params.water_level;

    vec3 p = vec3(static_cast<double>(i_idx)/scene_params.width, static_cast<double>(j_idx)/scene_params.width, height_origin);

    bool in_shadow = false;
    int x_index, y_index;
    double h;

    for(int i = 0; i < steps; i++)
    {
        x_index = floor(p[0]*scene_params.width);
        y_index = floor(p[1]*scene_params.height);
        if (x_index < 0 || x_index >= scene_params.width || y_index < 0 || y_index >= scene_params.height)
            break;

        h = scene_params.height_map[y_index][x_index];
        p += step_dir * std::max(scene_params.min_step, (p[2] - h) * 0.05);

        if (h > p[2]) {
            in_shadow = true;
            break;
        }
        
        if(p[2] > 1.0)
            break;
    }
    if (in_shadow) {
        return color_tile * scene_params.shadow_brightness;
    }
    else {
        return color_tile;
    }

}

#endif