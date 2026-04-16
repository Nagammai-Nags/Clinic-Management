#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Patient {
    int id;
    char name[50];
    int age;
    char problem[100];
    char doctor[50];
    char shift[30];
    char status[20];
    struct Patient *next;
};

struct Patient *head = NULL;

/* Load old data from file */
void loadFromFile() {
    FILE *fp = fopen("patients.txt", "r");

    if (fp == NULL) {
        return;
    }

    while (1) {
        struct Patient *newNode = malloc(sizeof(struct Patient));

        int result = fscanf(
            fp,
            "%d,%49[^,],%d,%99[^,],%49[^,],%29[^,],%19[^\n]\n",
            &newNode->id,
            newNode->name,
            &newNode->age,
            newNode->problem,
            newNode->doctor,
            newNode->shift,
            newNode->status
        );

        if (result != 7) {
            free(newNode);
            break;
        }

        newNode->next = NULL;

        if (head == NULL) {
            head = newNode;
        } else {
            struct Patient *temp = head;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }

    fclose(fp);
}

/* Save all data to file on exit */
void saveToFile() {
    FILE *fp = fopen("patients.txt", "w");

    if (fp == NULL) {
        printf("Error saving file.\n");
        return;
    }

    struct Patient *temp = head;

    while (temp != NULL) {
        fprintf(
            fp,
            "%d,%s,%d,%s,%s,%s,%s\n",
            temp->id,
            temp->name,
            temp->age,
            temp->problem,
            temp->doctor,
            temp->shift,
            temp->status
        );

        temp = temp->next;
    }

    fclose(fp);
}

/* Add patient + appointment */
void addPatient() {
    struct Patient *newNode = malloc(sizeof(struct Patient));
    int slotChoice;

    printf("\nEnter Patient ID: ");
    scanf("%d", &newNode->id);

    printf("Enter Name: ");
    scanf(" %[^\n]", newNode->name);

    printf("Enter Age: ");
    scanf("%d", &newNode->age);

    printf("Enter Problem: ");
    scanf(" %[^\n]", newNode->problem);

    printf("\n===== DOCTOR SCHEDULE =====\n");
    printf("1. Morning Shift : 9:00 AM - 1:00 PM  (Dr. Naveen Kumar)\n");
    printf("2. Evening Shift : 4:00 PM - 8:00 PM  (Dr. Vinayagamoorthy)\n");

    printf("\nSelect Appointment Shift: ");
    scanf("%d", &slotChoice);

    if (slotChoice == 1) {
        strcpy(newNode->doctor, "Naveen Kumar");
        strcpy(newNode->shift, "9:00 AM - 1:00 PM");
    }
    else if (slotChoice == 2) {
        strcpy(newNode->doctor, "Vinayagamoorthy");
        strcpy(newNode->shift, "4:00 PM - 8:00 PM");
    }
    else {
        printf("Invalid choice. Assigning Morning Shift.\n");
        strcpy(newNode->doctor, "Naveen Kumar");
        strcpy(newNode->shift, "9:00 AM - 1:00 PM");
    }

    strcpy(newNode->status, "Pending");

    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        struct Patient *temp = head;

        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = newNode;
    }

    printf("\n===== APPOINTMENT CONFIRMED =====\n");
    printf("Patient Name   : %s\n", newNode->name);
    printf("Assigned Doctor: %s\n", newNode->doctor);
    printf("Shift Timing   : %s\n", newNode->shift);
    printf("Status         : %s\n", newNode->status);
}

/* View all records */
void viewPatients() {
    struct Patient *temp = head;

    if (temp == NULL) {
        printf("\nNo records found.\n");
        return;
    }

    printf("\n===== PATIENT RECORDS =====\n");

    while (temp != NULL) {
        printf("\nID       : %d", temp->id);
        printf("\nName     : %s", temp->name);
        printf("\nAge      : %d", temp->age);
        printf("\nProblem  : %s", temp->problem);
        printf("\nDoctor   : %s", temp->doctor);
        printf("\nShift    : %s", temp->shift);
        printf("\nStatus   : %s", temp->status);
        printf("\n-----------------------------\n");

        temp = temp->next;
    }
}

/* Search patient */
void searchPatient() {
    int searchId;
    int found = 0;

    printf("\nEnter Patient ID to search: ");
    scanf("%d", &searchId);

    struct Patient *temp = head;

    while (temp != NULL) {
        if (temp->id == searchId) {
            printf("\nPatient Found!\n");
            printf("Name     : %s\n", temp->name);
            printf("Age      : %d\n", temp->age);
            printf("Problem  : %s\n", temp->problem);
            printf("Doctor   : %s\n", temp->doctor);
            printf("Shift    : %s\n", temp->shift);
            printf("Status   : %s\n", temp->status);
            found = 1;
            break;
        }

        temp = temp->next;
    }

    if (!found) {
        printf("\nPatient not found.\n");
    }
}

/* Update status */
void updateStatus() {
    int id;
    char newStatus[20];
    int found = 0;

    printf("\nEnter Patient ID: ");
    scanf("%d", &id);

    struct Patient *temp = head;

    while (temp != NULL) {
        if (temp->id == id) {
            printf("Enter new status (Completed/Pending): ");
            scanf(" %[^\n]", newStatus);

            strcpy(temp->status, newStatus);

            printf("Status updated successfully!\n");
            found = 1;
            break;
        }

        temp = temp->next;
    }

    if (!found) {
        printf("Patient not found.\n");
    }
}

/* Main menu */
int main() {
    int choice;

    loadFromFile();

    while (1) {
        printf("\n===== RECEPTIONIST MODULE =====");
        printf("\n1. Add Patient + Appointment");
        printf("\n2. View All Patients");
        printf("\n3. Search Patient");
        printf("\n4. Update Status");
        printf("\n5. Exit");
        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addPatient();
                break;

            case 2:
                viewPatients();
                break;

            case 3:
                searchPatient();
                break;

            case 4:
                updateStatus();
                break;

            case 5:
                saveToFile();
                printf("\nData saved successfully. Exiting...\n");
                return 0;

            default:
                printf("\nInvalid choice.\n");
        }
    }

    return 0;
}