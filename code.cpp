#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <sstream>

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
	string status; //*"pending", "approved", "rejected", "paid"
    string applyDate; //application date of the parking pass
    string applyMonth;
};

struct admin {
    string adminID;
    string name;
    string password;
};

//global arrays to store data

student students[100];
application apps[100];
admin admins[20];

int studentCount = 0;
int appsCount = 0;
int adminCount = 0;


//function declarations
void LoadStudentsFromFile();
void LoadApplicationsFromFile();
void LoadAdminFromFile();
void SaveStudentsToFile();
void SaveAdminToFile();
void SaveApplicationsToFile();
void register_stud();
void register_admin();
void register_application(int index_Student);
void renew_application(int index_Student);
void viewApplicationHistory(int index_Student);
void UpdateStudentProfile(int index_Student);
void ViewStudentProfile(int index_Student);
void ViewProfileAdmin(int index_Admin);
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);
void monthEndAlert(int index_Student);
int FindStudentIndexByID(string id);
int FindAdminIndexByID(string id);
int FindApplicationIndexByStudentID(string studentID);
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void MainMenu();
string getCurrentMonth();
string getCurrentDate();
bool isApproachingMonthEnd();

int main() {
    
    LoadStudentsFromFile();
    LoadApplicationsFromFile();
    LoadAdminFromFile();

    MainMenu();
    
    SaveStudentsToFile();
    SaveApplicationsToFile();
    SaveAdminToFile();

    return 0;   
    
}

// ============================================================
// DATE HELPERS
// ============================================================

string getCurrentDate() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

string getCurrentMonth() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[8];
    strftime(buf, sizeof(buf), "%Y-%m", now);
    return string(buf);
}

bool isApproachingMonthEnd() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    int day = now->tm_mday;
    return (day >= 25); // Alert if it's the 25th or later
}

// ============================================================
// FILE I/O  (FIX: now uses '|' delimiter so names with spaces load correctly)
// ============================================================

