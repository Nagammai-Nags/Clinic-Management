int add_diagnosis_record(char *appointmentIdText, char *symptoms, char *diagnosis, char *prescription, char *medicineFeeText, char *labFeeText) {
    Appointment *a = find_appointment(atoi(appointmentIdText));
    if (!a) {
        printf("ERROR|Appointment not found\n");
        return 1;
    }

    Doctor *d = find_doctor(a->doctorId);
    Visit *v = calloc(1, sizeof(Visit));
    v->id = next_visit_id();
    v->patientId = a->patientId;
    v->doctorId = a->doctorId;
    v->appointmentId = a->id;
    today(v->date);
    strncpy(v->symptoms, symptoms, 159);
    strncpy(v->diagnosis, diagnosis, 159);
    strncpy(v->prescription, prescription, 239);
    clean(v->symptoms);
    clean(v->diagnosis);
    clean(v->prescription);
    append_visit(v);

    Invoice *i = calloc(1, sizeof(Invoice));
    i->id = next_invoice_id();
    i->patientId = a->patientId;
    i->doctorId = a->doctorId;
    i->appointmentId = a->id;
    i->consultationFee = d ? d->fee : 0;
    i->medicineFee = (float)atof(medicineFeeText);
    i->labFee = (float)atof(labFeeText);
    i->total = i->consultationFee + i->medicineFee + i->labFee;
    i->paidAmount = 0;
    strcpy(i->status, "Pending");
    strcpy(i->method, "Not Paid");
    today(i->date);
    append_invoice(i);

    strcpy(a->status, "Billing Pending");
    save_all();
    printf("OK|Diagnosis saved and invoice generated|%d\n", i->id);
    return 0;
}

void print_history(int patientId) {
    Visit *v = visits;
    while (v) {
        if (v->patientId == patientId) {
            Doctor *d = find_doctor(v->doctorId);
            printf("%d|%s|%s|%s|%s|%s\n", v->id, v->date, d ? d->name : "-", v->symptoms, v->diagnosis, v->prescription);
        }
        v = v->next;
    }
}
