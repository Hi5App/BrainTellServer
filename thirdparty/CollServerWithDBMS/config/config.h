#pragma once

#include <fstream>
#include <QStandardPaths>
#include <string>
#include <QCoreApplication>
#include <iostream>
#include "json.hpp"

class Config {
public:
    enum class ConfigItem {
        eServerIP,
        dbmsServerPort,
        brainServerPort,
        apiVersion,
        redisServerIP
    };

    static Config &getInstance() {
        static Config instance;
        return instance;
    }

    void initialize(const std::string &appConfigFile) {
        m_AppConfigFile = appConfigFile;

        std::filesystem::path path(m_AppRoamingPath);
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
    }

    void readConfig() {
        std::filesystem::path path(m_AppRunningPath);
        path = path / m_AppConfigFile;
        std::ifstream f(path.string());
        try {
            m_AppConfig = nlohmann::json::parse(f);
        } catch (...) {

        }
        f.close();
    }

    std::string getConfig(ConfigItem configItem) {
        switch (configItem) {
        case ConfigItem::eServerIP: {
            if (m_AppConfig.contains("eServerIP")) {
                return m_AppConfig["eServerIP"];
            } else {
                return "";
            }
        }
        case ConfigItem::dbmsServerPort: {
            if (m_AppConfig.contains("dbmsServerPort")) {
                return m_AppConfig["dbmsServerPort"];
            } else {
                return "";
            }
        }
        case ConfigItem::brainServerPort: {
            if(m_AppConfig.contains("brainServerPort")){
                return m_AppConfig["brainServerPort"];
            } else{
                return "";
            }
        }
        case ConfigItem::apiVersion: {
            if(m_AppConfig.contains("apiVersion")){
                return m_AppConfig["apiVersion"];
            } else{
                return "";
            }
        }
        case ConfigItem::redisServerIP: {
            if(m_AppConfig.contains("redisServerIp")){
                return m_AppConfig["redisServerIp"];
            } else{
                return "";
            }
        }
        default: {
            return "ConfigError";
        }
        }
    }

private:
    Config() {
        m_AppRoamingPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
        m_AppRunningPath = QCoreApplication::applicationDirPath().toStdString();
//        std::cout<<m_AppRoamingPath<<std::endl;
//        std::cout<<m_AppRunningPath<<std::endl;
    }

    std::string m_AppRoamingPath;
    std::string m_AppRunningPath;

    std::string m_AppConfigFile;

    nlohmann::json m_AppConfig;
};