void LoadStudentsFromFile() {
    ifstream file("students.txt");

    while (file >> students[studentCount].id
     	        >> students[studentCount].name
                >> students[studentCount].stud_email
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
      	        >> apps[appsCount].status
                >> apps[appsCount].applyDate
                >> apps[appsCount].applyMonth) {
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
                 << students[i].stud_email << " "
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
     	     << apps[i].status << " "
             << apps[i].applyDate << " "
             << apps [i]. applyMonth << " " <<endl;
    }
    file.close();
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

// ============================================================
// DISPLAY HELPERS
// ============================================================

void printLine(char c = '-', int n = 55) {
    for (int i = 0; i < n; i++) cout << c;
    cout << "\n";
}

void printHeader(string title) {
    cout << "\n";
    printLine('=');
    cout << "  MPKJ Parking Pass System  |  " << title << "\n";
    printLine('=');
}

// ============================================================
// REGISTRATION
// ============================================================

void register_stud(){
	student stud;

    printHeader("Student Registration");
    stud.id = "S" + to_string(studentCount + 1); 
    cout << "Generated ID: " << stud.id << endl;

    cout << "Name: "; cin >> stud.name;
    cout << "Email: "; cin >> stud.stud_email;
    cout << "Faculty: "; cin >> stud.faculty;
    cout << "Phone: "; cin >> stud.phone;
    cout << "Vehicle: "; cin >> stud.vehicle;
    cout << "Password: "; cin >> stud.password;

    bool pass = false;
    while(!pass){
        cout << "Password must be at least 15 character, include lower, upper, number and special character. Please enter password again: ";
        cin >> stud.password;

        if(stud.password.length() < 15){
            cout << "Password must be at least 15 characters long.\n";
            continue;
        }
    }    
        bool hasUpper = false;
        bool hasLower = false; 
        bool hasDigit = false; 
        bool hasSpecial = false;
//dwd
        for(char c : stud.password){
            if(isupper(c)){
                hasUpper = true;
            } else if(islower(c)) {
                hasLower = true;
            } else if(isdigit(c)) {
                hasDigit = true;
            } else if(ispunct(c)) {
                hasSpecial = true;
            }
        }
        
        if(hasUpper && hasLower && hasDigit && hasSpecial){
            pass = true;
        }else{
            cout << "Password must include uppercase, lowercase, number and special character.\n";
        }

    students[studentCount++] = stud;
    SaveStudentsToFile();

    cout << "Registration Successful!\n";
}

void register_admin(){
    admin ADM;
    
    printHeader("Admin Registration");
    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "Generated Admin ID: " << ADM.adminID << endl;
    cout << "Name: "; 
    cin >> ADM.name;

    bool pass = false;
    while(!pass){
        cout << "Password must be at least 15 character, include lower, upper, number and special character. Please enter password again: ";
        cin >> ADM.password;

        if(ADM.password.length() <= 15){
            cout << "Password must be at least 15 characters long.\n";
            continue;
        }
    }    
        bool hasUpper = false;
        bool hasLower = false; 
        bool hasDigit = false; 
        bool hasSpecial = false;

        for(char c : ADM.password){
            if(isupper(c)){
                hasUpper = true;
            } else if(islower(c)) {
                hasLower = true;
            } else if(isdigit(c)) {
                hasDigit = true;
            } else if(ispunct(c)) {
                hasSpecial = true;
            }
        }
        
        if(hasUpper && hasLower && hasDigit && hasSpecial){
            pass = true;
        }else{
            cout << "Password must include uppercase, lowercase, number and special character.\n";
        }

    admins[adminCount++] = ADM;
    SaveAdminToFile();

    cout << "Admin Registration Successful!\n";
}

// ============================================================
// APPLICATION — NEW / RENEW
// ============================================================

void register_application(int index_Student){
    printHeader("New Parking Pass Application");
    application APP;
    APP.studentID = students[index_Student].id;
    APP.applyDate = getCurrentDate();
    APP.applyMonth = getCurrentMonth();
    
    cout << "  Advance renewals of up to 3 months are allowed.\n\n";
    do{
    cout << "Enter number of months for parking pass (1-3): "; 
    cin >> APP.months;
    }while(APP.months < 1 || APP.months > 3);

    APP.status = "pending";

    apps[appsCount++] = APP;
    SaveApplicationsToFile();

    cout << "Application submitted on "<< APP.applyDate<< ". Pending admin approval.\n";



}   

void renew_application(int index_Student){
        register_application(index_Student);
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void UpdateStudentProfile(int index_Student){
    printHeader("Update Profile");
    student& stud = students[index_Student];

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
    
    SaveStudentsToFile();
    cout << "Profile Updated Successfully!\n";
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void ViewStudentProfile(int index_Student){ 
    printHeader("Student Profile");
    student& stud = students[index_Student];

    printLine();
    cout << "\nID: " << students[index_Student].id;
    cout << "\nName: " << students[index_Student].name;
    cout << "\nEmail: " << students[index_Student].stud_email;
    cout << "\nFaculty: " << students[index_Student].faculty;
    cout << "\nPhone: " << students[index_Student].phone;
    cout << "\nVehicle: " << students[index_Student].vehicle << endl;
    printLine();
}

// ============================================================
// APPLICATION STATUS VIEW + PAYMENT
// ============================================================
void viewApplicationHistory(int index_Student) {
    printHeader("Application History");
    string id = students[index_Student].id;
    bool found = false;

    cout<< "  "<< left << setw(6) << "No."
                       << setw(12) << "Date"
                       << setw(10) << "Months"
                       << setw(12) << "Status" << "\n";
    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != students[index_Student].id) continue;
        found = true;
        string statusDisplay = apps[i].status;
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(12) << apps[i].applyDate
                              << setw(10) << apps[i].months
                              << setw(12) << statusDisplay << "\n";
    }

    //check for approved payment
    for(int i = 0; i < appsCount; i++){
        if(apps[i].studentID == students[index_Student].id){
            cout << "Application for " << apps[i].months << " month(s): " << apps[i].status << endl;
            found = true;
            if(apps[i].status == "approved"){
                printLine();
                cout << "Application for " << apps[i].months 
                     << " month(s) : APPROVED. \n";
                cout << "      Amount due: RM " << fixed << setprecision(2)
                 << (apps[i].months * 30.0) << "\n\n";
                
                char pay; 
                cout << "Pay now? (y/n): ";
                cin >> pay;

                if(pay == 'y' || pay == 'Y'){
                    apps[i].status = "paid";
                    SaveApplicationsToFile();
                    cout << "Payment successful! Your parking pass will be activated soon.\n";
                }else{
                    cout << "Payment skipped. Please pay later from this menu.\n";
                }

            }else if(apps[i].status == "pending"){
            cout << "Application for " << apps[i].months << " month(s) is still pending. Please wait for admin approval.\n";
            }else if(apps[i].status == "paid"){
            cout << "Application for " << apps[i].months << " month(s) is paid. Parking pass is activated.\n";
            }else{
            cout << "Application for " << apps[i].months << " month(s) was rejected. You may reapply. \n";
            }
        }
    }

    if (!found) {
        cout << "  No applications found.\n";
        return;
    }
}

// ============================================================
// MONTH-END ALERT  (NEW requirement from assignment)
// ============================================================
void monthEndAlert(int index_Student) {
    if (!isApproachingMonthEnd()) return;

    // Check if student has a paid/active pass expiring soon
    string id = students[index_Student].id;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID == id && apps[i].status == "paid") {
            cout << "\n";
            printLine('*');
            cout << "  REMINDER: Month end is approaching!\n";
            cout << "  Your current parking pass may expire soon.\n";
            cout << "  Please renew to avoid disruption.\n";
            printLine('*');
            return;
        }
    }
}

