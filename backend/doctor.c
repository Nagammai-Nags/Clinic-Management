void print_queue(int doctorId) {
    for (int b = 0; b < HASH_SIZE; b++) {
        DoctorQueue *q = doctorQueueTable[b];
        while (q) {
            if (doctorId == 0 || q->doctorId == doctorId) {
                if (is_waiting_status(q->active)) {
                    Appointment *a = q->active;
                    Patient *p = find_patient(a->patientId);
                    Doctor *d = find_doctor(a->doctorId);
                    printf("%d|%d|%s|%s|%s|%s|%s\n", a->id, a->patientId, p ? p->name : "-", d ? d->name : "-", a->problem, appointment_display_status(a), a->date);
                }

                AppointmentQueueNode *node = q->front;
                while (node) {
                    Appointment *a = node->appointment;
                    if (is_waiting_status(a)) {
                        Patient *p = find_patient(a->patientId);
                        Doctor *d = find_doctor(a->doctorId);
                        printf("%d|%d|%s|%s|%s|%s|%s\n", a->id, a->patientId, p ? p->name : "-", d ? d->name : "-", a->problem, appointment_display_status(a), a->date);
                    }
                    node = node->next;
                }
            }
            q = q->next;
        }
    }
}

int start_consultation(char *appointmentIdText) {
    Appointment *a = find_appointment(atoi(appointmentIdText));
    if (!a) {
        printf("ERROR|Appointment not found\n");
        return 1;
    }

    DoctorQueue *q = get_doctor_queue(a->doctorId, 0);
    if (!q || (!q->active && !q->front)) {
        printf("ERROR|No waiting patients for this doctor\n");
        return 1;
    }

    if (q->active) {
        if (q->active == a) {
            strcpy(a->status, "In Consultation");
            save_all();
            printf("OK|Consultation already started\n");
            return 0;
        }
        printf("ERROR|Finish the current consultation before starting another patient\n");
        return 1;
    }

    if (q->front->appointment != a) {
        printf("ERROR|Please start the next patient in the queue first\n");
        return 1;
    }

    dequeue_next_appointment(a->doctorId);
    strcpy(a->status, "In Consultation");
    save_all();
    printf("OK|Consultation started\n");
    return 0;
}
