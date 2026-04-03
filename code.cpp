#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct student {
	string id;
	string name;
	string stud_email;
	string faculty;
	string phone;
	string vehicle;
	string password;
};

struct application {
    string studentID;
	int months;
	string status;
};

struct admin {
    string adminID;
    string name;
    string password;
};

student students[100];
application apps[100];
admin admins[20];

int studentCount = 0;
int passCount = 0;
int adminCount = 0;

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

void LoadAdminFromFile(){
    ifstream file("admin.txt");

    while (file >> admins[adminCount].adminID
     	        >> admins[adminCount].name
     	        >> admins[adminCount].password) {
adminCount++;
}
file.close();
}

// ---------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ----------------------------------------------------------------------------------

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

void SaveAdminToFile() {
    ofstream file("admin.txt");
    
    for(int i = 0; i < adminCount; i++){
        file << admins[i].adminID << " "
             << admins[i].name << " "
             << admins[i].password << endl;
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

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

string register_stud(){
	student stud;

	cout << "Generated ID: " << stud.id << endl;
    cout << "Name: "; cin >> stud.name;
    cout << "Faculty: "; cin >> stud.faculty;
    cout << "Phone: "; cin >> stud.phone;
    cout << "Vehicle: "; cin >> stud.vehicle;
    cout << "Password: "; cin >> stud.password;

students[studentCount++] = stud;

    cout << "Registration Successful!\n";
}

string register_admin(){
    admin ADM;

    cout << "Generated ID: " << ADM.adminID << endl;
    cout << "Name: "; cin >> ADM.name;
    cout << "Password: "; cin >> ADM.password;

admins[adminCount++] = ADM;

    cout << "Admin Registration Successful!\n";
}

string register_application(int index_Student){
    application APP;

    APP.studentID = students[index_Student].id;

    cout << "Enter number of months for parking pass: "; 
    cin >> APP.months;

    APP.status = "Pending";

apps[passCount++] = APP;

    cout << "Application submitted successfully!\n";
}
// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void ViewStudentProfile(int index_Student){ 

    cout << "\nID: " << students[index_Student].id;
    cout << "\nName: " << students[index_Student].name;
    cout << "\nFaculty: " << students[index_Student].faculty;
    cout << "\nPhone: " << students[index_Student].phone;
    cout << "\nVehicle: " << students[index_Student].vehicle << endl;
    
    cout << "------------ Application Status ------------" << endl;
    
}

void ViewProfileAdmin(int index_Admin){

    cout << "Nothing";

}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void approveRejectApplication(){

string studentID;
int decision;

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


}

int FindStudentIndexByID(string id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id) {
            return i;
        }
    }
    return -1; // Not found
}

int FindAdminIndexByID(string id) {
    for (int i = 0; i < adminCount; i++) {
        if (admins[i].adminID == id) {
            return i;
        }
    }
    return -1; // Not found
}

// so here we got this
void MainMenu(){
    int choice, student_id, Admin_id;
    string password_Student, password_Admin;

    cout << "1. Sign Up as a Student. \n 2. Sign Up as an Admin. \n 3. Login as a Student. \n 4. Login as an Admin. \n 5. Exit. \n Enter your choice: ";
    cin >> choice;
    if(choice == 1){
        register_stud();
    }else if(choice == 2){
        register_admin();
    }else if(choice == 3){
        cout << "Enter Student ID: " << endl;
        cin >> student_id;
        cout << "Enter Password: " << endl;
        cin >> password_Student;

        int index_Student = FindStudentIndexByID(to_string(student_id));
        // -1 means not found, otherwise it will return the index of the student in the array
        if(index_Student != -1 && students[index_Student].password == password_Student){
            cout << "Login Successful!" << endl;
            ViewStudentProfile(index_Student);
        }else{
            cout << "Invalid ID or Password. Please try again. " << endl;
            return;
        }
    }else if(choice == 4){
        cout << "Enter Admin ID: " << endl;
        cin >> Admin_id;
        cout << "Enter Password: " << endl;
        cin >> password_Admin;

        int index_Admin = FindAdminIndexByID(to_string(Admin_id));

        if(index_Admin != -1 && admins[index_Admin].password == password_Admin){
            cout << "Login Successful!" << endl;
            ViewProfileAdmin(index_Admin);
        }else{
            cout << "Invalid ID or Password. Please try again. " << endl;
            return;
            }
    }else if(choice == 5){
        cout << "Exiting... " << endl;
        return; 
    }else{
        cout << "Invalid choice. Please enter 1, 2, 3, 4, or 5. " << endl;
        return;
    }

}

int main() {

    LoadStudentsFromFile();
    LoadApplicationsFromFile();
    LoadAdminFromFile();

    // Example usage
    register_stud();
    register_admin();
    FindAdminIndexByID();
    FindStudentIndexByID();
    approveRejectApplication();

    MainMenu();

    SaveStudentsToFile();
    SaveApplicationsToFile();
    SaveAdminToFile();

    return 0;   
}