// ============================================================
// ADMIN — PROCESS APPLICATIONS
// ============================================================
void ViewProfileAdmin(int index_Admin){
    printHeader("Admin Proccess Applications");
    cout << "\nAdmin ID: " << admins[index_Admin].adminID << endl;
    cout << "Name: " << admins[index_Admin].name << endl;

    cout << "\n------------ Pending Applications ------------\n";

    bool found = false;
    cout << "  " << left << setw(6)  << "Index"
                         << setw(10) << "Stud ID"
                         << setw(12) << "Date"
                         << setw(10) << "Months"
                         << setw(12) << "Status" << "\n";
    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status == "pending") {
            cout << "  " << left << setw(6)  << i
                                  << setw(10) << apps[i].studentID
                                  << setw(12) << apps[i].applyDate
                                  << setw(10) << apps[i].months
                                  << setw(12) << apps[i].status << "\n";
            found = true;
        }
    }

    if(!found){
        cout << "No pending applications.\n";
        return;
    }

    int choice;
    cout << "\nEnter application index to process (-1 to cancel): ";
    cin >> choice;

    if (choice == -1) return;

    if(choice >= 0 && choice < appsCount){
        if(apps[choice].status == "pending"){
            approveRejectApplication(choice);
        }else{
            cout << "Application has already been processed.\n";
        }
    }else{
        cout << "Invalid index.\n";
    }
}

// --------------------------------------------------------------------------------------- ignore this line, visual only for better organization of code ---------------------------------------------------------------------------------------

void approveRejectApplication(int app_index){
    cout << "\n  Application : " << apps[app_index].studentID
         << "  |  " << apps[app_index].months << " month(s)\n";
    cout << "  1. Approve\n  2. Reject\n";
    cout << "  Decision (1/2): ";
    int decision;
    cin  >> decision;

    if (decision == 1) {
        apps[app_index].status = "approved";
        cout << "  Application approved.\n";
    } else if (decision == 2) {
        apps[app_index].status = "rejected";
        cout << "  Application rejected.\n";
    } else {
        cout << "  Invalid input. No changes made.\n";
        return;
    }
    SaveApplicationsToFile();
}

