#include "ConsoleInterface.h"
#include <iostream>
#include <memory>

int main() {
    try {
        auto studentRepo = std::make_shared<DAL::JsonStorage<BLL::Student>>("students.json");
        auto groupRepo = std::make_shared<DAL::JsonStorage<BLL::Group>>("groups.json");

        auto studentService = std::make_shared<BLL::StudentService>(studentRepo);
        auto groupService = std::make_shared<BLL::GroupService>(groupRepo);

        PL::ConsoleInterface interface(studentService, groupService);
        interface.Run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}