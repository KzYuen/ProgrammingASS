// ============================================================
// MPKJ Monthly Car Parking Pass Management System
// Course      : UECS1104 Programming and Problem Solving
// Semester    : February 2026
// Group Leader: Kazin Yuen Kah Zing (2505264)
// Members     : Cynthia Chan Ern Tung (2505570)
// Members     : Wallace Chan Wai Hoong (2500582)
// Members     : Lim Yu Feng (2303363)
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <cstring> 
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

// ============================================================
// STRUCTS
// ============================================================

struct student {
    string id;
    string name;
    string stud_email;
    string faculty;   // validated: A / B / C / D
    string phone;
    string password;
};

struct vehicle {
    string vehicleID; // "V1", "V2", …
    string studentID;
    string plate;
    string type;      // Car / Motorcycle / Van
};

struct application {
    string studentID;  // owning student's ID
    string vehicleID;  // vehicle this pass is for
    string faculty;    // snapshot of student's faculty at application time
    int    months;     // duration of pass (1-3)
    string status;     // pending | approved | rejected | paid | expired
    string applyDate;  // exact submission date
    string applyMonth; // expiry calculation
};

struct admin {
    string adminID;
    string name;
    string email;
    string phone;
    string password;
};

// ============================================================
// GLOBALS
// ============================================================

student     students[200]; // max 200 students
vehicle     vehicles[600]; // max 600 vehicles (assuming some students have multiple vehicles, but not all do)
application apps[400];     // max 400 application records
admin       admins[20];    // max 20 admins accounts

int studentCount = 0;
int vehicleCount = 0;
int appsCount    = 0;
int adminCount   = 0;

// Faculty table — change display names here without touching any other code
const string FAC_CODES[11]  = { "A",    "B",    "C",    "D",    "E",    "F",    "G",    "H",    "I",    "J",   "K" };
const string FAC_LABELS[11] = {
    "A- M. Kandiah Faculty of Medicine and Health Sciences (MKF FMHS)",
    "B-Lee Kong Chian Faculty of Engineering and Science (LKC FES)",
    "C-Institute of Management and Leadership Development (IMLD)",
    "D-Institute of Postgraduate Studies and Research (IPSR)",
    "E-Centre for Corporate and Community Development (CCCD)",
    "F-Faculty of Accountancy and Management (FAM)",
    "G-Faculty of Chinese Studies (FCS)",
    "H-Faculty of Education (FOE)",
    "I-Centre for Foundation Studies (CFS)",
    "J-Faculty of Creative Industries (FCI)",
    "K-Institute of International Education (IIE)"
};
const int FAC_COUNT = 11;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

// File I/O
void loadStudentsFromFile();
void loadVehiclesFromFile();
void loadApplicationsFromFile();
void loadAdminFromFile();
void saveStudentsToFile();
void saveVehiclesToFile();
void saveAdminToFile();
void saveApplicationsToFile();

// Registration
void registerStudent();
void registerAdmin();

// Vehicle management
void registerVehicle(string studentID);
void viewVehicles(string studentID);
void manageVehicles(int index_Student);
int  getVehiclesForStudent(string studentID, int results[], int maxCount);
int  findVehicleIndexByID(string vehicleID);

// Applications
void registerApplication(int index_Student);
void renewApplication(int index_Student);
void viewApplicationHistory(int index_Student);   // student tracking & analytics

// Profile
void viewStudentProfile(int index_Student);
void updateStudentProfile(int index_Student);
void updateAdminProfile(int index_Admin);
void adminViewStudentProfile(int index_Admin);

// Admin processing
void viewProfileAdmin(int index_Admin);           // (a) approve / reject
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);            // (b) monthly / yearly analytics
void generateSummaryReport(int index_Admin);      // (c) negotiation report

// Helpers
int    findStudentIndexByID(string id);
int    findAdminIndexByID(string id);
bool   hasPendingOrApprovedForVehicle(string vehicleID);
bool   hasActivePaidPassForVehicle(string vehicleID);
int    facultyIndex(string fac);
bool   isRenewalApp(int appIdx);
string getValidPassword();
string getValidFaculty();
int    safeInputInt(int min, int max);
void   cleanupExpiredPasses(string studentID);

// Alerts
void monthEndAlert(int index_Student);            // (e) month-end reminder

// Menus
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void mainMenu();

// Date helpers
string getCurrentMonth();
string getCurrentDate();
bool   isApproachingMonthEnd();

// Display helpers
void printLine(char c = '-', int n = 55);
void printHeader(string title);
void buildMonthLabels(string labels[], int count);

// ============================================================
// MAIN
// ============================================================

int main() {
    loadStudentsFromFile();
    loadVehiclesFromFile();
    loadApplicationsFromFile();
    loadAdminFromFile();

    cleanupExpiredPasses("");   // auto-expire passes on startup
    mainMenu();

    saveStudentsToFile();
    saveVehiclesToFile();
    saveApplicationsToFile();
    saveAdminToFile();

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
    return (now->tm_mday >= 25);
}

// Fills labels[0..count-1] with "YYYY-MM" strings, oldest first, ending this month.
void buildMonthLabels(string labels[], int count) {
    time_t t = time(0);
    struct tm* now = localtime(&t);
    for (int m = count - 1; m >= 0; m--) {
        int mo = now->tm_mon - m;
        int yr = now->tm_year + 1900;
        while (mo < 0) {
            mo += 12; 
            yr--;
        }
        char buf[8];
        sprintf(buf, "%04d-%02d", yr, mo + 1);
        labels[count - 1 - m] = string(buf);
    }
}

// ============================================================
// DISPLAY HELPERS
// ============================================================

void printLine(char c, int n) {
    for (int i = 0; i < n; i++) {
        cout << c;
    }
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

int safeInputInt(int min, int max) {
    int n;
    while (true) {
        cin >> n;
        if (cin.fail() || n < min || n > max) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Please enter a number (" << min << "-" << max << "): ";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return n;
        }
    }
}

string getValidPassword() {
    string pw;
    while (true) {
        cout << "  Password (min 12 chars, upper + lower + digit + special): ";
        getline(cin >> ws, pw);

        if ((int)pw.length() < 12) {
            cout << "  Password must be at least 12 characters.\n";
            continue;
        }
        bool hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pw) {
            if (isupper(c))      hasU = true;
            else if (islower(c)) hasL = true;
            else if (isdigit(c)) hasD = true;
            else if (ispunct(c)) hasS = true;
        }
        if (hasU && hasL && hasD && hasS) {
            return pw;
        }
        cout << "  Must include uppercase, lowercase, digit, and special character.\n";
    }
}

// Forces the user to enter a valid faculty code (A-J).
string getValidFaculty(){
    string fac;
    while(true){
        cout << "  Faculty — A:M. Kandiah  B:Lee Kong Chian  C:FEG  D:FICT  E:FaS  F:FAM  G:FCS  H:CFFS  I:FCI  J:FOE — Enter code: ";
        getline(cin >> ws, fac);
        for(char& c : fac){
            c = toupper(c);
        }
        for(int i = 0; i < FAC_COUNT; i++){
            if (FAC_CODES[i] == fac) return fac;
        }
        cout << "  Invalid faculty. Please enter A, B, C, D, E, F, G, H, I or J.\n";
    }
}

