#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_DIR "data"
#define HASH_SIZE 101
#define CONSULTATION_MINUTES 15

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
    char problem[80], status[30], date[20], endDate[20];
    struct Appointment *next;
} Appointment;

typedef struct AppointmentQueueNode {
    Appointment *appointment;
    struct AppointmentQueueNode *next;
} AppointmentQueueNode;

typedef struct DoctorQueue {
    int doctorId;
    Appointment *active;
    AppointmentQueueNode *front, *rear;
    struct DoctorQueue *next;
} DoctorQueue;

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

typedef struct DoctorHeapNode {
    int doctorId;
    time_t nextAvailableTime;
    int activePatientCount;
} DoctorHeapNode;

typedef struct MinHeap {
    DoctorHeapNode nodes[HASH_SIZE];
    int size;
} MinHeap;

Patient *patientTable[HASH_SIZE] = { NULL };
Doctor *doctorTable[HASH_SIZE] = { NULL };
Appointment *appointmentTable[HASH_SIZE] = { NULL };
DoctorQueue *doctorQueueTable[HASH_SIZE] = { NULL };
Visit *visits = NULL;
Invoice *invoices = NULL;

int hash_id(int id) {
    if (id < 0) id = -id;
    return id % HASH_SIZE;
}

int is_digit_char(char c) {
    return c >= '0' && c <= '9';
}

int valid_appointment_slot(char *appointmentDate) {
    if (strlen(appointmentDate) != 16) return 0;
    if (appointmentDate[4] != '-' || appointmentDate[7] != '-' || appointmentDate[10] != ' ' || appointmentDate[13] != ':') return 0;

    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 7 || i == 10 || i == 13) continue;
        if (!is_digit_char(appointmentDate[i])) return 0;
    }

    int hour = (appointmentDate[11] - '0') * 10 + (appointmentDate[12] - '0');
    int minute = (appointmentDate[14] - '0') * 10 + (appointmentDate[15] - '0');
    if (hour < 10 || hour > 21) return 0;
    if (hour == 21 && minute != 0) return 0;
    return minute == 0 || minute == 15 || minute == 30 || minute == 45;
}

void today(char *out) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(out, 20, "%Y-%m-%d", tm_info);
}

int appointment_not_in_past(char *appointmentDate) {
    char currentDate[20];
    today(currentDate);
    return strncmp(appointmentDate, currentDate, 10) >= 0;
}

time_t parse_appointment_time(char *appointmentDate) {
    struct tm tm_info = { 0 };
    int year, month, day, hour, minute;
    if (sscanf(appointmentDate, "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) != 5) {
        return (time_t)-1;
    }

    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = day;
    tm_info.tm_hour = hour;
    tm_info.tm_min = minute;
    tm_info.tm_sec = 0;
    tm_info.tm_isdst = -1;
    return mktime(&tm_info);
}

void format_appointment_time(time_t value, char *out) {
    struct tm *tm_info = localtime(&value);
    strftime(out, 20, "%Y-%m-%d %H:%M", tm_info);
}

time_t appointment_end_time(Appointment *a) {
    time_t start = parse_appointment_time(a->date);
    if (strlen(a->endDate) == 16) {
        time_t savedEnd = parse_appointment_time(a->endDate);
        if (savedEnd != (time_t)-1) return savedEnd;
    }
    return start + (CONSULTATION_MINUTES * 60);
}

int time_ranges_overlap(time_t startA, time_t endA, time_t startB, time_t endB) {
    return startA < endB && startB < endA;
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
    Patient *p = patientTable[hash_id(id)];
    while (p) {
        if (p->id == id) return p;
        p = p->next;
    }
    return NULL;
}

Doctor *find_doctor(int id) {
    Doctor *d = doctorTable[hash_id(id)];
    while (d) {
        if (d->id == id) return d;
        d = d->next;
    }
    return NULL;
}

Appointment *find_appointment(int id) {
    Appointment *a = appointmentTable[hash_id(id)];
    while (a) {
        if (a->id == id) return a;
        a = a->next;
    }
    return NULL;
}

int doctor_slot_available(int doctorId, char *appointmentDate, Appointment *ignore) {
    time_t requestedStart = parse_appointment_time(appointmentDate);
    if (requestedStart == (time_t)-1) return 0;
    time_t requestedEnd = requestedStart + (CONSULTATION_MINUTES * 60);

    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            time_t appointmentStart = parse_appointment_time(a->date);
            time_t appointmentEnd = appointment_end_time(a);
            if (a != ignore &&
                a->doctorId == doctorId &&
                appointmentStart != (time_t)-1 &&
                time_ranges_overlap(requestedStart, requestedEnd, appointmentStart, appointmentEnd) &&
                strcmp(a->status, "Completed") != 0) {
                return 0;
            }
            a = a->next;
        }
    }
    return 1;
}

