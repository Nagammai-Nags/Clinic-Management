void print_invoices() {
    Invoice *i = invoices;
    while (i) {
        Patient *p = find_patient(i->patientId);
        Doctor *d = find_doctor(i->doctorId);
        printf("%d|%d|%s|%s|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%s|%s\n",
            i->id, i->patientId, p ? p->name : "-", d ? d->name : "-",
            i->consultationFee, i->medicineFee, i->labFee, i->total, i->paidAmount,
            i->status, i->method, i->date);
        i = i->next;
    }
}

int update_payment(char *invoiceIdText, char *status, char *method, char *paidAmountText) {
    int invoiceId = atoi(invoiceIdText);
    Invoice *i = invoices;
    while (i && i->id != invoiceId) i = i->next;
    if (!i) {
        printf("ERROR|Invoice not found\n");
        return 1;
    }

    strncpy(i->status, status, 29);
    strncpy(i->method, method, 29);
    i->paidAmount = (float)atof(paidAmountText);

    Appointment *a = find_appointment(i->appointmentId);
    if (a && strcmp(i->status, "Paid") == 0) strcpy(a->status, "Completed");

    save_all();
    printf("OK|Payment updated\n");
    return 0;
}
