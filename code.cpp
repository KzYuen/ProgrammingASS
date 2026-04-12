#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ctime>
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
    string studentID;
    string vehicleID;
    string faculty;   // snapshot of student's faculty at application time
    int    months;
    string status;    // pending | approved | rejected | paid | expired
    string applyDate;
    string applyMonth;
};

struct admin {
    string adminID;
    string name;
    string password;
};

// ============================================================
// GLOBALS
// ============================================================

student     students[200];
vehicle     vehicles[600];
application apps[400];
admin       admins[20];

int studentCount = 0;
int vehicleCount = 0;
int appsCount    = 0;
int adminCount   = 0;

// Faculty table — change display names here without touching any other code
const string FAC_CODES[9]  = { "A",      "B",    "C",     "D",    "E",    "F",    "G",    "H",    "I" };
const string FAC_LABELS[9] = { "A- M. Kandiah Faculty of Medicine and Health Sciences", "B-Lee Kong Chian Faculty of Engineering and Science", "C-Faculty of Engineering and Green Technologies", "D-Faculty of Information and Communication Technology", "E-Faculty of Science", "F-Faculty of Accountancy and Management", "G-Teh Hong Piow Faculty of Business and Finance", "H-Faculty of Arts and Social Sciences", "I-Faculty of Creative Industries" };
const int    FAC_COUNT = 9;

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

// File I/O
void LoadStudentsFromFile();
void LoadVehiclesFromFile();
void LoadApplicationsFromFile();
void LoadAdminFromFile();
void SaveStudentsToFile();
void SaveVehiclesToFile();
void SaveAdminToFile();
void SaveApplicationsToFile();

// Registration
void register_stud();
void register_admin();

// Vehicle management
void registerVehicle(string studentID);
void viewVehicles(string studentID);
void manageVehicles(int index_Student);
int  getVehiclesForStudent(string studentID, int results[], int maxCount);
int  FindVehicleIndexByID(string vehicleID);

// Applications
void register_application(int index_Student);
void renew_application(int index_Student);
void viewApplicationHistory(int index_Student);   // student tracking & analytics

// Profile
void ViewStudentProfile(int index_Student);
void UpdateStudentProfile(int index_Student);
void UpdatesAdminProfile(int index_Admin);
void AdminViewStudentProfile(int index_Admin);

// Admin processing
void ViewProfileAdmin(int index_Admin);           // (a) approve / reject
void approveRejectApplication(int app_index);
void statisticsUsage(int index_Admin);            // (b) monthly / yearly analytics
void generateSummaryReport(int index_Admin);      // (c) negotiation report

// Helpers
int    FindStudentIndexByID(string id);
int    FindAdminIndexByID(string id);
bool   hasPendingOrApprovedForVehicle(string vehicleID);
bool   hasActivePaidPassForVehicle(string vehicleID);
int    facultyIndex(string fac);
bool   isRenewalApp(int appIdx);
string getValidPassword();
string getValidFaculty();
int    safeInputInt(int min, int max);
void   CleanupExpiredPasses(string studentID);

// Alerts
void monthEndAlert(int index_Student);            // (e) month-end reminder

// Menus
void studentMenu(int index_Student);
void adminMenu(int index_Admin);
void MainMenu();

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
    LoadStudentsFromFile();
    LoadVehiclesFromFile();
    LoadApplicationsFromFile();
    LoadAdminFromFile();

    CleanupExpiredPasses("");   // auto-expire passes on startup
    MainMenu();

    SaveStudentsToFile();
    SaveVehiclesToFile();
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
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);//prevent buffer overflow - "overflow" can cause security vulnerabilities and crashes. Always ensure your buffers are large enough for the data they will hold, including the null terminator.
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
        while (mo < 0) { mo += 12; yr--; }
        char buf[8];
        sprintf(buf, "%04d-%02d", yr, mo + 1);
        labels[count - 1 - m] = string(buf);
    }
}

// ============================================================
// DISPLAY HELPERS
// ============================================================

void printLine(char c, int n) {
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
        cout << "  Password (min 12 chars, upper+lower+digit+special): ";
        getline(cin >> ws, pw);

        if ((int)pw.length() < 12) {
            cout << "  Password must be at least 12 characters.\n";
            continue;
        }
        bool hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pw) {
            if      (isupper(c)) hasU = true;
            else if (islower(c)) hasL = true;
            else if (isdigit(c)) hasD = true;
            else if (ispunct(c)) hasS = true;
        }
        if (hasU && hasL && hasD && hasS){
             return pw; 
        }
        cout << "  Must include uppercase, lowercase, digit, and special character.\n";
    }
}