int compare_doctor_nodes(DoctorHeapNode a, DoctorHeapNode b) {
    if (a.nextAvailableTime < b.nextAvailableTime) return -1;
    if (a.nextAvailableTime > b.nextAvailableTime) return 1;
    if (a.activePatientCount < b.activePatientCount) return -1;
    if (a.activePatientCount > b.activePatientCount) return 1;
    return a.doctorId - b.doctorId;
}

void swap_doctor_nodes(DoctorHeapNode *a, DoctorHeapNode *b) {
    DoctorHeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(MinHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (compare_doctor_nodes(heap->nodes[index], heap->nodes[parent]) >= 0) return;
        swap_doctor_nodes(&heap->nodes[index], &heap->nodes[parent]);
        index = parent;
    }
}

void heapifyDown(MinHeap *heap, int index) {
    while (1) {
        int left = (index * 2) + 1;
        int right = (index * 2) + 2;
        int smallest = index;

        if (left < heap->size && compare_doctor_nodes(heap->nodes[left], heap->nodes[smallest]) < 0) {
            smallest = left;
        }
        if (right < heap->size && compare_doctor_nodes(heap->nodes[right], heap->nodes[smallest]) < 0) {
            smallest = right;
        }
        if (smallest == index) return;

        swap_doctor_nodes(&heap->nodes[index], &heap->nodes[smallest]);
        index = smallest;
    }
}

void insertDoctor(MinHeap *heap, DoctorHeapNode node) {
    if (heap->size >= HASH_SIZE) return;
    heap->nodes[heap->size] = node;
    heapifyUp(heap, heap->size);
    heap->size++;
}

DoctorHeapNode extractMinDoctor(MinHeap *heap) {
    DoctorHeapNode empty = { 0, 0, 0 };
    if (heap->size <= 0) return empty;

    DoctorHeapNode root = heap->nodes[0];
    heap->size--;
    if (heap->size > 0) {
        heap->nodes[0] = heap->nodes[heap->size];
        heapifyDown(heap, 0);
    }
    return root;
}

void updateDoctorAvailability(DoctorHeapNode *node, time_t appointmentEndTime) {
    node->nextAvailableTime = appointmentEndTime;
    node->activePatientCount++;
}

int active_appointment_count(int doctorId) {
    int count = 0;
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            if (a->doctorId == doctorId && strcmp(a->status, "Completed") != 0) count++;
            a = a->next;
        }
    }
    return count;
}

time_t next_available_time_for_doctor(int doctorId, time_t requestedStart, Appointment *ignore) {
    time_t candidate = requestedStart;
    int changed = 1;

    while (changed) {
        changed = 0;
        time_t candidateEnd = candidate + (CONSULTATION_MINUTES * 60);

        for (int b = 0; b < HASH_SIZE; b++) {
            Appointment *a = appointmentTable[b];
            while (a) {
                if (a != ignore && a->doctorId == doctorId && strcmp(a->status, "Completed") != 0) {
                    time_t appointmentStart = parse_appointment_time(a->date);
                    time_t appointmentEnd = appointment_end_time(a);
                    if (appointmentStart != (time_t)-1 &&
                        time_ranges_overlap(candidate, candidateEnd, appointmentStart, appointmentEnd)) {
                        candidate = appointmentEnd;
                        changed = 1;
                    }
                }
                a = a->next;
            }
        }
    }

    return candidate;
}

