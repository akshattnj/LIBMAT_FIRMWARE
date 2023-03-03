import sqlite3

connection = sqlite3.connect('./database.db')

with open('./schema.sql') as f:
    connection.executescript(f.read())

cur = connection.cursor()

cur.execute("INSERT INTO customersData (email, password) VALUES ('support@movio.in', 'Admin123')")
id = cur.lastrowid
cur.execute("INSERT INTO devicesData (id, access_id, service_uuid, characteristic_uuid, customer_id) VALUES ('20ec6300-b54f-11ed-9d5b-213ae9a75e81', 'cfb62967-c874-486b', 'c5fcf513-4573-4cd2-b874-8c4906184a42', '93aed156-b4dd-4998-8303-423200f23c60'," + str(id) + ")")
cur.execute("INSERT INTO devicesData (id, access_id, service_uuid, characteristic_uuid, customer_id) VALUES ('ffa11f90-b6c7-11ed-9d5b-213ae9a75e81', '2fcae502-b43a-4ba4', 'd890d64a-b56a-4a83-8bec-a9942f19e467', '3f2a5cb5-ee64-4abe-b9c7-4becbbc0243c'," + str(id) + ")")

connection.commit()
connection.close()
