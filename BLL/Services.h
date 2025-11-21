#ifndef SERVICES_H
#define SERVICES_H

#include "Models.h"
#include "DataAccess.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <set>

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

class ValidationException : public BusinessLogicException {
public:
    explicit ValidationException(const std::string& message)
        : BusinessLogicException(message) {}
};

template<typename T>
class IEntityService {
public:
    virtual ~IEntityService() = default;
    virtual std::vector<T> GetAll() const = 0;
    virtual void ClearAll() = 0;
};

template<typename T>
class BaseService : public IEntityService<T> {
protected:
    std::shared_ptr<DAL::IDataStorage<T>> storage;
    std::vector<T> items;

    void LoadData() {
        try {
            items = storage->Load();
        } catch (const DAL::DataAccessException& e) {
            throw BusinessLogicException("Failed to load data: " + std::string(e.what()));
        }
    }

    void SaveData() {
        try {
            storage->Save(items);
        } catch (const DAL::DataAccessException& e) {
            throw BusinessLogicException("Failed to save data: " + std::string(e.what()));
        }
    }

    virtual void ValidateBeforeSave() {}

public:
    explicit BaseService(std::shared_ptr<DAL::IDataStorage<T>> dataStorage)
        : storage(dataStorage) {
        LoadData();
    }

    virtual ~BaseService() = default;

    std::vector<T> GetAll() const override {
        return items;
    }

    void ClearAll() override {
        items.clear();
        SaveData();
    }

    size_t Count() const {
        return items.size();
    }
};

class IIdGenerator {
public:
    virtual ~IIdGenerator() = default;
    virtual int GenerateNext() = 0;
};

class SequentialIdGenerator : public IIdGenerator {
private:
    int nextId;

public:
    SequentialIdGenerator() : nextId(1) {}

    int GenerateNext() override {
        return nextId++;
    }

    void Initialize(int maxExistingId) {
        nextId = maxExistingId + 1;
    }
};

class IStudentValidator {
public:
    virtual ~IStudentValidator() = default;
    virtual void ValidateStudent(const std::string& firstName, const std::string& lastName) const = 0;
    virtual void ValidateGrade(int score) const = 0;
};

class StudentValidator : public IStudentValidator {
public:
    void ValidateStudent(const std::string& firstName, const std::string& lastName) const override {
        if (firstName.empty() || lastName.empty()) {
            throw ValidationException("First name and last name cannot be empty");
        }
        if (firstName.length() > 50 || lastName.length() > 50) {
            throw ValidationException("Name too long (max 50 characters)");
        }
    }

    void ValidateGrade(int score) const override {
        if (score < 0 || score > 100) {
            throw InvalidGradeException("Grade must be between 0 and 100");
        }
    }
};

class IStudentSearchService {
public:
    virtual ~IStudentSearchService() = default;
    virtual std::vector<Student> FindByName(const std::string& firstName, const std::string& lastName) const = 0;
    virtual std::vector<Student> FindByGroup(const std::string& groupName) const = 0;
    virtual std::vector<Student> FindByAverageGrade(double minAvg, double maxAvg) const = 0;
    virtual std::vector<Student> FindByPerformance(bool successful, const std::string& subject) const = 0;
};

class StudentService : public BaseService<Student>, public IStudentSearchService {
private:
    std::unique_ptr<IIdGenerator> idGenerator;
    std::unique_ptr<IStudentValidator> validator;

    int GenerateId() {
        if (items.empty()) {
            return idGenerator->GenerateNext();
        }

        int maxId = 0;
        for (const auto& student : items) {
            if (student.GetId() > maxId) {
                maxId = student.GetId();
            }
        }

        auto* seqGen = dynamic_cast<SequentialIdGenerator*>(idGenerator.get());
        if (seqGen) {
            seqGen->Initialize(maxId);
        }

        return idGenerator->GenerateNext();
    }

    bool IsDuplicate(const std::string& firstName, const std::string& lastName,
                     const std::string& groupName) const {
        for (const auto& s : items) {
            if (s.GetFirstName() == firstName &&
                s.GetLastName() == lastName &&
                s.GetGroupName() == groupName) {
                return true;
            }
        }
        return false;
    }

protected:
    void ValidateBeforeSave() override {
        std::set<int> ids;
        for (const auto& student : items) {
            if (ids.count(student.GetId()) > 0) {
                throw BusinessLogicException("Duplicate student ID detected");
            }
            ids.insert(student.GetId());
        }
    }

public:
    explicit StudentService(std::shared_ptr<DAL::IDataStorage<Student>> dataStorage)
        : BaseService(dataStorage),
          idGenerator(std::make_unique<SequentialIdGenerator>()),
          validator(std::make_unique<StudentValidator>()) {}

    Student AddStudent(const std::string& firstName, const std::string& lastName,
                      const std::string& groupName) {
        validator->ValidateStudent(firstName, lastName);

        if (IsDuplicate(firstName, lastName, groupName)) {
            throw DuplicateEntityException("Student already exists in this group");
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
            throw StudentNotFoundException("Student with ID " + std::to_string(studentId) + " not found");
        }

        items.erase(it);
        SaveData();
    }