// ============================================================
// FILE I/O
// ============================================================

void loadStudentsFromFile(){
    ifstream file("students.txt");
    string line;
    while (getline(file, line)){
        if (line.empty() || studentCount >= 200) continue;
        stringstream ss(line);
        student& s = students[studentCount];
        if (getline(ss, s.id,         '|') &&
            getline(ss, s.name,       '|') &&
            getline(ss, s.stud_email, '|') &&
            getline(ss, s.faculty,    '|') &&
            getline(ss, s.phone,      '|') &&
            getline(ss, s.password)) {
            studentCount++;
        }
    }
}

void loadVehiclesFromFile() {
    ifstream file("vehicles.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || vehicleCount >= 600) continue;
        stringstream ss(line);
        vehicle& v = vehicles[vehicleCount];
        if (getline(ss, v.vehicleID, '|') &&
            getline(ss, v.studentID, '|') &&
            getline(ss, v.plate,     '|') &&
            getline(ss, v.type)) {
            vehicleCount++;
        }
    }
}

void loadApplicationsFromFile() {
    ifstream file("applications.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || appsCount >= 400) continue;
        stringstream ss(line);
        application temp;
        string monthsStr;
        if (getline(ss, temp.studentID,  '|') &&
            getline(ss, temp.vehicleID,  '|') &&
            getline(ss, temp.faculty,    '|') &&
            getline(ss, monthsStr,       '|') &&
            getline(ss, temp.status,     '|') &&
            getline(ss, temp.applyDate,  '|') &&
            getline(ss, temp.applyMonth)) {
            try {
                temp.months = stoi(monthsStr);
                if (temp.months >= 1 && temp.months <= 3) {
                    apps[appsCount++] = temp;
                }
            } catch (...) {}
        }
    }
}

void loadAdminFromFile() {
    ifstream file("admin.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || adminCount >= 20) continue;
        stringstream ss(line);
        admin& a = admins[adminCount];
        if (getline(ss, a.adminID,  '|') &&
            getline(ss, a.name,     '|') &&
            getline(ss, a.email,    '|') &&
            getline(ss, a.phone,    '|') && 
            getline(ss, a.password)) {
            adminCount++;
        }
    }
}

void saveStudentsToFile() {
    ofstream file("students.txt");
    for (int i = 0; i < studentCount; i++) {
        file << students[i].id         << "|"
             << students[i].name       << "|"
             << students[i].stud_email << "|"
             << students[i].faculty    << "|"
             << students[i].phone      << "|"
             << students[i].password   << "\n";
    }
}

void saveVehiclesToFile() {
    ofstream file("vehicles.txt");
    for (int i = 0; i < vehicleCount; i++) {
        file << vehicles[i].vehicleID << "|"
             << vehicles[i].studentID << "|"
             << vehicles[i].plate     << "|"
             << vehicles[i].type      << "\n";
    }
}

void saveAdminToFile() {
    ofstream file("admin.txt");
    for (int i = 0; i < adminCount; i++) {
        file << admins[i].adminID  << "|"
             << admins[i].name     << "|"
             << admins[i].email    << "|"
             << admins[i].phone    << "|"
             << admins[i].password << "\n";
    }
}

void saveApplicationsToFile() {
    ofstream file("applications.txt");
    for (int i = 0; i < appsCount; i++) {
        file << apps[i].studentID  << "|"
             << apps[i].vehicleID  << "|"
             << apps[i].faculty    << "|"
             << apps[i].months     << "|"
             << apps[i].status     << "|"
             << apps[i].applyDate  << "|"
             << apps[i].applyMonth << "\n";
    }
}

// ============================================================
// SMALL HELPERS
// ============================================================

int facultyIndex(string fac) {
    for (int i = 0; i < FAC_COUNT; i++) {
        if (FAC_CODES[i] == fac) return i;
    }
    return -1;
}

// Returns true if app at appIdx is a renewal
// (i.e. same vehicle had a paid/expired pass before it)
bool isRenewalApp(int appIdx) {
    for (int j = 0; j < appIdx; j++) {
        if (apps[j].vehicleID == apps[appIdx].vehicleID &&
           (apps[j].status == "paid" || apps[j].status == "expired")) {
            return true;
        }
    }
    return false;
}

int getVehiclesForStudent(string studentID, int results[], int maxCount) {
    int count = 0;
    for (int i = 0; i < vehicleCount && count < maxCount; i++) {
        if (vehicles[i].studentID == studentID) {
            results[count++] = i;
        }
    }
    return count;
}

int findVehicleIndexByID(string vehicleID) {
    for (int i = 0; i < vehicleCount; i++) {
        if (vehicles[i].vehicleID == vehicleID) return i;
    }
    return -1;
}

bool hasPendingOrApprovedForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].vehicleID == vehicleID &&
           (apps[i].status == "pending" || apps[i].status == "approved")) {
            return true;
        }
    }
    return false;
}

bool hasActivePaidPassForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].vehicleID == vehicleID && apps[i].status == "paid") {
            return true;
        }
    }
    return false;
}

int findStudentIndexByID(string id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id) return i;
    }
    return -1;
}

int findAdminIndexByID(string id) {
    for (int i = 0; i < adminCount; i++) {
        if (admins[i].adminID == id) return i;
    }
    return -1;
}

// ============================================================
// CLEANUP — auto-expire passes whose duration has elapsed
// ============================================================
/*
cleanupExpiredPasses(string studentID)
- marks "paid" or "approved" applications as "expired" if their duration has passed
- modifies apps[] status and saves to file if changed
- auto-expire on startup so all status checks are accurate
*/
void cleanupExpiredPasses(string studentID) {
    string cur = getCurrentMonth();
    // Convert "YYYY-MM" to total months for easy comparison
    int currentTotal = stoi(cur.substr(0, 4)) * 12 + stoi(cur.substr(5, 2));
    bool modified = false;

    for (int i = 0; i < appsCount; i++) {
        if (!studentID.empty() && apps[i].studentID != studentID) continue;
        if (apps[i].status != "paid" && apps[i].status != "approved") continue;
        try {
            int yr     = stoi(apps[i].applyMonth.substr(0, 4));
            int mo     = stoi(apps[i].applyMonth.substr(5, 2));
            int expiry = yr * 12 + mo + apps[i].months - 1;
            if (currentTotal > expiry) {
                apps[i].status = "expired";
                modified = true;
            }
        } catch (...) {}
    }
    if (modified) saveApplicationsToFile();
}

// ============================================================
// MONTH-END ALERT  (e)
// ============================================================

void monthEndAlert(int index_Student) {
    if (!isApproachingMonthEnd()) return;
    string id = students[index_Student].id;
    int idx[20];
    int vCount = getVehiclesForStudent(id, idx, 20);
    for (int i = 0; i < vCount; i++) {
        if (hasActivePaidPassForVehicle(vehicles[idx[i]].vehicleID)) {
            cout << "\n";
            printLine('*');
            cout << "  *** REMINDER: Month end is approaching! ***\n";
            cout << "  *** Please renew your parking pass to    ***\n";
            cout << "  *** avoid disruption next month.         ***\n";
            printLine('*');
            return;
        }
    }
    // Also alert if they have NO active pass at all
    printLine('*');
    cout << "  *** REMINDER: Month end approaching!     ***\n";
    cout << "  *** You have no active parking pass.     ***\n";
    cout << "  *** Consider applying for next month.    ***\n";
    printLine('*');
}

