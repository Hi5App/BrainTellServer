#pragma once

#include <fstream>
#include <utility>

#include "FileIoInterface.hpp"

struct AnoUnit {
    std::string APOFILE;
    std::string SWCFILE;
};

class AnoIo final : public FileIoInterface<AnoUnit> {
public:
    explicit AnoIo(std::string filePath): m_FilePath(std::move(filePath)) {
    }

    void ReadFromFile() override {
        std::ifstream file(m_FilePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file");
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 8) == "APOFILE=") {
                m_Value.APOFILE = line.substr(8);
            }
            else if (line.substr(0, 8) == "SWCFILE=") {
                m_Value.SWCFILE = line.substr(8);
            }
        }
    }

    bool WriteToFile() override {
        std::ofstream file(m_FilePath);
        if (!file.is_open()) {
            return false;
        }

        file << "APOFILE=" << m_Value.APOFILE;
        file << "\n";
        file << "SWCFILE=" << m_Value.SWCFILE;

        return true;
    }

    void setValue(AnoUnit&value) override {
        m_Value = value;
    }

    AnoUnit& getValue() override {
        return m_Value;
    }

private:
    std::string m_FilePath;

    AnoUnit m_Value;
};