// Forces the user to enter a valid faculty code (A-I).
string getValidFaculty() {
    string fac;
    while (true) {
        cout << "  Faculty — A:M. Kandiah  B:Lee Kong Chian  C:FEG  D:FICT  E:FAS  F:FAM  G:Teh Hong Piow  H:FAS  I:FCI — Enter code: ";
        getline(cin >> ws, fac);
        for (char& c : fac){
             c = toupper(c);
        }
        for (int i = 0; i < FAC_COUNT; i++){
            if (FAC_CODES[i] == fac) return fac;
        }
        cout << "  Invalid faculty. Please enter A, B, C, D, E, F, G, H, or I.\n";
    }
}

// ============================================================
// FILE I/O
// ============================================================

void LoadStudentsFromFile() {
    ifstream file("students.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || studentCount >= 200) continue;
        stringstream ss(line);
        student& s = students[studentCount];
        if (getline(ss, s.id,         '|') &&
            getline(ss, s.name,       '|') &&
            getline(ss, s.stud_email, '|') &&
            getline(ss, s.faculty,    '|') &&
            getline(ss, s.phone,      '|') &&
            getline(ss, s.password))
            studentCount++;
    }
}

void LoadVehiclesFromFile() {
    ifstream file("vehicles.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || vehicleCount >= 600) continue;
        stringstream ss(line);
        vehicle& v = vehicles[vehicleCount];
        if (getline(ss, v.vehicleID, '|') &&
            getline(ss, v.studentID, '|') &&
            getline(ss, v.plate,     '|') &&
            getline(ss, v.type))
            vehicleCount++;
    }
}

void LoadApplicationsFromFile() {
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
                if (temp.months >= 1 && temp.months <= 3)
                    apps[appsCount++] = temp;
            } catch (...) {}
        }
    }
}

void LoadAdminFromFile() {
    ifstream file("admin.txt");
    string line;
    while (getline(file, line)) {
        if (line.empty() || adminCount >= 20) continue;
        stringstream ss(line);
        admin& a = admins[adminCount];
        if (getline(ss, a.adminID,  '|') &&
            getline(ss, a.name,     '|') &&
            getline(ss, a.password))
            adminCount++;
    }
}

void SaveStudentsToFile() {
    ofstream file("students.txt");
    for (int i = 0; i < studentCount; i++)
        file << students[i].id         << "|"
             << students[i].name       << "|"
             << students[i].stud_email << "|"
             << students[i].faculty    << "|"
             << students[i].phone      << "|"
             << students[i].password   << "\n";
}

void SaveVehiclesToFile() {
    ofstream file("vehicles.txt");
    for (int i = 0; i < vehicleCount; i++)
        file << vehicles[i].vehicleID << "|"
             << vehicles[i].studentID << "|"
             << vehicles[i].plate     << "|"
             << vehicles[i].type      << "\n";
}

void SaveAdminToFile() {
    ofstream file("admin.txt");
    for (int i = 0; i < adminCount; i++)
        file << admins[i].adminID  << "|"
             << admins[i].name     << "|"
             << admins[i].password << "\n";
}

void SaveApplicationsToFile() {
    ofstream file("applications.txt");
    for (int i = 0; i < appsCount; i++)
        file << apps[i].studentID  << "|"
             << apps[i].vehicleID  << "|"
             << apps[i].faculty    << "|"
             << apps[i].months     << "|"
             << apps[i].status     << "|"
             << apps[i].applyDate  << "|"
             << apps[i].applyMonth << "\n";
}

// ============================================================
// SMALL HELPERS
// ============================================================

int facultyIndex(string fac) {
    for (int i = 0; i < FAC_COUNT; i++)
        if (FAC_CODES[i] == fac) return i;
    return -1;
}

// Returns true if app at appIdx is a renewal
// (i.e. same vehicle had a paid/expired pass before it)
bool isRenewalApp(int appIdx) {
    for (int j = 0; j < appIdx; j++)
        if (apps[j].vehicleID == apps[appIdx].vehicleID &&
           (apps[j].status == "paid" || apps[j].status == "expired"))
            return true;
    return false;
}

int getVehiclesForStudent(string studentID, int results[], int maxCount) {
    int count = 0;
    for (int i = 0; i < vehicleCount && count < maxCount; i++)
        if (vehicles[i].studentID == studentID)
            results[count++] = i;
    return count;
}

int FindVehicleIndexByID(string vehicleID) {
    for (int i = 0; i < vehicleCount; i++)
        if (vehicles[i].vehicleID == vehicleID) return i;
    return -1;
}

bool hasPendingOrApprovedForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++)
        if (apps[i].vehicleID == vehicleID &&
           (apps[i].status == "pending" || apps[i].status == "approved"))
            return true;
    return false;
}

