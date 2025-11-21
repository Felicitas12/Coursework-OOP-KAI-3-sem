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


}

#endif