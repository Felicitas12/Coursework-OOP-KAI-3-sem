#include <gtest/gtest.h>
#include "Services.h"
#include "DataAccess.h"
#include <memory>
#include <fstream>

class MockRepository : public DAL::IRepository<BLL::Student> {
private:
    std::vector<BLL::Student> data;

public:
    void Save(const std::vector<BLL::Student>& items) override {
        data = items;
    }

    std::vector<BLL::Student> Load() override {
        return data;
    }

    void Clear() override {
        data.clear();
    }
};

class MockGroupRepository : public DAL::IRepository<BLL::Group> {
private:
    std::vector<BLL::Group> data;

public:
    void Save(const std::vector<BLL::Group>& items) override {
        data = items;
    }

    std::vector<BLL::Group> Load() override {
        return data;
    }

    void Clear() override {
        data.clear();
    }
};

class StudentServiceTest : public ::testing::Test {
protected:
    std::shared_ptr<MockRepository> repo;
    std::shared_ptr<BLL::StudentService> service;

    void SetUp() override {
        repo = std::make_shared<MockRepository>();
        service = std::make_shared<BLL::StudentService>(repo);
    }
};

TEST_F(StudentServiceTest, AddStudent_ValidData_Success) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_EQ(student.GetFirstName(), "John");
    EXPECT_EQ(student.GetLastName(), "Doe");
    EXPECT_EQ(student.GetGroupName(), "CS-101");
    EXPECT_GT(student.GetId(), 0);
}

TEST_F(StudentServiceTest, AddStudent_EmptyName_ThrowsException) {
    EXPECT_THROW(
        service->AddStudent("", "Doe", "CS-101"),
        BLL::BusinessLogicException
    );
}

TEST_F(StudentServiceTest, AddStudent_DuplicateStudent_ThrowsException) {
    service->AddStudent("John", "Doe", "CS-101");

    EXPECT_THROW(
        service->AddStudent("John", "Doe", "CS-101"),
        BLL::DuplicateEntityException
    );
}

TEST_F(StudentServiceTest, RemoveStudent_ExistingStudent_Success) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_NO_THROW(service->RemoveStudent(student.GetId()));
    EXPECT_EQ(service->GetAll().size(), 0);
}

TEST_F(StudentServiceTest, RemoveStudent_NonExistingStudent_ThrowsException) {
    EXPECT_THROW(
        service->RemoveStudent(999),
        BLL::StudentNotFoundException
    );
}

TEST_F(StudentServiceTest, UpdateStudent_ValidData_Success) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_NO_THROW(
        service->UpdateStudent(student.GetId(), "Jane", "Smith", "CS-102")
    );

    auto updated = service->GetStudentById(student.GetId());
    EXPECT_EQ(updated->GetFirstName(), "Jane");
    EXPECT_EQ(updated->GetLastName(), "Smith");
    EXPECT_EQ(updated->GetGroupName(), "CS-102");
}

TEST_F(StudentServiceTest, UpdateStudent_NonExisting_ThrowsException) {
    EXPECT_THROW(
        service->UpdateStudent(999, "Jane", "Smith", "CS-102"),
        BLL::StudentNotFoundException
    );
}

TEST_F(StudentServiceTest, AddGrade_ValidGrade_Success) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_NO_THROW(
        service->AddGradeToStudent(student.GetId(), "Mathematics", 85)
    );

    auto updated = service->GetStudentById(student.GetId());
    EXPECT_EQ(updated->GetGrades().size(), 1);
}

TEST_F(StudentServiceTest, AddGrade_InvalidScore_ThrowsException) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_THROW(
        service->AddGradeToStudent(student.GetId(), "Mathematics", 150),
        BLL::InvalidGradeException
    );
}

TEST_F(StudentServiceTest, AddGrade_NegativeScore_ThrowsException) {
    auto student = service->AddStudent("John", "Doe", "CS-101");

    EXPECT_THROW(
        service->AddGradeToStudent(student.GetId(), "Mathematics", -10),
        BLL::InvalidGradeException
    );
}

