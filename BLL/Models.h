#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace BLL {

class IGradeCalculator {
public:
    virtual ~IGradeCalculator() = default;
    virtual double CalculateAverage() const = 0;
};

class IIdentifiable {
public:
    virtual ~IIdentifiable() = default;
    virtual int GetId() const = 0;
};

class Grade {
private:
    std::string subject;
    int score;

    void ValidateScore(int sc) const {
        if (sc < 0 || sc > 100) {
            throw std::invalid_argument("Score must be between 0 and 100");
        }
    }

public:
    Grade() : subject(""), score(0) {}

    Grade(const std::string& subj, int sc) : subject(subj), score(0) {
        ValidateScore(sc);
        score = sc;
    }

    std::string GetSubject() const { return subject; }
    int GetScore() const { return score; }

    void SetScore(int sc) {
        ValidateScore(sc);
        score = sc;
    }

    json ToJson() const {
        return json{
            {"subject", subject},
            {"score", score}
        };
    }

    static Grade FromJson(const json& j) {
        return Grade(
            j.value("subject", ""),
            j.value("score", 0)
        );
    }
};

class Student : public IIdentifiable, public IGradeCalculator {
private:
    int id;
    std::string firstName;
    std::string lastName;
    std::string groupName;
    std::vector<Grade> grades;

    void ValidateName(const std::string& name, const std::string& fieldName) const {
        if (name.empty()) {
            throw std::invalid_argument(fieldName + " cannot be empty");
        }
    }

public:
    Student() : id(0), firstName(""), lastName(""), groupName("") {}

    Student(int studentId, const std::string& first, const std::string& last,
            const std::string& group)
        : id(studentId), firstName(first), lastName(last), groupName(group) {
        ValidateName(first, "First name");
        ValidateName(last, "Last name");
    }

    int GetId() const override { return id; }
    std::string GetFirstName() const { return firstName; }
    std::string GetLastName() const { return lastName; }
    std::string GetGroupName() const { return groupName; }
    std::vector<Grade> GetGrades() const { return grades; }

    std::string GetFullName() const {
        return firstName + " " + lastName;
    }

    void SetFirstName(const std::string& name) {
        ValidateName(name, "First name");
        firstName = name;
    }

    void SetLastName(const std::string& name) {
        ValidateName(name, "Last name");
        lastName = name;
    }

    void SetGroupName(const std::string& group) {
        groupName = group;
    }

    void AddGrade(const Grade& grade) {
        for (auto& g : grades) {
            if (g.GetSubject() == grade.GetSubject()) {
                g.SetScore(grade.GetScore());
                return;
            }
        }
        grades.push_back(grade);
    }

    void RemoveGrade(const std::string& subject) {
        grades.erase(
            std::remove_if(grades.begin(), grades.end(),
                [&subject](const Grade& g) { return g.GetSubject() == subject; }),
            grades.end()
        );
    }

    double CalculateAverage() const override {
        if (grades.empty()) return 0.0;
        int sum = 0;
        for (const auto& grade : grades) {
            sum += grade.GetScore();
        }
        return static_cast<double>(sum) / grades.size();
    }

    double CalculateAverageGrade() const {
        return CalculateAverage();
    }

    Grade* GetGradeBySubject(const std::string& subject) {
        for (auto& grade : grades) {
            if (grade.GetSubject() == subject) {
                return &grade;
            }
        }
        return nullptr;
    }

    bool HasGrade(const std::string& subject) const {
        for (const auto& grade : grades) {
            if (grade.GetSubject() == subject) {
                return true;
            }
        }
        return false;
    }

    json ToJson() const {
        json j = {
            {"id", id},
            {"firstName", firstName},
            {"lastName", lastName},
            {"groupName", groupName},
            {"grades", json::array()}
        };
        for (const auto& grade : grades) {
            j["grades"].push_back(grade.ToJson());
        }
        return j;
    }

    static Student FromJson(const json& j) {
        Student student(
            j.value("id", 0),
            j.value("firstName", ""),
            j.value("lastName", ""),
            j.value("groupName", "")
        );
        if (j.contains("grades") && j["grades"].is_array()) {
            for (const auto& gradeJson : j["grades"]) {
                student.AddGrade(Grade::FromJson(gradeJson));
            }
        }
        return student;
    }
};

class Group {
private:
    std::string name;
    std::string specialization;
    int year;

    void ValidateName(const std::string& groupName) const {
        if (groupName.empty()) {
            throw std::invalid_argument("Group name cannot be empty");
        }
    }

    void ValidateYear(int y) const {
        if (y < 1 || y > 6) {
            throw std::invalid_argument("Year must be between 1 and 6");
        }
    }

public:
    Group() : name(""), specialization(""), year(0) {}

    Group(const std::string& groupName, const std::string& spec, int y)
        : name(groupName), specialization(spec), year(y) {
        ValidateName(groupName);
        if (y > 0) {
            ValidateYear(y);
        }
    }

    std::string GetName() const { return name; }
    std::string GetSpecialization() const { return specialization; }
    int GetYear() const { return year; }

    void SetSpecialization(const std::string& spec) {
        specialization = spec;
    }

    void SetYear(int y) {
        if (y > 0) {
            ValidateYear(y);
        }
        year = y;
    }

    json ToJson() const {
        return json{
            {"name", name},
            {"specialization", specialization},
            {"year", year}
        };
    }

    static Group FromJson(const json& j) {
        return Group(
            j.value("name", ""),
            j.value("specialization", ""),
            j.value("year", 0)
        );
    }
};

}

#endif