bool hasActivePaidPassForVehicle(string vehicleID) {
    for (int i = 0; i < appsCount; i++)
        if (apps[i].vehicleID == vehicleID && apps[i].status == "paid")
            return true;
    return false;
}

int FindStudentIndexByID(string id) {
    for (int i = 0; i < studentCount; i++)
        if (students[i].id == id) return i;
    return -1;
}

int FindAdminIndexByID(string id) {
    for (int i = 0; i < adminCount; i++)
        if (admins[i].adminID == id) return i;
    return -1;
}

// ============================================================
// CLEANUP — auto-expire passes whose duration has elapsed
// ============================================================

void CleanupExpiredPasses(string studentID) {
    string cur = getCurrentMonth();
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
    if (modified) SaveApplicationsToFile();
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

void register_stud() {
    if (studentCount >= 200) { cout << "  System full.\n"; return; }

    student stud;
    printHeader("Student Registration");

    stud.id = "S" + to_string(studentCount + 1);
    cout << "  Generated ID : " << stud.id << "\n\n";
    cout << "  Name    : "; getline(cin >> ws, stud.name);
    cout << "  Email   : "; getline(cin >> ws, stud.stud_email);
    stud.faculty = getValidFaculty();
    cout << "  Phone   : "; getline(cin >> ws, stud.phone);
    stud.password = getValidPassword();

    students[studentCount++] = stud;
    SaveStudentsToFile();
    cout << "\n  Registration successful! Your ID: " << stud.id << "\n";
    cout << "  Add your vehicle(s) via Student Menu > Manage Vehicles.\n";
}

void register_admin() {
    if (adminCount >= 20) { cout << "  System full.\n"; return; }

    admin ADM;
    printHeader("Admin Registration");

    ADM.adminID = "A" + to_string(adminCount + 1);
    cout << "  Generated Admin ID : " << ADM.adminID << "\n\n";
    cout << "  Name : "; getline(cin >> ws, ADM.name);
    ADM.password = getValidPassword();

    admins[adminCount++] = ADM;
    SaveAdminToFile();
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
    cout << "  Plate number (e.g. WXY1234) : "; getline(cin >> ws, v.plate);
    cout << "  Type (Car/Motorcycle/Van)   : "; getline(cin >> ws, v.type);

    vehicles[vehicleCount++] = v;
    SaveVehiclesToFile();
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

void register_application(int index_Student) {
    printHeader("New Parking Pass Application");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles registered. Add one via Manage Vehicles first.\n";
        return;
    }
    if (appsCount >= 400) { cout << "  System full.\n"; return; }

    cout << "  Select vehicle to apply for:\n";
    printLine();
    for (int i = 0; i < vCount; i++) {
        vehicle& v = vehicles[idx[i]];
        string tag;
        if      (hasPendingOrApprovedForVehicle(v.vehicleID)) tag = " [Pending/Approved]";
        else if (hasActivePaidPassForVehicle(v.vehicleID))    tag = " [Active — use Renew]";
        else                                                   tag = " [No active pass]";
        cout << "  " << (i + 1) << ". " << v.plate << " (" << v.type << ")" << tag << "\n";
    }
    printLine();
    cout << "  Pick (1-" << vCount << ", 0 to cancel): ";
    int pick = safeInputInt(0, vCount);
    if (pick == 0) return;

    vehicle& chosen = vehicles[idx[pick - 1]];
    if (hasPendingOrApprovedForVehicle(chosen.vehicleID)) {
        cout << "  Already has a pending/approved application. Please wait.\n"; return;
    }
    if (hasActivePaidPassForVehicle(chosen.vehicleID)) {
        cout << "  Has an active pass — use Renew Pass instead.\n"; return;
    }

    application APP;
    APP.studentID  = studentID;
    APP.vehicleID  = chosen.vehicleID;
    APP.faculty    = students[index_Student].faculty;   // snapshot
    APP.applyDate  = getCurrentDate();
    APP.applyMonth = getCurrentMonth();
    APP.status     = "pending";

    cout << "  Months (1-3): ";
    APP.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2) << (APP.months * 30.0) << "\n";

    apps[appsCount++] = APP;
    SaveApplicationsToFile();
    cout << "  Application for " << chosen.plate << " submitted on "
         << APP.applyDate << ". Pending admin approval.\n";
}

// ============================================================
// APPLICATION — RENEW  (a)
// ============================================================