// ============================================================
// REGISTRATION
// ============================================================
/*
registerStudent()
- collects student details, validates input, assigns a unique ID,
  validates faculty and password, then saves to file
*/
void registerStudent() {
    if (studentCount >= 200) { cout << "  System full.\n"; return; }

    student stud;
    printHeader("Student Registration");

    stud.id = "S" + to_string(studentCount + 1);
    cout << "  Generated ID : " << stud.id << "\n\n";
    cout << "  (Only 20 characters allowed) | Name  : ";
    getline(cin >> ws, stud.name);
    cout << "  Email   : ";
    getline(cin >> ws, stud.stud_email);
    stud.faculty  = getValidFaculty();
    cout << "  Phone   : ";
    getline(cin >> ws, stud.phone);
    stud.password = getValidPassword();

    students[studentCount++] = stud;
    saveStudentsToFile();
    cout << "\n  Registration successful! Your ID: " << stud.id << "\n";
    cout << "  Add your vehicle(s) via Student Menu > Manage Vehicles.\n";
}

void registerAdmin() {
    if (adminCount >= 20) { cout << "  System full.\n"; return; }

    admin ADM;
    printHeader("Admin Registration");

    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "  Generated Admin ID : " << ADM.adminID << "\n\n";
    cout << "  Name : ";
    getline(cin >> ws, ADM.name);
    ADM.password = getValidPassword();

    admins[adminCount++] = ADM;
    saveAdminToFile();
    cout << "\n  Admin registration successful!\n";
}

// ============================================================
// VEHICLE MANAGEMENT
// ============================================================

void registerVehicle(string studentID) {
    if (vehicleCount >= 600) { cout << "  Vehicle capacity reached.\n"; return; }

    vehicle v;
    v.vehicleID = "V" + to_string(vehicleCount + 1);
    v.studentID = studentID;
    cout << "  Plate number (e.g. WXY1234) : ";
    getline(cin >> ws, v.plate);
    cout << "  Type (Car/Motorcycle/Van)   : ";
    getline(cin >> ws, v.type);

    vehicles[vehicleCount++] = v;
    saveVehiclesToFile();
    cout << "  Vehicle registered. ID: " << v.vehicleID << "\n";
}

void viewVehicles(string studentID) {
    int idx[20];
    int count = getVehiclesForStudent(studentID, idx, 20);

    if (count == 0) { cout << "  No vehicles registered.\n"; return; }

    cout << "  " << left << setw(10) << "VehicleID"
                         << setw(16) << "Plate"
                         << setw(14) << "Type"
                         << "Active Pass\n";
    printLine();
    for (int i = 0; i < count; i++) {
        vehicle& v = vehicles[idx[i]];
        string pass = hasActivePaidPassForVehicle(v.vehicleID) ? "Yes" : "No";
        cout << "  " << left << setw(10) << v.vehicleID
                             << setw(16) << v.plate
                             << setw(14) << v.type
                             << pass << "\n";
    }
}

void manageVehicles(int index_Student) {
    string id = students[index_Student].id;
    while (true) {
        printHeader("Manage Vehicles");
        viewVehicles(id);
        printLine();
        cout << "  1. Add Vehicle\n  2. Back\n  Choice: ";
        int ch = safeInputInt(1, 2);
        if (ch == 1) registerVehicle(id);
        else break;
    }
}

// ============================================================
// APPLICATION — NEW  (a)
// ============================================================

void registerApplication(int index_Student) {
    printHeader("New Parking Pass Application");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered. Add one via Manage Vehicles first.\n";
        return;
    }
    if (appsCount >= 400) { 
        cout << "  System full.\n"; 
        return; 
    }

    cout << "  Select vehicle to apply for:\n";
    printLine();
    for (int i = 0; i < vCount; i++) {
        vehicle& v = vehicles[idx[i]];
        string tag;
        if (hasPendingOrApprovedForVehicle(v.vehicleID)){  
             tag = " [Pending/Approved]";
        }else if (hasActivePaidPassForVehicle(v.vehicleID)){ 
             tag = " [Active — use Renew]";
        }else{                                                 
            tag = " [No active pass]";
        }
        cout << "  " << (i + 1) << ". " << v.plate << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << vCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, vCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[idx[pick - 1]];
    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  Already has a pending/approved application. Please wait.\n";
        return;
    }
    if (hasActivePaidPassForVehicle(chosen.vehicleID)) {
        cout << "  Has an active pass — use Renew Pass instead.\n";
        return;
    }

    application APP;
    APP.studentID  = studentID;
    APP.vehicleID  = chosen.vehicleID;
    APP.faculty    = students[index_Student].faculty;
    APP.applyDate  = getCurrentDate();
    APP.applyMonth = getCurrentMonth();
    APP.status     = "pending";

    cout << "  Months (1-3): ";
    APP.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2) << (APP.months * 30.0) << "\n";

    apps[appsCount++] = APP;
    saveApplicationsToFile();
    cout << "  Application for " << chosen.plate << " submitted on "
         << APP.applyDate << ". Pending admin approval.\n";
}

// ============================================================
// APPLICATION — RENEW  (a)
// ============================================================

