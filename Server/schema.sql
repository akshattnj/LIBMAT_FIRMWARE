BEGIN TRANSACTION;

DROP TABLE IF EXISTS customersData;
DROP TABLE IF EXISTS devicesData;

CREATE TABLE customersData (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    created TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE devicesData (
    id TEXT PRIMARY KEY,
    access_id TEXT NOT NULL UNIQUE,
    service_uuid TEXT NOT NULL UNIQUE,
    characteristic_uuid TEXT NOT NULL UNIQUE,
    customer_id INTEGER NOT NULL,
    FOREIGN KEY (customer_id) REFERENCES customersData(id)
);

COMMIT;