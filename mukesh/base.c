#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "data"

typedef struct Patient {
    int id;
    char name[60], gender[20], phone[20], address[120], blood[10], firstVisit[20];
    int age;
    struct Patient *next;
} Patient;

typedef struct Doctor {
    int id;
    char name[60], specialization[60], available[10];
    float fee;
    struct Doctor *next;
} Doctor;

typedef struct Appointment {
    int id, patientId, doctorId;
    char problem[80], status[30], date[20];
    struct Appointment *next;
} Appointment;

typedef struct Visit {
    int id, patientId, doctorId, appointmentId;
    char date[20], symptoms[160], diagnosis[160], prescription[240];
    struct Visit *next;
} Visit;

typedef struct Invoice {
    int id, patientId, doctorId, appointmentId;
    float consultationFee, medicineFee, labFee, total, paidAmount;
    char status[30], method[30], date[20];
    struct Invoice *next;
} Invoice;

Patient *patients = NULL;
Doctor *doctors = NULL;
Appointment *appointments = NULL;
Visit *visits = NULL;
Invoice *invoices = NULL;

void today(char *out) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(out, 20, "%Y-%m-%d", tm_info);
}

void trim_newline(char *s) {
    s[strcspn(s, "\r\n")] = 0;
}

void clean(char *s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] == '|') s[i] = '/';
        if (s[i] == '\n' || s[i] == '\r') s[i] = ' ';
    }
}

FILE *open_data_file(const char *name, const char *mode) {
    char path[160];
    snprintf(path, sizeof(path), "%s/%s", DATA_DIR, name);
    return fopen(path, mode);
}

Patient *find_patient(int id) {
    Patient *p = patients;
    while (p) {
        if (p->id == id) return p;
        p = p->next;
    }
    return NULL;
}

Doctor *find_doctor(int id) {
    Doctor *d = doctors;
    while (d) {
        if (d->id == id) return d;
        d = d->next;
    }
    return NULL;
}

Appointment *find_appointment(int id) {
    Appointment *a = appointments;
    while (a) {
        if (a->id == id) return a;
        a = a->next;
    }
    return NULL;
}

Appointment *current_appointment_for_patient(int patientId) {
    Appointment *a = appointments;
    Appointment *latest = NULL;
    while (a) {
        if (a->patientId == patientId && strcmp(a->status, "Completed") != 0) latest = a;
        a = a->next;
    }
    return latest;
}

int next_patient_id() {
    int max = 0;
    Patient *p = patients;
    while (p) {
        if (p->id > max) max = p->id;
        p = p->next;
    }
    return max + 1;
}

int next_appointment_id() {
    int max = 500;
    Appointment *a = appointments;
    while (a) {
        if (a->id > max) max = a->id;
        a = a->next;
    }
    return max + 1;
}

int next_visit_id() {
    int max = 800;
    Visit *v = visits;
    while (v) {
        if (v->id > max) max = v->id;
        v = v->next;
    }
    return max + 1;
}

int next_invoice_id() {
    int max = 1000;
    Invoice *i = invoices;
    while (i) {
        if (i->id > max) max = i->id;
        i = i->next;
    }
    return max + 1;
}

void append_patient(Patient *p) {
    p->next = patients;
    patients = p;
}

void append_doctor(Doctor *d) {
    d->next = doctors;
    doctors = d;
}

void append_appointment(Appointment *a) {
    a->next = appointments;
    appointments = a;
}

void append_visit(Visit *v) {
    v->next = visits;
    visits = v;
}

void append_invoice(Invoice *i) {
    i->next = invoices;
    invoices = i;
}

void load_patients() {
    FILE *f = open_data_file("patients.csv", "r");
    if (!f) return;
    char line[700];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) < 3) continue;
        Patient *p = calloc(1, sizeof(Patient));
        char *tok = strtok(line, "|"); if (!tok) { free(p); continue; } p->id = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->name, tok);
        tok = strtok(NULL, "|"); if (tok) p->age = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->gender, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->phone, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->address, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->blood, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(p->firstVisit, tok);
        append_patient(p);
    }
    fclose(f);
}