void renewApplication(int index_Student) {
    printHeader("Renew Parking Pass");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) { cout << "  No vehicles registered.\n"; return; }

    // Only show vehicles with an active paid pass
    int eligIdx[20]; int eligCount = 0;
    for (int i = 0; i < vCount; i++) {
        if (hasActivePaidPassForVehicle(vehicles[idx[i]].vehicleID)) {
            eligIdx[eligCount++] = idx[i];
        }
    }

    if (eligCount == 0) {
        cout << "  No vehicle has an active pass to renew.\n";
        cout << "  Use New Application instead.\n";
        return;
    }

    cout << "  Select vehicle to renew:\n"; printLine();
    for (int i = 0; i < eligCount; i++) {
        vehicle& v = vehicles[eligIdx[i]];
        string tag = hasPendingOrApprovedForVehicle(v.vehicleID) ? " [Renewal pending]" : "";
        cout << "  " << (i + 1) << ". " << v.plate << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << eligCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, eligCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[eligIdx[pick - 1]];
    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  A renewal for this vehicle is already pending.\n";
        return;
    }

    // Show current pass details — search backwards for most recent paid pass
    int paidIdx = -1;
    for (int i = appsCount - 1; i >= 0; i--) {
        if (apps[i].vehicleID == chosen.vehicleID && apps[i].status == "paid") {
            paidIdx = i; break;
        }
    }
    cout << "\n  Active pass details:\n";
    cout << "  Vehicle    : " << chosen.plate << " (" << chosen.type << ")\n";
    cout << "  Applied on : " << apps[paidIdx].applyDate << "\n";
    cout << "  Duration   : " << apps[paidIdx].months << " month(s)\n";
    printLine();

    char confirm;
    cout << "  Confirm renewal? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') { cout << "  Renewal cancelled.\n"; return; }
    if (appsCount >= 400) { cout << "  System full.\n"; return; }

    application renewal;
    renewal.studentID  = studentID;
    renewal.vehicleID  = chosen.vehicleID;
    renewal.faculty    = students[index_Student].faculty;
    renewal.applyDate  = getCurrentDate();
    renewal.applyMonth = getCurrentMonth();
    renewal.status     = "pending";

    cout << "  Months to renew (1-3): ";
    renewal.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2) << (renewal.months * 30.0) << "\n";

    apps[appsCount++] = renewal;
    saveApplicationsToFile();
    cout << "\n  Renewal submitted on " << renewal.applyDate << ".\n";
    cout << "  Current pass stays active until you pay for this renewal.\n";
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void viewStudentProfile(int index_Student) {
    if (index_Student < 0 || index_Student >= studentCount) return;
    printHeader("Student Profile");
    student& s = students[index_Student];

    printLine();
    cout << "  ID      : " << s.id         << "\n";
    cout << "  Name    : " << s.name       << "\n";
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";
    printLine();

    cout << "  REGISTERED VEHICLES\n"; printLine();
    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID"
                              << setw(14) << "Plate"
                              << setw(14) << "Type"
                              << "Linked Application\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v = vehicles[idx[i]];
            string appInfo = "None";
            for (int j = appsCount - 1; j >= 0; j--) {
                if (apps[j].vehicleID == v.vehicleID &&
                    apps[j].status != "rejected" && apps[j].status != "expired") {
                    appInfo = "App#" + to_string(j) + " [" + apps[j].status + "]";
                    break;
                }
            }
            cout << "  " << left << setw(10) << v.vehicleID
                                  << setw(14) << v.plate
                                  << setw(14) << v.type
                                  << appInfo  << "\n";
        }
    }
    printLine();
}

void updateStudentProfile(int index_Student) {
    printHeader("Update Profile");
    cout << "  Name    : ";
    getline(cin >> ws, students[index_Student].name);
    cout << "  Phone   : ";
    getline(cin >> ws, students[index_Student].phone);

    char chFac;
    cout << "  Change faculty? (y/n): ";
    cin >> chFac;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chFac == 'y' || chFac == 'Y') {
        students[index_Student].faculty = getValidFaculty();
    }

    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y') {
        students[index_Student].password = getValidPassword();
    } else {
        cout << "  Password unchanged.\n";
    }

    saveStudentsToFile();
    cout << "  Profile updated successfully!\n";
}

// ============================================================
// STUDENT — TRACK & ANALYSE TRANSACTIONS  (d)
// ============================================================

void viewApplicationHistory(int index_Student) {
    printHeader("My Parking Pass Transactions");
    string id = students[index_Student].id;
    bool found = false;
    int count  = 1;

    // Counters for analytics
    int myNewApps = 0;
    int myRenewals = 0;
    int myPaidMonths = 0;
    double myTotalSpent = 0;

    // Monthly spend for last 12 months
    string mLabels[12]; buildMonthLabels(mLabels, 12);
    double monthSpend[12] = {};

    cout << "  " << left
         << setw(5)  << "No."
         << setw(10) << "Vehicle"
         << setw(12) << "Date"
         << setw(8)  << "Months"
         << setw(12) << "Status"
         << "Type\n";
    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != id) continue;
        found = true;

        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate   = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string appType = isRenewalApp(i) ? "Renewal" : "New";

        cout << "  " << left
             << setw(5)  << count++
             << setw(10) << plate
             << setw(12) << apps[i].applyDate
             << setw(8)  << apps[i].months
             << setw(12) << apps[i].status
             << appType  << "\n";

        // Payment prompt for approved applications
        if (apps[i].status == "approved") {
            printLine();
            cout << "  APPROVED — " << plate << " | "<< apps[i].months << " month(s) | RM " << fixed << setprecision(2) << (apps[i].months * 30.0) << "\n";
            char pay;
            cout << "  Pay now? (y/n): ";
            cin >> pay;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (pay == 'y' || pay == 'Y') {
                apps[i].status = "paid";
                //still need to work on, it just delete their current old pass, instead of renew it
                if (isRenewalApp(i)) {
                    for (int k = 0; k < appsCount; k++) {
                        if (k == i) continue;
                        if (apps[k].vehicleID == apps[i].vehicleID &&
                            apps[k].status == "paid") {
                            apps[k].status = "expired";
                        }
                    }
                }
                saveApplicationsToFile();
                cout << "  Payment successful! Pass is now active.\n";
            } else {
                cout << "  Payment skipped. You can pay later from this menu.\n";
            }
        }

        if (apps[i].status == "paid" || apps[i].status == "expired") {
            myPaidMonths += apps[i].months;
            myTotalSpent += apps[i].months * 30.0;
            for (int m = 0; m < 12; m++) {
                if (apps[i].applyMonth == mLabels[m]) {
                    monthSpend[m] += apps[i].months * 30.0;
                    break;
                }
            }
        }

        // Tally new vs renewal counts (all statuses)
        if (appType == "New"){
         myNewApps++;
        } else { 
            myRenewals++;
        }                
    }
    if (!found) { cout << "  No applications found.\n"; return; }

    // ── Personal transaction summary ─────────────────────────────
    printLine();
    cout << "  MY TRANSACTION SUMMARY\n"; printLine();
    cout << "  New applications  : " << myNewApps    << "\n";
    cout << "  Renewals          : " << myRenewals   << "\n";
    cout << "  Total months paid : " << myPaidMonths << "\n";
    cout << "  Total spent       : RM " << fixed << setprecision(2) << myTotalSpent << "\n";
    if (myPaidMonths > 0) {
        cout << "  Avg cost/month    : RM " << fixed << setprecision(2) << (myTotalSpent / myPaidMonths) << "\n";
    }

    // ── Monthly spend breakdown ──────────────────────────────────
    cout << "\n  MONTHLY PAYMENT HISTORY (last 12 months)\n";
    printLine();
    bool anyPayment = false;
    int maxSpend = 1;
    for (int m = 0; m < 12; m++) {
        if (monthSpend[m] > maxSpend){ 
            maxSpend = (int)monthSpend[m];
        }
    }
    for (int m = 0; m < 12; m++) {
        if (monthSpend[m] == 0) continue;
        anyPayment = true;
        int bar = (int)(monthSpend[m] * 20 / maxSpend);
        cout << "  " << mLabels[m] << " RM"<< setw(7) << fixed << setprecision(2) << monthSpend[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "\n";
    }
    if (!anyPayment) cout << "  No payments in the last 12 months.\n";

    // ── Trend note ───────────────────────────────────────────────
    if (myRenewals > 0 && myNewApps > 0) {
        double renewalRate = (double)myRenewals / (myNewApps + myRenewals) * 100.0;
        cout << "\n  Your renewal rate: " << fixed << setprecision(0) << renewalRate << "%";
        if (renewalRate >= 80) {
            cout << "  (Very consistent user!)";
        } else if (renewalRate >= 50) {
            cout << "  (Average User!)";
        } else {
            cout << "  (Mostly new applications.)";
        }
        cout << "\n";
    }

    printLine();

    // ── Month-end inline alert ───────────────────────────────────
    if (isApproachingMonthEnd()) {
        printLine('*');
        cout << "  *** REMINDER: Month end is approaching!       ***\n";
        cout << "  *** Consider renewing your pass now to avoid  ***\n";
        cout << "  *** disruption at the start of next month.    ***\n";
        printLine('*');
    }
}

