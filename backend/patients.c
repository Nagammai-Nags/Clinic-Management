void print_patient_list() {
    for (int b = 0; b < HASH_SIZE; b++) {
        Patient *p = patientTable[b];
        while (p) {
            /*
                Reception must show the real latest status.
                current_appointment_for_patient() ignores completed appointments,
                so a paid patient was wrongly displayed as "Not Assigned".
            */
            Appointment *a = latest_appointment_for_patient(p->id);
            if (a && strcmp(a->status, "Completed") == 0) {
                p = p->next;
                continue;
            }

            Doctor *d = a ? find_doctor(a->doctorId) : NULL;
            printf("%d|%s|%d|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%s\n",
                p->id, p->name, p->age, p->gender, p->phone, p->address, p->blood,
                a ? a->problem : "Not Assigned",
                d ? d->name : "Not Assigned",
                d ? d->specialization : "-",
                appointment_display_status(a),
                a ? a->id : 0,
                a ? a->doctorId : 0,
                a ? a->date : "-");
            p = p->next;
        }
    }
}

void print_doctors() {
    for (int b = 0; b < HASH_SIZE; b++) {
        Doctor *d = doctorTable[b];
        while (d) {
            printf("%d|%s|%s|%s|%.2f\n", d->id, d->name, d->specialization, d->available, d->fee);
            d = d->next;
        }
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
    if (old && strcmp(old->status, "Not Assigned") == 0) {
        old->doctorId = doctorId;
        strncpy(old->problem, problem, 79);
        clean(old->problem);
        strcpy(old->status, "Waiting");
        enqueue_appointment(old);
        save_all();
        printf("OK|Patient assigned|%d\n", old->id);
        return 0;
    }

    if (old && strcmp(old->status, "Completed") != 0) {
        remove_appointment_from_queue(old);
        strcpy(old->status, "Completed");
    }

    Appointment *a = calloc(1, sizeof(Appointment));
    a->id = next_appointment_id();
    a->patientId = patientId;
    a->doctorId = doctorId;
    strncpy(a->problem, problem, 79);
    clean(a->problem);
    strcpy(a->status, "Waiting");
    today(a->date);
    append_appointment(a);
    enqueue_appointment(a);
    save_all();
    printf("OK|Patient assigned|%d\n", a->id);
    return 0;
}

int change_appointment_date(char *appointmentIdText, char *appointmentDate) {
    Appointment *a = find_appointment(atoi(appointmentIdText));
    if (!a) {
        printf("ERROR|Appointment not found\n");
        return 1;
    }

    if (strcmp(a->status, "Completed") == 0) {
        printf("ERROR|Completed appointment cannot be changed\n");
        return 1;
    }

    if (!valid_appointment_slot(appointmentDate)) {
        printf("ERROR|Appointment time must be a 15-minute slot from 10:00 AM to 09:00 PM\n");
        return 1;
    }

    if (!doctor_slot_available(a->doctorId, appointmentDate, a)) {
        printf("ERROR|Doctor already has another patient in this slot\n");
        return 1;
    }

    strncpy(a->date, appointmentDate, 19);
    clean(a->date);
    save_all();
    printf("OK|Appointment date and time changed|%d\n", a->id);
    return 0;
}