void renew_application(int index_Student) {
    printHeader("Renew Parking Pass");
    string studentID = students[index_Student].id;

    int idx[20];
    int vCount = getVehiclesForStudent(studentID, idx, 20);
    if (vCount == 0) { cout << "  No vehicles registered.\n"; return; }

    // Only show vehicles with an active paid pass
    int eligIdx[20]; int eligCount = 0;
    for (int i = 0; i < vCount; i++)
        if (hasActivePaidPassForVehicle(vehicles[idx[i]].vehicleID))
            eligIdx[eligCount++] = idx[i];

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
        cout << "  A renewal for this vehicle is already pending.\n"; return;
    }

    // Show current pass details
    int paidIdx = -1;
    for (int i = appsCount - 1; i >= 0; i--)
        if (apps[i].vehicleID == chosen.vehicleID && apps[i].status == "paid")
            { paidIdx = i; break; }

    cout << "\n  Active pass details:\n";
    cout << "  Vehicle    : " << chosen.plate << " (" << chosen.type << ")\n";
    cout << "  Applied on : " << apps[paidIdx].applyDate << "\n";
    cout << "  Duration   : " << apps[paidIdx].months << " month(s)\n";
    printLine();

    char confirm;
    cout << "  Confirm renewal? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if(confirm != 'y' && confirm != 'Y'){ 
        cout << "  Renewal cancelled.\n"; return; 
    }
    if(appsCount >= 400){ 
        cout << "  System full.\n"; return; 
    }

    application renewal;
    renewal.studentID  = studentID;
    renewal.vehicleID  = chosen.vehicleID;
    renewal.faculty    = students[index_Student].faculty;   // snapshot
    renewal.applyDate  = getCurrentDate();
    renewal.applyMonth = getCurrentMonth();
    renewal.status     = "pending";

    cout << "  Months to renew (1-3): ";
    renewal.months = safeInputInt(1, 3);
    cout << "  Estimated cost: RM " << fixed << setprecision(2) << (renewal.months * 30.0) << "\n";

    apps[appsCount++] = renewal;
    SaveApplicationsToFile();
    cout << "\n  Renewal submitted on " << renewal.applyDate << ".\n";
    cout << "  Current pass stays active until admin approves this renewal.\n";
}

// ============================================================
// STUDENT PROFILE
// ============================================================