// ============================================================
// ADMIN — PROCESS APPLICATIONS  (a)
// ============================================================

void viewProfileAdmin(int index_Admin) {
    printHeader("Admin: Process Applications");
    cout << "  Admin ID : " << admins[index_Admin].adminID << "\n";
    cout << "  Name     : " << admins[index_Admin].name    << "\n";
    cout << "  Email    : " << admins[index_Admin].email   << "\n";
    cout << "  Phone    : " << admins[index_Admin].phone   << "\n\n";

    bool found = false;
    cout << "  " << left << setw(7)  << "Index"
                         << setw(9)  << "StudID"
                         << setw(12) << "Vehicle"
                         << setw(10) << "Faculty"
                         << setw(12) << "Date"
                         << setw(8)  << "Months"
                         << "Type\n";
    printLine();

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status != "pending") continue;
        found = true;
        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate   = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string appType = isRenewalApp(i) ? "Renewal" : "New";
        cout << "  " << left  << setw(7)  << i
                              << setw(9)  << apps[i].studentID
                              << setw(12) << plate
                              << setw(10) << apps[i].faculty
                              << setw(12) << apps[i].applyDate
                              << setw(8)  << apps[i].months
                              << appType  << "\n";
    }

    if(!found){ 
        cout << "  No pending applications.\n"; 
        return; 
    }

    cout << "\n  Enter index to process (-1 to cancel): ";
    int choice = safeInputInt(-1, appsCount - 1);
    if (choice == -1) return;

    if (choice >= 0 && choice < appsCount) {
        if (apps[choice].status == "pending") approveRejectApplication(choice);
        else cout << "  Already processed.\n";
    } else {
        cout << "  Invalid index.\n";
    }
}

void approveRejectApplication(int app_index) {
    if (app_index < 0 || app_index >= appsCount) return;

    int vehicle_id = findVehicleIndexByID(apps[app_index].vehicleID);
    string plate = (vehicle_id != -1) ? vehicles[vehicle_id].plate : apps[app_index].vehicleID;

    cout << "\n  Student  : " << apps[app_index].studentID << "\n";
    cout << "  Vehicle  : " << plate << "\n";
    cout << "  Faculty  : " << apps[app_index].faculty << "\n";
    cout << "  Duration : " << apps[app_index].months << " month(s)\n";
    cout << "  Type     : " << (isRenewalApp(app_index) ? "Renewal" : "New") << "\n";
    cout << "\n  1. Approve\n  2. Reject\n  Decision: ";
    int decision = safeInputInt(1, 2);

    if (decision == 1) {
        apps[app_index].status = "approved";
        cout << "  Application approved. Student's current pass remains active until payment.\n";
    } else {
        apps[app_index].status = "rejected";
        cout << "  Application rejected.\n";
    }
    saveApplicationsToFile();
}

// ============================================================
// ADMIN — STATISTICS & ANALYTICS  (b)
// ============================================================

void statisticsUsage(int index_Admin) {
    printHeader("Admin: Statistics & Analytics");
    cout << "  Admin: " << admins[index_Admin].name << "\n";
    cout << "  Date : " << getCurrentDate() << "\n\n";

    int total    = appsCount;
    int approved = 0, rejected = 0, pending = 0, paid = 0, expired = 0;
    int durationCount[4] = {0};

    int facApps[FAC_COUNT]     = {0};
    int facApproved[FAC_COUNT] = {0};
    int facPaid[FAC_COUNT]     = {0};
    int facRejected[FAC_COUNT] = {0};
    int facNew[FAC_COUNT]      = {0};
    int facRenew[FAC_COUNT]    = {0};

    string mLabels[12]; buildMonthLabels(mLabels, 12);
    int monthApps[12]     = {0};
    int monthRevenue[12]  = {0};

    for (int i = 0; i < appsCount; i++) {
        if      (apps[i].status == "approved") approved++;
        else if (apps[i].status == "rejected") rejected++;
        else if (apps[i].status == "pending")  pending++;
        else if (apps[i].status == "paid")     paid++;
        else if (apps[i].status == "expired")  expired++;

        if (apps[i].months >= 1 && apps[i].months <= 3){
            durationCount[apps[i].months]++;
        }

        int fi = facultyIndex(apps[i].faculty);

        if (fi != -1) {
            facApps[fi]++;
            if (apps[i].status == "approved")  facApproved[fi]++;
            if (apps[i].status == "paid")      facPaid[fi]++;
            if (apps[i].status == "rejected")  facRejected[fi]++;
            if (isRenewalApp(i)) facRenew[fi]++;
            else                 facNew[fi]++;
        }

        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                monthApps[m]++;
                if (apps[i].status == "paid") monthRevenue[m] += apps[i].months * 30;
                break;
            }
        }
    }

    int activeNow = paid + approved;
    double utilRate = studentCount > 0 ? (double)activeNow / studentCount * 100.0 : 0.0;

    // ── Overall summary ──────────────────────────────────────────
    printLine();
    cout << "  OVERALL SUMMARY\n"; printLine();
    cout << "  Total students       : " << studentCount << "\n";
    cout << "  Total vehicles       : " << vehicleCount << "\n";
    cout << "  Total applications   : " << total        << "\n";
    cout << "  Approved (unpaid)    : " << approved     << "\n";
    cout << "  Paid / Active        : " << paid         << "\n";
    cout << "  Rejected             : " << rejected     << "\n";
    cout << "  Pending              : " << pending      << "\n";
    cout << "  Expired              : " << expired      << "\n";
    cout << "  Utilisation rate     : " << fixed << setprecision(1) << utilRate << "%\n";

    // ── Faculty breakdown ────────────────────────────────────────
    printLine();
    cout << "  FACULTY BREAKDOWN\n"; printLine();

    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++) {
        if (facApps[f] > facApps[peakFac]) peakFac = f;
    }

    cout << "  " << left
         << setw(12) << "Faculty"
         << setw(8)  << "Apps"
         << setw(8)  << "%"
         << setw(8)  << "Paid"
         << setw(10) << "Rejected"
         << setw(8)  << "New"
         << "Renewals\n";
    printLine();

    for (int f = 0; f < FAC_COUNT; f++) {
        double pct = (total > 0) ? (double)facApps[f] / total * 100.0 : 0.0;
        string star = (f == peakFac && facApps[peakFac] > 0) ? " <-- Most" : "";
        cout << "  " << left
             << setw(12) << FAC_LABELS[f]
             << setw(8)  << facApps[f]
             << setw(8)  << (to_string((int)round(pct)) + "%")
             << setw(8)  << facPaid[f]
             << setw(10) << facRejected[f]
             << setw(8)  << facNew[f]
             << facRenew[f] << star << "\n";
    }

    // Faculty bar chart
    cout << "\n  FACULTY CHART\n"; printLine();
    for (int f = 0; f < FAC_COUNT; f++) {
        cout << "  " << left << setw(10) << FAC_LABELS[f] << ": ";
        for (int j = 0; j < facApps[f] && j < 40; j++) cout << "#";
        if (facApps[f] > 40) cout << "+";
        cout << "  (" << facApps[f] << ")\n";
    }

    // ── Duration breakdown ───────────────────────────────────────
    printLine();
    cout << "  BY PASS DURATION\n"; printLine();
    for (int m = 1; m <= 3; m++) {
        cout << "  " << m << " month(s) : ";
        for (int j = 0; j < durationCount[m] && j < 40; j++) cout << "#";
        if (durationCount[m] > 40) cout << "+";
        cout << "  (" << durationCount[m] << ")\n";
    }

    // ── Monthly trend ────────────────────────────────────────────
    printLine();
    cout << "  MONTHLY APPLICATION TREND (last 12 months)\n"; 
    printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) {
        if (monthApps[m] > maxBar) maxBar = monthApps[m];
    }
    for (int m = 0; m < 12; m++) {
        int bar = (monthApps[m] * 30) / maxBar;
        cout << "  " << mLabels[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << " apps, RM " << fixed << setprecision(0) << (double)monthRevenue[m] << ")\n";
    }
    printLine();
    cout << "  Run 'Generate Summary Report' for the full printable report.\n";
}