int schedule_appointment_with_heap(Appointment *appointment, char *requestedStartText, int preferredDoctorId) {
    time_t requestedStart = parse_appointment_time(requestedStartText);
    if (requestedStart == (time_t)-1) return 0;

    MinHeap heap = { 0 };
    for (int b = 0; b < HASH_SIZE; b++) {
        Doctor *d = doctorTable[b];
        while (d) {
            if ((preferredDoctorId <= 0 || d->id == preferredDoctorId) && strcmp(d->available, "Yes") == 0) {
                DoctorHeapNode node;
                node.doctorId = d->id;
                node.nextAvailableTime = next_available_time_for_doctor(d->id, requestedStart, appointment);
                node.activePatientCount = active_appointment_count(d->id);
                insertDoctor(&heap, node);
            }
            d = d->next;
        }
    }

    if (heap.size == 0) return 0;

    DoctorHeapNode chosen = extractMinDoctor(&heap);
    time_t appointmentEnd = chosen.nextAvailableTime + (CONSULTATION_MINUTES * 60);
    updateDoctorAvailability(&chosen, appointmentEnd);

    appointment->doctorId = chosen.doctorId;
    format_appointment_time(chosen.nextAvailableTime, appointment->date);
    format_appointment_time(appointmentEnd, appointment->endDate);
    return 1;
}

void print_hash_tables() {
    printf("PATIENT HASH TABLE\n");
    for (int b = 0; b < HASH_SIZE; b++) {
        Patient *p = patientTable[b];
        if (!p) continue;

        printf("bucket[%d]", b);
        while (p) {
            printf(" -> id=%d addr=%p next=%p", p->id, (void *)p, (void *)p->next);
            p = p->next;
        }
        printf("\n");
    }

    printf("DOCTOR HASH TABLE\n");
    for (int b = 0; b < HASH_SIZE; b++) {
        Doctor *d = doctorTable[b];
        if (!d) continue;

        printf("bucket[%d]", b);
        while (d) {
            printf(" -> id=%d addr=%p next=%p", d->id, (void *)d, (void *)d->next);
            d = d->next;
        }
        printf("\n");
    }

    printf("APPOINTMENT HASH TABLE\n");
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        if (!a) continue;

        printf("bucket[%d]", b);
        while (a) {
            printf(" -> id=%d patient=%d doctor=%d addr=%p next=%p",
                a->id, a->patientId, a->doctorId, (void *)a, (void *)a->next);
            a = a->next;
        }
        printf("\n");
    }
}

Appointment *current_appointment_for_patient(int patientId) {
    Appointment *latest = NULL;
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            if (a->patientId == patientId && strcmp(a->status, "Completed") != 0) {
                if (!latest || a->id > latest->id) latest = a;
            }
            a = a->next;
        }
    }
    return latest;
}

Appointment *latest_appointment_for_patient(int patientId) {
    Appointment *latest = NULL;
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            if (a->patientId == patientId) {
                if (!latest || a->id > latest->id) latest = a;
            }
            a = a->next;
        }
    }
    return latest;
}

Patient *find_patient_by_phone(char *phone) {
    for (int b = 0; b < HASH_SIZE; b++) {
        Patient *p = patientTable[b];
        while (p) {
            if (strcmp(p->phone, phone) == 0) return p;
            p = p->next;
        }
    }
    return NULL;
}

char *appointment_display_status(Appointment *a) {
    if (!a) return "Not Assigned";
    if (strcmp(a->status, "Waiting") == 0) return "Assigned";
    return a->status;
}

int next_patient_id() {
    int max = 0;
    for (int b = 0; b < HASH_SIZE; b++) {
        Patient *p = patientTable[b];
        while (p) {
            if (p->id > max) max = p->id;
            p = p->next;
        }
    }
    return max + 1;
}

int next_appointment_id() {
    int max = 500;
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            if (a->id > max) max = a->id;
            a = a->next;
        }
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
    int bucket = hash_id(p->id);
    p->next = patientTable[bucket];
    patientTable[bucket] = p;
}

void append_doctor(Doctor *d) {
    int bucket = hash_id(d->id);
    d->next = doctorTable[bucket];
    doctorTable[bucket] = d;
}

