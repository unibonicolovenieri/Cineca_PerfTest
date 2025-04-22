from mpi4py import MPI
import os

def read_vector(filename):
    with open(filename, 'r') as f:
        return [float(line.strip()) for line in f]

def main():
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()

    assert size == 2, "⚠️ Questo script è pensato per girare con esattamente 2 rank"

    # Percorso dei vettori
    generation_path = os.path.join(os.path.dirname(__file__), "../generation")
    vector1_path = os.path.join(generation_path, "vector1.txt")
    vector2_path = os.path.join(generation_path, "vector2.txt")

    if rank == 0:
        # Rank 0 legge entrambi i vettori
        v1 = read_vector(vector1_path)
        v2 = read_vector(vector2_path)

        # Somma i due vettori elemento per elemento
        total_vector = [a + b for a, b in zip(v1, v2)]

        # Divide il lavoro: prima metà a rank 0, seconda a rank 1
        mid = len(total_vector) // 2
        local_chunk = total_vector[:mid]
        comm.send(total_vector[mid:], dest=1, tag=11)

    else:
        # Rank 1 riceve la seconda metà
        local_chunk = comm.recv(source=0, tag=11)

    # Ogni rank calcola somma parziale
    partial_sum = sum(local_chunk)

    # Rank 1 manda la sua somma a rank 0
    if rank == 1:
        comm.send(partial_sum, dest=0, tag=22)
    else:
        other_sum = comm.recv(source=1, tag=22)
        total_sum = partial_sum + other_sum
        print(f"✅ Somma totale: {total_sum:.4f}")

if __name__ == "__main__":
    main()

