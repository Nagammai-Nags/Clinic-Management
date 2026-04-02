#include <stdio.h>
#include <string.h>

struct Patient {
    int id;
    char name[50];
    int age;
    char problem[100];
};

struct Patient patients[100];
int count = 0;

// Add patient
void addPatient() {
    printf("\nEnter Patient ID: ");
    scanf("%d", &patients[count].id);

    printf("Enter Name: ");
    scanf(" %[^\n]", patients[count].name);

    printf("Enter Age: ");
    scanf("%d", &patients[count].age);

    printf("Enter Problem: ");
    scanf(" %[^\n]", patients[count].problem);

    count++;
    printf("✅ Patient added successfully!\n");
}

// View patients
void viewPatients() {
    if (count == 0) {
        printf("\nNo patients found.\n");
        return;
    }

    printf("\n--- Patient List ---\n");
    for (int i = 0; i < count; i++) {
        printf("ID: %d\n", patients[i].id);
        printf("Name: %s\n", patients[i].name);
        printf("Age: %d\n", patients[i].age);
        printf("Problem: %s\n\n", patients[i].problem);
    }
}

// Search patient
void searchPatient() {
    char name[50];
    int found = 0;

    printf("\nEnter name to search: ");
    scanf(" %[^\n]", name);

    for (int i = 0; i < count; i++) {
        if (strcmp(patients[i].name, name) == 0) {
            printf("\nPatient Found:\n");
            printf("ID: %d\n", patients[i].id);
            printf("Name: %s\n", patients[i].name);
            printf("Age: %d\n", patients[i].age);
            printf("Problem: %s\n", patients[i].problem);
            found = 1;
        }
    }

    if (!found) {
        printf("❌ Patient not found.\n");
    }
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