void append_appointment(Appointment *a) {
    int bucket = hash_id(a->id);
    a->next = appointmentTable[bucket];
    appointmentTable[bucket] = a;
}

void append_visit(Visit *v) {
    v->next = visits;
    visits = v;
}

void append_invoice(Invoice *i) {
    i->next = invoices;
    invoices = i;
}

DoctorQueue *get_doctor_queue(int doctorId, int create) {
    int bucket = hash_id(doctorId);
    DoctorQueue *q = doctorQueueTable[bucket];
    while (q) {
        if (q->doctorId == doctorId) return q;
        q = q->next;
    }

    if (!create) return NULL;

    q = calloc(1, sizeof(DoctorQueue));
    q->doctorId = doctorId;
    q->next = doctorQueueTable[bucket];
    doctorQueueTable[bucket] = q;
    return q;
}

int is_waiting_status(Appointment *a) {
    return a &&
        (strcmp(a->status, "Assigned") == 0 ||
         strcmp(a->status, "Waiting") == 0 ||
         strcmp(a->status, "In Consultation") == 0);
}

int appointment_queued(DoctorQueue *q, Appointment *a) {
    if (q && q->active == a) return 1;

    AppointmentQueueNode *node = q ? q->front : NULL;
    while (node) {
        if (node->appointment == a) return 1;
        node = node->next;
    }
    return 0;
}

void enqueue_appointment(Appointment *a) {
    if (!a || a->doctorId <= 0 || !is_waiting_status(a)) return;

    DoctorQueue *q = get_doctor_queue(a->doctorId, 1);
    if (appointment_queued(q, a)) return;

    AppointmentQueueNode *node = calloc(1, sizeof(AppointmentQueueNode));
    node->appointment = a;
    if (!q->rear) {
        q->front = q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
}

Appointment *dequeue_next_appointment(int doctorId) {
    DoctorQueue *q = get_doctor_queue(doctorId, 0);
    if (!q || !q->front) return NULL;

    AppointmentQueueNode *node = q->front;
    Appointment *a = node->appointment;
    q->front = node->next;
    if (!q->front) q->rear = NULL;
    free(node);
    q->active = a;
    return a;
}

void remove_appointment_from_queue(Appointment *a) {
    if (!a || a->doctorId <= 0) return;

    DoctorQueue *q = get_doctor_queue(a->doctorId, 0);
    if (!q) return;

    if (q->active == a) q->active = NULL;

    AppointmentQueueNode *prev = NULL;
    AppointmentQueueNode *node = q->front;
    while (node) {
        if (node->appointment == a) {
            if (prev) prev->next = node->next;
            else q->front = node->next;
            if (q->rear == node) q->rear = prev;
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
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
        tok = strtok(NULL, "|"); if (tok) strcpy(a->endDate, tok);
        if (strlen(a->endDate) == 0 && valid_appointment_slot(a->date)) {
            format_appointment_time(parse_appointment_time(a->date) + (CONSULTATION_MINUTES * 60), a->endDate);
        }
        append_appointment(a);
        enqueue_appointment(a);
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
    for (int b = 0; b < HASH_SIZE; b++) {
        Patient *p = patientTable[b];
        while (p) {
            fprintf(f, "%d|%s|%d|%s|%s|%s|%s|%s\n", p->id, p->name, p->age, p->gender, p->phone, p->address, p->blood, p->firstVisit);
            p = p->next;
        }
    }
    fclose(f);
}

void save_doctors() {
    FILE *f = open_data_file("doctors.csv", "w");
    for (int b = 0; b < HASH_SIZE; b++) {
        Doctor *d = doctorTable[b];
        while (d) {
            fprintf(f, "%d|%s|%s|%s|%.2f\n", d->id, d->name, d->specialization, d->available, d->fee);
            d = d->next;
        }
    }
    fclose(f);
}

void save_appointments() {
    FILE *f = open_data_file("appointments.csv", "w");
    for (int b = 0; b < HASH_SIZE; b++) {
        Appointment *a = appointmentTable[b];
        while (a) {
            fprintf(f, "%d|%d|%d|%s|%s|%s|%s\n", a->id, a->patientId, a->doctorId, a->problem, a->status, a->date, a->endDate);
            a = a->next;
        }
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
