import sys

def lire_fichier_nfc(fichier):
    """Lit le fichier .nfc et extrait les blocs."""
    blocs = {}
    try:
        with open(fichier, 'r') as f:
            for ligne in f:
                if ligne.startswith("Block"):
                    num_bloc = int(ligne.split()[1].strip(":"))
                    data_bloc = ligne.split(":")[1].strip()
                    blocs[num_bloc] = data_bloc
    except FileNotFoundError:
        print(f"Fichier {fichier} non trouvé.")
        sys.exit(1)
    return blocs

def decoder_blocs(blocs):
    """Tente de décoder les données hexadécimales des blocs."""
    for num_bloc, data in blocs.items():
        bytes_bloc = bytes.fromhex(data.replace(' ', ''))
        try:
            texte = bytes_bloc.decode('utf-8')
        except UnicodeDecodeError:
            texte = "Non décodable en UTF-8"
        print(f"Bloc {num_bloc}: {data} | Décodé: {texte}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <fichier.nfc>")
        sys.exit(1)

    fichier_nfc = sys.argv[1]
    blocs = lire_fichier_nfc(fichier_nfc)
    decoder_blocs(blocs)
