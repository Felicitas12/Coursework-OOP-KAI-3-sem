#ifndef SERVICES_H
#define SERVICES_H

#include "Models.h"
#include "DataAccess.h"
#include <algorithm>
#include <memory>
#include <stdexcept>

namespace BLL {

class BusinessLogicException : public std::runtime_error {
public:
    explicit BusinessLogicException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidGradeException : public BusinessLogicException {
public:
    explicit InvalidGradeException(const std::string& message)
        : BusinessLogicException(message) {}
};

class StudentNotFoundException : public BusinessLogicException {
public:
    explicit StudentNotFoundException(const std::string& message)
        : BusinessLogicException(message) {}
};

class GroupNotFoundException : public BusinessLogicException {
public:
    explicit GroupNotFoundException(const std::string& message)
        : BusinessLogicException(message) {}
};

class DuplicateEntityException : public BusinessLogicException {
public:
    explicit DuplicateEntityException(const std::string& message)
        : BusinessLogicException(message) {}
};

template<typename T>
class BaseService {
protected:
    std::shared_ptr<DAL::IRepository<T>> repository;
    std::vector<T> items;

    void LoadData() {
        try {
            items = repository->Load();
        } catch (const DAL::DataAccessException& e) {
            throw BusinessLogicException("Failed to load data: " + std::string(e.what()));
        }
    }

    void SaveData() {
        try {
            repository->Save(items);
        } catch (const DAL::DataAccessException& e) {
            throw BusinessLogicException("Failed to save data: " + std::string(e.what()));
        }
    }

public:
    explicit BaseService(std::shared_ptr<DAL::IRepository<T>> repo)
        : repository(repo) {
        LoadData();
    }

    virtual ~BaseService() = default;

    std::vector<T> GetAll() const {
        return items;
    }

    void ClearAll() {
        items.clear();
        SaveData();
    }
};

class StudentService : public BaseService<Student> {
private:
    int nextId;

    int GenerateId() {
        if (items.empty()) {
            nextId = 1;
        } else {
            int maxId = 0;
            for (const auto& student : items) {
                if (student.GetId() > maxId) {
                    maxId = student.GetId();
                }
            }
            nextId = maxId + 1;
        }
        return nextId++;
    }

public:
    explicit StudentService(std::shared_ptr<DAL::IRepository<Student>> repo)
        : BaseService(repo), nextId(1) {}

    Student AddStudent(const std::string& firstName, const std::string& lastName,
                      const std::string& groupName) {
        if (firstName.empty() || lastName.empty()) {
            throw BusinessLogicException("First name and last name cannot be empty");
        }

        for (const auto& s : items) {
            if (s.GetFirstName() == firstName && s.GetLastName() == lastName
                && s.GetGroupName() == groupName) {
                throw DuplicateEntityException("Student already exists in this group");
            }
        }

        Student student(GenerateId(), firstName, lastName, groupName);
        items.push_back(student);
        SaveData();
        return student;
    }

    void RemoveStudent(int studentId) {
        auto it = std::find_if(items.begin(), items.end(),
            [studentId](const Student& s) { return s.GetId() == studentId; });

        if (it == items.end()) {
            throw StudentNotFoundException("Student not found");
        }

        items.erase(it);
        SaveData();
    }

    void UpdateStudent(int studentId, const std::string& firstName,
                      const std::string& lastName, const std::string& groupName) {
        auto it = std::find_if(items.begin(), items.end(),
            [studentId](const Student& s) { return s.GetId() == studentId; });

        if (it == items.end()) {
            throw StudentNotFoundException("Student not found");
        }

        if (!firstName.empty()) it->SetFirstName(firstName);
        if (!lastName.empty()) it->SetLastName(lastName);
        if (!groupName.empty()) it->SetGroupName(groupName);

        SaveData();
    }

    Student* GetStudentById(int studentId) {
        for (auto& student : items) {
            if (student.GetId() == studentId) {
                return &student;
            }
        }
        return nullptr;
    }