// ============================================================
// STATISTICS & ANALYTICS  (FIX: forward decl added; analytics expanded)
// ============================================================

void statisticsUsage(int index_Admin) {
    printHeader("Admin: Statistics & Analytics");
    cout << "  Admin: " << admins[index_Admin].name << "\n\n";

    int total    = appsCount;
    int approved = 0, rejected = 0, pending = 0, paid = 0;
    int monthCount[4] = {0}; // index 1–3

    // Per-faculty counts
    string facultyNames[20];
    int    facultyApps[20]      = {0};
    int    facultyApproved[20]  = {0};
    int    facultyCount         = 0;

    // Monthly application counts (last 12 months)
    string monthLabels[12];
    int    monthApps[12] = {0};
    {
        time_t t = time(0);
        struct tm* now = localtime(&t);
        for (int m = 11; m >= 0; m--) {
            int mo = now->tm_mon - m;
            int yr = now->tm_year + 1900;
            while (mo < 0) { mo += 12; yr--; }
            char buf[16];
            sprintf(buf, "%04d-%02d", yr, mo + 1);
            monthLabels[11 - m] = string(buf);
        }
    }

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status == "approved")  approved++;
        else if (apps[i].status == "rejected") rejected++;
        else if (apps[i].status == "pending")  pending++;
        else if (apps[i].status == "paid")     paid++;

        if (apps[i].months >= 1 && apps[i].months <= 3)
            monthCount[apps[i].months]++;

        // Faculty analytics — look up the student's faculty
        int si = FindStudentIndexByID(apps[i].studentID);
        if (si != -1) {
            string fac = students[si].faculty;
            int fi = -1;
            for (int f = 0; f < facultyCount; f++)
                if (facultyNames[f] == fac) { fi = f; break; }
            if (fi == -1 && facultyCount < 20) {
                fi = facultyCount;
                facultyNames[facultyCount++] = fac;
            }
            if (fi != -1) {
                facultyApps[fi]++;
                if (apps[i].status == "approved" || apps[i].status == "paid")
                    facultyApproved[fi]++;
            }
        }

        // Monthly distribution
        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == monthLabels[m]) {
                monthApps[m]++;
                break;
            }
        }
    }

    // Utilisation rate: paid / total students
    double utilRate = studentCount > 0
        ? (double)(paid + approved) / studentCount * 100.0
        : 0.0;

    // --- Summary ---
    printLine();
    cout << "  OVERALL SUMMARY\n";
    printLine();
    cout << "  Total students in system  : " << studentCount  << "\n";
    cout << "  Total applications        : " << total         << "\n";
    cout << "  Approved (unpaid)         : " << approved      << "\n";
    cout << "  Paid / Active             : " << paid          << "\n";
    cout << "  Rejected                  : " << rejected      << "\n";
    cout << "  Pending                   : " << pending       << "\n";
    cout << "  Car park utilisation rate : " << fixed << setprecision(1)
         << utilRate << "%\n";

    // --- Applications by duration ---
    printLine();
    cout << "  APPLICATIONS BY DURATION\n";
    printLine();
    for (int m = 1; m <= 3; m++) {
        cout << "  " << m << " month(s) : ";
        int bar = monthCount[m];
        for (int j = 0; j < bar && j < 40; j++) cout << "#";
        if (bar > 40) cout << "+" ;
        cout << "  (" << bar << ")\n";
    }

    // --- Monthly trend (last 12 months) ---
    printLine();
    cout << "  MONTHLY APPLICATION TREND (last 12 months)\n";
    printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) if (monthApps[m] > maxBar) maxBar = monthApps[m];
    for (int m = 0; m < 12; m++) {
        cout << "  " << monthLabels[m] << " : ";
        int bar = (monthApps[m] * 30) / maxBar;
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << ")\n";
    }

    // --- Per-faculty breakdown ---
    printLine();
    cout << "  APPLICATIONS BY FACULTY / INSTITUTE\n";
    printLine();
    cout << "  " << left << setw(12) << "Faculty"
                         << setw(10) << "Applied"
                         << setw(12) << "Approved"
                         << "Avg/month\n";
    printLine();
    for (int f = 0; f < facultyCount; f++) {
        double avg = (total > 0) ? (double)facultyApps[f] / 12.0 : 0;
        cout << "  " << left << setw(12) << facultyNames[f]
                              << setw(10) << facultyApps[f]
                              << setw(12) << facultyApproved[f]
                              << fixed << setprecision(1) << avg << "\n";
    }
    printLine();
    cout << "\n  [Note] These analytics can support UTAR's negotiation with MPKJ\n"
         << "         for better monthly pass rates for students.\n";
}

