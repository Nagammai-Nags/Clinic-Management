#include "base.c"
#include "login.c"
#include "dashboard.c"
#include "patients.c"
#include "add_patient.c"
#include "doctor.c"
#include "diagnosis.c"
#include "billing.c"

int main(int argc, char *argv[]) {
    load_all();
    if (argc < 2) {
        printf("Missing command\n");
        return 1;
    }

    char *cmd = argv[1];
    if (strcmp(cmd, "login_info") == 0) {
        return backend_login_info();
    } else if (strcmp(cmd, "stats") == 0) {
        print_stats();
    } else if (strcmp(cmd, "list_patients") == 0) {
        print_patient_list();
    } else if (strcmp(cmd, "list_doctors") == 0) {
        print_doctors();
    } else if (strcmp(cmd, "add_patient") == 0 && argc >= 8) {
        return add_patient_record(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    } else if (strcmp(cmd, "assign") == 0 && argc >= 5) {
        return assign_patient_to_doctor(argv[2], argv[3], argv[4]);
    } else if (strcmp(cmd, "queue") == 0) {
        int doctorId = argc >= 3 ? atoi(argv[2]) : 0;
        print_queue(doctorId);
    } else if (strcmp(cmd, "start_consult") == 0 && argc >= 3) {
        return start_consultation(argv[2]);
    } else if (strcmp(cmd, "add_diagnosis") == 0 && argc >= 8) {
        return add_diagnosis_record(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
    } else if (strcmp(cmd, "history") == 0 && argc >= 3) {
        print_history(atoi(argv[2]));
    } else if (strcmp(cmd, "billing") == 0) {
        print_invoices();
    } else if (strcmp(cmd, "pay") == 0 && argc >= 6) {
        return update_payment(argv[2], argv[3], argv[4], argv[5]);
    } else {
        printf("ERROR|Invalid command or arguments\n");
        return 1;
    }

    return 0;
}
