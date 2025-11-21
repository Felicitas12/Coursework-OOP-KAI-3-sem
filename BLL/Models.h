#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace BLL {

class Grade {
private:
    std::string subject;
    int score;

public:
    Grade() : subject(""), score(0) {}
    Grade(const std::string& subj, int sc) : subject(subj), score(sc) {}

    std::string GetSubject() const { return subject; }
    int GetScore() const { return score; }
    void SetScore(int sc) { score = sc; }

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

class Student {
private:
    int id;
    std::string firstName;
    std::string lastName;
    std::string groupName;
    std::vector<Grade> grades;

public:
    Student() : id(0), firstName(""), lastName(""), groupName("") {}

    Student(int studentId, const std::string& first, const std::string& last,
            const std::string& group)
        : id(studentId), firstName(first), lastName(last), groupName(group) {}

    int GetId() const { return id; }
    std::string GetFirstName() const { return firstName; }
    std::string GetLastName() const { return lastName; }
    std::string GetGroupName() const { return groupName; }
    std::vector<Grade> GetGrades() const { return grades; }

    void SetFirstName(const std::string& name) { firstName = name; }
    void SetLastName(const std::string& name) { lastName = name; }
    void SetGroupName(const std::string& group) { groupName = group; }

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

    double CalculateAverageGrade() const {
        if (grades.empty()) return 0.0;
        int sum = 0;
        for (const auto& grade : grades) {
            sum += grade.GetScore();
        }
        return static_cast<double>(sum) / grades.size();
    }

    Grade* GetGradeBySubject(const std::string& subject) {
        for (auto& grade : grades) {
            if (grade.GetSubject() == subject) {
                return &grade;
            }
        }
        return nullptr;
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

public:
    Group() : name(""), specialization(""), year(0) {}

    Group(const std::string& groupName, const std::string& spec, int y)
        : name(groupName), specialization(spec), year(y) {}

    std::string GetName() const { return name; }
    std::string GetSpecialization() const { return specialization; }
    int GetYear() const { return year; }

    void SetSpecialization(const std::string& spec) { specialization = spec; }
    void SetYear(int y) { year = y; }

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