TEST_F(StudentServiceTest, RemoveGrade_ExistingGrade_Success) {
    auto student = service->AddStudent("John", "Doe", "CS-101");
    service->AddGradeToStudent(student.GetId(), "Mathematics", 85);

    EXPECT_NO_THROW(
        service->RemoveGradeFromStudent(student.GetId(), "Mathematics")
    );

    auto updated = service->GetStudentById(student.GetId());
    EXPECT_EQ(updated->GetGrades().size(), 0);
}

TEST_F(StudentServiceTest, FindStudentsByName_FirstName_ReturnsMatching) {
    service->AddStudent("John", "Doe", "CS-101");
    service->AddStudent("John", "Smith", "CS-102");
    service->AddStudent("Jane", "Doe", "CS-103");

    auto result = service->FindStudentsByName("John", "");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(StudentServiceTest, FindStudentsByName_LastName_ReturnsMatching) {
    service->AddStudent("John", "Doe", "CS-101");
    service->AddStudent("Jane", "Doe", "CS-102");
    service->AddStudent("Bob", "Smith", "CS-103");

    auto result = service->FindStudentsByName("", "Doe");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(StudentServiceTest, FindStudentsByGroup_ReturnsCorrectStudents) {
    service->AddStudent("John", "Doe", "CS-101");
    service->AddStudent("Jane", "Smith", "CS-101");
    service->AddStudent("Bob", "Johnson", "CS-102");

    auto result = service->FindStudentsByGroup("CS-101");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(StudentServiceTest, CalculateAverageGrade_MultipleGrades_ReturnsCorrect) {
    auto student = service->AddStudent("John", "Doe", "CS-101");
    service->AddGradeToStudent(student.GetId(), "Math", 80);
    service->AddGradeToStudent(student.GetId(), "Physics", 90);
    service->AddGradeToStudent(student.GetId(), "Chemistry", 70);

    auto updated = service->GetStudentById(student.GetId());
    EXPECT_DOUBLE_EQ(updated->CalculateAverageGrade(), 80.0);
}

TEST_F(StudentServiceTest, FindStudentsByAverageGrade_ReturnsInRange) {
    auto s1 = service->AddStudent("John", "Doe", "CS-101");
    auto s2 = service->AddStudent("Jane", "Smith", "CS-102");
    auto s3 = service->AddStudent("Bob", "Johnson", "CS-103");

    service->AddGradeToStudent(s1.GetId(), "Math", 90);
    service->AddGradeToStudent(s2.GetId(), "Math", 70);
    service->AddGradeToStudent(s3.GetId(), "Math", 50);

    auto result = service->FindStudentsByAverageGrade(60, 80);
    EXPECT_EQ(result.size(), 1);
}

TEST_F(StudentServiceTest, FindStudentsByPerformance_Successful_ReturnsCorrect) {
    auto s1 = service->AddStudent("John", "Doe", "CS-101");
    auto s2 = service->AddStudent("Jane", "Smith", "CS-102");

    service->AddGradeToStudent(s1.GetId(), "Math", 90);
    service->AddGradeToStudent(s2.GetId(), "Math", 40);

    auto result = service->FindStudentsByPerformance(true);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].GetFirstName(), "John");
}

TEST_F(StudentServiceTest, FindStudentsByPerformance_Unsuccessful_ReturnsCorrect) {
    auto s1 = service->AddStudent("John", "Doe", "CS-101");
    auto s2 = service->AddStudent("Jane", "Smith", "CS-102");

    service->AddGradeToStudent(s1.GetId(), "Math", 90);
    service->AddGradeToStudent(s2.GetId(), "Math", 40);

    auto result = service->FindStudentsByPerformance(false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].GetFirstName(), "Jane");
}

TEST_F(StudentServiceTest, CalculateGroupAverageGrade_ReturnsCorrect) {
    auto s1 = service->AddStudent("John", "Doe", "CS-101");
    auto s2 = service->AddStudent("Jane", "Smith", "CS-101");

    service->AddGradeToStudent(s1.GetId(), "Math", 80);
    service->AddGradeToStudent(s2.GetId(), "Math", 90);

    double avg = service->CalculateGroupAverageGrade("CS-101");
    EXPECT_DOUBLE_EQ(avg, 85.0);
}

