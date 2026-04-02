from flask import Flask, render_template, request
import subprocess

app = Flask(__name__)

# Home page
@app.route('/')
def home():
    return render_template('patient.html', patients=[])

# Add patient (HTML → Flask → C → File)
@app.route('/add', methods=['POST'])
def add_patient():
    id = request.form['id']
    name = request.form['name']
    age = request.form['age']
    problem = request.form['problem']

    # Send input to C program
    data = f"1\n{id}\n{name}\n{age}\n{problem}\n4\n"
    subprocess.run("patient.exe", input=data, text=True)

    return "<h3>Patient Added Successfully</h3><a href='/view'>View Patients</a>"

# View patients (File → Flask → HTML)
@app.route('/view')
def view_patients():
    patients = []
    with open("patients.txt", "r") as file:
        for line in file:
            data = line.strip().split(",")
            patients.append(data)
    return render_template("patient.html", patients=patients)

if __name__ == '__main__':
    app.run(debug=True)