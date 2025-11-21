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
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual void Save(const std::vector<T>& items) = 0;
    virtual std::vector<T> Load() = 0;
    virtual void Clear() = 0;
};

template<typename T>
class JsonRepository : public IRepository<T> {
private:
    std::string filePath;

public:
    explicit JsonRepository(const std::string& path) : filePath(path) {}

    void Save(const std::vector<T>& items) override {
        try {
            json j = json::array();
            for (const auto& item : items) {
                j.push_back(item.ToJson());
            }

            std::ofstream file(filePath);
            if (!file.is_open()) {
                throw DataAccessException("Cannot open file for writing: " + filePath);
            }
            file << j.dump(4);
            file.close();
        } catch (const std::exception& e) {
            throw DataAccessException("Error saving data: " + std::string(e.what()));
        }
    }

    std::vector<T> Load() override {
        std::vector<T> items;
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return items;
            }

            json j;
            file >> j;
            file.close();

            if (j.is_array()) {
                for (const auto& elem : j) {
                    items.push_back(T::FromJson(elem));
                }
            }
        } catch (const std::exception& e) {
            throw DataAccessException("Error loading data: " + std::string(e.what()));
        }
        return items;
    }

    void Clear() override {
        try {
            std::ofstream file(filePath, std::ofstream::trunc);
            if (!file.is_open()) {
                throw DataAccessException("Cannot open file for clearing: " + filePath);
            }
            file << "[]";
            file.close();
        } catch (const std::exception& e) {
            throw DataAccessException("Error clearing data: " + std::string(e.what()));
        }
    }
};

}

#endif