    void UpdateStudent(int studentId, const std::string& firstName,
                      const std::string& lastName, const std::string& groupName) {
        auto it = std::find_if(items.begin(), items.end(),
            [studentId](const Student& s) { return s.GetId() == studentId; });

        if (it == items.end()) {
            throw StudentNotFoundException("Student with ID " + std::to_string(studentId) + " not found");
        }

        if (!firstName.empty()) {
            validator->ValidateStudent(firstName, it->GetLastName());
            it->SetFirstName(firstName);
        }
        if (!lastName.empty()) {
            validator->ValidateStudent(it->GetFirstName(), lastName);
            it->SetLastName(lastName);
        }
        if (!groupName.empty()) {
            it->SetGroupName(groupName);
        }

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
        validator->ValidateGrade(score);

        auto student = GetStudentById(studentId);
        if (!student) {
            throw StudentNotFoundException("Student with ID " + std::to_string(studentId) + " not found");
        }

        student->AddGrade(Grade(subject, score));
        SaveData();
    }

    void RemoveGradeFromStudent(int studentId, const std::string& subject) {
        auto student = GetStudentById(studentId);
        if (!student) {
            throw StudentNotFoundException("Student with ID " + std::to_string(studentId) + " not found");
        }

        student->RemoveGrade(subject);
        SaveData();
    }

    std::vector<Student> FindByName(const std::string& firstName,
                                    const std::string& lastName) const override {
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

    std::vector<Student> FindStudentsByName(const std::string& firstName,
                                           const std::string& lastName) const {
        return FindByName(firstName, lastName);
    }

    std::vector<Student> FindByGroup(const std::string& groupName) const override {
        std::vector<Student> result;
        for (const auto& student : items) {
            if (student.GetGroupName() == groupName) {
                result.push_back(student);
            }
        }
        return result;
    }

    std::vector<Student> FindStudentsByGroup(const std::string& groupName) const {
        return FindByGroup(groupName);
    }

    std::vector<Student> FindByAverageGrade(double minAverage, double maxAverage) const override {
        std::vector<Student> result;
        for (const auto& student : items) {
            double avg = student.CalculateAverageGrade();
            if (avg >= minAverage && avg <= maxAverage) {
                result.push_back(student);
            }
        }
        return result;
    }

    std::vector<Student> FindStudentsByAverageGrade(double minAverage, double maxAverage) const {
        return FindByAverageGrade(minAverage, maxAverage);
    }

    std::vector<Student> FindByPerformance(bool successful,
                                          const std::string& subject = "") const override {
        std::vector<Student> result;
        const double threshold = 60.0;

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

    std::vector<Student> FindStudentsByPerformance(bool successful,
                                                   const std::string& subject = "") const {
        return FindByPerformance(successful, subject);
    }

    double CalculateGroupAverageGrade(const std::string& groupName) const {
        auto groupStudents = FindByGroup(groupName);
        if (groupStudents.empty()) return 0.0;

        double sum = 0.0;
        for (const auto& student : groupStudents) {
            sum += student.CalculateAverageGrade();
        }
        return sum / groupStudents.size();
    }
};

class IGroupValidator {
public:
    virtual ~IGroupValidator() = default;
    virtual void ValidateGroup(const std::string& name) const = 0;
};

class GroupValidator : public IGroupValidator {
public:
    void ValidateGroup(const std::string& name) const override {
        if (name.empty()) {
            throw ValidationException("Group name cannot be empty");
        }
        if (name.length() > 20) {
            throw ValidationException("Group name too long (max 20 characters)");
        }
    }
};

class GroupService : public BaseService<Group> {
private:
    std::unique_ptr<IGroupValidator> validator;

    bool IsDuplicate(const std::string& name) const {
        for (const auto& g : items) {
            if (g.GetName() == name) {
                return true;
            }
        }
        return false;
    }

public:
    explicit GroupService(std::shared_ptr<DAL::IDataStorage<Group>> dataStorage)
        : BaseService(dataStorage),
          validator(std::make_unique<GroupValidator>()) {}

    Group AddGroup(const std::string& name, const std::string& specialization, int year) {
        validator->ValidateGroup(name);

        if (IsDuplicate(name)) {
            throw DuplicateEntityException("Group with name '" + name + "' already exists");
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
            throw GroupNotFoundException("Group '" + name + "' not found");
        }

        items.erase(it);
        SaveData();
    }

    void UpdateGroup(const std::string& name, const std::string& specialization, int year) {
        auto it = std::find_if(items.begin(), items.end(),
            [&name](const Group& g) { return g.GetName() == name; });

        if (it == items.end()) {
            throw GroupNotFoundException("Group '" + name + "' not found");
        }

        if (!specialization.empty()) {
            it->SetSpecialization(specialization);
        }
        if (year > 0) {
            it->SetYear(year);
        }

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