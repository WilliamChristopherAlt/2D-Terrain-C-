#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <vector>
#include <map>
#include <string>

#include "vec3.h"
#include "goodies.h"

class terrain_color{
    public:
        terrain_color(double min_b, double max_b, vec3 color_) : min_bound(min_b), max_bound(max_b), color(color_/255.999f) {}

        vec3 get_color() const {return color;}

        bool contain_height(double height) const
        {
            return (height >= min_bound) && (height <= max_bound);
        }

    private:
        double min_bound;
        double max_bound;
        vec3 color;
};

vec3 get_color_by_height(const std::map<std::string, terrain_color>& colors_dict, double height, const vec3& normal_vec, double water_level, double slope_cutoff)
{
    bool is_flat;
    bool is_water = (height < water_level);
    if (is_water)
        is_flat = true;
    else  
        is_flat = dot(normal_vec, vec3(0.0, 0.0, 1.0)) > slope_cutoff;
    
    if (is_water) {
        double water_depth = water_level - height;
        double water_lerp = ease_out(water_depth / water_level);
        return lerp(colors_dict.at("sand").get_color(), colors_dict.at("water").get_color(), water_lerp);
    }
    else {
        if (is_flat) {
            for (const auto& pair: colors_dict)
                if (pair.second.contain_height(height))
                    return pair.second.get_color();
                // int a;
                // std::cout << "purple, height: " << height << std::endl;
                // std::cin >> a;
            return vec3(0.0, 0.0, 0.0);
        }
        else
            return colors_dict.at("slate").get_color();
    }    
    return vec3(1.0, 0.0, 1.0);
}

#endif