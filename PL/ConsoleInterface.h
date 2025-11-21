#ifndef CONSOLEINTERFACE_H
#define CONSOLEINTERFACE_H

#include "Services.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace PL {

class ConsoleInterface {
private:
    std::shared_ptr<BLL::StudentService> studentService;
    std::shared_ptr<BLL::GroupService> groupService;

    void ClearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void PauseScreen() {
        std::cout << "\n\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    std::string GetStringInput(const std::string& prompt) {
        std::string input;
        std::cout << prompt;
        std::getline(std::cin, input);
        return input;
    }

    int GetIntInput(const std::string& prompt) {
        int value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    double GetDoubleInput(const std::string& prompt) {
        double value;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    void DisplayStudent(const BLL::Student& student) {
        std::cout << "ID: " << student.GetId()
                  << " | Name: " << student.GetFirstName() << " " << student.GetLastName()
                  << " | Group: " << student.GetGroupName()
                  << " | Avg: " << std::fixed << std::setprecision(2)
                  << student.CalculateAverageGrade() << "\n";
    }

    void DisplayStudentDetailed(const BLL::Student& student) {
        std::cout << "\n=== Student Details ===\n";
        std::cout << "ID: " << student.GetId() << "\n";
        std::cout << "Name: " << student.GetFirstName() << " " << student.GetLastName() << "\n";
        std::cout << "Group: " << student.GetGroupName() << "\n";
        std::cout << "Average Grade: " << std::fixed << std::setprecision(2)
                  << student.CalculateAverageGrade() << "\n";
        std::cout << "\nGrades:\n";
        auto grades = student.GetGrades();
        if (grades.empty()) {
            std::cout << "  No grades recorded\n";
        } else {
            for (const auto& grade : grades) {
                std::cout << "  " << std::left << std::setw(30) << grade.GetSubject()
                          << ": " << grade.GetScore() << "\n";
            }
        }
    }

    void DisplayGroup(const BLL::Group& group) {
        std::cout << "Name: " << group.GetName()
                  << " | Specialization: " << group.GetSpecialization()
                  << " | Year: " << group.GetYear() << "\n";
    }

    void StudentManagementMenu() {
        while (true) {
            ClearScreen();
            std::cout << "\n=== STUDENT MANAGEMENT ===\n";
            std::cout << "1. Add Student\n";
            std::cout << "2. Remove Student\n";
            std::cout << "3. Update Student\n";
            std::cout << "4. View All Students\n";
            std::cout << "5. View Student Details\n";
            std::cout << "0. Back\n";
            std::cout << "Choice: ";

            int choice = GetIntInput("");

            try {
                switch (choice) {
                    case 1: AddStudentMenu(); break;
                    case 2: RemoveStudentMenu(); break;
                    case 3: UpdateStudentMenu(); break;
                    case 4: ViewAllStudentsMenu(); break;
                    case 5: ViewStudentDetailsMenu(); break;
                    case 0: return;
                    default: std::cout << "Invalid choice!\n"; PauseScreen();
                }
            } catch (const BLL::BusinessLogicException& e) {
                std::cout << "Error: " << e.what() << "\n";
                PauseScreen();
            }
        }
    }

    void AddStudentMenu() {
        ClearScreen();
        std::cout << "\n=== ADD STUDENT ===\n";

        std::string firstName = GetStringInput("First Name: ");
        std::string lastName = GetStringInput("Last Name: ");
        std::string groupName = GetStringInput("Group Name: ");

        if (firstName.empty() || lastName.empty()) {
            std::cout << "First name and last name cannot be empty!\n";
            PauseScreen();
            return;
        }

        auto student = studentService->AddStudent(firstName, lastName, groupName);
        std::cout << "\nStudent added successfully! ID: " << student.GetId() << "\n";
        PauseScreen();
    }

    void RemoveStudentMenu() {
        ClearScreen();
        std::cout << "\n=== REMOVE STUDENT ===\n";

        int studentId = GetIntInput("Student ID: ");
        studentService->RemoveStudent(studentId);

        std::cout << "\nStudent removed successfully!\n";
        PauseScreen();
    }

    void UpdateStudentMenu() {
        ClearScreen();
        std::cout << "\n=== UPDATE STUDENT ===\n";

        int studentId = GetIntInput("Student ID: ");
        auto student = studentService->GetStudentById(studentId);

        if (!student) {
            std::cout << "Student not found!\n";
            PauseScreen();
            return;
        }

        std::cout << "\nCurrent data:\n";
        DisplayStudentDetailed(*student);
        std::cout << "\nEnter new data (leave empty to keep current):\n";

        std::string firstName = GetStringInput("First Name: ");
        std::string lastName = GetStringInput("Last Name: ");
        std::string groupName = GetStringInput("Group Name: ");

        studentService->UpdateStudent(studentId, firstName, lastName, groupName);
        std::cout << "\nStudent updated successfully!\n";
        PauseScreen();
    }

    void ViewAllStudentsMenu() {
        ClearScreen();
        std::cout << "\n=== ALL STUDENTS ===\n";

        auto students = studentService->GetAll();
        if (students.empty()) {
            std::cout << "No students found.\n";
        } else {
            for (const auto& student : students) {
                DisplayStudent(student);
            }
        }
        PauseScreen();
    }

    void ViewStudentDetailsMenu() {
        ClearScreen();
        std::cout << "\n=== STUDENT DETAILS ===\n";

        int studentId = GetIntInput("Student ID: ");
        auto student = studentService->GetStudentById(studentId);

        if (!student) {
            std::cout << "Student not found!\n";
        } else {
            DisplayStudentDetailed(*student);
        }
        PauseScreen();
    }

    void GroupManagementMenu() {
        while (true) {
            ClearScreen();
            std::cout << "\n=== GROUP MANAGEMENT ===\n";
            std::cout << "1. Add Group\n";
            std::cout << "2. Remove Group\n";
            std::cout << "3. Update Group\n";
            std::cout << "4. View All Groups\n";
            std::cout << "5. View Group Details\n";
            std::cout << "0. Back\n";
            std::cout << "Choice: ";

            int choice = GetIntInput("");

            try {
                switch (choice) {
                    case 1: AddGroupMenu(); break;
                    case 2: RemoveGroupMenu(); break;
                    case 3: UpdateGroupMenu(); break;
                    case 4: ViewAllGroupsMenu(); break;
                    case 5: ViewGroupDetailsMenu(); break;
                    case 0: return;
                    default: std::cout << "Invalid choice!\n"; PauseScreen();
                }
            } catch (const BLL::BusinessLogicException& e) {
                std::cout << "Error: " << e.what() << "\n";
                PauseScreen();
            }
        }
    }

    void AddGroupMenu() {
        ClearScreen();
        std::cout << "\n=== ADD GROUP ===\n";

        std::string name = GetStringInput("Group Name: ");
        std::string specialization = GetStringInput("Specialization: ");
        int year = GetIntInput("Year: ");

        if (name.empty()) {
            std::cout << "Group name cannot be empty!\n";
            PauseScreen();
            return;
        }

        groupService->AddGroup(name, specialization, year);
        std::cout << "\nGroup added successfully!\n";
        PauseScreen();
    }

    void RemoveGroupMenu() {
        ClearScreen();
        std::cout << "\n=== REMOVE GROUP ===\n";

        std::string name = GetStringInput("Group Name: ");
        groupService->RemoveGroup(name);

        std::cout << "\nGroup removed successfully!\n";
        PauseScreen();
    }

    void UpdateGroupMenu() {
        ClearScreen();
        std::cout << "\n=== UPDATE GROUP ===\n";

        std::string name = GetStringInput("Group Name: ");
        auto group = groupService->GetGroupByName(name);

        if (!group) {
            std::cout << "Group not found!\n";
            PauseScreen();
            return;
        }

        std::cout << "\nCurrent data:\n";
        DisplayGroup(*group);
        std::cout << "\nEnter new data (leave empty to keep current):\n";

        std::string specialization = GetStringInput("Specialization: ");
        int year = GetIntInput("Year (0 to keep current): ");

        groupService->UpdateGroup(name, specialization, year);
        std::cout << "\nGroup updated successfully!\n";
        PauseScreen();
    }

    void ViewAllGroupsMenu() {
        ClearScreen();
        std::cout << "\n=== ALL GROUPS ===\n";

        auto groups = groupService->GetAll();
        if (groups.empty()) {
            std::cout << "No groups found.\n";
        } else {
            for (const auto& group : groups) {
                DisplayGroup(group);
            }
        }
        PauseScreen();
    }

    void ViewGroupDetailsMenu() {
        ClearScreen();
        std::cout << "\n=== GROUP DETAILS ===\n";

        std::string name = GetStringInput("Group Name: ");
        auto group = groupService->GetGroupByName(name);

        if (!group) {
            std::cout << "Group not found!\n";
            PauseScreen();
            return;
        }

        std::cout << "\n";
        DisplayGroup(*group);

        auto students = studentService->FindStudentsByGroup(name);
        std::cout << "\nStudents in group (" << students.size() << "):\n";
        if (students.empty()) {
            std::cout << "  No students in this group\n";
        } else {
            for (const auto& student : students) {
                DisplayStudent(student);
            }
            double avgGrade = studentService->CalculateGroupAverageGrade(name);
            std::cout << "\nGroup Average Grade: " << std::fixed
                      << std::setprecision(2) << avgGrade << "\n";
        }
        PauseScreen();
    }

    void GradeManagementMenu() {
        while (true) {
            ClearScreen();
            std::cout << "\n=== GRADE MANAGEMENT ===\n";
            std::cout << "1. Add/Update Grade\n";
            std::cout << "2. Remove Grade\n";
            std::cout << "3. View Student Grades\n";
            std::cout << "4. View Grades by Subject\n";
            std::cout << "0. Back\n";
            std::cout << "Choice: ";

            int choice = GetIntInput("");

            try {
                switch (choice) {
                    case 1: AddGradeMenu(); break;
                    case 2: RemoveGradeMenu(); break;
                    case 3: ViewStudentGradesMenu(); break;
                    case 4: ViewGradesBySubjectMenu(); break;
                    case 0: return;
                    default: std::cout << "Invalid choice!\n"; PauseScreen();
                }
            } catch (const BLL::BusinessLogicException& e) {
                std::cout << "Error: " << e.what() << "\n";
                PauseScreen();
            }
        }
    }

    void AddGradeMenu() {
        ClearScreen();
        std::cout << "\n=== ADD/UPDATE GRADE ===\n";

        int studentId = GetIntInput("Student ID: ");
        std::string subject = GetStringInput("Subject: ");
        int score = GetIntInput("Score (0-100): ");

        if (score < 0 || score > 100) {
            std::cout << "Score must be between 0 and 100!\n";
            PauseScreen();
            return;
        }

        studentService->AddGradeToStudent(studentId, subject, score);
        std::cout << "\nGrade added/updated successfully!\n";
        PauseScreen();
    }

    void RemoveGradeMenu() {
        ClearScreen();
        std::cout << "\n=== REMOVE GRADE ===\n";

        int studentId = GetIntInput("Student ID: ");
        std::string subject = GetStringInput("Subject: ");

        studentService->RemoveGradeFromStudent(studentId, subject);
        std::cout << "\nGrade removed successfully!\n";
        PauseScreen();
    }

    void ViewStudentGradesMenu() {
        ClearScreen();
        std::cout << "\n=== STUDENT GRADES ===\n";

        int studentId = GetIntInput("Student ID: ");
        auto student = studentService->GetStudentById(studentId);

        if (!student) {
            std::cout << "Student not found!\n";
        } else {
            DisplayStudentDetailed(*student);
        }
        PauseScreen();
    }

    void ViewGradesBySubjectMenu() {
        ClearScreen();
        std::cout << "\n=== GRADES BY SUBJECT ===\n";

        std::string subject = GetStringInput("Subject: ");

        auto students = studentService->GetAll();
        bool found = false;

        std::cout << "\nGrades for subject: " << subject << "\n";
        std::cout << std::string(60, '-') << "\n";

        for (const auto& student : students) {
            auto grades = student.GetGrades();
            for (const auto& grade : grades) {
                if (grade.GetSubject() == subject) {
                    std::cout << std::left << std::setw(25)
                              << (student.GetFirstName() + " " + student.GetLastName())
                              << " | Group: " << std::setw(10) << student.GetGroupName()
                              << " | Score: " << grade.GetScore() << "\n";
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cout << "No grades found for this subject.\n";
        }
        PauseScreen();
    }

    void SearchMenu() {
        while (true) {
            ClearScreen();
            std::cout << "\n=== SEARCH ===\n";
            std::cout << "1. Search by Name\n";
            std::cout << "2. Search by Group\n";
            std::cout << "3. Search by Average Grade\n";
            std::cout << "4. Search Successful/Unsuccessful Students\n";
            std::cout << "0. Back\n";
            std::cout << "Choice: ";

            int choice = GetIntInput("");

            try {
                switch (choice) {
                    case 1: SearchByNameMenu(); break;
                    case 2: SearchByGroupMenu(); break;
                    case 3: SearchByAverageGradeMenu(); break;
                    case 4: SearchByPerformanceMenu(); break;
                    case 0: return;
                    default: std::cout << "Invalid choice!\n"; PauseScreen();
                }
            } catch (const BLL::BusinessLogicException& e) {
                std::cout << "Error: " << e.what() << "\n";
                PauseScreen();
            }
        }
    }

    void SearchByNameMenu() {
        ClearScreen();
        std::cout << "\n=== SEARCH BY NAME ===\n";

        std::string firstName = GetStringInput("First Name (optional): ");
        std::string lastName = GetStringInput("Last Name (optional): ");

        auto students = studentService->FindStudentsByName(firstName, lastName);

        std::cout << "\nFound " << students.size() << " student(s):\n";
        for (const auto& student : students) {
            DisplayStudent(student);
        }
        PauseScreen();
    }

    void SearchByGroupMenu() {
        ClearScreen();
        std::cout << "\n=== SEARCH BY GROUP ===\n";

        std::string groupName = GetStringInput("Group Name: ");

        auto students = studentService->FindStudentsByGroup(groupName);

        std::cout << "\nFound " << students.size() << " student(s):\n";
        for (const auto& student : students) {
            DisplayStudent(student);
        }
        PauseScreen();
    }

    void SearchByAverageGradeMenu() {
        ClearScreen();
        std::cout << "\n=== SEARCH BY AVERAGE GRADE ===\n";

        double minGrade = GetDoubleInput("Minimum Average Grade: ");
        double maxGrade = GetDoubleInput("Maximum Average Grade: ");

        auto students = studentService->FindStudentsByAverageGrade(minGrade, maxGrade);

        std::cout << "\nFound " << students.size() << " student(s):\n";
        for (const auto& student : students) {
            DisplayStudent(student);
        }
        PauseScreen();
    }

    void SearchByPerformanceMenu() {
        ClearScreen();
        std::cout << "\n=== SEARCH BY PERFORMANCE ===\n";
        std::cout << "1. Successful students (avg >= 60)\n";
        std::cout << "2. Unsuccessful students (avg < 60)\n";
        std::cout << "3. Successful by subject\n";
        std::cout << "4. Unsuccessful by subject\n";

        int choice = GetIntInput("\nChoice: ");

        std::vector<BLL::Student> students;

        if (choice == 1) {
            students = studentService->FindStudentsByPerformance(true);
        } else if (choice == 2) {
            students = studentService->FindStudentsByPerformance(false);
        } else if (choice == 3 || choice == 4) {
            std::string subject = GetStringInput("Subject: ");
            students = studentService->FindStudentsByPerformance(choice == 3, subject);
        } else {
            std::cout << "Invalid choice!\n";
            PauseScreen();
            return;
        }

        std::cout << "\nFound " << students.size() << " student(s):\n";
        for (const auto& student : students) {
            DisplayStudent(student);
        }
        PauseScreen();
    }

public:
    ConsoleInterface(std::shared_ptr<BLL::StudentService> studServ,
                    std::shared_ptr<BLL::GroupService> grpServ)
        : studentService(studServ), groupService(grpServ) {}

    void Run() {
        while (true) {
            ClearScreen();
            std::cout << "\n=== ELECTRONIC GRADE JOURNAL ===\n";
            std::cout << "1. Student Management\n";
            std::cout << "2. Group Management\n";
            std::cout << "3. Grade Management\n";
            std::cout << "4. Search\n";
            std::cout << "0. Exit\n";
            std::cout << "Choice: ";

            int choice = GetIntInput("");

            try {
                switch (choice) {
                    case 1: StudentManagementMenu(); break;
                    case 2: GroupManagementMenu(); break;
                    case 3: GradeManagementMenu(); break;
                    case 4: SearchMenu(); break;
                    case 0:
                        std::cout << "\nGoodbye!\n";
                        return;
                    default:
                        std::cout << "Invalid choice!\n";
                        PauseScreen();
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
                PauseScreen();
            }
        }
    }
};

}

#endif