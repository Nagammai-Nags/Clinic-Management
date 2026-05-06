void print_queue(int doctorId) {
    Appointment *a = appointments;
    while (a) {
        if ((doctorId == 0 || a->doctorId == doctorId) &&
            (strcmp(a->status, "Waiting") == 0 || strcmp(a->status, "In Consultation") == 0)) {
            Patient *p = find_patient(a->patientId);
            Doctor *d = find_doctor(a->doctorId);
            printf("%d|%d|%s|%s|%s|%s|%s\n", a->id, a->patientId, p ? p->name : "-", d ? d->name : "-", a->problem, a->status, a->date);
        }
        a = a->next;
    }
}

int start_consultation(char *appointmentIdText) {
    Appointment *a = find_appointment(atoi(appointmentIdText));
    if (!a) {
        printf("ERROR|Appointment not found\n");
        return 1;
    }
    strcpy(a->status, "In Consultation");
    save_all();
    printf("OK|Consultation started\n");
    return 0;
}
