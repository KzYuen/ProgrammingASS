#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>

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

// ============================================================
// GLOBAL ARRAYS TO STORE DATA
// ============================================================

student students[200];
application apps[400];
admin admins[20];

int studentCount = 0;
int appsCount = 0;
int adminCount = 0;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

// FILE I/O
void LoadStudentsFromFile();
void LoadApplicationsFromFile();
void LoadAdminFromFile();
void SaveStudentsToFile();
void SaveAdminToFile();
void SaveApplicationsToFile();

//Registration
void register_stud();
void register_admin();

//Applications
void register_application(int index_Student);
void renew_application(int index_Student);
void viewApplicationHistory(int index_Student);

//Profile
void UpdateStudentProfile(int index_Student);
void UpdatesAdminProfile(int index_Admin);
void ViewStudentProfile(int index_Student);
void ViewProfileAdmin(int index_Admin); //removed in modified

//Admin processing
//void viewPendingApplications(int index_Admin); //combined into ViewProfileAdmin for better flow
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);

//Helpers
int FindStudentIndexByID(string id);
int FindAdminIndexByID(string id);
int FindApplicationIndexByStudentID(string studentID);
bool hasPendingOrApproved(string studentID);
string getValidPassword();
int safeInputInt(int min, int max);
void CleanupExpiredPasses(string studentID);


//Alerts
void monthEndAlert(int index_Student);

//Menus
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void MainMenu();

//Date helpers
string getCurrentMonth();
string getCurrentDate();
bool isApproachingMonthEnd();

//-------------------------------------------------------------------------------------

int main() {
    
    LoadStudentsFromFile();
    LoadApplicationsFromFile();
    LoadAdminFromFile();

    CleanupExpiredPasses(""); // Clean up any expired passes on startup
    MainMenu();
    
    SaveStudentsToFile();
    SaveApplicationsToFile();
    SaveAdminToFile();

    return 0;   
    
}

// ============================================================
// DATE HELPERS
// ============================================================

//Returns today's date in "YYYY-MM-DD" format
string getCurrentDate() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

//Returns current month in "YYYY-MM" format
string getCurrentMonth() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    char buf[8];
    strftime(buf, sizeof(buf), "%Y-%m", now);
    return string(buf);
}

//Returns true if today is the 25th or later of the month, indicating month-end is approaching
bool isApproachingMonthEnd() {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    int day = now->tm_mday;
    return (day >= 25);
}

// ============================================================
// FILE I/O  (FIX: now uses '|' delimiter so names with spaces load correctly)
// ============================================================

//Loads students, applications from their respective text files into the global arrays.
void LoadStudentsFromFile() {
    ifstream file("students.txt");
    string line;

    if (!file) {
        cout << "Error: Could not open students.txt\n";
        return;
    }
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (studentCount >= 200) {
            cout << "Warning: Maximum student capacity reached.\n";
            break; // prevent overflow
        }

        stringstream ss(line);
        student& s = students[studentCount];
        if (
            getline(ss, s.id,'|') &&
            getline(ss, s.name,'|') &&
            getline(ss, s.stud_email,'|') &&
            getline(ss, s.faculty,'|') &&
            getline(ss, s.phone,'|') &&
            getline(ss, s.vehicle,'|') &&
            getline(ss, s.password)
        ) {
            studentCount++;
        }
    }
    file.close();
}

//Loads applications from applications.txt into the global array
void LoadApplicationsFromFile() {
    ifstream file("applications.txt");
    if (!file) {
        cout << "Error: Could not open applications.txt\n";
        return;
    }
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;
        if (appsCount >= 400) {
            cout << "Warning: Maximum applications reached.\n";
            break; 
        }

        stringstream ss(line);
        application temp; // Use a temp object
        string monthsStr;

        // Check if all fields are read successfully
        if (getline(ss, temp.studentID, '|') &&
            getline(ss, monthsStr, '|') &&
            getline(ss, temp.status, '|') &&
            getline(ss, temp.applyDate, '|') &&
            getline(ss, temp.applyMonth)) { 

            try {
                temp.months = stoi(monthsStr);
            } catch (...) { // Catches both invalid_argument and out_of_range
                cout << "Warning: Invalid months for ID " << temp.studentID << ". Skipping application.\n";
                continue;
            }

            if (temp.months < 1 || temp.months > 3) {
                cout << "Warning: Months out of range for ID " << temp.studentID << ". Skipping application.\n";
                continue;
            }

            // Only now commit to the array
            apps[appsCount] = temp;
            appsCount++;
        }
    }
    file.close();
}