void ViewStudentProfile(int index_Student) {
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
                if (apps[j].vehicleID == v.vehicleID && apps[j].status != "rejected" && apps[j].status != "expired") {
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

void UpdateStudentProfile(int index_Student) {
    printHeader("Update Profile");
    cout << "  Name    : "; getline(cin >> ws, students[index_Student].name);
    cout << "  Phone   : "; getline(cin >> ws, students[index_Student].phone);

    char chFac;
    cout << "  Change faculty? (y/n): ";
    cin >> chFac;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chFac == 'y' || chFac == 'Y')
        students[index_Student].faculty = getValidFaculty();

    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y')
        students[index_Student].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    SaveStudentsToFile();
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
    int myNewApps = 0, myRenewals = 0;
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

        int vi = FindVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
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
            cout << "  APPROVED — " << plate << " | "
                 << apps[i].months << " month(s) | RM "
                 << fixed << setprecision(2) << (apps[i].months * 30.0) << "\n";
            char pay;
            cout << "  Pay now? (y/n): ";
            cin >> pay;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (pay == 'y' || pay == 'Y') {
                apps[i].status = "paid";
                SaveApplicationsToFile();
                cout << "  Payment successful! Pass is now active.\n";
            } else {
                cout << "  Payment skipped. You can pay later from this menu.\n";
            }
        }

        // Tally analytics
        if (appType == "New") myNewApps++; else myRenewals++;
        if (apps[i].status == "paid") {
            myPaidMonths += apps[i].months;
            myTotalSpent += apps[i].months * 30.0;
            for (int m = 0; m < 12; m++){
                if (apps[i].applyMonth == mLabels[m])
                    { monthSpend[m] += apps[i].months * 30.0; 
                        break; 
                    }
            } 
        }
    }

    if(!found){ 
        cout << "  No applications found.\n"; 
        return; 
    }

    // ── Personal transaction summary ──────────────────────────────
    printLine();
    cout << "  MY TRANSACTION SUMMARY\n"; printLine();
    cout << "  New applications  : " << myNewApps    << "\n";
    cout << "  Renewals          : " << myRenewals   << "\n";
    cout << "  Total months paid : " << myPaidMonths << "\n";
    cout << "  Total spent       : RM " << fixed << setprecision(2) << myTotalSpent << "\n";

    if (myPaidMonths > 0) {
        cout << "  Avg cost/month    : RM "
             << fixed << setprecision(2) << (myTotalSpent / myPaidMonths) << "\n";
    }

    // ── Monthly spend breakdown ───────────────────────────────────
    cout << "\n  MONTHLY PAYMENT HISTORY (last 12 months)\n"; 
    printLine();
    bool anyPayment = false;
    int maxSpend = 1;
    for(int m = 0; m < 12; m++){ 
        if (monthSpend[m] > maxSpend) maxSpend = (int)monthSpend[m];
    }
    for(int m = 0; m < 12; m++){
        if(monthSpend[m] == 0) continue;
        anyPayment = true;
        int bar = (int)(monthSpend[m] * 20 / maxSpend);
        cout << "  " << mLabels[m] << " RM"
             << setw(7) << fixed << setprecision(2) << monthSpend[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "\n";
    }
    if(!anyPayment){
        cout << "  No payments in the last 12 months.\n";
    }

    // ── Trend note ────────────────────────────────────────────────
    if(myRenewals > 0 && myNewApps > 0){
        double renewalRate = (double)myRenewals / (myNewApps + myRenewals) * 100.0;
        cout << "\n  Your renewal rate: " << fixed << setprecision(0) << renewalRate << "%";
        if(renewalRate >= 80){
            cout << "  (Very consistent user!)";
        }else if(renewalRate >= 50){
            cout << "  (Average User!.)";
        }else{
            cout << "  (Mostly new applications.)";
        }
        cout << "\n";
    }

    printLine();

    // ── Month-end inline alert ────────────────────────────────────
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

void ViewProfileAdmin(int index_Admin) {
    printHeader("Admin: Process Applications");
    cout << "  Admin ID : " << admins[index_Admin].adminID << "\n";
    cout << "  Name     : " << admins[index_Admin].name    << "\n\n";

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
        int vi = FindVehicleIndexByID(apps[i].vehicleID);
        string plate = (vi != -1) ? vehicles[vi].plate : apps[i].vehicleID;
        string appType = isRenewalApp(i) ? "Renewal" : "New";
        cout << "  " << left << setw(7)  << i
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

    int vi = FindVehicleIndexByID(apps[app_index].vehicleID);
    string plate = (vi != -1) ? vehicles[vi].plate : apps[app_index].vehicleID;

    cout << "\n  Student  : " << apps[app_index].studentID << "\n";
    cout << "  Vehicle  : " << plate << "\n";
    cout << "  Faculty  : " << apps[app_index].faculty << "\n";
    cout << "  Duration : " << apps[app_index].months << " month(s)\n";
    cout << "  Type     : " << (isRenewalApp(app_index) ? "Renewal" : "New") << "\n";
    cout << "\n  1. Approve\n  2. Reject\n  Decision: ";
    int decision = safeInputInt(1, 2);

    if (decision == 1) {
        // Expire old paid pass for the same vehicle (renewal scenario)
        for (int i = 0; i < appsCount; i++) {
            if (i == app_index) continue;
            if (apps[i].vehicleID == apps[app_index].vehicleID && apps[i].status == "paid")
                apps[i].status = "expired";
        }
        apps[app_index].status = "approved";
        cout << "  Application approved.\n";
    } else {
        apps[app_index].status = "rejected";
        cout << "  Application rejected.\n";
    }
    SaveApplicationsToFile();
}

// ============================================================
// ADMIN — STATISTICS & ANALYTICS  (b)
// ============================================================

void statisticsUsage(int index_Admin) {
    printHeader("Admin: Statistics & Analytics");
    cout << "  Admin: " << admins[index_Admin].name << "\n";
    cout << "  Date : " << getCurrentDate() << "\n\n";

    // Status counts
    int total    = appsCount;
    int approved = 0, rejected = 0, pending = 0, paid = 0, expired = 0;
    int durationCount[4] = {0};

    // Faculty counters
    int facApps[4]     = {0};
    int facApproved[4] = {0};
    int facPaid[4]     = {0};
    int facRejected[4] = {0};
    int facNew[4]      = {0};
    int facRenew[4]    = {0};

    // Monthly trend (last 12 months)
    string mLabels[12]; buildMonthLabels(mLabels, 12);
    int monthApps[12] = {0};
    int monthRevenue[12] = {0};

    for (int i = 0; i < appsCount; i++) {
        if      (apps[i].status == "approved") approved++;
        else if (apps[i].status == "rejected") rejected++;
        else if (apps[i].status == "pending")  pending++;
        else if (apps[i].status == "paid")     paid++;
        else if (apps[i].status == "expired")  expired++;

        if (apps[i].months >= 1 && apps[i].months <= 3)
            durationCount[apps[i].months]++;

        int fi = facultyIndex(apps[i].faculty);
        if (fi != -1) {
            facApps[fi]++;
            if (apps[i].status == "approved") facApproved[fi]++;
            if (apps[i].status == "paid")     facPaid[fi]++;
            if (apps[i].status == "rejected") facRejected[fi]++;
            if (isRenewalApp(i)) facRenew[fi]++; else facNew[fi]++;
        }

        for (int m = 0; m < 12; m++) {
            if (apps[i].applyMonth == mLabels[m]) {
                monthApps[m]++;
                if (apps[i].status == "paid")
                    monthRevenue[m] += apps[i].months * 30;
                break;
            }
        }
    }

    int activeNow = paid + approved;
    double utilRate = studentCount > 0
        ? (double)activeNow / studentCount * 100.0 : 0.0;

    // ── Overall summary ───────────────────────────────────────────
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

    // ── Faculty breakdown with % share ────────────────────────────
    printLine();
    cout << "  FACULTY BREAKDOWN\n"; printLine();

    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++)
        if (facApps[f] > facApps[peakFac]) peakFac = f;

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

    // ── Duration breakdown ────────────────────────────────────────
    printLine();
    cout << "  BY PASS DURATION\n"; printLine();
    for (int m = 1; m <= 3; m++) {
        cout << "  " << m << " month(s) : ";
        for (int j = 0; j < durationCount[m] && j < 40; j++) cout << "#";
        if (durationCount[m] > 40) cout << "+";
        cout << "  (" << durationCount[m] << ")\n";
    }

    // ── Monthly trend ─────────────────────────────────────────────
    printLine();
    cout << "  MONTHLY APPLICATION TREND (last 12 months)\n"; printLine();
    int maxBar = 1;
    for (int m = 0; m < 12; m++) if (monthApps[m] > maxBar) maxBar = monthApps[m];
    for (int m = 0; m < 12; m++) {
        int bar = (monthApps[m] * 30) / maxBar;
        cout << "  " << mLabels[m] << " ";
        for (int j = 0; j < bar; j++) cout << "#";
        cout << "  (" << monthApps[m] << " apps, RM "
             << fixed << setprecision(0) << (double)monthRevenue[m] << ")\n";
    }
    printLine();
    cout << "  Run 'Generate Summary Report' for the full printable report.\n";
}