class GroupServiceTest : public ::testing::Test {
protected:
    std::shared_ptr<MockGroupRepository> repo;
    std::shared_ptr<BLL::GroupService> service;

    void SetUp() override {
        repo = std::make_shared<MockGroupRepository>();
        service = std::make_shared<BLL::GroupService>(repo);
    }
};

TEST_F(GroupServiceTest, AddGroup_ValidData_Success) {
    auto group = service->AddGroup("CS-101", "Computer Science", 1);

    EXPECT_EQ(group.GetName(), "CS-101");
    EXPECT_EQ(group.GetSpecialization(), "Computer Science");
    EXPECT_EQ(group.GetYear(), 1);
}

TEST_F(GroupServiceTest, AddGroup_EmptyName_ThrowsException) {
    EXPECT_THROW(
        service->AddGroup("", "Computer Science", 1),
        BLL::BusinessLogicException
    );
}

TEST_F(GroupServiceTest, AddGroup_DuplicateName_ThrowsException) {
    service->AddGroup("CS-101", "Computer Science", 1);

    EXPECT_THROW(
        service->AddGroup("CS-101", "Software Engineering", 2),
        BLL::DuplicateEntityException
    );
}

TEST_F(GroupServiceTest, RemoveGroup_ExistingGroup_Success) {
    service->AddGroup("CS-101", "Computer Science", 1);

    EXPECT_NO_THROW(service->RemoveGroup("CS-101"));
    EXPECT_EQ(service->GetAll().size(), 0);
}

TEST_F(GroupServiceTest, RemoveGroup_NonExisting_ThrowsException) {
    EXPECT_THROW(
        service->RemoveGroup("CS-999"),
        BLL::GroupNotFoundException
    );
}

TEST_F(GroupServiceTest, UpdateGroup_ValidData_Success) {
    service->AddGroup("CS-101", "Computer Science", 1);

    EXPECT_NO_THROW(
        service->UpdateGroup("CS-101", "Software Engineering", 2)
    );

    auto updated = service->GetGroupByName("CS-101");
    EXPECT_EQ(updated->GetSpecialization(), "Software Engineering");
    EXPECT_EQ(updated->GetYear(), 2);
}

TEST_F(GroupServiceTest, UpdateGroup_NonExisting_ThrowsException) {
    EXPECT_THROW(
        service->UpdateGroup("CS-999", "Computer Science", 1),
        BLL::GroupNotFoundException
    );
}

TEST_F(GroupServiceTest, GetGroupByName_Existing_ReturnsGroup) {
    service->AddGroup("CS-101", "Computer Science", 1);

    auto group = service->GetGroupByName("CS-101");
    EXPECT_NE(group, nullptr);
    EXPECT_EQ(group->GetName(), "CS-101");
}

TEST_F(GroupServiceTest, GetGroupByName_NonExisting_ReturnsNull) {
    auto group = service->GetGroupByName("CS-999");
    EXPECT_EQ(group, nullptr);
}

class GradeTest : public ::testing::Test {};

TEST_F(GradeTest, Constructor_InitializesCorrectly) {
    BLL::Grade grade("Mathematics", 85);

    EXPECT_EQ(grade.GetSubject(), "Mathematics");
    EXPECT_EQ(grade.GetScore(), 85);
}

TEST_F(GradeTest, SetScore_UpdatesScore) {
    BLL::Grade grade("Mathematics", 85);
    grade.SetScore(90);

    EXPECT_EQ(grade.GetScore(), 90);
}

TEST_F(GradeTest, ToJson_SerializesCorrectly) {
    BLL::Grade grade("Mathematics", 85);
    json j = grade.ToJson();

    EXPECT_EQ(j["subject"], "Mathematics");
    EXPECT_EQ(j["score"], 85);
}

TEST_F(GradeTest, FromJson_DeserializesCorrectly) {
    json j = {{"subject", "Physics"}, {"score", 92}};
    BLL::Grade grade = BLL::Grade::FromJson(j);

    EXPECT_EQ(grade.GetSubject(), "Physics");
    EXPECT_EQ(grade.GetScore(), 92);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}