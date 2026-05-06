void print_stats() {
    int pc = 0, ac = 0, ic = 0, pending = 0;
    float total = 0, paid = 0;

    Patient *p = patients;
    while (p) {
        pc++;
        p = p->next;
    }

    Appointment *a = appointments;
    while (a) {
        ac++;
        a = a->next;
    }

    Invoice *i = invoices;
    while (i) {
        ic++;
        total += i->total;
        paid += i->paidAmount;
        if (strcmp(i->status, "Paid") != 0) pending++;
        i = i->next;
    }

    printf("%d|%d|%d|%d|%.2f|%.2f\n", pc, ac, ic, pending, total, paid);
}