// ============================================================
// ADMIN — SUMMARY REPORT  (c)
// ============================================================

void generateSummaryReport(int index_Admin) {
    printHeader("Admin: Summary Report");
    cout << "  Generated by : " << admins[index_Admin].name << "\n";
    cout << "  Date         : " << getCurrentDate() << "\n\n";

    // Build 12-month and 12-month labels
    string mLabels[12]; buildMonthLabels(mLabels, 12);

    // Per-faculty, per-month matrix
    int facMonthApps[4][12]     = {};
    int facTotal[4]             = {0};
    int facNew[4]               = {0};
    int facRenew[4]             = {0};
    int facPaidMonths[4]        = {0};
    int facRevenue[4]           = {0};

    int monthTotal[12]          = {0};
    int monthRevenue[12]        = {0};
    int totalRevenue            = 0;
    int paidCount               = 0;
    int totalPaidMonths         = 0;

    for (int i = 0; i < appsCount; i++) {
        int fi = facultyIndex(apps[i].faculty);
        if (fi != -1) {
            facTotal[fi]++;
            if (isRenewalApp(i)) facRenew[fi]++; else facNew[fi]++;
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
                    if (apps[i].status == "paid")
                        monthRevenue[m] += apps[i].months * 30;
                    break;
                }
            }
        }
    }

    // ── Section 1: Faculty averages ───────────────────────────────
    cout << "  ================================================\n";
    cout << "  SECTION 1: Application Averages by Faculty\n";
    cout << "  ================================================\n";
    cout << "  " << left
         << setw(12) << "Faculty"
         << setw(10) << "Total"
         << setw(8)  << "New"
         << setw(10) << "Renewals"
         << setw(12) << "Avg/Month"
         << setw(14) << "Paid Months"
         << "Revenue\n";
    printLine();

    int peakFac = 0;
    for (int f = 1; f < FAC_COUNT; f++)
        if (facTotal[f] > facTotal[peakFac]) peakFac = f;

    for (int f = 0; f < FAC_COUNT; f++) {
        double avg = facTotal[f] / 12.0;
        cout << "  " << left
             << setw(12) << FAC_LABELS[f]
             << setw(10) << facTotal[f]
             << setw(8)  << facNew[f]
             << setw(10) << facRenew[f]
             << setw(12) << fixed << setprecision(1) << avg
             << setw(14) << facPaidMonths[f]
             << "RM " << fixed << setprecision(2) << (double)facRevenue[f] << "\n";
    }
    printLine();
    if (facTotal[peakFac] > 0)
        cout << "  Highest demand: " << FAC_LABELS[peakFac] << "\n";

    // ── Section 2: Monthly cross-tab by faculty ───────────────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 2: Monthly Volume by Faculty (last 12 months)\n";
    cout << "  ================================================\n";
    cout << "  " << left << setw(10) << "Month";
    for (int f = 0; f < FAC_COUNT; f++) cout << setw(10) << FAC_LABELS[f].substr(0,6);
    cout << setw(8) << "Total" << "Revenue\n";
    printLine();

    int peakMonth = 0;
    for (int m = 0; m < 12; m++) {
        cout << "  " << left << setw(10) << mLabels[m];
        for (int f = 0; f < FAC_COUNT; f++){
            cout << setw(10) << facMonthApps[f][m];
            cout << setw(8) << monthTotal[m] << "RM " << fixed << setprecision(0) << (double)monthRevenue[m] << "\n";
        }
        if (monthTotal[m] > monthTotal[peakMonth]) peakMonth = m;
    }
    printLine();
    cout << "  Peak month: " << mLabels[peakMonth]
         << " (" << monthTotal[peakMonth] << " apps)\n";

    // ── Section 3: Utilisation rate ───────────────────────────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 3: Car Park Utilisation — Sungai Long\n";
    cout << "  ================================================\n";

    int activeNow = 0;
    for (int i = 0; i < appsCount; i++)
        if (apps[i].status == "paid" || apps[i].status == "approved") activeNow++;

    double utilRate = studentCount > 0
        ? (double)activeNow / studentCount * 100.0 : 0.0;
    double avgMonths = paidCount > 0
        ? (double)totalPaidMonths / paidCount : 0.0;

    cout << "  Registered students       : " << studentCount << "\n";
    cout << "  Registered vehicles       : " << vehicleCount << "\n";
    cout << "  Currently active passes   : " << activeNow   << "\n";
    cout << "  Utilisation rate          : " << fixed << setprecision(1) << utilRate << "%\n";
    cout << "  Total revenue collected   : RM " << fixed << setprecision(2) << (double)totalRevenue << "\n";
    cout << "  Avg months per paid pass  : " << fixed << setprecision(1) << avgMonths << "\n";
    cout << "  Avg monthly revenue       : RM "
         << fixed << setprecision(2) << (totalRevenue / 12.0) << "\n";

    // ── Section 4: UTAR-MPKJ negotiation talking points ──────────
    cout << "\n  ================================================\n";
    cout << "  SECTION 4: Negotiation Insights (UTAR vs MPKJ)\n";
    cout << "  ================================================\n";
    cout << "  1. " << activeNow << " students actively use MPKJ passes — " << "steady guaranteed revenue for MPKJ.\n";
    cout << "  2. Utilisation of " << fixed << setprecision(1) << utilRate << "% shows consistent, predictable demand.\n";
    if(facTotal[peakFac] > 0){
        cout << "  3. Highest demand from " << FAC_LABELS[peakFac] << " — consider faculty-bundle pricing.\n"; 
        cout << "  4. Peak application month: " << mLabels[peakMonth] << " — helps MPKJ plan capacity.\n";
        cout << "  5. Avg " << fixed << setprecision(1) << avgMonths << " months per pass — students prefer short-term; bulk discounts\n"
             << "     for 3-month passes could increase revenue and loyalty.\n";
        cout << "  6. Total RM " << fixed << setprecision(2) << (double)totalRevenue
            << " paid — demonstrates UTAR's collective bargaining power.\n";
    printLine();
    cout << "  [End of Report]\n";
    }
}