    void AddGradeToStudent(int studentId, const std::string& subject, int score) {
        if (score < 0 || score > 100) {
            throw InvalidGradeException("Grade must be between 0 and 100");
        }

        auto student = GetStudentById(studentId);
        if (!student) {
            throw StudentNotFoundException("Student not found");
        }

        student->AddGrade(Grade(subject, score));
        SaveData();
    }

    void RemoveGradeFromStudent(int studentId, const std::string& subject) {
        auto student = GetStudentById(studentId);
        if (!student) {
            throw StudentNotFoundException("Student not found");
        }

        student->RemoveGrade(subject);
        SaveData();
    }

    std::vector<Student> FindStudentsByName(const std::string& firstName,
                                           const std::string& lastName) {
        std::vector<Student> result;
        for (const auto& student : items) {
            bool matchFirst = firstName.empty() ||
                student.GetFirstName().find(firstName) != std::string::npos;
            bool matchLast = lastName.empty() ||
                student.GetLastName().find(lastName) != std::string::npos;

            if (matchFirst && matchLast) {
                result.push_back(student);
            }
        }
        return result;
    }

    std::vector<Student> FindStudentsByGroup(const std::string& groupName) {
        std::vector<Student> result;
        for (const auto& student : items) {
            if (student.GetGroupName() == groupName) {
                result.push_back(student);
            }
        }
        return result;
    }

    std::vector<Student> FindStudentsByAverageGrade(double minAverage, double maxAverage) {
        std::vector<Student> result;
        for (const auto& student : items) {
            double avg = student.CalculateAverageGrade();
            if (avg >= minAverage && avg <= maxAverage) {
                result.push_back(student);
            }
        }
        return result;
    }

    std::vector<Student> FindStudentsByPerformance(bool successful,
                                                   const std::string& subject = "") {
        std::vector<Student> result;
        double threshold = 60.0;

        for (const auto& student : items) {
            if (subject.empty()) {
                double avg = student.CalculateAverageGrade();
                if ((successful && avg >= threshold) || (!successful && avg < threshold && avg > 0)) {
                    result.push_back(student);
                }
            } else {
                auto grades = student.GetGrades();
                for (const auto& grade : grades) {
                    if (grade.GetSubject() == subject) {
                        if ((successful && grade.GetScore() >= threshold) ||
                            (!successful && grade.GetScore() < threshold)) {
                            result.push_back(student);
                        }
                        break;
                    }
                }
            }
        }
        return result;
    }

    double CalculateGroupAverageGrade(const std::string& groupName) {
        auto groupStudents = FindStudentsByGroup(groupName);
        if (groupStudents.empty()) return 0.0;

        double sum = 0.0;
        for (const auto& student : groupStudents) {
            sum += student.CalculateAverageGrade();
        }
        return sum / groupStudents.size();
    }
};

class GroupService : public BaseService<Group> {
public:
    explicit GroupService(std::shared_ptr<DAL::IRepository<Group>> repo)
        : BaseService(repo) {}

    Group AddGroup(const std::string& name, const std::string& specialization, int year) {
        if (name.empty()) {
            throw BusinessLogicException("Group name cannot be empty");
        }

        for (const auto& g : items) {
            if (g.GetName() == name) {
                throw DuplicateEntityException("Group already exists");
            }
        }

        Group group(name, specialization, year);
        items.push_back(group);
        SaveData();
        return group;
    }

    void RemoveGroup(const std::string& name) {
        auto it = std::find_if(items.begin(), items.end(),
            [&name](const Group& g) { return g.GetName() == name; });

        if (it == items.end()) {
            throw GroupNotFoundException("Group not found");
        }

        items.erase(it);
        SaveData();
    }

    void UpdateGroup(const std::string& name, const std::string& specialization, int year) {
        auto it = std::find_if(items.begin(), items.end(),
            [&name](const Group& g) { return g.GetName() == name; });

        if (it == items.end()) {
            throw GroupNotFoundException("Group not found");
        }

        if (!specialization.empty()) it->SetSpecialization(specialization);
        if (year > 0) it->SetYear(year);

        SaveData();
    }

    Group* GetGroupByName(const std::string& name) {
        for (auto& group : items) {
            if (group.GetName() == name) {
                return &group;
            }
        }
        return nullptr;
    }
};

}

#endif