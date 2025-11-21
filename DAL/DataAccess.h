#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace DAL {

class DataAccessException : public std::runtime_error {
public:
    explicit DataAccessException(const std::string& message)
        : std::runtime_error(message) {}
};

template<typename T>
class IDataStorage {
public:
    virtual ~IDataStorage() = default;
    virtual void Save(const std::vector<T>& items) = 0;
    virtual std::vector<T> Load() = 0;
    virtual void Clear() = 0;
};

template<typename T>
class IJsonSerializable {
public:
    virtual ~IJsonSerializable() = default;
    virtual json ToJson() const = 0;
};

template<typename T>
class JsonStorage : public IDataStorage<T> {
private:
    std::string filePath;

    void ValidateFilePath() const {
        if (filePath.empty()) {
            throw DataAccessException("File path cannot be empty");
        }
    }

    void WriteToFile(const json& data) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw DataAccessException("Cannot open file for writing: " + filePath);
        }
        file << data.dump(4);
        if (!file.good()) {
            throw DataAccessException("Error writing to file: " + filePath);
        }
        file.close();
    }

    json ReadFromFile() {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return json::array();
        }

        json data;
        try {
            file >> data;
        } catch (const json::exception& e) {
            throw DataAccessException("JSON parse error: " + std::string(e.what()));
        }
        file.close();
        return data;
    }

public:
    explicit JsonStorage(const std::string& path) : filePath(path) {
        ValidateFilePath();
    }

    void Save(const std::vector<T>& items) override {
        try {
            json j = json::array();
            for (const auto& item : items) {
                j.push_back(item.ToJson());
            }
            WriteToFile(j);
        } catch (const json::exception& e) {
            throw DataAccessException("Serialization error: " + std::string(e.what()));
        } catch (const std::exception& e) {
            throw DataAccessException("Error saving data: " + std::string(e.what()));
        }
    }

    std::vector<T> Load() override {
        std::vector<T> items;
        try {
            json j = ReadFromFile();

            if (!j.is_array()) {
                throw DataAccessException("Invalid data format: expected array");
            }

            for (const auto& elem : j) {
                items.push_back(T::FromJson(elem));
            }
        } catch (const DataAccessException&) {
            throw;
        } catch (const json::exception& e) {
            throw DataAccessException("Deserialization error: " + std::string(e.what()));
        } catch (const std::exception& e) {
            throw DataAccessException("Error loading data: " + std::string(e.what()));
        }
        return items;
    }

    void Clear() override {
        try {
            WriteToFile(json::array());
        } catch (const std::exception& e) {
            throw DataAccessException("Error clearing data: " + std::string(e.what()));
        }
    }
};

    template<typename T>
    class XmlStorage : public IDataStorage<T> {
        //some code
    };


    template<typename T>
    class DatabaseStorage : public IDataStorage<T> {
        //some code
    };

}

#endif