// ============================================================
// ADMIN — VIEW STUDENT PROFILE
// ============================================================

void AdminViewStudentProfile(int index_Admin) {
    printHeader("Admin: View Student Profile");
    if (studentCount == 0) { cout << "  No students registered.\n"; return; }

    cout << "  " << left << setw(6) << "No."
                         << setw(8) << "ID"
                         << setw(22) << "Name"
                         << setw(10) << "Faculty"
                         << "Phone\n";
    printLine();
    for (int i = 0; i < studentCount; i++)
        cout << "  " << left << setw(6)  << (i + 1)
                              << setw(8)  << students[i].id
                              << setw(22) << students[i].name
                              << setw(10) << students[i].faculty
                              << students[i].phone << "\n";
    printLine();

    cout << "  Select student (0 to cancel): ";
    int pick = safeInputInt(0, studentCount);
    if (pick == 0) return;

    student& s = students[pick - 1];
    printLine('=');
    cout << "  " << s.name << " (" << s.id << ")\n"; printLine('=');
    cout << "  Email   : " << s.stud_email << "\n";
    cout << "  Faculty : " << s.faculty    << "\n";
    cout << "  Phone   : " << s.phone      << "\n";

    // Vehicles
    printLine();
    cout << "  VEHICLES\n"; printLine();
    int idx[20];
    int vCount = getVehiclesForStudent(s.id, idx, 20);
    if (vCount == 0) {
        cout << "  No vehicles.\n";
    } else {
        cout << "  " << left << setw(10) << "VehicleID" << setw(16) << "Plate"
                              << setw(14) << "Type" << "Active Pass\n";
        printLine();
        for (int i = 0; i < vCount; i++) {
            vehicle& v = vehicles[idx[i]];
            string pass = hasActivePaidPassForVehicle(v.vehicleID) ? "Yes" : "No";
            cout << "  " << left << setw(10) << v.vehicleID
                                  << setw(16) << v.plate
                                  << setw(14) << v.type << pass << "\n";
        }
    }

    // Application history
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
        int vi = FindVehicleIndexByID(apps[i].vehicleID);
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

void UpdatesAdminProfile(int index_Admin) {
    printHeader("Update Admin Profile");
    cout << "  Name : "; getline(cin >> ws, admins[index_Admin].name);

    char chPass;
    cout << "  Change password? (y/n): ";
    cin >> chPass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (chPass == 'y' || chPass == 'Y')
        admins[index_Admin].password = getValidPassword();
    else
        cout << "  Password unchanged.\n";

    SaveAdminToFile();
    cout << "  Admin profile updated!\n";
}

// ============================================================
// MENUS
// ============================================================

void studentMenu(int index_Student) {
    monthEndAlert(index_Student);   // (e) alert on login
    int choice;
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
        choice = safeInputInt(1, 7);

        if      (choice == 1) ViewStudentProfile(index_Student);
        else if (choice == 2) UpdateStudentProfile(index_Student);
        else if (choice == 3) manageVehicles(index_Student);
        else if (choice == 4) register_application(index_Student);
        else if (choice == 5) renew_application(index_Student);
        else if (choice == 6) viewApplicationHistory(index_Student);
        else if (choice == 7) { cout << "  Logging out...\n"; break; }
    }
}

