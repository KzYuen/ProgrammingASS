#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

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
int appsCount = 0;
int adminCount = 0;

void LoadStudentsFromFile();
void LoadApplicationsFromFile();
void LoadAdminFromFile();
void SaveStudentsToFile();
void SaveAdminToFile();
void SaveApplicationsToFile();
void register_stud();
void register_admin();
void register_application(int index_Student);
void UpdateStudentProfile(int index_Student);
void ViewStudentProfile(int index_Student);
void ViewProfileAdmin(int index_Admin);
void approveRejectApplication(int app_index);
int FindStudentIndexByID(string id);
int FindAdminIndexByID(string id);
int FindApplicationIndexByStudentID(string studentID);
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void MainMenu();

int main() {

    /*LoadStudentsFromFile();
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
}*/

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

    while (file >> apps[appsCount].studentID
     	        >> apps[appsCount].months
      	        >> apps[appsCount].status) {
appsCount++;
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

for (int i = 0; i < appsCount; i++) {
        file << apps[i].studentID << " "
     	     << apps[i].months << " "
     	     << apps[i].status << endl;
}
file.close();
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void register_stud(){
	student stud;

    stud.id = "S" + to_string(studentCount + 1); 
    cout << "Generated ID: " << stud.id << endl;

    cout << "Name: "; cin >> stud.name;
    cout << "Faculty: "; cin >> stud.faculty;
    cout << "Phone: "; cin >> stud.phone;
    cout << "Vehicle: "; cin >> stud.vehicle;
    cout << "Password: "; cin >> stud.password;

students[studentCount++] = stud;

    cout << "Registration Successful!\n";
}

void register_admin(){
    admin ADM;
    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "Generated ID: " << ADM.adminID << endl;

    cout << "Name: "; cin >> ADM.name;
    cout << "Password: "; cin >> ADM.password;

admins[adminCount++] = ADM;

    cout << "Admin Registration Successful!\n";
}

void register_application(int index_Student){
    application APP;

    APP.studentID = students[index_Student].id;

    do{
    cout << "Enter number of months for parking pass: "; 
    cin >> APP.months;
    }while(APP.months <= 0 || APP.months > 3);

    APP.status = "pending";

apps[appsCount++] = APP;

    cout << "Application submitted successfully!\n";
}

void UpdateStudentProfile(int index_Student){
    
    cout << "Update Name: "; 
    cin >> students[index_Student].name;
    cout << "Update Faculty: "; 
    cin >> students[index_Student].faculty;
    cout << "Update Phone: "; 
    cin >> students[index_Student].phone;
    cout << "Update Vehicle: "; 
    cin >> students[index_Student].vehicle;
    cout << "Update Password: "; 
    cin >> students[index_Student].password;

    cout << "Profile Updated Successfully!\n";
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void ViewStudentProfile(int index_Student){ 

    cout << "\nID: " << students[index_Student].id;
    cout << "\nName: " << students[index_Student].name;
    cout << "\nFaculty: " << students[index_Student].faculty;
    cout << "\nPhone: " << students[index_Student].phone;
    cout << "\nVehicle: " << students[index_Student].vehicle << endl;
    
    cout << "------------ Application Status ------------" << endl;

    bool found = false;

    for(int i = 0; i < appsCount; i++){
        if(apps[i].studentID == students[index_Student].id){
            cout << "Application for " << apps[i].months << " month(s): " << apps[i].status << endl;
            found = true;
            if(apps[i].status == "approved"){
                cout << "Application for " << apps[i].months << " month(s) : APPROVED - Please make payment. \n";

                char pay; 
                cout << "Pay now? (y/n): ";
                cin >> pay;

                if(pay == 'y' || pay == 'Y'){
                    apps[i].status = "paid";
                    cout << "Payment successful! Your parking pass will be activated soon.\n";
        
            }else if(apps[i].status == "pending"){
                cout << "Your application is still pending. Please wait for admin approval.\n";
            }else{
            cout << "Application for " << apps[i].months << " month(s) : " << apps[i].status << endl;
            }
        }
    }

    if(!found){
        cout << "No applications found." << endl;
    }   
}
}

void ViewProfileAdmin(int index_Admin){

    cout << "\nAdmin ID: " << admins[index_Admin].adminID << endl;
    cout << "Name: " << admins[index_Admin].name << endl;

    cout << "\n------------ Pending Applications ------------\n";

    bool found = false;

    for(int i = 0; i < appsCount; i++){
        if(apps[i].status == "pending"){
            cout << "[" << i << "] "
                 << "Student ID: " << apps[i].studentID 
                 << ", Months: " << apps[i].months << endl;
            found = true;
        }
    }

    if(!found){
        cout << "No pending applications.\n";
        return;
    }

    int choice;
    cout << "\nEnter application index to process: ";
    cin >> choice;

    if(choice >= 0 && choice < appsCount){
        if(apps[choice].status == "Pending"){
            approveRejectApplication(choice);
        }else{
            cout << "Already processed.\n";
        }
    }else{
        cout << "Invalid index.\n";
    }
}
// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void approveRejectApplication(int app_index){
    string  decision;

    cout << "\n1. Approve \n2. Reject \nEnter your decision: ";
    cin >> decision;
    transform(decision.begin(), decision.end(), decision.begin(), ::tolower);

    if(decision == "approve"){
        apps[app_index].status = "approved";
        cout << "Application Approved!" << endl;
    }else if(decision == "reject"){
        apps[app_index].status = "rejected";
        cout << "Application Rejected!" << endl;
    }else{
        cout << "Invalid choice. Please enter 1 or 2." << endl;
    }

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

int FindApplicationIndexByStudentID(string studentID) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID == studentID) {
            return i;
        }
    }
    return -1; // Not found
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void studentMenu(int index_Student){
    int choice;

    while(true){
        cout << "1. View Profile \n2. Apply for Parking Pass \n3. Logout \n4. Update Profile \nEnter your choice: ";
        cin >> choice;

        if(choice == 1){
            ViewStudentProfile(index_Student);
        }else if(choice == 2){
            register_application(index_Student);
        }else if(choice == 3){
            cout << "Logging out... " << endl;
            break;
        }else if(choice == 4){
            UpdateStudentProfile(index_Student);
        }else{
            cout << "Invalid choice. Please enter 1, 2, or 3. " << endl;
            return;
        }
    }
}

void adminMenu(int index_Admin){
    int choice;

    while(true){
        cout << "1. View Profile \n2. View Statistics \n3. Logout \nEnter your choice: ";
        cin >> choice;

        if(choice == 1){
            ViewProfileAdmin(index_Admin);
        }else if(choice == 2){
            StatisticsUsage();
        }else if(choice == 3){
            cout << "Logging out... " << endl;
            break;
        }else{
            cout << "Invalid choice. Please enter 1, 2, or 3. " << endl;
            return;
        }
    }
}

//---------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

// so here we got this
void MainMenu(){
    int choice;
    string password_Student, password_Admin, student_id, Admin_id;

    while(true){
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

    int index_Student = FindStudentIndexByID(student_id);
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

    int index_Admin = FindAdminIndexByID(Admin_id);

            if(index_Admin != -1 && admins[index_Admin].password == password_Admin){
                cout << "Login Successful!" << endl;
                ViewProfileAdmin(index_Admin);
            }else{
                cout << "Invalid ID or Password. Please try again. " << endl;
                return;
            }
        }else if(choice == 5){
            cout << "Exiting... " << endl;
            break;
        }else{
            cout << "Invalid choice. Please enter 1, 2, 3, 4, or 5. " << endl;
            return;
        }
    }
}

void StatisticsUsage(){
    // only admin can see, summarize year end application of rejected and approval by showing month to month in a histogram or table format, 
    // also show the total number of applications, approved, and rejected for the year.
    // average, maximum, minimum number of application per months 

    cout << "\nHistogram (Months Applied):\n";
    for(int i = 0; i < appsCount; i++){
        cout << apps[i].months << " months: ";
        for(int j = 0; j < apps[i].months; j++){
            cout << "*";
        }
        cout << endl;
    }

    int totalApplications = appsCount;
    int approvedCount = 0;
    int rejectedCount = 0;

    for(int i = 0; i < appsCount; i++){
        if(apps[i].status == "approved"){
            approvedCount++;
        } else if(apps[i].status == "rejected"){
            rejectedCount++;
        }
    }
    cout << "\nTotal Applications: " << totalApplications << endl;
    cout << "Approved: " << approvedCount << endl;
    cout << "Rejected: " << rejectedCount << endl;
    
}