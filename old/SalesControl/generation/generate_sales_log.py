import csv
import random
from datetime import datetime, timedelta

# Configurazione
NUM_RECORDS = 1_000_000  # Numero di righe da generare
OUT_FILE = 'cpu_sales_log.csv'

brands = {
    "Intel": ["Pentium 4", "Core 2 Duo", "i3-6100", "i5-8400", "i7-9700K", "i9-13900K"],
    "AMD": ["Athlon 64", "Phenom II", "FX-8350", "Ryzen 5 3600", "Ryzen 7 5800X", "Ryzen 9 7950X"]
}

clients = ["ACME Corp", "ByteHouse", "MegaSoft", "DataWare", "NeuralWorks", "CyberMart", "QuantumSys"]

start_date = datetime(2005, 1, 1)
end_date = datetime(2025, 1, 1)
delta_days = (end_date - start_date).days

def random_date():
    return start_date + timedelta(days=random.randint(0, delta_days))

def random_price(brand, model):
    base = {
        "Pentium 4": 50,
        "Core 2 Duo": 80,
        "i3-6100": 100,
        "i5-8400": 180,
        "i7-9700K": 300,
        "i9-13900K": 600,
        "Athlon 64": 60,
        "Phenom II": 90,
        "FX-8350": 150,
        "Ryzen 5 3600": 200,
        "Ryzen 7 5800X": 350,
        "Ryzen 9 7950X": 650
    }
    return round(base.get(model, 200) * (0.9 + random.random() * 0.2), 2)

def main():
    with open(OUT_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Data', 'Marca', 'Modello', 'Prezzo', 'Quantità', 'Cliente'])

        for _ in range(NUM_RECORDS):
            brand = random.choice(list(brands.keys()))
            model = random.choice(brands[brand])
            date = random_date().strftime("%Y-%m-%d")
            price = random_price(brand, model)
            quantity = random.randint(1, 500)
            client = random.choice(clients)

            writer.writerow([date, brand, model, price, quantity, client])

    print(f"[✔] Generato file {OUT_FILE} con {NUM_RECORDS} record.")

if __name__ == "__main__":
    main()

