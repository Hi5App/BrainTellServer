#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <ranges>
#include <sstream>

//inline std::vector<std::string> string_split(const std::string& s, char delim = ' ') {
//	std::vector<std::string> tokens;
//	for (auto token : s | std::views::split(delim)) {
//		tokens.emplace_back(token.begin(), token.end());
//	}
//	return tokens;
//}

inline std::vector<std::string> string_split(const std::string& str, char delim) {
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

struct NeuronUnit
{
	NeuronUnit() {

	}
	NeuronUnit(float x, float y, float z) : x(x),y(y),z(z) {

	}
	std::string getString(bool isApoOutput = false) {
        if(!isApoOutput){
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
        }else{
            //for apo format zxy
            std::string str = "0, , , , " +
                std::to_string(z) + ", " + std::to_string(x) + ", " + std::to_string(y) +
            ", 0.000, 0.000, 0.000, 314.159, 0.000, , , , 128, 168, 255";
                return str;
        }
	}

	int n;
	int type;
	float x;
	float y;
	float z;
	float radius;
	int parent;
	int seg_id;
	int level;
	int mode;
	int timestamp;
	int feature_value;
};

class ESwc
{
public:
	ESwc(std::string filePath)
	: m_FilePath(filePath) {
		std::ifstream infile;
		infile.open(m_FilePath);
		if (!infile.is_open()) {
			throw std::runtime_error("Open file failed!");
		}

		std::string rowContent;
		while (std::getline(infile, rowContent)) {
			auto splitResult = string_split(rowContent, ' ');

            if (rowContent.empty() || rowContent[0] == '#') {
                rowContent.clear();
                continue;
            }

			if (splitResult.size() < 12) {
				throw std::runtime_error("File content error!");
			}
			NeuronUnit unit;
			unit.n = std::stoi(splitResult[0]);
			unit.type = std::stoi(splitResult[1]);
			unit.x = std::stof(splitResult[2]);
			unit.y = std::stof(splitResult[3]);
			unit.z = std::stof(splitResult[4]);
			unit.radius = std::stof(splitResult[5]);
			unit.parent = std::stoi(splitResult[6]);
			unit.seg_id = std::stoi(splitResult[7]);
			unit.level = std::stoi(splitResult[8]);
			unit.mode = std::stoi(splitResult[9]);
			unit.timestamp = std::stoi(splitResult[10]);
			unit.feature_value = std::stoi(splitResult[11]);

			m_Neuron.push_back(unit);

			rowContent.clear();
		}

		int a = 1;
	}

	std::vector<NeuronUnit>& getNeuron() {
		return m_Neuron;
	}

private:
	std::vector<NeuronUnit> m_Neuron;

	std::string m_FilePath;
};

class Swc
{
public:
    Swc(std::string filePath)
            : m_FilePath(filePath) {
        std::ifstream infile;
        infile.open(m_FilePath);
        if (!infile.is_open()) {
            throw std::runtime_error("Open file failed!");
        }

        std::string rowContent;
        while (std::getline(infile, rowContent)) {
            auto splitResult = string_split(rowContent, ' ');

            if (rowContent.empty() || rowContent[0] == '#') {
                rowContent.clear();
                continue;
            }

            if (splitResult.size() < 7) {
                throw std::runtime_error("File content error!");
            }
            NeuronUnit unit;
            unit.n = std::stoi(splitResult[0]);
            unit.type = std::stoi(splitResult[1]);
            unit.x = std::stof(splitResult[2]);
            unit.y = std::stof(splitResult[3]);
            unit.z = std::stof(splitResult[4]);
            unit.radius = std::stof(splitResult[5]);
            unit.parent = std::stoi(splitResult[6]);

            m_Neuron.push_back(unit);

            rowContent.clear();
        }

        int a = 1;
    }

    std::vector<NeuronUnit>& getNeuron() {
        return m_Neuron;
    }

private:
    std::vector<NeuronUnit> m_Neuron;

    std::string m_FilePath;
};
