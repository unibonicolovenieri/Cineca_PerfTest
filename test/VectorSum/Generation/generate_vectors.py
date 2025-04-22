import random

def generate_vector(size):
    return [random.randint(1, 100) for _ in range(size)]

def save_vector(filename, vector):
    with open(filename, 'w') as f:
        for value in vector:
            f.write(f"{value}\n")

if __name__ == "__main__":
    size = 50
    vector1 = generate_vector(size)
    vector2 = generate_vector(size)

    save_vector("vector1.txt", vector1)
    save_vector("vector2.txt", vector2)

    print("✅ Vettori generati e salvati in 'vector1.txt' e 'vector2.txt'")
