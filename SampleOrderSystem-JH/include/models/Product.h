#pragma once
#include <string>

namespace models {

struct Product {
    int id = 0;
    std::string name;
    int batch_size = 0;
    int batch_days = 0;
    double yield_rate = 0.9;
};

}  // namespace models