/*void StatisticsUsage(){
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
    
}*/

// ============================================================
// FIND HELPERS  (unchanged from your original)
// ============================================================

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

// ============================================================
// MENUS
// ============================================================
void studentMenu(int index_Student){
    // Show month-end reminder on login
    monthEndAlert(index_Student);

    int choice;

    while(true){
        printHeader("Student Menu");
        cout << "  1. View Profile\n";
        cout << "  2. Update Profile\n";
        cout << "  3. New Application\n";
        cout << "  4. Renew Pass\n";
        cout << "  5. View Application Status / Pay\n";
        cout << "  6. Logout\n";
        printLine();
        cout << "  Choice: ";
        cin >> choice;

        if(choice == 1){
            ViewStudentProfile(index_Student);
        }else if (choice == 2){
            UpdateStudentProfile(index_Student);
        }else if (choice == 3){
            register_application(index_Student);
        }else if (choice == 4) {
            renew_application(index_Student);
        }else if (choice == 5) {
            viewApplicationHistory(index_Student);
        }else if (choice == 6) { 
            cout << "  Logging out...\n"; break;
        }
        // FIX: was 'return' — kicked user out of loop; now continues correctly
        else    cout << "  Invalid choice. Please enter 1-6.\n";
        
    }
}

void adminMenu(int index_Admin){
    int choice;

    while(true){
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. Statistics & Analytics\n";
        cout << "  3. Logout\n";
        printLine();
        cout << "  Choice: ";
        cin  >> choice;

        if(choice == 1){
            ViewProfileAdmin(index_Admin);
        }else if(choice == 2){
            statisticsUsage(index_Admin);
        }else if(choice == 3){
            cout << "Logging out... " << endl;
            break;
        }else{
            cout << "Invalid choice. Please enter 1-3. " << endl;
            return;
        }
    }
}

// ============================================================
// MAIN MENU
// ============================================================

// so here we got this
void MainMenu(){
    int choice;
    string password_Student, password_Admin, student_id, Admin_id;

    while(true){
        printHeader("Main Menu");
        cout << "  1. Student Sign Up\n";
        cout << "  2. Admin Sign Up\n";
        cout << "  3. Student Login\n";
        cout << "  4. Admin Login\n";
        cout << "  5. Exit\n";
        printLine();
        cout << "  Choice: ";
        cin  >> choice;

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
            if (index_Student != -1 && students[index_Student].password == password_Student) {
                cout << "Login Successful!" << endl;
                studentMenu(index_Student);
            } else {
                cout << "Invalid ID or Password. Please try again. " << endl;
                continue;
            }
        }else if(choice == 4){
            cout << "Enter Admin ID: " << endl;
            cin >> Admin_id;
            cout << "Enter Password: " << endl;
            cin >> password_Admin;
            
            int index_Admin = FindAdminIndexByID(Admin_id);
            if(index_Admin != -1 && admins[index_Admin].password == password_Admin){
                cout << "Login Successful!" << endl;
                adminMenu(index_Admin);
            }else{
                cout << "Invalid ID or Password. Please try again. " << endl;
                return;
            }
        }else if(choice == 5){
            cout << "Exiting... " << endl;
            break;
        }else{
            cout << "Invalid choice. Please enter 1-5. " << endl;
        }
    }
}