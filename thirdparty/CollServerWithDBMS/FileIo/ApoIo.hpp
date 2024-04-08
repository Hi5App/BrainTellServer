#pragma once

#include <fstream>
#include <utility>
#include "utils.h"
#include "FileIoInterface.hpp"

struct ApoUnit {
    int n{};
    std::string orderinfo;
    std::string name;
    std::string comment;
    float z{}, x{}, y{};
    float pixmax{}, intensity{}, sdev{};
    float volsize{}, mass{};
    int color_r{}, color_g{}, color_b{};
};

class ApoIo final : public FileIoInterface<std::vector<ApoUnit>> {
public:
    explicit ApoIo(std::string filePath) : m_FilePath(std::move(filePath)) {
    }

    void ReadFromFile() override {
        std::ifstream file(m_FilePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file");
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            ApoUnit unit;
            std::vector<std::string> elems = stringSplit(line, ',');

            for (size_t i = 0; i < elems.size(); i++) {
                if (i == 0) unit.n = std::stoi(elems[i]);
                else if (i == 1) unit.orderinfo = elems[i];
                else if (i == 2) unit.name = elems[i];
                else if (i == 3) unit.comment = elems[i];
                else if (i == 4) unit.z = std::stof(elems[i]);
                else if (i == 5) unit.x = std::stof(elems[i]);
                else if (i == 6) unit.y = std::stof(elems[i]);
                else if (i == 7) unit.pixmax = std::stof(elems[i]);
                else if (i == 8) unit.intensity = std::stof(elems[i]);
                else if (i == 9) unit.sdev = std::stof(elems[i]);
                else if (i == 10) unit.volsize = std::stof(elems[i]);
                else if (i == 11) unit.mass = std::stof(elems[i]);
                else if (i == 15) unit.color_r = std::stoi(elems[i]);
                else if (i == 16) unit.color_g = std::stoi(elems[i]);
                else if (i == 17) unit.color_b = std::stoi(elems[i]);
            }

            m_ApoUnits.push_back(unit);
        }
    }

    bool WriteToFile() override {
        std::ofstream file(m_FilePath);
        if (!file.is_open()) {
            return false;
        }
        file << "##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b" <<
             "\n";
        for (const auto &unit: m_ApoUnits) {
            file << unit.n << ","
                 << unit.orderinfo << ","
                 << unit.name << ","
                 << unit.comment << ","
                 << unit.z << ","
                 << unit.x << ","
                 << unit.y << ","
                 << unit.pixmax << ","
                 << unit.intensity << ","
                 << unit.sdev << ","
                 << unit.volsize << ","
                 << unit.mass << ",,,,"
                 << unit.color_r << ","
                 << unit.color_g << ","
                 << unit.color_b << "\n";
        }

        return true;
    }

    void setValue(std::vector<ApoUnit> &value) override {
        m_ApoUnits = value;
    }

    std::vector<ApoUnit> &getValue() override {
        return m_ApoUnits;
    }

private:
    std::string m_FilePath;

    std::vector<ApoUnit> m_ApoUnits;
};
