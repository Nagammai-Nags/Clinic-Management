#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "doctor_appointments.txt"

void addAppointment() {
    FILE *fp = fopen(FILE_NAME, "a");

    int id;
    char name[50], doctor[50], time[20], status[20];

    printf("Enter Patient ID: ");
    scanf("%d", &id);

    printf("Enter Patient Name: ");
    scanf(" %[^\n]", name);

    printf("Select Doctor:\n");
    printf("1. Dr. Naveen Kumar\n");
    printf("2. Dr. Vinayagamoorthy\n");
    printf("Enter choice: ");
    int choice;
    scanf("%d", &choice);

    if (choice == 1)
        strcpy(doctor, "Dr.Naveen Kumar");
    else
        strcpy(doctor, "Dr.Vinayagamoorthy");

    printf("Enter Appointment Time: ");
    scanf(" %[^\n]", time);

    // Simple logic: first doctor always confirmed, second pending
    if (choice == 1)
        strcpy(status, "Confirmed");
    else
        strcpy(status, "Pending");

    fprintf(fp, "%d,%s,%s,%s,%s\n", id, name, doctor, time, status);

    fclose(fp);

    printf("Appointment Added Successfully!\n");
}

void viewAppointments() {
    FILE *fp = fopen(FILE_NAME, "r");

    int id;
    char name[50], doctor[50], time[20], status[20];

    printf("\n--- Appointments ---\n");

    while (fscanf(fp, "%d,%[^,],%[^,],%[^,],%[^\n]\n",
                  &id, name, doctor, time, status) != EOF) {
        printf("ID: %d | Name: %s | Doctor: %s | Time: %s | Status: %s\n",
               id, name, doctor, time, status);
    }

    fclose(fp);
}

int main() {
    int choice;

    printf("===== DOCTOR MODULE =====\n");
    printf("1. Add Appointment\n");
    printf("2. View Appointments\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            addAppointment();
            break;
        case 2:
            viewAppointments();
            break;
        default:
            printf("Invalid choice\n");
    }

    return 0;
}