#ifndef STORAGEFACTORY_H
#define STORAGEFACTORY_H

#include "DataAccess.h"
#include "WALJsonStorage.h"
//#include "SqliteStorage.h"
#include <memory>
#include <string>

#include "JsonStorage.h"

namespace DAL {

enum class StorageType {
    Simple,
    WAL,
    Sqlite
};

template<typename T>
class UniversalStorageAdapter : public IDataStorage<T> {
private:
    StorageType type;
    std::shared_ptr<void> storage;

public:
    UniversalStorageAdapter(StorageType t, std::shared_ptr<void> s)
        : type(t), storage(s) {}

    void Save(const std::vector<T>& items) override {
        switch (type) {
            case StorageType::Simple: {
                auto s = std::static_pointer_cast<JsonStorage<T>>(storage);
                s->Save(items);
                break;
            }
            case StorageType::WAL: {
                auto s = std::static_pointer_cast<WALJsonStorage<T>>(storage);
                s->Save(items);
                break;
            }
            /*case StorageType::Sqlite: {
                auto s = std::static_pointer_cast<SqliteStorage<T>>(storage);
                s->Save(items);
                break;
            }*/
        }
    }

    std::vector<T> Load() override {
        switch (type) {
            case StorageType::Simple: {
                auto s = std::static_pointer_cast<JsonStorage<T>>(storage);
                return s->Load();
            }
            case StorageType::WAL: {
                auto s = std::static_pointer_cast<WALJsonStorage<T>>(storage);
                return s->LoadAll();
            }
            /*case StorageType::Sqlite: {
                auto s = std::static_pointer_cast<SqliteStorage<T>>(storage);
                return s->LoadAll();
            }*/
        }
        return std::vector<T>();
    }

    void Clear() override {
        switch (type) {
            case StorageType::Simple: {
                auto s = std::static_pointer_cast<JsonStorage<T>>(storage);
                s->Clear();
                break;
            }
            case StorageType::WAL: {
                auto s = std::static_pointer_cast<WALJsonStorage<T>>(storage);
                s->Clear();
                break;
            }
            /*case StorageType::Sqlite: {
                auto s = std::static_pointer_cast<SqliteStorage<T>>(storage);
                s->Clear();
                break;
            }*/
        }
    }
};

template<typename T>
class StorageFactory {
public:
    static std::shared_ptr<IDataStorage<T>> Create(
        StorageType type,
        const std::string& path) {

        std::shared_ptr<void> storage;

        switch (type) {
            case StorageType::Simple:
                storage = std::make_shared<JsonStorage<T>>(path);
                break;

            case StorageType::WAL:
                storage = std::make_shared<WALJsonStorage<T>>(path, 50);
                break;

            /*case StorageType::Sqlite:
                storage = std::make_shared<SqliteStorage<T>>(path, "data");
                break;*/

            default:
                storage = std::make_shared<JsonStorage<T>>(path);
        }

        return std::make_shared<UniversalStorageAdapter<T>>(type, storage);
    }

    static std::string GetRecommendation(int expectedRecords, int writesPerSecond) {
        if (expectedRecords < 1000 && writesPerSecond < 10) {
            return "Simple - sufficient for small datasets";
        } else if (expectedRecords < 100000 && writesPerSecond < 100) {
            return "WAL - good balance for frequent updates";
        } else if (expectedRecords > 100000) {
            return "SQLite - required for large datasets";
        }
        return "WAL - good default choice";
    }
};

}

#endif