//Loads admin records from admin.txt into the global array
void LoadAdminFromFile() {
    ifstream file("admin.txt");
    string line;
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (adminCount >= 20) {
            cout << "Warning: Maximum admin capacity reached.\n";
            break;
        }
        stringstream ss(line);
        admin& a = admins[adminCount];
        if (
            getline(ss, a.adminID,'|') &&
            getline(ss, a.name,'|') &&
            getline(ss, a.password)
        ) {
            adminCount++;
        }
    }
    file.close();
}
//-------------------------------------------------------------------------------------

//Saves all student records back to students.txt
void SaveStudentsToFile() {
    ofstream file("students.txt");

    for (int i = 0; i < studentCount; i++) {
            file << students[i].id << "|"
     	         << students[i].name << "|"
                 << students[i].stud_email << "|"
     	         << students[i].faculty << "|"
    	         << students[i].phone << "|"
                 << students[i].vehicle << "|"
                 << students[i].password << endl;
    }
    file.close();
}

//Saves all admin records back to admin.txt
void SaveAdminToFile() {
    ofstream file("admin.txt");
    
    for(int i = 0; i < adminCount; i++){
        file << admins[i].adminID << "|"
             << admins[i].name << "|"
             << admins[i].password << endl;
    }
    file.close();
}

//Saves all application records back to applications.txt
void SaveApplicationsToFile() {
    ofstream file("applications.txt");

    for (int i = 0; i < appsCount; i++) {
        file << apps[i].studentID << "|"
     	     << apps[i].months << "|"
     	     << apps[i].status << "|"
             << apps[i].applyDate << "|"
             << apps [i].applyMonth << "\n";
    }
    file.close();
}

//-------------------------------------------------------------------------------------

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
// INPUT HELPERS
// ============================================================

// Validates an integer input within [min, max]
int safeInputInt(int min, int max) {
    int n;
    while (true) {
        cin >> n;
        if (cin.fail() || n < min || n > max) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a valid number (" << min << "–" << max << "): ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard extras
            return n;
        }
    }
}

string getValidPassword() {
    string password;
    while (true) {
        cout << "Password must be at least 8 characters, include uppercase, lowercase, number and special character. Please enter password: ";
        getline(cin >> ws, password);

        if (password.length() < 8) {
            cout << "Password must be at least 8 characters long.\n";
            continue;
        }

        bool hasUpper = false;
        bool hasLower = false; 
        bool hasDigit = false; 
        bool hasSpecial = false;

        for (char c : password) {
            if (isupper(c)) hasUpper = true;
            else if (islower(c)) hasLower = true;
            else if (isdigit(c)) hasDigit = true;
            else if (ispunct(c)) hasSpecial = true;
        }

        if (hasUpper && hasLower && hasDigit && hasSpecial) {
            return password; // valid password
        } else {
            cout << "Password must include uppercase, lowercase, number and special character.\n";
        }
    }
}

// ============================================================
// REGISTRATION
// ============================================================

void register_stud(){
    if (studentCount >= 200) {
        cout << "System is full. Cannot register more students.\n";
        return;
    }
	student stud;

    printHeader("Student Registration");
    stud.id = "S" + to_string(studentCount + 1); 
    cout << "Generated ID: " << stud.id << endl;

    cout << "Name: "; 
    getline(cin >> ws,stud.name);
    cout << "Email: "; 
    getline(cin >> ws,stud.stud_email);
    cout << "Faculty: "; 
    getline(cin >> ws,stud.faculty);
    cout << "Phone: "; 
    getline(cin >> ws,stud.phone);
    cout << "Vehicle: "; 
    getline(cin >> ws,stud.vehicle);

    stud.password = getValidPassword();
    
    students[studentCount++] = stud;
    SaveStudentsToFile();

    cout << "Registration Successful!\n";
}

void register_admin(){
    admin ADM;
    
    if (adminCount >= 20) {
        cout << "System is full. Cannot register more admins.\n";
        return;
    }

    printHeader("Admin Registration");
    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "Generated Admin ID: " << ADM.adminID << endl;
    cout << "Name: "; 
    getline(cin >> ws,ADM.name);
    
    ADM.password = getValidPassword();
    admins[adminCount++] = ADM;
    SaveAdminToFile();

    cout << "Admin Registration Successful!\n";
}

// ============================================================
// APPLICATION HELPERS
// ============================================================

