#pragma once

#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <ranges>
#include "FileIoInterface.hpp"
#include "utils.h"

struct NeuronUnit {
    NeuronUnit() = default;

    NeuronUnit(float x, float y, float z) : x(x), y(y), z(z) {
    }

    std::string getString(bool isApoOutput = false) {
        std::string str =
                std::to_string(n) + " " +
                std::to_string(type) + " " +
                std::to_string(x) + " " +
                std::to_string(y) + " " +
                std::to_string(z) + " " +
                std::to_string(radius) + " " +
                std::to_string(parent) + " " +
                std::to_string(seg_id) + " " +
                std::to_string(level) + " " +
                std::to_string(mode) + " " +
                std::to_string(timestamp) + " " +
                std::to_string(feature_value);
        return str;
    }

    int n = 0;
    int type = 0;
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    float radius = 0.0;
    int parent = 0;
    int seg_id = 0;
    int level = 0;
    int mode = 0;
    int timestamp = 0;
    int feature_value = 0;
};

class ESwc final : public FileIoInterface<std::vector<NeuronUnit>> {
public:
    explicit ESwc(std::string filePath)
            : m_FilePath(std::move(filePath)) {
    }

    void ReadFromFile() override {
        std::ifstream infile;
        infile.open(m_FilePath);
        if (!infile.is_open()) {
            return;
        }

        std::string rowContent;
        while (std::getline(infile, rowContent)) {
            auto splitResult = stringSplit(rowContent, ' ');

            if (rowContent.empty() || rowContent[0] == '#') {
                rowContent.clear();
                continue;
            }

            NeuronUnit unit;
            for (int i = 0; i < splitResult.size(); i++) {
                switch (i) {
                    case 0: {
                        unit.n = std::stoi(splitResult[0]);
                        break;
                    }
                    case 1: {
                        unit.type = std::stoi(splitResult[1]);
                        break;
                    }
                    case 2: {
                        unit.x = std::stof(splitResult[2]);
                        break;
                    }
                    case 3: {
                        unit.y = std::stof(splitResult[3]);
                        break;
                    }
                    case 4: {
                        unit.z = std::stof(splitResult[4]);
                        break;
                    }
                    case 5: {
                        unit.radius = std::stof(splitResult[5]);
                        break;
                    }
                    case 6: {
                        unit.parent = std::stoi(splitResult[6]);
                        break;
                    }
                    case 7: {
                        unit.seg_id = std::stoi(splitResult[7]);
                        break;
                    }
                    case 8: {
                        unit.level = std::stoi(splitResult[8]);
                        break;
                    }
                    case 9: {
                        unit.mode = std::stoi(splitResult[9]);
                        break;
                    }
                    case 10: {
                        unit.timestamp = std::stoi(splitResult[10]);
                        break;
                    }
                    case 11: {
                        unit.feature_value = std::stoi(splitResult[11]);
                        break;
                    }
                    default:;
                }
            }

            m_Neuron.push_back(unit);

            rowContent.clear();
        }
    }

    bool WriteToFile() override {
        std::ofstream outfile;
        outfile.open(m_FilePath);
        if (!outfile.is_open()) {
            return false;
        }

        outfile << "# Generated by SwcManagerClient\n"
                << "# Source File(s):\n"
                << "##n,type,x,y,z,radius,parent,seg_id,level,mode,timestamp,feature_value\n";

        for (auto &neuron: m_Neuron) {
            outfile << std::to_string(neuron.n) << " " <<
                    std::to_string(neuron.type) + " " <<
                    std::fixed << std::setprecision(3) << neuron.x << " " <<
                    std::fixed << std::setprecision(3) << neuron.y << " " <<
                    std::fixed << std::setprecision(3) << neuron.z << " " <<
                    std::to_string(neuron.radius) << " " <<
                    std::to_string(neuron.parent) << " " <<
                    std::to_string(neuron.seg_id) << " " <<
                    std::to_string(neuron.level) << " " <<
                    std::to_string(neuron.mode) << " " <<
                    std::to_string(neuron.timestamp) << " " <<
                    std::to_string(neuron.feature_value) << "\n";
        }
        outfile.close();
        return true;
    }

    std::vector<NeuronUnit> &getValue() override {
        return m_Neuron;
    }

    void setValue(std::vector<NeuronUnit> &neuron) override {
        m_Neuron = neuron;
    }

private:
    std::vector<NeuronUnit> m_Neuron;

    std::string m_FilePath;
};

class Swc final : public FileIoInterface<std::vector<NeuronUnit>> {
public:
    explicit Swc(std::string filePath)
            : m_FilePath(std::move(filePath)) {
    }

    void ReadFromFile() override {
        std::ifstream infile;
        infile.open(m_FilePath);
        if (!infile.is_open()) {
            throw std::runtime_error("Could not open file");
        }

        std::string rowContent;
        while (std::getline(infile, rowContent)) {
            auto splitResult = stringSplit(rowContent, ' ');

            if (rowContent.empty() || rowContent[0] == '#') {
                rowContent.clear();
                continue;
            }

            NeuronUnit unit;
            for (int i = 0; i < splitResult.size(); i++) {
                switch (i) {
                    case 0: {
                        unit.n = std::stoi(splitResult[0]);
                        break;
                    }
                    case 1: {
                        unit.type = std::stoi(splitResult[1]);
                        break;
                    }
                    case 2: {
                        unit.x = std::stof(splitResult[2]);
                        break;
                    }
                    case 3: {
                        unit.y = std::stof(splitResult[3]);
                        break;
                    }
                    case 4: {
                        unit.z = std::stof(splitResult[4]);
                        break;
                    }
                    case 5: {
                        unit.radius = std::stof(splitResult[5]);
                        break;
                    }
                    case 6: {
                        unit.parent = std::stoi(splitResult[6]);
                        break;
                    }
                    default:;
                }
            }

            m_Neuron.push_back(unit);

            rowContent.clear();
        }
    }

    bool WriteToFile() override {
        std::ofstream outfile;
        outfile.open(m_FilePath);
        if (!outfile.is_open()) {
            return false;
        }

        outfile << "#name\n"
                << "#Generated by SwcManagerClient\n"
                << "##n,type,x,y,z,radius,parent\n";

        for (const auto &neuron: m_Neuron) {
            outfile << std::setprecision(3) << std::to_string(neuron.n) << " " <<
                    std::to_string(neuron.type) << " " <<
                    std::fixed << std::setprecision(3) << neuron.x << " " <<
                    std::fixed << std::setprecision(3) << neuron.y << " " <<
                    std::fixed << std::setprecision(3) << neuron.z << " " <<
                    std::to_string(neuron.radius) << " " <<
                    std::to_string(neuron.parent) << "\n";
        }
        outfile.close();
        return true;
    }

    std::vector<NeuronUnit> &getValue() override {
        return m_Neuron;
    }

    void setValue(std::vector<NeuronUnit> &neuron) override {
        m_Neuron = neuron;
    }

private:
    std::vector<NeuronUnit> m_Neuron;

    std::string m_FilePath;
};