void adminMenu(int index_Admin) {
    int choice;
    while (true) {
        printHeader("Admin Menu");
        cout << "  1. Process Pending Applications\n";
        cout << "  2. View Student Profile\n";
        cout << "  3. Statistics & Analytics\n";
        cout << "  4. Generate Summary Report\n";
        cout << "  5. Update Profile\n";
        cout << "  6. Logout\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 6);

        if      (choice == 1) ViewProfileAdmin(index_Admin);
        else if (choice == 2) AdminViewStudentProfile(index_Admin);
        else if (choice == 3) statisticsUsage(index_Admin);
        else if (choice == 4) generateSummaryReport(index_Admin);
        else if (choice == 5) UpdatesAdminProfile(index_Admin);
        else if (choice == 6) { cout << "  Logging out...\n"; break; }
    }
}

void MainMenu() {
    int choice;
    string password, id;

    while (true) {
        printHeader("Main Menu");
        cout << "  1. Student Sign Up\n";
        cout << "  2. Admin Sign Up\n";
        cout << "  3. Student Login\n";
        cout << "  4. Admin Login\n";
        cout << "  5. Exit\n";
        printLine();
        cout << "  Choice: ";
        choice = safeInputInt(1, 5);

        if (choice == 1) {
            register_stud();

        } else if (choice == 2) {
            register_admin();

        } else if (choice == 3) {
            while (true) {
                cout << "  Student ID: "; cin >> id;
                int si = FindStudentIndexByID(id);
                if (si == -1) { cout << "  Invalid ID. Try again.\n"; continue; }

                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (students[si].password == password) {
                        cout << "  Login Successful!\n";
                        CleanupExpiredPasses(students[si].id);
                        studentMenu(si);
                        ok = true; break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if (!ok){
                    cout << "  Too many failed attempts. Returning to main menu.\n";
                }
                break;
            }

        } else if (choice == 4) {
            while (true) {
                cout << "  Admin ID: "; cin >> id;
                int ai = FindAdminIndexByID(id);
                if (ai == -1) { cout << "  Invalid ID. Try again.\n"; continue; }

                bool ok = false;
                for (int att = 0; att < 3; att++) {
                    cout << "  Password: ";
                    getline(cin >> ws, password);
                    if (admins[ai].password == password) {
                        cout << "  Login Successful!\n";
                        adminMenu(ai);
                        ok = true; break;
                    }
                    cout << "  Wrong password (" << (2 - att) << " attempt(s) left).\n";
                }
                if(!ok){
                    cout << "  Too many failed attempts. Returning to main menu.\n";
                    break;
                }
            }

        }else if(choice == 5) {
            cout << "  Exiting...\n";
            break;
        }
    }
}