//Returns true if the student has a pending or approved application
bool hasPendingOrApproved(string studentID, bool includePaid = true) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != studentID) continue;

        if (apps[i].status == "pending" || apps[i].status == "approved") {
            return true; //block new application
        }

        if (includePaid && apps[i].status == "paid") {
            return true; //block renewal if active pass exists
        }
    }
    return false;
}

void CleanupExpiredPasses(string studentID) {
    string currentMonthStr = getCurrentMonth();
    // Convert "2026-04" to 202604 for easy math comparison
    int currentTotalMonths = stoi(currentMonthStr.substr(0, 4)) * 12 + stoi(currentMonthStr.substr(5, 2));
    bool modified = false;

    for (int i = 0; i < appsCount; i++) {
        if ((studentID.empty() || apps[i].studentID == studentID) && 
            (apps[i].status == "paid" || apps[i].status == "approved")) {
            
            // Calculate when THIS specific pass expires
            try{
                int startYear = stoi(apps[i].applyMonth.substr(0, 4));
                int startMonth = stoi(apps[i].applyMonth.substr(5, 2));
                int expiryTotalMonths = (startYear * 12 + startMonth) + (apps[i].months - 1);
            
                if (currentTotalMonths > expiryTotalMonths) {
                    apps[i].status = "expired";
                    modified = true;
                }
            }catch (...) {
                cout << "Warning: Invalid applyMonth for student " << apps[i].studentID << ". Skipping.\n";
            }
        }
    }
    if (modified) SaveApplicationsToFile();
}
// ============================================================
// APPLICATION — NEW / RENEW
// ============================================================

//submits a new parking pass application for the given student
// Blocks submission if a pending/approved application already exists.
void register_application(int index_Student){
    printHeader("New Parking Pass Application");
    string id = students[index_Student].id;

    //Blocks duplicate applications
    if (hasPendingOrApproved(id, true)) {
        cout << "You already have a pending or approved application.\n";
        cout << "Please wait for it to be processed before applying again.\n";
        return;
    }

    if (appsCount >= 400) {
        cout << "System is full. Cannot accept more applications.\n";
        cout << "Please contact admin for assistance.\n";
        return;
    }
    
    application APP;
    APP.studentID = students[index_Student].id;
    APP.applyDate = getCurrentDate();
    APP.applyMonth = getCurrentMonth();
    APP.status = "pending";
    
    cout << "  Advance renewals of up to 3 months are allowed.\n\n";
    cout << "Enter number of months for parking pass (1-3): "; 
    APP.months = safeInputInt(1, 3); // Ensure valid input

    apps[appsCount++] = APP;
    SaveApplicationsToFile();

    cout << "Application submitted on "<< APP.applyDate<< ". Pending admin approval.\n";



}   

