#include <stdio.h>
#include <string.h>

struct Patient {
    int id;
    char name[50];
    int age;
    char problem[100];
};

// Add patient and save to file
void addPatient() {
    struct Patient p;
    FILE *fp = fopen("patients.txt", "a");

    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    printf("Enter Patient ID: ");
    scanf("%d", &p.id);

    printf("Enter Name: ");
    scanf(" %[^\n]", p.name);

    printf("Enter Age: ");
    scanf("%d", &p.age);

    printf("Enter Problem: ");
    scanf(" %[^\n]", p.problem);

    fprintf(fp, "%d,%s,%d,%s\n", p.id, p.name, p.age, p.problem);
    fclose(fp);

    printf("Patient added successfully!\n");
}

// View patients
void viewPatients() {
    struct Patient p;
    FILE *fp = fopen("patients.txt", "r");

    if (fp == NULL) {
        printf("No records found.\n");
        return;
    }

    printf("\n--- Patient List ---\n");
    while (fscanf(fp, "%d,%49[^,],%d,%99[^\n]\n", &p.id, p.name, &p.age, p.problem) != EOF) {
        printf("ID: %d\nName: %s\nAge: %d\nProblem: %s\n\n", p.id, p.name, p.age, p.problem);
    }

    fclose(fp);
}

// Search patient
void searchPatient() {
    struct Patient p;
    char name[50];
    int found = 0;

    FILE *fp = fopen("patients.txt", "r");

    if (fp == NULL) {
        printf("No records found.\n");
        return;
    }

    printf("Enter name to search: ");
    scanf(" %[^\n]", name);

    while (fscanf(fp, "%d,%49[^,],%d,%99[^\n]\n", &p.id, p.name, &p.age, p.problem) != EOF) {
        if (strcmp(p.name, name) == 0) {
            printf("\nPatient Found:\n");
            printf("ID: %d\nName: %s\nAge: %d\nProblem: %s\n", p.id, p.name, p.age, p.problem);
            found = 1;
        }
    }

    if (!found) {
        printf("Patient not found.\n");
    }

    fclose(fp);
}

int main() {
    int choice;

    while (1) {
        printf("\n--- Clinic Management System ---\n");
        printf("1. Add Patient\n");
        printf("2. View Patients\n");
        printf("3. Search Patient\n");
        printf("4. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addPatient(); break;
            case 2: viewPatients(); break;
            case 3: searchPatient(); break;
            case 4: return 0;
            default: printf("Invalid choice!\n");
        }
    }
}