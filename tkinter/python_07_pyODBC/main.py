import pyodbc

conn = pyodbc.connect(r'Driver={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=/home/asharma/PycharmProjects/python_07_pyODBC/QuickQuote.accdb;');

cursor = conn.cursor()
cursor.execute('SELECT * FROM Terrasat_Products')

for row in cursor.fetchall():
    print(row)