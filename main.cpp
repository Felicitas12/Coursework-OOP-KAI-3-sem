#include "ConsoleInterface.h"
#include <iostream>
#include <memory>

#include "StorageFactory.h"

int main() {
    try {
        auto groupRepo = std::make_shared<DAL::JsonStorage<BLL::Group>>("groups.json");

        auto storage = DAL::StorageFactory<BLL::Student>::Create(
            DAL::StorageType::WAL,
            "students.json"
        );
        auto studentService = std::make_shared<BLL::StudentService>(storage);
        auto groupService = std::make_shared<BLL::GroupService>(groupRepo);

        PL::ConsoleInterface interface(studentService, groupService);
        interface.Run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
