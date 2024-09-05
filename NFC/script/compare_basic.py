import sys

def compare_nfc_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        lines1 = f1.readlines()
        lines2 = f2.readlines()
    
    blocks1 = [line.strip() for line in lines1 if line.startswith("Block")]
    blocks2 = [line.strip() for line in lines2 if line.startswith("Block")]

    changes_detected = False
    
    for block1, block2 in zip(blocks1, blocks2):
        block_num1, data1 = block1.split(": ")
        block_num2, data2 = block2.split(": ")
        
        if data1 != data2:
            changes_detected = True
            print(f"\n{block_num1} has changed:")
            print(f"  {file1}: {data1}")
            print(f"  {file2}: {data2}")
            
            bytes1 = data1.split()
            bytes2 = data2.split()
            
            for i in range(len(bytes1)):
                if i >= len(bytes2) or bytes1[i] != bytes2[i]:
                    print(f"\n  Byte {i} has changed:")
                    print(f"    {file1}: {bytes1[i]}")
                    print(f"    {file2}: {bytes2[i]}")
                    
                    # Essai de conversion en valeurs numériques pour observer un changement potentiel de crédits
                    try:
                        byte1_int = int(bytes1[i], 16)
                        byte2_int = int(bytes2[i], 16)
                        difference = byte2_int - byte1_int
                        print(f"    Difference: {difference}")
                    except ValueError:
                        print("    Unable to interpret as integer")
    
    if not changes_detected:
        print("\nNo changes detected between the two files.")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 creds.py <file1> <file2>")
    else:
        file1 = sys.argv[1]
        file2 = sys.argv[2]
        compare_nfc_files(file1, file2)