// BE AWARE OF NOT UNCOMMENT THIS PART ONLY UNDER PROTOTYPE
// ============================================================
// ADMIN — STATISTICS BY YEAR (COMMENTED OUT — NOT IN MAIN MENU)
// To activate: uncomment this function and add option 7 to adminMenu().
// ============================================================
/*
void statisticsUsageByYear(int index_Admin) {
    printHeader("Admin: Statistics by Year");
    cout << "  Admin: " << admins[index_Admin].name << "\n\n";

    // Determine the range of years that appear in applications data
    int minYear = 9999, maxYear = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].applyMonth.size() < 4) continue;
        try {
            int yr = stoi(apps[i].applyMonth.substr(0, 4));
            if (yr < minYear) minYear = yr;
            if (yr > maxYear) maxYear = yr;
        } catch (...) {}
    }

    if (minYear > maxYear) {
        cout << "  No application data available.\n";
        return;
    }

    // Build list of available years
    int yearList[50]; int yearCount = 0;
    for (int y = minYear; y <= maxYear && yearCount < 50; y++) {
        yearList[yearCount++] = y;
    }

    // Let admin choose which year to view
    cout << "  Available years:\n";
    for (int i = 0; i < yearCount; i++) {
        cout << "  " << (i + 1) << ". " << yearList[i] << "\n";
    }
    cout << "  0. Cancel\n  Choice: ";
    int pick = safeInputInt(0, yearCount);
    if (pick == 0) return;

    int selectedYear = yearList[pick - 1];

    // Build 12 month labels for the selected year
    string mLabels[12];
    for (int m = 0; m < 12; m++) {
        char buf[8];
        sprintf(buf, "%04d-%02d", selectedYear, m + 1);
        mLabels[m] = string(buf);
    }

    cout << "\n  Year: " << selectedYear << "\n";
    printLine();

    int monthApps[12]    = {0};
    int monthRevenue[12] = {0};
    int facApps[FAC_COUNT]  = {0};
    int facPaid[FAC_COUNT]  = {0};
    int totalApps = 0, totalRevenue = 0;

    for (int i = 0; i < appsCount; i++) {
        if (apps[i].applyMonth.substr(0, 4) != to_string(selectedYear)) continue;
        totalApps++;
        int fi = facultyIndex(apps[i].faculty);
        if (fi != -1) {
            facApps[fi]++;
            if (apps[i].status == "paid") facPaid[fi]++;
        }
        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                monthApps[m]++;
                if (apps[i].status == "paid") {
                    int rev = apps[i].months * 30;
                    monthRevenue[m] += rev;
                    totalRevenue    += rev;
                }
                break;
            }
        }
    }

    cout << "  Total applications in " << selectedYear << " : " << totalApps    << "\n";
    cout << "  Total revenue collected               : RM "
         << fixed << setprecision(2) << (double)totalRevenue << "\n";

    // Monthly bar chart for selected year
    cout << "\n  MONTHLY TREND — " << selectedYear << "\n"; printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) {
        if (monthApps[m] > maxBar) maxBar = monthApps[m];
    }
    for (int m = 0; m < 12; m++) {
        int bar = (monthApps[m] * 30) / maxBar;
        cout << "  " << mLabels[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << " apps, RM "
             << fixed << setprecision(0) << (double)monthRevenue[m] << ")\n";
    }

    // Faculty breakdown for selected year
    printLine();
    cout << "  FACULTY BREAKDOWN — " << selectedYear << "\n"; printLine();
    cout << "  " << left << setw(12) << "Faculty" << setw(8) << "Apps" << "Paid\n";
    printLine();
    for (int f = 0; f < FAC_COUNT; f++) {
        cout << "  " << left << setw(12) << FAC_LABELS[f]
                              << setw(8)  << facApps[f]
                              << facPaid[f] << "\n";
    }
    printLine();
}
*/
// To add this to the admin menu, replace adminMenu() option block with:
//   cout << "  7. Statistics by Year (Year Filter)\n";
// and add: else if (choice == 7) { statisticsUsageByYear(index_Admin); }
// and update safeInputInt(1, 6) to safeInputInt(1, 7).

// ============================================================
// ADMIN — SUMMARY REPORT  (c)
// ============================================================

