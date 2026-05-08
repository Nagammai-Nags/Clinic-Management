int add_patient_record(
    char *name,
    char *age,
    char *gender,
    char *phone,
    char *address,
    char *blood
) {
    /*
        Diagnosis history is connected to patient id.
        If the same phone number comes again, we reuse the old patient id
        so the doctor can see previous visits and diagnosis records.
    */
    clean(phone);
    Patient *existingPatient = find_patient_by_phone(phone);
    if (existingPatient) {
        printf("OK|Existing patient found|%d\n", existingPatient->id);
        return 0;
    }

    /*
        Step 1: Create memory for one new patient.
        calloc makes all fields empty/zero at the beginning.
    */
    Patient *newPatient = calloc(1, sizeof(Patient));

    /*
        Step 2: Store the details received from the form.
        next_patient_id() gives a new unique patient number.
    */
    newPatient->id = next_patient_id();
    newPatient->age = atoi(age);

    strncpy(newPatient->name, name, 59);
    strncpy(newPatient->gender, gender, 19);
    strncpy(newPatient->phone, phone, 19);
    strncpy(newPatient->address, address, 119);
    strncpy(newPatient->blood, blood, 9);

    /*
        Step 3: Clean text fields before saving.
        The clean() function removes characters that can break the CSV file.
    */
    clean(newPatient->name);
    clean(newPatient->gender);
    clean(newPatient->phone);
    clean(newPatient->address);
    clean(newPatient->blood);

    /*
        Step 4: Add today's date as the patient's first visit date.
    */
    today(newPatient->firstVisit);

    /*
        Step 5: Add the patient to the linked list and save all records.
    */
    append_patient(newPatient);
    save_all();

    /*
        Step 6: Send success message back to the Python/Flask application.
    */
    printf("OK|Patient added|%d\n", newPatient->id);

    return 0;
}
