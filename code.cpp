#include <iostream>
#include <fstream>
#include <string>

using namespace std;

const int max = 200;

struct student {
	string id;
	string name;
	string stud_email;
	string faculty;
	string phone;
	string vehicle;
	string password;
};

struct admin {
    string studentID;
	int months;
	string status;
};

student students[max];
admin apps[max];

int studentCount = 0;
int passCount = 0;

void LoadStudentsFromFile() {
ifstream file("students.txt");

while (file >> students[studentCount].id
     	    >> students[studentCount].name
     	    >> students[studentCount].faculty
     	    >> students[studentCount].phone
    	    >> students[studentCount].vehicle
    	    >> students[studentCount].password) {
studentCount++;
}
file.close();
}

void LoadApplicationsFromFile() {
ifstream file("applications.txt");

while (file >> apps[passCount].studentID
     	     >> apps[passCount].months
      	     >> apps[passCount].status) {
passCount++;
 	}
file.close();
}

void SaveStudentsToFile() {
ofstream file("students.txt");

for (int i = 0; i < studentCount; i++) {
file << students[i].id << " "
     	    << students[i].name << " "
     	    << students[i].faculty << " "
    	    << students[i].phone << " "
            << students[i].vehicle << " "
            << students[i].password << endl;
}
file.close();
}

void SaveApplicationsToFile() {
ofstream file("applications.txt");

for (int i = 0; i < passCount; i++) {
file << apps[i].studentID << " "
     	     << apps[i].months << " "
     	     << apps[i].status << endl;
}
file.close();
}

void SaveApplicationsToFile() {
ofstream file("applications.txt");

for (int i = 0; i < passCount; i++) {
file << apps[i].studentID << " "
     	     << apps[i].months << " "
                 << apps[i].status << endl;
}
file.close();
}

string register_stud(){
	string stud;

	cout << "Generated ID: " << stud.id << endl;
cout << "Name: "; cin >> stud.name;
cout << "Faculty: "; cin >> stud.faculty;
cout << "Phone: "; cin >> stud.phone;
cout << "Vehicle: "; cin >> stud.vehicle;
cout << "Password: "; cin >> stud.password;

students[studentCount++] = stud;
cout << "Registration Successful!\n";
}

void ViewProfile(int index) {
cout << "\nID: " << students[index].id;
cout << "\nName: " << students[index].name;
cout << "\nFaculty: " << students[index].faculty;
cout << "\nPhone: " << students[index].phone;
cout << "\nVehicle: " << students[index].vehicle << endl;
}

Void studentmenu(){
void approveRejectApplication()
{
string studentID;
int decision;

```
// Sample application list
string applicationList[3] = {"S001", "S002", "S003"};
string status[3] = {"Pending", "Pending", "Pending"};

// INPUT studentID
cout << "Enter Student ID: ";
cin >> studentID;

// SEARCH application list
for (int i = 0; i < 3; i++)
{
    if (applicationList[i] == studentID)
    {
       
        cout << "1. Approve\n2. Reject\nEnter decision: ";
        cin >> decision;

        // UPDATE application status
        if (decision == 1)
        {
            status[i] = "Approved";
        }
        else if (decision == 2)
        {
            status[i] = "Rejected";
        }
        else
        {
            cout << "Invalid input" << endl;
            return;
        }

        // DISPLAY result
        cout << "Status Updated" << endl;
        return;
    }
}

// If not found
cout << "Application not found" << endl;
```

}