void generateSummaryReport(int index_Admin) {
    printHeader("Admin: Summary Report");
    cout << "  Generated by : " << admins[index_Admin].name << "\n";
    cout << "  Date         : " << getCurrentDate() << "\n\n";

    string mLabels[12];
    buildMonthLabels(mLabels, 12);

    int facMonthApps[FAC_COUNT][12] = {};
    int facTotal[FAC_COUNT]         = {0};
    int facNew[FAC_COUNT]           = {0};
    int facRenew[FAC_COUNT]         = {0};
    int facPaidMonths[FAC_COUNT]    = {0};
    int facRevenue[FAC_COUNT]       = {0};
    int monthTotal[12]              = {0};
    int monthRevenue[12]            = {0};

    int totalRevenue = 0, paidCount = 0, totalPaidMonths = 0;

    for (int i = 0; i < appsCount; i++) {
        int fi = facultyIndex(apps[i].faculty);
        if (fi == -1) continue;
            facTotal[fi]++;
        if (isRenewalApp(i)) facRenew[fi]++;
        else facNew[fi]++;

        if (apps[i].status == "paid") {
            facPaidMonths[fi] += apps[i].months;
            facRevenue[fi]    += apps[i].months * 30;
            totalRevenue      += apps[i].months * 30;
            totalPaidMonths   += apps[i].months;
            paidCount++;
        }

        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                facMonthApps[fi][m]++;
                monthTotal[m]++;
                if (apps[i].status == "paid") monthRevenue[m] += apps[i].months * 30;
                break;
            }
        }
    }

    if (appsCount == 0) {
        cout << "  No applications data available yet.\n";
        printLine('=', 55);
        cout << "  [End of Report]\n";
        return;
    }

    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++) {
        if (facTotal[f] > facTotal[peakFac])  {
            peakFac = f;
        }
    }

    int peakMonth = 0;
    for (int m = 1; m < 12; m++) {
        if (monthTotal[m] > monthTotal[peakMonth]) {
            peakMonth = m;
        }
    }

    // ── SECTION 1 ────────────────────────────────────────────────
    cout << "  ================================================\n";
    cout << "  SECTION 1: Application Averages by Faculty\n";
    cout << "  ================================================\n";

    for (int f = 0; f < FAC_COUNT; f++) {
        double avg = facTotal[f] / 12.0;
        cout << "\n  [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";
        printLine('-', 55);
        cout << "  " << left
             << setw(10) << "Total"
             << setw(10) << "New"
             << setw(12) << "Renewals"
             << setw(12) << "Avg/Month"
             << setw(13) << "PaidMonths"
             << "Revenue\n";
        cout << "  " << left
             << setw(10) << facTotal[f]
             << setw(10) << facNew[f]
             << setw(12) << facRenew[f]
             << setw(12) << fixed << setprecision(1) << avg
             << setw(13) << facPaidMonths[f]
             << "RM " << fixed << setprecision(2) << (double)facRevenue[f] << "\n";
    }

    cout << "\n";
    printLine('=', 55);
    if (facTotal[peakFac] > 0) {
        cout << "  Highest demand: [" << FAC_CODES[peakFac]
             << "] " << FAC_LABELS[peakFac] << "\n";
    }

    // ── SECTION 2 ────────────────────────────────────────────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 2: Monthly Volume by Faculty (last 12 months)\n";
    cout << "  ================================================\n";

    cout << "\n  [ Faculties A to E ]\n";
    cout << "  " << left << setw(10) << "Month";
    for (int f = 0; f < 5 && f < FAC_COUNT; f++) cout << setw(6) << ("[" + FAC_CODES[f] + "]");
    cout << "  Total\n";
    printLine('-', 46);
    for (int m = 0; m < 12; m++) {
        cout << "  " << left << setw(10) << mLabels[m];
        for (int f = 0; f < 5 && f < FAC_COUNT; f++) cout << setw(6) << facMonthApps[f][m];
        cout << "  " << monthTotal[m] << "\n";
    }
    printLine('-', 46);

    if (FAC_COUNT > 5) {
        cout << "\n  [ Faculties F to " << FAC_CODES[FAC_COUNT - 1] << " ]\n";
        cout << "  " << left << setw(10) << "Month";
        for (int f = 5; f < FAC_COUNT; f++) cout << setw(6) << ("[" + FAC_CODES[f] + "]");
        cout << "  Revenue\n";
        printLine('-', 44);
        for (int m = 0; m < 12; m++) {
            cout << "  " << left << setw(10) << mLabels[m];
            for (int f = 5; f < FAC_COUNT; f++) cout << setw(6) << facMonthApps[f][m];
            cout << "  RM " << right << fixed << setprecision(0)
                 << (double)monthRevenue[m] << "\n";
        }
        printLine('-', 44);
    }

    cout << "\n  Faculty Code Reference:\n";
    for (int f = 0; f < FAC_COUNT; f++) {
        cout << "    [" << FAC_CODES[f] << "] " << FAC_LABELS[f] << "\n";
    }
    if (monthTotal[peakMonth] > 0) {
        cout << "\n  Peak month: " << mLabels[peakMonth]
             << " (" << monthTotal[peakMonth] << " apps)\n";
    } else {
        cout << "\n  Peak month: No data yet.\n";
    }

    // ── SECTION 3 ────────────────────────────────────────────────
    int activeNow = 0;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].status == "paid" || apps[i].status == "approved") activeNow++;
    }
    double utilRate  = studentCount > 0 ? (double)activeNow / studentCount * 100.0 : 0.0;
    double avgMonths = paidCount > 0 ? (double)totalPaidMonths / paidCount : 0.0;

    cout << "\n  ================================================\n";
    cout << "  SECTION 3: Car Park Utilisation - Sungai Long\n";
    cout << "  ================================================\n";
    printLine('-', 55);
    cout << "  Registered students       : " << studentCount  << "\n";
    cout << "  Registered vehicles       : " << vehicleCount  << "\n";
    cout << "  Currently active passes   : " << activeNow     << "\n";
    cout << "  Utilisation rate          : "
         << fixed << setprecision(1) << utilRate << "%\n";
    cout << "  Total revenue collected   : RM "
         << fixed << setprecision(2) << (double)totalRevenue << "\n";
    cout << "  Avg months per paid pass  : ";
    if (paidCount > 0) cout << fixed << setprecision(1) << avgMonths << "\n";
    else               cout << "N/A (no paid passes yet)\n";
    cout << "  Avg monthly revenue       : RM ";
    if (totalRevenue > 0) cout << fixed << setprecision(2) << (totalRevenue / 12.0) << "\n";
    else                  cout << "0.00\n";
    printLine('-', 55);

    // ── SECTION 4 ────────────────────────────────────────────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 4: Negotiation Insights (UTAR vs MPKJ)\n";
    cout << "  ================================================\n";
    printLine('-', 55);

    if (activeNow == 0) {
        cout << "  No active passes yet — insufficient data for insights.\n";
        printLine('=', 55);
        cout << "  [End of Report]\n";
        return;
    }

    cout << "  1. " << activeNow
         << " students actively using MPKJ passes = steady revenue.\n";
    cout << "  2. Utilisation rate of "
         << fixed << setprecision(1) << utilRate << "% shows consistent demand.\n";
    if (facTotal[peakFac] > 0) {
        cout << "  3. Highest demand from [" << FAC_CODES[peakFac] << "] "
             << FAC_LABELS[peakFac]
             << "\n     -- consider faculty-bundle pricing.\n";
    }
    if (monthTotal[peakMonth] > 0) {
        cout << "  4. Peak month: " << mLabels[peakMonth]
             << " -- useful for MPKJ capacity planning.\n";
    }
    cout << "  5. Avg " << fixed << setprecision(1) << avgMonths
         << " months/pass -- bulk discount for 3-month passes\n"
         << "     could increase revenue and student loyalty.\n";
    cout << "  6. Total RM " << fixed << setprecision(2) << (double)totalRevenue
         << " paid -- demonstrates UTAR's bargaining power.\n";

    printLine('=', 55);
    cout << "  [End of Report]\n";
}

// ============================================================
// ADMIN — VIEW STUDENT PROFILE
// ============================================================

