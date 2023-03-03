from flask import Flask, request, jsonify
import sqlite3


def getDatabase():
    """
    Internally used to gain SQLite3 connection
    """
    conn = sqlite3.connect('./database.db')
    conn.row_factory = sqlite3.Row
    return conn


app = Flask(__name__)


@app.route('/registerCustomer', methods=['POST'])
def registerCustomer():
    """
        API to register a new customer\n
        Request Body: {\n
            email: String,\n
            password: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
    """
    if request.method == 'POST':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}  # Default response
        try:
            print(request.json)
            email = request.json['email']
            password = request.json['password']
            cur.execute(
                "INSERT INTO customersData (email, password) VALUES (?, ?)", (email, password,))
            conn.commit()
        except sqlite3.IntegrityError:
            response = {'status': 'error', 'message': 'Email already exists'}
        except Exception as e:
            response = {'status': 'error', 'message': str(e)}
        finally:
            conn.close()
        return jsonify(response)


@app.route('/swapDevice', methods=['POST'])
def swapDevice():
    """
        API to register a new device\n
        Request Body: {\n
            oldDeviceId: String,\n
            newDeviceId: String,\n
            customerId: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
    """
    if request.method == 'POST':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            oldDevice = request.json['oldDeviceId']
            newDevice = request.json['newDeviceId']
            customerId = request.json['customerId']

            cur.execute('SELECT * FROM customersData WHERE id = ?',
                        (customerId,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Customer does not exist')

            cur.execute('SELECT * FROM devicesData WHERE id = ?', (oldDevice,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Old Device does not exist')

            if (row['customer_id'] != customerId):
                raise Exception('Old Device not assigned to customer')

            cur.execute('SELECT * FROM devicesData WHERE id = ?', (newDevice,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('New Device does not exist')

            if (row['customer_id'] != 1):
                raise Exception('Device already in use')

            cur.execute(
                'UPDATE devicesData SET customer_id = 1 WHERE id = ?', (oldDevice,))
            cur.execute(
                'UPDATE devicesData SET customer_id = ? WHERE id = ?', (customerId, newDevice,))
            conn.commit()
        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()
        return jsonify(response)


@app.route('/loginCustomer', methods=['POST'])
def loginCustomer():
    """
        API to login a customer\n
        Request Body: {\n
            email: String,\n
            password: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional),\n
            customerId: String (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
        customerId: id of the customer (optional)\n
    """
    if request.method == 'POST':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            print(request.json)
            email = request.json['email']
            password = request.json['password']
            cur.execute(
                "SELECT * FROM customersData WHERE email = ? AND password = ?", (email, password,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Invalid credentials')
            else:
                response['customerId'] = row['id']

        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()

        return jsonify(response)


@app.route('/getDevices/<customerId>', methods=['GET'])
def getCustomerDevices(customerId):
    """
        API to get all devices of a customer\n
        Request Body: {\n
            customerId: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional),\n
            devices: Array of devices (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
        devices: array of devices (optional)\n
        \n
        device structure:\n
        {\n
            id: String,\n
            customer_id: Integer,\n
            access_id: String,\n
            service_uuid: String,\n
            characteristic_uuid: String\n
        }\n
    """
    if request.method == 'GET':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            cur.execute('SELECT * FROM customersData WHERE id = ?',
                        (customerId,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Customer does not exist')

            cur.execute(
                "SELECT * FROM devicesData WHERE customer_id = ?", (customerId,))
            rows = cur.fetchall()

            if (rows == []):
                raise Exception('No devices found')

            result = []
            for row in rows:
                device = {}
                for key in row.keys():
                    device[key] = row[key]
                result.append(device)
            response['devices'] = result

        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()

        return jsonify(response)


@app.route('/getDevice/<deviceId>', methods=['GET'])
def getDevice(deviceId):
    """
        API to get a device\n
        Request Body: {\n
            deviceId: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional),\n
            device: Device (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
        device: device (optional)\n
        \n
        device structure:\n
        {\n
            id: String,\n
            customer_id: Integer,\n
            access_id: String,\n
            service_uuid: String,\n
            characteristic_uuid: String\n
        }\n
    """
    if request.method == 'GET':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            cur.execute("SELECT * FROM devicesData WHERE id = ?", (deviceId,))
            row = cur.fetchone()

            if (row == None):
                raise Exception('Device not found')

            result = {}
            for key in row.keys():
                result[key] = row[key]
            response['device'] = result
        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()
        return jsonify(response)


@app.route('/assignDevice', methods=['POST'])
def assignDevice():
    """
        API to assign a device to a customer\n
        Request Body: { \n
            deviceId: String,\n
            customerId: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
    """
    if request.method == 'POST':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            deviceId = request.json['deviceId']
            customerId = request.json['customerId']

            cur.execute('SELECT * FROM customersData WHERE id = ?',
                        (customerId,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Customer does not exist')

            cur.execute('SELECT * FROM devicesData WHERE id = ?', (deviceId,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Device does not exist')

            if (row['customer_id'] != 1):
                raise Exception('Device already in use')

            cur.execute(
                'UPDATE devicesData SET customer_id = ? WHERE id = ?', (customerId, deviceId,))
            conn.commit()

        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()

        return jsonify(response)


@app.route('/unassignDevice', methods=['POST'])
def unassignDevice():
    """
        API to unassign a device from a customer\n
        Request Body: {\n
            deviceId: String\n
        }\n
        Response: {\n
            status: String,\n
            message: String (optional)\n
        }\n
        status: success/error\n
        message: error message (optional)\n
    """
    if request.method == 'POST':
        conn = getDatabase()
        cur = conn.cursor()
        response = {'status': 'success'}
        try:
            deviceId = request.json['deviceId']

            cur.execute('SELECT * FROM devicesData WHERE id = ?', (deviceId,))
            row = cur.fetchone()
            if (row == None):
                raise Exception('Device does not exist')

            if (row['customer_id'] == 1):
                raise Exception('Device not in use')

            cur.execute(
                'UPDATE devicesData SET customer_id = 1 WHERE id = ?', (deviceId,))
            conn.commit()

        except Exception as e:
            response = {'status': 'error', 'message': str(e)}

        finally:
            conn.close()

        return jsonify(response)


if (__name__ == '__main__'):
    app.run(port=8080, debug=True)