//Renews an existing paid parking pass
//Requires a paid pass to exitst; blocks if pending/approved application already exists
void renew_application(int index_Student) {
    printHeader("Renew Parking Pass");

    if (index_Student < 0 || index_Student >= studentCount) return;

    string id = students[index_Student].id;

    // Find the most recent paid pass belonging to this student
    int paidIndex = -1;
    for (int i = appsCount - 1; i >= 0; i--) {
        if (apps[i].studentID == id && apps[i].status == "paid") {
            paidIndex = i;
            break;
        }
    }

    if (paidIndex == -1) {
        cout << "  No active (paid) parking pass found.\n";
        cout << "  You can only renew an existing paid pass.\n";
        cout << "  Please apply for a new pass instead (Option 3).\n";
        return;
    }

    // Show the pass being renewed
    cout << "  Active pass found:\n";
    printLine();
    cout << "  Applied on : " << apps[paidIndex].applyDate  << "\n";
    cout << "  Duration   : " << apps[paidIndex].months     << " month(s)\n";
    cout << "  Status     : " << apps[paidIndex].status     << "\n";
    printLine();

    char confirm;
    cout << "  Renew this pass? (y/n): ";
    cin  >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer
    
    if (confirm != 'y' && confirm != 'Y') {
        cout << "  Renewal cancelled.\n";
        return;
    }

    // Check no pending/approved renewal already exists
    bool duplicateRenewal = false;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID == id && apps[i].status == "pending") {
            duplicateRenewal = true;
            break;
        }
    }

    if (duplicateRenewal) {
        cout << "  You already have a pending renewal application.\n";
        return;
    }

    if (appsCount >= 400) {
        cout << "  System is full. Cannot accept more applications.\n";
        cout << "Please contact admin for assistance.\n";
        return;
    }

    // Create the renewal application
    application renewal;
    renewal.studentID  = id;
    renewal.applyDate  = getCurrentDate();
    renewal.applyMonth = getCurrentMonth();
    renewal.status     = "pending";

    cout << "  Advance renewals of up to 3 months are allowed.\n\n";
    cout << "  Enter number of months to renew (1-3): ";
    renewal.months = safeInputInt(1, 3);

    /*apps[paidIndex].status = "expired"; too early 
    — only mark as expired after renewal is approved; 
    otherwise student may end up with no active pass 
    if renewal is rejected
    */

    cout << "  Cost: RM " << fixed << setprecision(2) << (renewal.months * 30.0) << "\n";

    apps[appsCount++] = renewal;
    SaveApplicationsToFile();

    cout << "\n  Renewal submitted on " << renewal.applyDate << ".\n";
    cout << "  Your current pass remains active while pending admin approval.\n";
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void ViewStudentProfile(int index_Student){ 
    if (index_Student < 0 || index_Student >= studentCount) return;
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

//-------------------------------------------------------------------------------------
void UpdateStudentProfile(int index_Student){
    printHeader("Update Profile");
    student& stud = students[index_Student];

    cout << "Update Name: "; 
    getline(cin >> ws, students[index_Student].name);
    cout << "Update Faculty: "; 
    getline(cin >> ws, students[index_Student].faculty);
    cout << "Update Phone: "; 
    getline(cin >> ws, students[index_Student].phone);
    cout << "Update Vehicle: "; 
    getline(cin >> ws, students[index_Student].vehicle);
    
    char changePass;
    cout << "Change Password? (y/n): ";
    cin >> changePass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer
    if (changePass == 'y' || changePass == 'Y') {
        students[index_Student].password = getValidPassword();
    } else {
        cout << "Password unchanged.\n";
    }
    
    SaveStudentsToFile();
    cout << "Profile Updated Successfully!\n";
}

void UpdatesAdminProfile(int index_Admin){
    printHeader("Update Admin Profile");
    admin& ADM = admins[index_Admin];

    cout << "Update Name: "; 
    getline(cin >> ws, admins[index_Admin].name);
    
    char changePass;
    cout << "Change Password? (y/n): ";
    cin >> changePass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer
    if (changePass == 'y' || changePass == 'Y') {
        admins[index_Admin].password = getValidPassword();
    } else {
        cout << "Password unchanged.\n";
    }

    SaveAdminToFile();
    cout << "Admin Profile Updated Successfully!\n";
}

// ============================================================
// APPLICATION STATUS VIEW + PAYMENT
// ============================================================
void viewApplicationHistory(int index_Student) {
    printHeader("Application History");

    string id = students[index_Student].id;
    bool found = false;
    int count = 1;

    cout << "  " << left << setw(6) << "No."
         << setw(12) << "Date"
         << setw(10) << "Months"
         << setw(12) << "Status" << "\n";

    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != id) continue;

        found = true;

        cout << "  " << left << setw(6)  << count++
             << setw(12) << apps[i].applyDate
             << setw(10) << apps[i].months
             << setw(12) << apps[i].status << "\n";

        // Payment logic
        if (apps[i].status == "approved") {
            printLine();
            cout << "Application for " << apps[i].months 
                 << " month(s): APPROVED\n";
            cout << "Amount due: RM " << fixed << setprecision(2)
                 << (apps[i].months * 30.0) << "\n";

            char pay;
            cout << "Pay now? (y/n): ";
            cin >> pay;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (pay == 'y' || pay == 'Y') {
                apps[i].status = "paid";
                SaveApplicationsToFile();
                cout << "Payment successful!\n";
            }

            break; // Only allow payment for the most recent approved application
        }
    }

    if (!found) {
        cout << "  No applications found.\n";
    } else {
        // Spending summary
        double totalSpent = 0;
        int totalMonths = 0;
        for (int i = 0; i < appsCount; i++) {
            if (apps[i].studentID == id && apps[i].status == "paid") {
                totalMonths += apps[i].months;
                totalSpent  += apps[i].months * 30.0;
            }
        }
        printLine();
        cout << "  SUMMARY\n";
        printLine();
        cout << "  Total months active : " << totalMonths << "\n";
        cout << "  Total spent         : RM " << fixed << setprecision(2) << totalSpent << "\n";
    }

    printLine();
}

