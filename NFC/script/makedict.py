import os

def read_file_lines(filename):
    with open(filename, 'r', encoding='utf-8') as file:
        return set(line.strip() for line in file if line.strip() and not line.strip().startswith('#'))

def process_files(reference_file, output_file):
    reference_lines = read_file_lines(reference_file)
    unique_lines = set()

    for filename in os.listdir('.'):
        if filename.endswith('.txt') and filename != output_file:
            file_lines = read_file_lines(filename)
            unique_lines.update(file_lines - reference_lines)

    with open(output_file, 'w', encoding='utf-8') as outfile:
        for line in sorted(unique_lines):
            outfile.write(line + '\n')

    print(f"Traitement terminé. Les lignes uniques ont été écrites dans {output_file}")

# Utilisation du script
reference_file = 'mf_classic_dict.nfc'
output_file = 'mf_classic_user_dict.nfc'
process_files(reference_file, output_file)
