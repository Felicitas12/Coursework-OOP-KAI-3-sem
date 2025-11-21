#ifndef WALJSONSTORAGE_H
#define WALJSONSTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <chrono>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace DAL {

enum class OperationType {
    INSERT,
    UPDATE,
    DELETE
};

template<typename T>
struct Operation {
    OperationType type;
    int id;
    T data;
    std::chrono::system_clock::time_point timestamp;

    json ToJson() const {
        json j = {
            {"type", static_cast<int>(type)},
            {"id", id},
            {"timestamp", std::chrono::system_clock::to_time_t(timestamp)}
        };
        if (type != OperationType::DELETE) {
            j["data"] = data.ToJson();
        }
        return j;
    }

    static Operation FromJson(const json& j) {
        Operation op;
        op.type = static_cast<OperationType>(j["type"].get<int>());
        op.id = j["id"];
        op.timestamp = std::chrono::system_clock::from_time_t(j["timestamp"]);
        if (op.type != OperationType::DELETE && j.contains("data")) {
            op.data = T::FromJson(j["data"]);
        }
        return op;
    }
};

template<typename T>
class WALJsonStorage {
private:
    std::string dataFilePath;
    std::string walFilePath;
    std::map<int, T> memoryIndex;
    std::set<int> deletedIds;
    int operationsSinceCompact;
    int compactThreshold;
    bool indexLoaded;

    void LoadIndex() {
        if (indexLoaded) return;

        std::ifstream dataFile(dataFilePath);
        if (dataFile.is_open()) {
            try {
                json j;
                dataFile >> j;
                if (j.is_array()) {
                    for (const auto& elem : j) {
                        T item = T::FromJson(elem);
                        memoryIndex[item.GetId()] = item;
                    }
                }
            } catch (...) {}
            dataFile.close();
        }

        ApplyWAL();
        indexLoaded = true;
    }

    void ApplyWAL() {
        std::ifstream walFile(walFilePath);
        if (!walFile.is_open()) return;

        std::string line;
        while (std::getline(walFile, line)) {
            if (line.empty()) continue;
            try {
                json j = json::parse(line);
                Operation<T> op = Operation<T>::FromJson(j);

                switch (op.type) {
                    case OperationType::INSERT:
                    case OperationType::UPDATE:
                        memoryIndex[op.id] = op.data;
                        deletedIds.erase(op.id);
                        break;
                    case OperationType::DELETE:
                        memoryIndex.erase(op.id);
                        deletedIds.insert(op.id);
                        break;
                }
            } catch (...) {}
        }
        walFile.close();
    }

    void AppendToWAL(const Operation<T>& op) {
        std::ofstream walFile(walFilePath, std::ios::app);
        if (walFile.is_open()) {
            walFile << op.ToJson().dump() << "\n";
            walFile.close();
        }
        operationsSinceCompact++;

        if (operationsSinceCompact >= compactThreshold) {
            Compact();
        }
    }

    void Compact() {
        std::vector<T> allItems;
        for (const auto& pair : memoryIndex) {
            allItems.push_back(pair.second);
        }

        std::ofstream dataFile(dataFilePath);
        if (!dataFile.is_open()) {
            throw std::runtime_error("Cannot open data file for compaction");
        }

        json j = json::array();
        for (const auto& item : allItems) {
            j.push_back(item.ToJson());
        }
        dataFile << j.dump(2);
        dataFile.close();

        std::ofstream walFile(walFilePath, std::ofstream::trunc);
        walFile.close();

        operationsSinceCompact = 0;
        deletedIds.clear();
    }

public:
    WALJsonStorage(const std::string& dataPath, int compactAfter = 100)
        : dataFilePath(dataPath),
          walFilePath(dataPath + ".wal"),
          operationsSinceCompact(0),
          compactThreshold(compactAfter),
          indexLoaded(false) {}

    void Insert(const T& item) {
        LoadIndex();

        Operation<T> op;
        op.type = OperationType::INSERT;
        op.id = item.GetId();
        op.data = item;
        op.timestamp = std::chrono::system_clock::now();

        memoryIndex[item.GetId()] = item;
        AppendToWAL(op);
    }

    void Update(const T& item) {
        LoadIndex();

        if (memoryIndex.find(item.GetId()) == memoryIndex.end()) {
            throw std::runtime_error("Item not found for update");
        }

        Operation<T> op;
        op.type = OperationType::UPDATE;
        op.id = item.GetId();
        op.data = item;
        op.timestamp = std::chrono::system_clock::now();

        memoryIndex[item.GetId()] = item;
        AppendToWAL(op);
    }

    void Delete(int id) {
        LoadIndex();

        if (memoryIndex.find(id) == memoryIndex.end()) {
            throw std::runtime_error("Item not found for deletion");
        }

        Operation<T> op;
        op.type = OperationType::DELETE;
        op.id = id;
        op.timestamp = std::chrono::system_clock::now();

        memoryIndex.erase(id);
        deletedIds.insert(id);
        AppendToWAL(op);
    }

    T LoadById(int id) {
        LoadIndex();

        auto it = memoryIndex.find(id);
        if (it == memoryIndex.end()) {
            throw std::runtime_error("Item not found");
        }
        return it->second;
    }

    std::vector<T> LoadAll() {
        LoadIndex();

        std::vector<T> result;
        for (const auto& pair : memoryIndex) {
            result.push_back(pair.second);
        }
        return result;
    }

    std::vector<T> LoadRange(int offset, int limit) {
        LoadIndex();

        std::vector<T> result;
        auto it = memoryIndex.begin();
        std::advance(it, std::min(offset, static_cast<int>(memoryIndex.size())));

        int count = 0;
        while (it != memoryIndex.end() && count < limit) {
            result.push_back(it->second);
            ++it;
            ++count;
        }
        return result;
    }

    void Save(const std::vector<T>& items) {
        memoryIndex.clear();
        for (const auto& item : items) {
            memoryIndex[item.GetId()] = item;
        }
        Compact();
        indexLoaded = true;
    }

    void Clear() {
        memoryIndex.clear();
        deletedIds.clear();
        Compact();
    }

    int GetCount() const {
        return memoryIndex.size();
    }

    void ForceCompact() {
        Compact();
    }

    int GetOperationsSinceCompact() const {
        return operationsSinceCompact;
    }

    bool Exists(int id) {
        LoadIndex();
        return memoryIndex.find(id) != memoryIndex.end();
    }

    std::vector<T> LoadByIds(const std::vector<int>& ids) {
        LoadIndex();

        std::vector<T> result;
        for (int id : ids) {
            auto it = memoryIndex.find(id);
            if (it != memoryIndex.end()) {
                result.push_back(it->second);
            }
        }
        return result;
    }
};

}

#endif