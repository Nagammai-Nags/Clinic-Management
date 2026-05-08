void print_patient_list() {
    Patient *p = patients;
    while (p) {
        /*
            Reception must show the real latest status.
            current_appointment_for_patient() ignores completed appointments,
            so a paid patient was wrongly displayed as "Not Assigned".
        */
        Appointment *a = latest_appointment_for_patient(p->id);
        Doctor *d = a ? find_doctor(a->doctorId) : NULL;
        printf("%d|%s|%d|%s|%s|%s|%s|%s|%s|%s|%s|%d\n",
            p->id, p->name, p->age, p->gender, p->phone, p->address, p->blood,
            a ? a->problem : "Not Assigned",
            d ? d->name : "Not Assigned",
            d ? d->specialization : "-",
            appointment_display_status(a),
            a ? a->id : 0);
        p = p->next;
    }
}

void print_doctors() {
    Doctor *d = doctors;
    while (d) {
        printf("%d|%s|%s|%s|%.2f\n", d->id, d->name, d->specialization, d->available, d->fee);
        d = d->next;
    }
}

int assign_patient_to_doctor(char *patientIdText, char *doctorIdText, char *problem) {
    int patientId = atoi(patientIdText);
    int doctorId = atoi(doctorIdText);
    if (!find_patient(patientId) || !find_doctor(doctorId)) {
        printf("ERROR|Invalid patient or doctor\n");
        return 1;
    }

    Appointment *old = current_appointment_for_patient(patientId);
    if (old && strcmp(old->status, "Completed") != 0) strcpy(old->status, "Completed");

    Appointment *a = calloc(1, sizeof(Appointment));
    a->id = next_appointment_id();
    a->patientId = patientId;
    a->doctorId = doctorId;
    strncpy(a->problem, problem, 79);
    clean(a->problem);
    strcpy(a->status, "Assigned");
    today(a->date);
    append_appointment(a);
    save_all();
    printf("OK|Patient assigned|%d\n", a->id);
    return 0;
}