void adminViewStudentProfile(int index_Admin) {
    printHeader("Admin: View Student Profile");
    if (studentCount == 0) { 
        cout << "  No students registered.\n"; 
        return; 
    }

    cout << "  " << left << setw(6) << "No."
                         << setw(8) << "ID"
                         << setw(22) << "Name"
                         << setw(10) << "Faculty"
                         << "Phone\n";
    printLine();
    for (int i = 0; i < studentCount; i++) {
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(8)  << students[i].id
                              << setw(22) << students[i].name
                              << setw(10) << students[i].faculty
                              << students[i].phone << "\n";
    }
    printLine();

    cout << "  Select student (0 to cancel): ";
    int pick = safeInputInt(0, studentCount);
    if (pick == 0) return;

    student& s = students[pick - 1];
    printLine('=');
    cout << "  " << s.name << " (" << s.id << ")\n";
    printLine('=');
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";

    printLine();
    cout << "  VEHICLES\n"; printLine();
    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID"
                             << setw(16) << "Plate"
                             << setw(14) << "Type"
                             << "Active Pass\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v = vehicles[idx[i]];
            string pass = hasActivePaidPassForVehicle(v.vehicleID) ? "Yes" : "No";
            cout << "  " << left << setw(10) << v.vehicleID
                                 << setw(16) << v.plate
                                 << setw(14) << v.type << pass << "\n";
        }
    }

    printLine();
    cout << "  APPLICATION HISTORY\n"; printLine();
    cout << "  " << left << setw(6) << "Idx" << setw(12) << "Vehicle"
                         << setw(12) << "Date" << setw(8) << "Months"
                         << setw(12) << "Status" << "Type\n";
    printLine();
    bool any = false;
    for (int i = 0; i < appsCount; i++) {
        if (apps[i].studentID != s.id) continue;
        any = true;
        int vi = findVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        cout << "  " << left << setw(6) << i
                              << setw(12) << plate
                              << setw(12) << apps[i].applyDate
                              << setw(8)  << apps[i].months
                              << setw(12) << apps[i].status
                              << (isRenewalApp(i) ? "Renewal" : "New") << "\n";
    }
    if (!any) cout << "  No applications found.\n";
    printLine();
}

// ============================================================
// ADMIN — UPDATE PROFILE
// ============================================================

void updateAdminProfile(int index_Admin) {
    printHeader("Update Admin Profile");
    cout << "  Name : ";
    getline(cin >> ws, admins[index_Admin].name);
    cout << "  Email: ";
    getline(cin >> ws, admins[index_Admin].email);
    cout << "  Phone: ";
    getline(cin >> ws, admins[index_Admin].phone);
    
    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y') {
        admins[index_Admin].password = getValidPassword();
    } else {
        cout << "  Password unchanged.\n";
    }

    saveAdminToFile();
    cout << "  Admin profile updated!\n";
}

// ============================================================
// MENUS
// ============================================================

void studentMenu(int index_Student) {
    monthEndAlert(index_Student);
    while (true) {
        printHeader("Student Menu");
        cout << "  1. View Profile\n";
        cout << "  2. Update Profile\n";
        cout << "  3. Manage Vehicles\n";
        cout << "  4. New Application\n";
        cout << "  5. Renew Pass\n";
        cout << "  6. My Transactions & Analytics\n";
        cout << "  7. Logout\n";
        printLine();
        cout << "  Choice: ";
        int choice = safeInputInt(1, 7);

        if      (choice == 1) viewStudentProfile(index_Student);
        else if (choice == 2) updateStudentProfile(index_Student);
        else if (choice == 3) manageVehicles(index_Student);
        else if (choice == 4) registerApplication(index_Student);
        else if (choice == 5) renewApplication(index_Student);
        else if (choice == 6) viewApplicationHistory(index_Student);
        else if (choice == 7) { cout << "  Logging out...\n"; break; }
    }
}

void adminMenu(int index_Admin) {
    while (true) {
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. View Student Profile\n";
        cout << "  3. Statistics & Analytics\n";
        cout << "  4. Generate Summary Report\n";
        cout << "  5. Update Profile\n";
        cout << "  6. Logout\n";
        // NOTE: Option 7 (Statistics by Year) is implemented but commented out.
        // Uncomment statisticsUsageByYear() above, change safeInputInt to (1,7),
        // and add: cout << "  7. Statistics by Year\n"; plus the else-if below.
        printLine();
        cout << "  Choice: ";
        int choice = safeInputInt(1, 6);

        if      (choice == 1) viewProfileAdmin(index_Admin);
        else if (choice == 2) adminViewStudentProfile(index_Admin);
        else if (choice == 3) statisticsUsage(index_Admin);
        else if (choice == 4) generateSummaryReport(index_Admin);
        else if (choice == 5) updateAdminProfile(index_Admin);
        else if (choice == 6) { cout << "  Logging out...\n"; break; }
        // else if (choice == 7) statisticsUsageByYear(index_Admin);
    }
}

void mainMenu() {
    int choice;
    string password, id;

    while (true) {
        printHeader("Main Menu");
        cout << "  1. Student Sign Up\n \t\t 2. Admin Sign up" << "\n";
        cout << "  2. Admin Sign Up\n";
        cout << "  3. Student Login\n";
        cout << "  4. Admin Login\n";
        cout << "  5. Exit\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 5);

        if (choice == 1) {
            registerStudent();

        } else if (choice == 2) {
            registerAdmin();

        } else if (choice == 3) {
            bool loginDone = false;
            for (int idAttempt = 0; idAttempt < 3 && !loginDone; idAttempt++) {
                cout << "  Student ID: ";
                cin >> id;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                int student_index = findStudentIndexByID(id);
                if (student_index == -1) {
                    if (idAttempt < 2) {
                        cout << "  Invalid ID. (" << (2 - idAttempt) << " attempt(s) left).\n";
                    } else {
                        cout << "  Invalid ID. Too many failed attempts. Returning to main menu.\n";
                        loginDone = true;
                    }
                    continue;
                }
                // Valid ID — try password up to 3 times
                bool ok = false;
                for (int att = 0; att < 3; att++) {

                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (students[student_index].password == password) {
                        cout << "  Login Successful!\n";
                        cleanupExpiredPasses(students[student_index].id);
                        studentMenu(student_index);
                        ok = true;
                        break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if (!ok) {
                    cout << "  Too many failed password attempts. Returning to main menu.\n";
                }
                loginDone = true; // always return to main menu after one complete attempt
            }

        } else if (choice == 4) {
            bool loginDone = false;
            for (int idAttempt = 0; idAttempt < 3 && !loginDone; idAttempt++) {
                cout << "  Admin ID: ";
                cin >> id;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                int ai = findAdminIndexByID(id);
                if (ai == -1) {
                    if (idAttempt < 2) {
                        cout << "  Invalid ID. (" << (2 - idAttempt) << " attempt(s) left).\n";
                    } else {
                        cout << "  Invalid ID. Too many failed attempts. Returning to main menu.\n";
                        loginDone = true;
                    }
                    continue;
                }
                // Valid ID — try password up to 3 times
                bool ok = false;

                for (int att = 0; att < 3; att++) {
                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (admins[ai].password == password) {
                        cout << "  Login Successful!\n";
                        adminMenu(ai);
                        ok = true;
                        break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if (!ok) {
                    cout << "  Too many failed password attempts. Returning to main menu.\n";
                }
                loginDone = true; // always return to main menu after one complete attempt
            }

        } else if (choice == 5) {
            cout << "  Exiting...\n";
            break;
        }
    }
}