// ============================================================
// MONTH-END ALERT
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
    printHeader("Admin Process Applications");
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

    
    cout << "\nEnter application index to process (-1 to cancel): ";
    int choice = safeInputInt(-1, appsCount - 1); //-1 to cancel

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

//-------------------------------------------------------------------------------------

void approveRejectApplication(int app_index){
    if (app_index < 0 || app_index >= appsCount) return;

    cout << "\n  Application : " << apps[app_index].studentID  << "  |  " << apps[app_index].months << " month(s)\n";
    cout << "  1. Approve\n  2. Reject\n";
    cout << "  Decision (1/2): ";
    int decision;
    cin  >> decision;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear input buffer

    if (decision == 1) { // Admin Approves
    for (int i = 0; i < appsCount; i++) {
        // Find the student's OLD paid pass
        if (apps[i].studentID == apps[app_index].studentID && apps[i].status == "paid") {
            // ONLY expire the old pass if the new one is starting in the same month
            // or if the old pass is technically in the past.
            if (apps[app_index].applyMonth == apps[i].applyMonth) {
                apps[i].status = "expired";
            }
        }
    }
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
    int    facultyApps[20] = {0};
    int    facultyApproved[20] = {0};
    int    facultyCount = 0;

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

// ============================================================
// FIND HELPERS 
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
        choice = safeInputInt(1, 6); // Ensure valid input

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
            cout << "  Logging out...\n"; 
            break;
        }
        // FIX: was 'return' — kicked user out of loop; now continues correctly
        else    cout << "  Invalid choice. Please enter 1-6.\n";
        
    }
}

//-------------------------------------------------------------------------------------

void adminMenu(int index_Admin){
    int choice;

    while(true){
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. Statistics & Analytics\n";
        cout << "  3. Update Profile\n";
        cout << "  4. Logout\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 4); // Ensure valid input

        if(choice == 1){
            ViewProfileAdmin(index_Admin);
        }else if(choice == 2){
            statisticsUsage(index_Admin);
        }else if(choice == 3){
            UpdatesAdminProfile(index_Admin);
        }else if(choice == 4){
            cout << "Logging out... " << endl;
            break;
        }else{
            cout << "Invalid choice. Please enter 1-4. " << endl;
            continue;
        }
    }
}

// ============================================================
// MAIN MENU
// ============================================================

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
        choice = safeInputInt(1, 5); // Ensure valid input

        if(choice == 1){
            register_stud();
        }else if(choice == 2){
            register_admin();
        }else if(choice == 3){

            while(true){

                cout << "Enter Student ID: " << endl;
                cin >> student_id;
            
                int index_Student = FindStudentIndexByID(student_id);

                if(index_Student == -1) {
                    cout << "Invalid Student ID. Please try again. " << endl;
                    continue;
                }
            
                int attempts = 0;
                bool loginOK  = false;

                while(attempts < 3){
                    cout << "Enter Password: " << endl;
                    getline(cin >> ws, password_Student);

                    if(students[index_Student].password == password_Student){
                        cout << "Login Successful!" << endl;
                        studentMenu(index_Student);
                        loginOK = true;
                        break;
                    }else{
                        cout << "Invalid Password. Please try again. " << endl; 
                        attempts++;
                    }
                }

                if(!loginOK){
                    cout << "Too many failed attempts. Returning to main menu.\n";
                }

                break;
            }

        }else if(choice == 4){

            while(true){
            cout << "Enter Admin ID: " << endl;
            cin >> Admin_id;

            int index_Admin = FindAdminIndexByID(Admin_id);

            if(index_Admin == -1){
                cout << "Invalid Admin ID. Please try again. " << endl;
                continue;
            }
            
            int attempts = 0;
            bool loginOK  = false;

            while(attempts < 3){
                cout << "Enter Password: " << endl;
                getline(cin >> ws, password_Admin);

                if(admins[index_Admin].password == password_Admin){
                    cout << "Login Successful!" << endl;
                    adminMenu(index_Admin);
                    loginOK = true;
                    break;
                }else{
                    cout << "Invalid Password. Please try again. " << endl; 
                    attempts++;
                }
            }
            if(!loginOK){
                cout << "Too many failed attempts. Returning to main menu.\n";
            }
            break;
        }
    }else if(choice == 5){
        cout << "Exiting... " << endl;
        break;
    }else{
        cout << "Invalid choice. Please enter 1-5. " << endl;
        }
    }
}