void load_doctors() {
    FILE *f = open_data_file("doctors.csv", "r");
    if (!f) return;
    char line[400];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) < 3) continue;
        Doctor *d = calloc(1, sizeof(Doctor));
        char *tok = strtok(line, "|"); if (!tok) { free(d); continue; } d->id = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(d->name, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(d->specialization, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(d->available, tok);
        tok = strtok(NULL, "|"); if (tok) d->fee = (float)atof(tok);
        append_doctor(d);
    }
    fclose(f);
}

void load_appointments() {
    FILE *f = open_data_file("appointments.csv", "r");
    if (!f) return;
    char line[500];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) < 3) continue;
        Appointment *a = calloc(1, sizeof(Appointment));
        char *tok = strtok(line, "|"); if (!tok) { free(a); continue; } a->id = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) a->patientId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) a->doctorId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(a->problem, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(a->status, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(a->date, tok);
        append_appointment(a);
    }
    fclose(f);
}

void load_visits() {
    FILE *f = open_data_file("visits.csv", "r");
    if (!f) return;
    char line[900];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) < 3) continue;
        Visit *v = calloc(1, sizeof(Visit));
        char *tok = strtok(line, "|"); if (!tok) { free(v); continue; } v->id = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) v->patientId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) v->doctorId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) v->appointmentId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(v->date, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(v->symptoms, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(v->diagnosis, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(v->prescription, tok);
        append_visit(v);
    }
    fclose(f);
}

void load_invoices() {
    FILE *f = open_data_file("invoices.csv", "r");
    if (!f) return;
    char line[700];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (strlen(line) < 3) continue;
        Invoice *i = calloc(1, sizeof(Invoice));
        char *tok = strtok(line, "|"); if (!tok) { free(i); continue; } i->id = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) i->patientId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) i->doctorId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) i->appointmentId = atoi(tok);
        tok = strtok(NULL, "|"); if (tok) i->consultationFee = (float)atof(tok);
        tok = strtok(NULL, "|"); if (tok) i->medicineFee = (float)atof(tok);
        tok = strtok(NULL, "|"); if (tok) i->labFee = (float)atof(tok);
        tok = strtok(NULL, "|"); if (tok) i->total = (float)atof(tok);
        tok = strtok(NULL, "|"); if (tok) i->paidAmount = (float)atof(tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(i->status, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(i->method, tok);
        tok = strtok(NULL, "|"); if (tok) strcpy(i->date, tok);
        append_invoice(i);
    }
    fclose(f);
}

void load_all() {
    load_patients();
    load_doctors();
    load_appointments();
    load_visits();
    load_invoices();
}

void save_patients() {
    FILE *f = open_data_file("patients.csv", "w");
    Patient *p = patients;
    while (p) {
        fprintf(f, "%d|%s|%d|%s|%s|%s|%s|%s\n", p->id, p->name, p->age, p->gender, p->phone, p->address, p->blood, p->firstVisit);
        p = p->next;
    }
    fclose(f);
}

void save_doctors() {
    FILE *f = open_data_file("doctors.csv", "w");
    Doctor *d = doctors;
    while (d) {
        fprintf(f, "%d|%s|%s|%s|%.2f\n", d->id, d->name, d->specialization, d->available, d->fee);
        d = d->next;
    }
    fclose(f);
}

void save_appointments() {
    FILE *f = open_data_file("appointments.csv", "w");
    Appointment *a = appointments;
    while (a) {
        fprintf(f, "%d|%d|%d|%s|%s|%s\n", a->id, a->patientId, a->doctorId, a->problem, a->status, a->date);
        a = a->next;
    }
    fclose(f);
}

void save_visits() {
    FILE *f = open_data_file("visits.csv", "w");
    Visit *v = visits;
    while (v) {
        fprintf(f, "%d|%d|%d|%d|%s|%s|%s|%s\n", v->id, v->patientId, v->doctorId, v->appointmentId, v->date, v->symptoms, v->diagnosis, v->prescription);
        v = v->next;
    }
    fclose(f);
}

void save_invoices() {
    FILE *f = open_data_file("invoices.csv", "w");
    Invoice *i = invoices;
    while (i) {
        fprintf(f, "%d|%d|%d|%d|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%s|%s\n", i->id, i->patientId, i->doctorId, i->appointmentId, i->consultationFee, i->medicineFee, i->labFee, i->total, i->paidAmount, i->status, i->method, i->date);
        i = i->next;
    }
    fclose(f);
}

void save_all() {
    save_patients();
    save_doctors();
    save_appointments();
    save_visits();
    save_invoices();
}
